#include "bruce/lhme/cache.hpp"
#include "bruce/lhme/trace.hpp"

#include <algorithm>

namespace bruce::lhme {
namespace {
CacheKey make_key(const FormulaSpec& spec, std::int64_t index, EvaluationMode mode) {
  return {.domain = "formula",
          .specification_hash = spec.specification_hash,
          .formula_identity = canonical_formula_identity(spec),
          .specification_version = spec.version,
          .index = index,
          .mode = mode};
}

CacheKey make_technique_key(const TechniqueIdentity& identity,
                            std::string_view parameters, std::int64_t index,
                            EvaluationMode mode) {
  return {.domain = "technique",
          .technique_id = identity.id,
          .technique_version = identity.version,
          .technique_parameters = std::string{parameters},
          .index = index,
          .mode = mode};
}
std::size_t dynamic_string_bytes(const std::string& value) {
  return value.capacity();
}

std::size_t trace_bytes(const std::shared_ptr<const TraceRecord>& trace) {
  if (trace == nullptr) return 0;

  std::size_t bytes = sizeof(TraceRecord) + dynamic_string_bytes(trace->id) +
                      trace->events.capacity() * sizeof(GateEvent);
  for (const auto& event : trace->events) {
    bytes += dynamic_string_bytes(event.reason);
  }
  return bytes;
}

std::size_t result_bytes(const EvaluationResult& result) {
  std::size_t bytes = sizeof(EvaluationResult) + trace_bytes(result.trace);
  if (result.integer.has_value() && *result.integer != 0) {
    const auto magnitude = *result.integer < 0 ? -*result.integer : *result.integer;
    bytes += static_cast<std::size_t>(boost::multiprecision::msb(magnitude) / 8U) + 1U;
  }
  if (result.symbolic_expression.has_value()) {
    bytes += dynamic_string_bytes(*result.symbolic_expression);
  }
  return bytes;
}

std::size_t key_bytes(const CacheKey& key) {
  return sizeof(CacheKey) + dynamic_string_bytes(key.domain) +
         dynamic_string_bytes(key.formula_identity) +
         dynamic_string_bytes(key.technique_id) +
         dynamic_string_bytes(key.technique_parameters);
}

std::size_t entry_bytes(const CacheKey& key, const EvaluationResult& result) {
  constexpr std::size_t map_node_overhead = sizeof(void*) * 4U;
  return key_bytes(key) + result_bytes(result) + sizeof(std::uint64_t) +
         sizeof(std::size_t) + map_node_overhead;
}
}  // namespace

CacheKey CacheKey::exact(const FormulaSpec& spec, std::int64_t index) { return make_key(spec, index, EvaluationMode::Exact); }
CacheKey CacheKey::metric(const FormulaSpec& spec, std::int64_t index) { return make_key(spec, index, EvaluationMode::Metric); }
CacheKey CacheKey::technique(const TechniqueIdentity& identity,
                             std::string_view parameters, std::int64_t index,
                             EvaluationMode mode) {
  return make_technique_key(identity, parameters, index, mode);
}
ResultCache::ResultCache(CacheConfig config)
    : config_(config), mutex_(std::make_shared<std::mutex>()) {}
std::optional<EvaluationResult> ResultCache::lookup(const CacheKey& key) const {
  const std::lock_guard lock(*mutex_);
  const auto it = entries_.find(key);
  return it == entries_.end() ? std::nullopt : std::optional<EvaluationResult>{it->second.result};
}
void ResultCache::admit(const CacheKey& key, const EvaluationResult& result) {
  const std::lock_guard lock(*mutex_);
  if (result.status != Status::Confirmed || result.trace == nullptr) return;
  const auto bytes = entry_bytes(key, result);
  if (config_.max_entries == 0 || config_.max_bytes == 0 ||
      bytes > config_.max_bytes) return;
  if (const auto existing = entries_.find(key); existing != entries_.end()) {
    resident_bytes_ -= existing->second.bytes;
    entries_.erase(existing);
  }
  while (!entries_.empty() &&
         (entries_.size() >= config_.max_entries ||
          bytes > config_.max_bytes - resident_bytes_)) {
    auto victim = std::min_element(entries_.begin(), entries_.end(), [](const auto& left, const auto& right) { return left.second.sequence < right.second.sequence; });
    resident_bytes_ -= victim->second.bytes;
    entries_.erase(victim);
  }
  entries_[key] = {.result = result, .bytes = bytes, .sequence = ++sequence_};
  resident_bytes_ += bytes;
}
CacheStatistics ResultCache::statistics() const {
  const std::lock_guard lock(*mutex_);
  return {.resident_entries = entries_.size(), .resident_bytes = resident_bytes_};
}
}  // namespace bruce::lhme
