#pragma once

#include <atomic>
#include <map>
#include <optional>
#include <shared_mutex>
#include <string>
#include <variant>
#include <vector>

#include "bruce/lhme/engine.hpp"

namespace bruce::lhme {

struct TechniqueBinding {
  TechniqueIdentity identity;
  std::string canonical_parameters;
  Budget default_budget = Budget::unlimited();
};

using FormulaBinding = std::variant<FormulaSpec, TechniqueBinding>;

[[nodiscard]] bool valid_formula_binding(const FormulaBinding& binding);
[[nodiscard]] std::string canonical_formula_binding_identity(
    const FormulaBinding& binding);

struct FormulaOverrideRequest {
  FormulaBinding binding;
  FormulaAddress address;
  EvaluationMode mode = EvaluationMode::Symbolic;
  Budget budget = Budget::unlimited();
  std::optional<BigInt> modulus;
};

enum class FormulaValueSource { Override, Computed };

struct FormulaValue {
  EvaluationResult result;
  FormulaValueSource source = FormulaValueSource::Computed;
};

struct FormulaOverrideRecord {
  FormulaBinding binding;
  FormulaAddress address;
  BigInt value;
};

struct FormulaOverrideSnapshot {
  std::vector<FormulaOverrideRecord> overrides;
};

enum class FormulaOverrideSnapshotStatus { Confirmed, IoError, InvalidSnapshot };

struct FormulaOverrideStoreStatistics {
  std::size_t override_count = 0;
  std::uint64_t computed_exact_evaluation_count = 0;
  CacheStatistics engine_cache;
};

class FormulaOverrideStore {
 public:
  explicit FormulaOverrideStore(Engine& engine);

  [[nodiscard]] Status set_override(const FormulaBinding& binding,
                                    FormulaAddress address, BigInt value);
  [[nodiscard]] bool remove_override(const FormulaBinding& binding,
                                     FormulaAddress address);
  [[nodiscard]] FormulaValue evaluate(const FormulaOverrideRequest& request);
  [[nodiscard]] FormulaOverrideSnapshot snapshot() const;
  [[nodiscard]] FormulaOverrideSnapshotStatus replace_snapshot(
      const FormulaOverrideSnapshot& snapshot);
  [[nodiscard]] FormulaOverrideStoreStatistics inspect() const;

 private:
  Engine& engine_;
  std::map<std::string, FormulaOverrideRecord> overrides_;
  std::atomic<std::uint64_t> computed_exact_evaluation_count_ = 0;
  std::shared_ptr<std::shared_mutex> mutex_;
};

}  // namespace bruce::lhme
