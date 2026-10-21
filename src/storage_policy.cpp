#include "bruce/lhme/storage_policy.hpp"

#include "bruce/lhme/formula.hpp"

namespace bruce::lhme {
namespace {

constexpr std::uint32_t kBasisPointsPerWhole = 10'000;

[[nodiscard]] bool is_within_sparse_limit(const StorageDecisionInput& input,
                                          const StoragePolicy& policy) {
  return BigInt{input.override_count} * kBasisPointsPerWhole <=
         BigInt{input.record_count} * policy.sparse_override_limit_basis_points;
}

}  // namespace

StorageDecision choose_storage_strategy(StorageDecisionInput input,
                                        const StoragePolicy& policy) {
  if (input.record_count == 0 || input.override_count > input.record_count ||
      policy.sparse_override_limit_basis_points > kBasisPointsPerWhole) {
    return {};
  }
  if (input.override_count == 0) {
    return {.status = Status::Confirmed, .strategy = StorageStrategy::FormulaOnly};
  }
  if (is_within_sparse_limit(input, policy)) {
    return {.status = Status::Confirmed,
            .strategy = StorageStrategy::FormulaWithOverrides};
  }
  return {.status = Status::Confirmed,
          .strategy = input.access_pattern == AccessPattern::Sequential
                          ? StorageStrategy::DenseTable
                          : StorageStrategy::HashTable};
}

}  // namespace bruce::lhme
