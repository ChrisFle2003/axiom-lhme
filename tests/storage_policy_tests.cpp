#include "test.hpp"

#include "axiom/lhme/storage_policy.hpp"

#include <cstdint>

namespace {

using axiom::lhme::AccessPattern;
using axiom::lhme::Status;
using axiom::lhme::StoragePolicy;
using axiom::lhme::StorageStrategy;
using axiom::lhme::choose_storage_strategy;

}  // namespace

TEST(storage_policy_uses_formula_when_no_exceptions_exist) {
  const auto decision = choose_storage_strategy(
      {.record_count = 1'000'000, .override_count = 0,
       .access_pattern = AccessPattern::Random}, StoragePolicy{});

  CHECK(decision.status == Status::Confirmed);
  CHECK(decision.strategy == StorageStrategy::FormulaOnly);
}

TEST(storage_policy_keeps_small_exception_sets_sparse) {
  const auto decision = choose_storage_strategy(
      {.record_count = 1'000'000, .override_count = 50'000,
       .access_pattern = AccessPattern::Sequential}, StoragePolicy{});

  CHECK(decision.status == Status::Confirmed);
  CHECK(decision.strategy == StorageStrategy::SparseOverrides);
}

TEST(storage_policy_names_sparse_as_formula_with_overrides) {
  const auto decision = choose_storage_strategy(
      {.record_count = 1'000'000, .override_count = 50'000,
       .access_pattern = AccessPattern::Random}, StoragePolicy{});

  CHECK(decision.status == Status::Confirmed);
  CHECK(decision.strategy == StorageStrategy::FormulaWithOverrides);
}

TEST(storage_policy_chooses_materialized_layout_from_access_pattern) {
  const auto sequential = choose_storage_strategy(
      {.record_count = 1'000'000, .override_count = 50'001,
       .access_pattern = AccessPattern::Sequential}, StoragePolicy{});
  const auto random = choose_storage_strategy(
      {.record_count = 1'000'000, .override_count = 50'001,
       .access_pattern = AccessPattern::Random}, StoragePolicy{});

  CHECK(sequential.status == Status::Confirmed);
  CHECK(sequential.strategy == StorageStrategy::DenseTable);
  CHECK(random.status == Status::Confirmed);
  CHECK(random.strategy == StorageStrategy::HashTable);
}

TEST(storage_policy_rejects_invalid_domains_and_thresholds) {
  const auto zero_records = choose_storage_strategy(
      {.record_count = 0, .override_count = 0,
       .access_pattern = AccessPattern::Sequential}, StoragePolicy{});
  const auto too_many_overrides = choose_storage_strategy(
      {.record_count = 5, .override_count = 6,
       .access_pattern = AccessPattern::Sequential}, StoragePolicy{});
  const auto invalid_threshold = choose_storage_strategy(
      {.record_count = 5, .override_count = 1,
       .access_pattern = AccessPattern::Sequential},
      {.sparse_override_limit_basis_points = 10'001});

  CHECK(zero_records.status == Status::InvalidArgument);
  CHECK(too_many_overrides.status == Status::InvalidArgument);
  CHECK(invalid_threshold.status == Status::InvalidArgument);
}
