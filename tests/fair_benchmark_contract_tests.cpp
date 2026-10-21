#include "test.hpp"

#include "benchmark_contract.hpp"
#include "fair_storage.hpp"

#include <array>
#include <cstdint>
#include <string>
#include <vector>

namespace {

using bruce::lhme::benchmark::fair::FairBenchmarkConfig;
using bruce::lhme::benchmark::fair::fair_storage_report_contract;
using bruce::lhme::benchmark::fair::StorageStrategyKind;
using bruce::lhme::benchmark::fair::WorkloadKind;
using bruce::lhme::benchmark::fair::build_storage_case;
using bruce::lhme::benchmark::fair::lookup_scalar;
using bruce::lhme::benchmark::fair::make_workload;
using bruce::lhme::benchmark::fair::storage_case_label;
using bruce::lhme::benchmark::fair::summarize_samples;
using bruce::lhme::benchmark::fair::validate_config;
using bruce::lhme::benchmark::fair::verify_workload;

constexpr FairBenchmarkConfig valid_config{
    .record_count = 1'024,
    .query_count = 257,
    .modulus = 2'305'843'009'213'693'951ULL,
    .override_count = 0,
    .seed = 0x4252554345ULL,
};

}  // namespace

TEST(fair_workloads_are_repeatable_and_in_range) {
  for (const auto kind : {WorkloadKind::Sequential, WorkloadKind::PseudoRandom,
                          WorkloadKind::RepeatHeavy}) {
    const auto first = make_workload(valid_config, kind);
    const auto second = make_workload(valid_config, kind);
    CHECK(first.has_value());
    CHECK(second.has_value());
    CHECK(first->addresses.size() == valid_config.query_count);
    CHECK(first->addresses == second->addresses);
    CHECK(first->checksum == second->checksum);
    for (const auto address : first->addresses) {
      CHECK(address < valid_config.record_count);
    }
  }
}

TEST(fair_config_rejects_invalid_domain_modulus_and_override_count) {
  CHECK(validate_config(valid_config));

  auto invalid_domain = valid_config;
  invalid_domain.record_count = 0;
  CHECK(!validate_config(invalid_domain));

  auto invalid_modulus = valid_config;
  invalid_modulus.modulus = 0;
  CHECK(!validate_config(invalid_modulus));

  auto invalid_overrides = valid_config;
  invalid_overrides.override_count = invalid_overrides.record_count + 1;
  CHECK(!validate_config(invalid_overrides));

  auto invalid_queries = valid_config;
  invalid_queries.query_count = 0;
  CHECK(!validate_config(invalid_queries));
}

TEST(eleven_sample_summary_reports_ordered_minimum_median_and_maximum) {
  const std::vector<std::uint64_t> samples{11, 5, 9, 3, 7, 1,
                                           13, 15, 17, 19, 21};
  const auto summary = summarize_samples(samples);
  CHECK(summary.has_value());
  CHECK(summary->minimum_microseconds == 1);
  CHECK(summary->median_microseconds == 11);
  CHECK(summary->maximum_microseconds == 21);

  const std::vector<std::uint64_t> wrong_count{1, 2, 3};
  CHECK(!summarize_samples(wrong_count).has_value());
}

TEST(all_track_a_strategies_return_the_same_modular_scalars) {
  const auto workload = make_workload(valid_config, WorkloadKind::PseudoRandom);
  CHECK(workload.has_value());

  const std::array strategies{
      StorageStrategyKind::FormulaRecipe,
      StorageStrategyKind::FormulaRecipeWithOverrides,
      StorageStrategyKind::DenseVector,
      StorageStrategyKind::SortedPairs,
      StorageStrategyKind::HashTable,
  };
  std::optional<std::vector<std::uint64_t>> reference;
  for (const auto strategy : strategies) {
    const auto storage = build_storage_case(valid_config, strategy);
    CHECK(storage.has_value());
    CHECK(verify_workload(*storage, *workload));

    std::vector<std::uint64_t> observed;
    observed.reserve(workload->addresses.size());
    for (const auto address : workload->addresses) {
      const auto value = lookup_scalar(*storage, address);
      CHECK(value.has_value());
      observed.push_back(*value);
    }
    if (!reference.has_value()) {
      reference = observed;
    } else {
      CHECK(observed == *reference);
    }
  }
}

TEST(track_a_full_override_layout_is_correct_and_explicit) {
  auto full_override_config = valid_config;
  full_override_config.override_count = full_override_config.record_count;
  const auto workload = make_workload(full_override_config, WorkloadKind::Sequential);
  CHECK(workload.has_value());

  const std::array strategies{
      StorageStrategyKind::FormulaRecipeWithOverrides,
      StorageStrategyKind::DenseVector,
      StorageStrategyKind::SortedPairs,
      StorageStrategyKind::HashTable,
  };
  std::optional<std::vector<std::uint64_t>> reference;
  for (const auto strategy : strategies) {
    const auto storage = build_storage_case(full_override_config, strategy);
    CHECK(storage.has_value());
    CHECK(verify_workload(*storage, *workload));
    CHECK(storage_case_label(*storage).find("100-percent-overrides") !=
          std::string::npos);

    std::vector<std::uint64_t> observed;
    for (const auto address : workload->addresses) {
      const auto value = lookup_scalar(*storage, address);
      CHECK(value.has_value());
      observed.push_back(*value);
    }
    if (!reference.has_value()) {
      reference = observed;
    } else {
      CHECK(observed == *reference);
    }
  }
}

TEST(track_a_override_density_label_preserves_fractional_percentages) {
  auto fractional_override_config = valid_config;
  fractional_override_config.override_count = 1;
  const auto storage = build_storage_case(
      fractional_override_config, StorageStrategyKind::FormulaRecipeWithOverrides);
  CHECK(storage.has_value());
  CHECK(storage_case_label(*storage).find("0.0977-percent-overrides") !=
        std::string::npos);
}

TEST(formula_recipe_cold_case_keeps_cache_empty) {
  const auto workload = make_workload(valid_config, WorkloadKind::RepeatHeavy);
  const auto storage = build_storage_case(valid_config, StorageStrategyKind::FormulaRecipe);
  CHECK(workload.has_value());
  CHECK(storage.has_value());
  CHECK(verify_workload(*storage, *workload));
  CHECK(storage->cache_statistics().resident_entries == 0);
  CHECK(storage->cache_statistics().resident_bytes == 0);
}

TEST(track_a_report_contract_is_explicit_and_non_ranking) {
  const auto contract = fair_storage_report_contract();
  CHECK(contract.find("formula-recipe") != std::string::npos);
  CHECK(contract.find("formula-recipe-with-overrides") != std::string::npos);
  CHECK(contract.find("dense-vector") != std::string::npos);
  CHECK(contract.find("sorted-pairs") != std::string::npos);
  CHECK(contract.find("hash-table") != std::string::npos);
  CHECK(contract.find("sequential") != std::string::npos);
  CHECK(contract.find("pseudo-random") != std::string::npos);
  CHECK(contract.find("repeat-heavy") != std::string::npos);
  CHECK(contract.find("not a global ranking") != std::string::npos);
  CHECK(contract.find("sample count 11") != std::string::npos);
  CHECK(contract.find("peak RAM bytes or peak RAM unavailable") != std::string::npos);
  CHECK(contract.find("100-percent-overrides") != std::string::npos);
}
