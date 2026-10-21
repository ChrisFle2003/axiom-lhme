#pragma once

#include "benchmark_contract.hpp"

#include "bruce/lhme/engine.hpp"
#include "bruce/lhme/formula_override_store.hpp"

#include <algorithm>
#include <cstdint>
#include <limits>
#include <memory>
#include <optional>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

namespace bruce::lhme::benchmark::fair {

enum class StorageStrategyKind {
  FormulaRecipe,
  FormulaRecipeWithOverrides,
  DenseVector,
  SortedPairs,
  HashTable,
};

struct FairStorageCase {
  StorageStrategyKind strategy = StorageStrategyKind::FormulaRecipe;
  FairBenchmarkConfig config;
  FormulaSpec formula;
  std::shared_ptr<Engine> engine;
  std::shared_ptr<FormulaOverrideStore> overrides;
  std::vector<std::uint64_t> dense_values;
  std::vector<std::pair<std::uint64_t, std::uint64_t>> sorted_values;
  std::unordered_map<std::uint64_t, std::uint64_t> hash_values;

  [[nodiscard]] CacheStatistics cache_statistics() const {
    return engine ? engine->inspect() : CacheStatistics{};
  }
};

[[nodiscard]] inline bool has_override(const FairBenchmarkConfig& config,
                                       std::uint64_t address) {
  return address < config.override_count;
}

[[nodiscard]] inline std::uint64_t override_scalar(const FairBenchmarkConfig& config,
                                                    std::uint64_t address) {
  return mix(config.seed ^ (address + 0xd6e8feb86659fd93ULL)) % config.modulus;
}

[[nodiscard]] inline std::optional<std::uint64_t> formula_scalar(
    const FormulaSpec& formula, const FairBenchmarkConfig& config,
    std::uint64_t address) {
  if (address >= config.record_count || address >
      static_cast<std::uint64_t>(std::numeric_limits<std::int64_t>::max())) {
    return std::nullopt;
  }
  const auto result = evaluate({.spec = formula,
                                .address = {.index = static_cast<std::int64_t>(address)},
                                .mode = EvaluationMode::Modular,
                                .budget = Budget::unlimited(),
                                .modulus = BigInt{config.modulus}});
  if (result.status != Status::Confirmed || !result.integer.has_value()) {
    return std::nullopt;
  }
  return result.integer->convert_to<std::uint64_t>();
}

[[nodiscard]] inline std::optional<std::uint64_t> expected_scalar(
    const FairBenchmarkConfig& config, const FormulaSpec& formula,
    StorageStrategyKind strategy, std::uint64_t address) {
  if (address >= config.record_count) return std::nullopt;
  if (strategy != StorageStrategyKind::FormulaRecipe && has_override(config, address)) {
    return override_scalar(config, address);
  }
  return formula_scalar(formula, config, address);
}

[[nodiscard]] inline std::optional<FairStorageCase> build_storage_case(
    FairBenchmarkConfig config, StorageStrategyKind strategy) {
  if (!validate_config(config) ||
      config.record_count > static_cast<std::uint64_t>(std::numeric_limits<std::size_t>::max()) ||
      (strategy == StorageStrategyKind::FormulaRecipe && config.override_count != 0)) {
    return std::nullopt;
  }

  FairStorageCase output;
  output.strategy = strategy;
  output.config = config;
  output.formula = make_power_offset_spec(2, 100, 1);

  if (strategy == StorageStrategyKind::FormulaRecipe ||
      strategy == StorageStrategyKind::FormulaRecipeWithOverrides) {
    output.engine = std::make_shared<Engine>(Engine::create(
        {.cache_max_entries = 0, .cache_max_bytes = 0, .worker_count = 1, .tile_size = 1}));
    output.formula = output.engine->register_formula(output.formula);
    if (strategy == StorageStrategyKind::FormulaRecipeWithOverrides) {
      output.overrides = std::make_shared<FormulaOverrideStore>(*output.engine);
      const FormulaBinding binding{output.formula};
      for (std::uint64_t address = 0; address < config.override_count; ++address) {
        if (output.overrides->set_override(
                binding, {.index = static_cast<std::int64_t>(address)},
                BigInt{override_scalar(config, address)}) != Status::Confirmed) {
          return std::nullopt;
        }
      }
    }
    return output;
  }

  const auto count = static_cast<std::size_t>(config.record_count);
  if (strategy == StorageStrategyKind::DenseVector) {
    output.dense_values.reserve(count);
  } else if (strategy == StorageStrategyKind::SortedPairs) {
    output.sorted_values.reserve(count);
  } else {
    output.hash_values.reserve(count);
  }

  for (std::uint64_t address = 0; address < config.record_count; ++address) {
    const auto value = expected_scalar(config, output.formula, strategy, address);
    if (!value.has_value()) return std::nullopt;
    if (strategy == StorageStrategyKind::DenseVector) {
      output.dense_values.push_back(*value);
    } else if (strategy == StorageStrategyKind::SortedPairs) {
      output.sorted_values.emplace_back(address, *value);
    } else {
      output.hash_values.emplace(address, *value);
    }
  }
  return output;
}

[[nodiscard]] inline std::optional<std::uint64_t> lookup_scalar(
    const FairStorageCase& storage, std::uint64_t address) {
  if (address >= storage.config.record_count) return std::nullopt;
  const auto formula_address = FormulaAddress{.index = static_cast<std::int64_t>(address)};
  if (storage.strategy == StorageStrategyKind::FormulaRecipe) {
    const auto result = storage.engine->evaluate({.spec = storage.formula,
                                                   .address = formula_address,
                                                   .mode = EvaluationMode::Modular,
                                                   .budget = Budget::unlimited(),
                                                   .modulus = BigInt{storage.config.modulus}});
    if (result.status != Status::Confirmed || !result.integer.has_value()) return std::nullopt;
    return result.integer->convert_to<std::uint64_t>();
  }
  if (storage.strategy == StorageStrategyKind::FormulaRecipeWithOverrides) {
    const FormulaBinding binding{storage.formula};
    const auto result = storage.overrides->evaluate({.binding = binding,
                                                      .address = formula_address,
                                                      .mode = EvaluationMode::Modular,
                                                      .budget = Budget::unlimited(),
                                                      .modulus = BigInt{storage.config.modulus}});
    if (result.result.status != Status::Confirmed || !result.result.integer.has_value()) {
      return std::nullopt;
    }
    return result.result.integer->convert_to<std::uint64_t>();
  }
  if (storage.strategy == StorageStrategyKind::DenseVector) {
    return storage.dense_values.at(static_cast<std::size_t>(address));
  }
  if (storage.strategy == StorageStrategyKind::SortedPairs) {
    const auto found = std::lower_bound(
        storage.sorted_values.begin(), storage.sorted_values.end(), address,
        [](const auto& entry, std::uint64_t key) { return entry.first < key; });
    return found == storage.sorted_values.end() || found->first != address
               ? std::nullopt
               : std::optional<std::uint64_t>{found->second};
  }
  const auto found = storage.hash_values.find(address);
  return found == storage.hash_values.end() ? std::nullopt
                                             : std::optional<std::uint64_t>{found->second};
}

[[nodiscard]] inline bool verify_workload(const FairStorageCase& storage,
                                          const Workload& workload) {
  for (const auto address : workload.addresses) {
    const auto observed = lookup_scalar(storage, address);
    const auto expected = expected_scalar(storage.config, storage.formula,
                                          storage.strategy, address);
    if (!observed.has_value() || observed != expected) return false;
  }
  return true;
}

[[nodiscard]] inline std::string storage_case_label(const FairStorageCase& storage) {
  std::string label;
  switch (storage.strategy) {
    case StorageStrategyKind::FormulaRecipe: label = "formula-recipe"; break;
    case StorageStrategyKind::FormulaRecipeWithOverrides:
      label = "formula-recipe-with-overrides";
      break;
    case StorageStrategyKind::DenseVector: label = "dense-vector"; break;
    case StorageStrategyKind::SortedPairs: label = "sorted-pairs"; break;
    case StorageStrategyKind::HashTable: label = "hash-table"; break;
  }
  const auto numerator = static_cast<unsigned __int128>(storage.config.override_count) *
                         1'000'000U;
  const auto rounded_percent_ten_thousandths =
      (numerator + storage.config.record_count / 2U) / storage.config.record_count;
  const auto whole_percent = static_cast<std::uint64_t>(
      rounded_percent_ten_thousandths / 10'000U);
  auto fractional_percent = std::to_string(static_cast<std::uint64_t>(
      rounded_percent_ten_thousandths % 10'000U));
  fractional_percent.insert(0, 4U - fractional_percent.size(), '0');
  while (!fractional_percent.empty() && fractional_percent.back() == '0') {
    fractional_percent.pop_back();
  }
  const auto density = fractional_percent.empty()
      ? std::to_string(whole_percent)
      : std::to_string(whole_percent) + "." + fractional_percent;
  return label + "-" + density + "-percent-overrides";
}

[[nodiscard]] inline std::string fair_storage_report_contract() {
  return "Track A; formula-recipe; formula-recipe-with-overrides; dense-vector; "
         "sorted-pairs; hash-table; sequential; pseudo-random; repeat-heavy; "
         "not a global ranking; sample count 11; peak RAM bytes or peak RAM unavailable; "
         "100-percent-overrides";
}

}  // namespace bruce::lhme::benchmark::fair
