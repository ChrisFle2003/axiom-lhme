#include "axiom/lhme/kevin_memory_store.hpp"

#include "axiom/lhme/domain_projection.hpp"
#include "axiom/lhme/trace.hpp"

#include <algorithm>
#include <mutex>

namespace axiom::lhme { namespace {
std::string key(const GridIdentity& id) { return std::to_string(id.id.size()) + ":" + id.id + ":" + std::to_string(id.version); }
FormulaAddress legacy_depth_path_address(const GridAddress& address) {
  static const DepthPathProjection projection;
  return *projection.project(address);
}
std::string override_key(const GridIdentity& id, const GridAddress& address) { return key(id) + ":" + std::to_string(legacy_depth_path_address(address).index); }
std::uint64_t bits(const BigInt& value) { if (value == 0) return 0; const auto magnitude = value < 0 ? -value : value; return static_cast<std::uint64_t>(boost::multiprecision::msb(magnitude)) + 1U; }
Budget stricter(Budget a, const Budget& b) {
  a.max_bit_length = std::min(a.max_bit_length, b.max_bit_length);
  a.max_operations = std::min(a.max_operations, b.max_operations);
  a.max_temporary_bytes = std::min(a.max_temporary_bytes, b.max_temporary_bytes);
  a.max_trace_bytes = std::min(a.max_trace_bytes, b.max_trace_bytes);
  a.max_candidate_count = std::min(a.max_candidate_count, b.max_candidate_count); return a;
}
KevinValue invalid(EvaluationMode mode, const char* reason) {
  EvaluationResult r; r.status = Status::InvalidArgument; r.mode = mode;
  r.trace = make_trace({{.scope=GateScope::Branch,.status=r.status,.reason=reason}}); return {.result=std::move(r)};
}
}  // namespace
KevinMemoryStore::KevinMemoryStore(Engine& engine)
    : engine_(engine), mutex_(std::make_shared<std::shared_mutex>()) {}
bool KevinMemoryStore::register_grid(GridSpec spec) {
  if (!valid_grid_spec(spec) || !engine_.has_technique(spec.technique)) return false;
  const std::unique_lock lock(*mutex_);
  grids_.insert_or_assign(key(spec.identity), std::move(spec)); return true;
}
Status KevinMemoryStore::set_override(const GridIdentity& identity, const GridAddress& address, BigInt value) {
  const std::unique_lock lock(*mutex_);
  const auto found = grids_.find(key(identity));
  if (found == grids_.end() || address.depth() > found->second.max_depth) return Status::InvalidArgument;
  if (bits(value) > found->second.default_budget.max_bit_length) return Status::BudgetExceeded;
  overrides_.insert_or_assign(override_key(identity, address), SnapshotOverrideRecord{identity, address, std::move(value)}); return Status::Confirmed;
}
bool KevinMemoryStore::remove_override(const GridIdentity& identity, const GridAddress& address) {
  const std::unique_lock lock(*mutex_);
  return overrides_.erase(override_key(identity, address)) != 0;
}
KevinValue KevinMemoryStore::get(const GridIdentity& identity, const GridAddress& address,
                                 EvaluationMode mode, Budget budget, std::optional<BigInt> modulus) {
  if (!valid_grid_identity(identity)) return invalid(mode, "grid identity is invalid");
  GridSpec grid;
  std::optional<BigInt> override_value;
  {
    const std::shared_lock lock(*mutex_);
    const auto found = grids_.find(key(identity));
    if (found == grids_.end()) return invalid(mode, "grid is not registered");
    if (address.depth() > found->second.max_depth) {
      return invalid(mode, "grid address exceeds maximum depth");
    }
    grid = found->second;
    if (const auto overridden = overrides_.find(override_key(identity, address));
        overridden != overrides_.end()) {
      override_value = overridden->second.value;
    }
  }
  const auto effective_budget = stricter(budget, grid.default_budget);
  if (override_value.has_value()) {
    const auto& value = *override_value;
    EvaluationResult result; result.status = Status::Confirmed; result.mode = mode; result.trace = make_trace({{.scope=GateScope::Branch,.status=Status::Confirmed,.reason="kevin override applied"}});
    if (mode == EvaluationMode::Exact) {
      const auto bit_length = bits(value);
      const auto byte_length = bit_length / 8U + (bit_length % 8U == 0 ? 0U : 1U);
      if (bit_length > effective_budget.max_bit_length || byte_length > effective_budget.max_temporary_bytes) {
        result.status = Status::BudgetExceeded; result.metric_bit_length = bit_length;
        result.trace = make_trace({{.scope=GateScope::Publish,.status=result.status,.reason="override exceeded caller resource budget"}});
      } else result.integer = value;
    }
    else if (mode == EvaluationMode::Metric) result.metric_bit_length = bits(value);
    else if (mode == EvaluationMode::Modular) { if (!modulus.has_value() || *modulus <= 0) return invalid(mode, "modulus is invalid"); result.integer = value % *modulus; if (*result.integer < 0) *result.integer += *modulus; }
    else result.symbolic_expression = "override(" + value.convert_to<std::string>() + ")";
    return {.result=std::move(result), .source=KevinValueSource::Override};
  }
  if (mode == EvaluationMode::Exact) computed_exact_evaluation_count_.fetch_add(1);
  return {.result = engine_.evaluate_technique({.identity=grid.technique, .parameters=grid.canonical_parameters, .address=legacy_depth_path_address(address), .mode=mode, .budget=effective_budget, .modulus=std::move(modulus)}), .source=KevinValueSource::Computed};
}
KevinStoreStatistics KevinMemoryStore::inspect() const {
  std::size_t registered_grid_count = 0;
  std::size_t override_count = 0;
  {
    const std::shared_lock lock(*mutex_);
    registered_grid_count = grids_.size();
    override_count = overrides_.size();
  }
  return {.registered_grid_count=registered_grid_count, .override_count=override_count,
          .computed_exact_evaluation_count=computed_exact_evaluation_count_.load(),
          .engine_cache=engine_.inspect()};
}
KevinSnapshot KevinMemoryStore::snapshot() const {
  KevinSnapshot output;
  const std::shared_lock lock(*mutex_);
  for (const auto& [_, grid] : grids_) output.grids.push_back(grid);
  for (const auto& [_, value] : overrides_) output.overrides.push_back(value);
  return output;
}
SnapshotStatus KevinMemoryStore::replace_snapshot(const KevinSnapshot& snapshot) {
  std::map<std::string, GridSpec> grids; std::map<std::string, SnapshotOverrideRecord> overrides;
  for (const auto& grid : snapshot.grids) { const auto grid_key=key(grid.identity); if (!valid_grid_spec(grid) || !engine_.has_technique(grid.technique) || grids.contains(grid_key)) return SnapshotStatus::InvalidSnapshot; grids.emplace(grid_key, grid); }
  for (const auto& entry : snapshot.overrides) { const auto grid=grids.find(key(entry.identity)); const auto entry_key=override_key(entry.identity,entry.address); if (grid==grids.end() || entry.address.depth()>grid->second.max_depth || overrides.contains(entry_key)) return SnapshotStatus::InvalidSnapshot; overrides.emplace(entry_key,entry); }
  const std::unique_lock lock(*mutex_);
  grids_=std::move(grids); overrides_=std::move(overrides); return SnapshotStatus::Confirmed;
}
}  // namespace axiom::lhme
