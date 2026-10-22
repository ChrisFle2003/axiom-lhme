#pragma once

#include <cstdint>

#include "axiom/lhme/types.hpp"

namespace axiom::lhme {

enum class AccessPattern { Sequential, Random };

enum class StorageStrategy {
  FormulaOnly,
  FormulaWithOverrides,
  // Temporary source-compatible name for callers using the former vocabulary.
  SparseOverrides = FormulaWithOverrides,
  DenseTable,
  HashTable,
};

struct StoragePolicy {
  // Conservative default; callers may calibrate this for their workload.
  std::uint32_t sparse_override_limit_basis_points = 500;
};

struct StorageDecisionInput {
  std::uint64_t record_count = 0;
  std::uint64_t override_count = 0;
  AccessPattern access_pattern = AccessPattern::Random;
};

struct StorageDecision {
  Status status = Status::InvalidArgument;
  StorageStrategy strategy = StorageStrategy::FormulaOnly;
};

[[nodiscard]] StorageDecision choose_storage_strategy(
    StorageDecisionInput input, const StoragePolicy& policy);

}  // namespace axiom::lhme
