#pragma once

#include <compare>
#include <cstdint>
#include <map>
#include <memory>
#include <mutex>
#include <optional>
#include <string>

#include "axiom/lhme/formula.hpp"
#include "axiom/lhme/technique.hpp"

namespace axiom::lhme {

struct CacheKey {
  std::string domain;
  std::array<std::byte, 32> specification_hash{};
  std::string formula_identity;
  std::uint32_t specification_version = 0;
  std::string technique_id;
  std::uint32_t technique_version = 0;
  std::string technique_parameters;
  std::int64_t index = 0;
  EvaluationMode mode = EvaluationMode::Symbolic;

  auto operator<=>(const CacheKey&) const = default;

  [[nodiscard]] static CacheKey exact(const FormulaSpec& spec, std::int64_t index);
  [[nodiscard]] static CacheKey metric(const FormulaSpec& spec, std::int64_t index);
  [[nodiscard]] static CacheKey technique(const TechniqueIdentity& identity,
                                          std::string_view parameters,
                                          std::int64_t index,
                                          EvaluationMode mode);
};

struct CacheConfig { std::size_t max_entries = 0; std::size_t max_bytes = 0; };
struct CacheStatistics { std::size_t resident_entries = 0; std::size_t resident_bytes = 0; };

class ResultCache {
 public:
  explicit ResultCache(CacheConfig config);
  [[nodiscard]] std::optional<EvaluationResult> lookup(const CacheKey& key) const;
  void admit(const CacheKey& key, const EvaluationResult& result);
  [[nodiscard]] CacheStatistics statistics() const;

 private:
  struct Entry { EvaluationResult result; std::size_t bytes = 0; std::uint64_t sequence = 0; };
  CacheConfig config_;
  std::map<CacheKey, Entry> entries_;
  std::size_t resident_bytes_ = 0;
  std::uint64_t sequence_ = 0;
  std::shared_ptr<std::mutex> mutex_;
};
}  // namespace axiom::lhme
