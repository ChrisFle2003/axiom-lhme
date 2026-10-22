#include "axiom/lhme/formula_override_store.hpp"

#include "axiom/lhme/trace.hpp"

#include <algorithm>
#include <mutex>
#include <utility>

namespace axiom::lhme {
namespace {

std::string technique_binding_identity(const TechniqueBinding& binding) {
  return std::to_string(binding.identity.id.size()) + ":" + binding.identity.id +
         ":" + std::to_string(binding.identity.version) + ":" +
         std::to_string(binding.canonical_parameters.size()) + ":" +
         binding.canonical_parameters;
}

std::string override_key(const FormulaBinding& binding, FormulaAddress address) {
  return canonical_formula_binding_identity(binding) + ":" +
         std::to_string(address.index);
}

std::uint64_t bit_length(const BigInt& value) {
  if (value == 0) return 0;
  const auto magnitude = value < 0 ? -value : value;
  return static_cast<std::uint64_t>(boost::multiprecision::msb(magnitude)) + 1U;
}

std::uint64_t bytes_for_bits(std::uint64_t bits) {
  return bits / 8U + (bits % 8U == 0 ? 0U : 1U);
}

Budget stricter_budget(Budget first, const Budget& second) {
  first.max_bit_length = std::min(first.max_bit_length, second.max_bit_length);
  first.max_operations = std::min(first.max_operations, second.max_operations);
  first.max_temporary_bytes =
      std::min(first.max_temporary_bytes, second.max_temporary_bytes);
  first.max_trace_bytes = std::min(first.max_trace_bytes, second.max_trace_bytes);
  first.max_candidate_count =
      std::min(first.max_candidate_count, second.max_candidate_count);
  return first;
}

EvaluationResult invalid_result(EvaluationMode mode, const char* reason) {
  EvaluationResult result;
  result.status = Status::InvalidArgument;
  result.mode = mode;
  result.trace = make_trace({{.scope = GateScope::Branch,
                              .status = result.status,
                              .reason = reason}});
  return result;
}

EvaluationResult override_result(const BigInt& value, EvaluationMode mode,
                                 const Budget& budget,
                                 const std::optional<BigInt>& modulus) {
  EvaluationResult result;
  result.status = Status::Confirmed;
  result.mode = mode;
  result.trace = make_trace({{.scope = GateScope::Branch,
                              .status = result.status,
                              .reason = "formula override applied"}});
  switch (mode) {
    case EvaluationMode::Exact: {
      const auto bits = bit_length(value);
      if (bits > budget.max_bit_length ||
          bytes_for_bits(bits) > budget.max_temporary_bytes) {
        result.status = Status::BudgetExceeded;
        result.metric_bit_length = bits;
        result.trace = make_trace({{.scope = GateScope::Publish,
                                    .status = result.status,
                                    .reason = "override exceeded caller resource budget"}});
      } else {
        result.integer = value;
      }
      return result;
    }
    case EvaluationMode::Metric:
      result.metric_bit_length = bit_length(value);
      return result;
    case EvaluationMode::Modular:
      if (!modulus.has_value() || *modulus <= 0) {
        return invalid_result(mode, "modulus is invalid");
      }
      result.integer = value % *modulus;
      if (*result.integer < 0) *result.integer += *modulus;
      return result;
    case EvaluationMode::Symbolic:
      result.symbolic_expression = "override(" + value.convert_to<std::string>() + ")";
      return result;
  }
  return invalid_result(mode, "evaluation mode is invalid");
}

}  // namespace

bool valid_formula_binding(const FormulaBinding& binding) {
  return std::visit(
      [](const auto& value) {
        using Binding = std::decay_t<decltype(value)>;
        if constexpr (std::is_same_v<Binding, FormulaSpec>) {
          if (value.operation != FormulaOperation::PowerOffset ||
              value.base <= 0 || value.scale < 0 ||
              !value.nonnegative_index || value.version == 0) {
            return false;
          }
          const auto canonical =
              make_power_offset_spec(value.base, value.scale, value.version);
          return value.specification_hash == canonical.specification_hash;
        } else {
          return valid_technique_identity(value.identity) &&
                 !value.canonical_parameters.empty() &&
                 value.canonical_parameters.size() <= 4096;
        }
      },
      binding);
}

std::string canonical_formula_binding_identity(const FormulaBinding& binding) {
  return std::visit(
      [](const auto& value) {
        using Binding = std::decay_t<decltype(value)>;
        if constexpr (std::is_same_v<Binding, FormulaSpec>) {
          return "formula:" + canonical_formula_identity(value);
        } else {
          return "technique:" + technique_binding_identity(value);
        }
      },
      binding);
}

FormulaOverrideStore::FormulaOverrideStore(Engine& engine)
    : engine_(engine), mutex_(std::make_shared<std::shared_mutex>()) {}

Status FormulaOverrideStore::set_override(const FormulaBinding& binding,
                                          FormulaAddress address, BigInt value) {
  if (!valid_formula_binding(binding)) return Status::InvalidArgument;
  const std::unique_lock lock(*mutex_);
  overrides_.insert_or_assign(override_key(binding, address),
                              FormulaOverrideRecord{binding, address,
                                                    std::move(value)});
  return Status::Confirmed;
}

bool FormulaOverrideStore::remove_override(const FormulaBinding& binding,
                                           FormulaAddress address) {
  if (!valid_formula_binding(binding)) return false;
  const std::unique_lock lock(*mutex_);
  return overrides_.erase(override_key(binding, address)) != 0;
}

FormulaValue FormulaOverrideStore::evaluate(const FormulaOverrideRequest& request) {
  if (!valid_formula_binding(request.binding)) {
    return {.result = invalid_result(request.mode, "formula binding is invalid")};
  }

  std::optional<BigInt> override_value;
  {
    const std::shared_lock lock(*mutex_);
    const auto found = overrides_.find(override_key(request.binding, request.address));
    if (found != overrides_.end()) override_value = found->second.value;
  }
  if (override_value.has_value()) {
    Budget effective_budget = request.budget;
    if (const auto* technique = std::get_if<TechniqueBinding>(&request.binding)) {
      effective_budget = stricter_budget(effective_budget, technique->default_budget);
    }
    return {.result = override_result(*override_value, request.mode, effective_budget,
                                      request.modulus),
            .source = FormulaValueSource::Override};
  }

  if (request.mode == EvaluationMode::Exact) {
    computed_exact_evaluation_count_.fetch_add(1, std::memory_order_relaxed);
  }
  return std::visit(
      [this, &request](const auto& binding) -> FormulaValue {
        using Binding = std::decay_t<decltype(binding)>;
        if constexpr (std::is_same_v<Binding, FormulaSpec>) {
          return {.result = engine_.evaluate({.spec = binding,
                                              .address = request.address,
                                              .mode = request.mode,
                                              .budget = request.budget,
                                              .modulus = request.modulus}),
                  .source = FormulaValueSource::Computed};
        } else {
          return {.result = engine_.evaluate_technique(
                      {.identity = binding.identity,
                       .parameters = binding.canonical_parameters,
                       .address = request.address,
                       .mode = request.mode,
                       .budget = stricter_budget(request.budget,
                                                 binding.default_budget),
                       .modulus = request.modulus}),
                  .source = FormulaValueSource::Computed};
        }
      },
      request.binding);
}

FormulaOverrideSnapshot FormulaOverrideStore::snapshot() const {
  FormulaOverrideSnapshot output;
  const std::shared_lock lock(*mutex_);
  output.overrides.reserve(overrides_.size());
  for (const auto& [_, record] : overrides_) output.overrides.push_back(record);
  return output;
}

FormulaOverrideSnapshotStatus FormulaOverrideStore::replace_snapshot(
    const FormulaOverrideSnapshot& snapshot) {
  std::map<std::string, FormulaOverrideRecord> replacements;
  for (const auto& record : snapshot.overrides) {
    if (!valid_formula_binding(record.binding)) {
      return FormulaOverrideSnapshotStatus::InvalidSnapshot;
    }
    const auto key = override_key(record.binding, record.address);
    if (replacements.contains(key)) return FormulaOverrideSnapshotStatus::InvalidSnapshot;
    replacements.emplace(key, record);
  }
  const std::unique_lock lock(*mutex_);
  overrides_ = std::move(replacements);
  return FormulaOverrideSnapshotStatus::Confirmed;
}

FormulaOverrideStoreStatistics FormulaOverrideStore::inspect() const {
  std::size_t override_count = 0;
  {
    const std::shared_lock lock(*mutex_);
    override_count = overrides_.size();
  }
  return {.override_count = override_count,
          .computed_exact_evaluation_count =
              computed_exact_evaluation_count_.load(std::memory_order_relaxed),
          .engine_cache = engine_.inspect()};
}

}  // namespace axiom::lhme
