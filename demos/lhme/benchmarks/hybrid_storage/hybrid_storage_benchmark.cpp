#include "demo_report.hpp"

#include "bruce/lhme/formula_override_snapshot.hpp"
#include "bruce/lhme/formula_override_store.hpp"
#include "table_storage.hpp"

#include <chrono>
#include <cstdint>
#include <filesystem>
#include <optional>
#include <string>
#include <string_view>
#include <vector>

namespace {

using Clock = std::chrono::steady_clock;
using bruce::lhme::benchmark::table::HashValueTable;

constexpr std::uint64_t kVirtualAddressCount = 1'000'000;

struct Tier {
  std::string_view name;
  std::uint64_t override_count;
};

struct Measurement {
  bool success = false;
  std::string failure_reason;
  std::uint64_t override_count = 0;
  std::uint64_t snapshot_bytes = 0;
  std::int64_t insert_milliseconds = 0;
  std::uint64_t computed_exact_evaluations = 0;
};

[[nodiscard]] std::filesystem::path snapshot_path(std::uint64_t override_count) {
  return std::filesystem::temp_directory_path() /
         ("bruce-hybrid-" + std::to_string(override_count) + ".snapshot");
}

[[nodiscard]] Measurement measure_tier(std::uint64_t override_count) {
  using namespace bruce::lhme;
  auto engine = Engine::create({});
  const auto binding = engine.register_formula(make_power_offset_spec(2, 100, 1));
  FormulaOverrideStore store{engine};

  const auto started = Clock::now();
  for (std::uint64_t ordinal = 0; ordinal < override_count; ++ordinal) {
    const FormulaAddress address{.index = static_cast<std::int64_t>(
        ordinal * kVirtualAddressCount / override_count)};
    if (store.set_override(binding, address,
                           BigInt{static_cast<std::int64_t>(ordinal % 97U + 1U)}) !=
        Status::Confirmed) {
      return {.failure_reason = "override insertion failed"};
    }
  }
  const auto inserted = std::chrono::duration_cast<std::chrono::milliseconds>(
      Clock::now() - started);

  if (override_count != 0) {
    const auto override = store.evaluate({.binding = binding,
                                          .address = {.index = 0},
                                          .mode = EvaluationMode::Exact,
                                          .budget = Budget::unlimited()});
    if (override.source != FormulaValueSource::Override ||
        override.result.status != Status::Confirmed) {
      return {.failure_reason = "override lookup failed"};
    }
  }
  if (override_count < kVirtualAddressCount) {
    const auto formula = store.evaluate(
        {.binding = binding,
         .address = {.index = static_cast<std::int64_t>(kVirtualAddressCount - 1)},
         .mode = EvaluationMode::Metric,
         .budget = Budget::unlimited()});
    if (formula.source != FormulaValueSource::Computed ||
        formula.result.status != Status::Confirmed) {
      return {.failure_reason = "formula lookup failed"};
    }
  }

  const auto path = snapshot_path(override_count);
  std::error_code error;
  std::filesystem::remove(path, error);
  if (save_formula_override_snapshot(store, path) !=
      FormulaOverrideSnapshotStatus::Confirmed) {
    return {.failure_reason = "snapshot write failed"};
  }
  const auto snapshot_bytes = std::filesystem::file_size(path, error);
  std::filesystem::remove(path, error);
  if (error) return {.failure_reason = "snapshot cleanup failed"};

  const auto statistics = store.inspect();
  return {.success = statistics.override_count == override_count,
          .failure_reason = statistics.override_count == override_count
                                ? ""
                                : "stored override count differs from requested count",
          .override_count = statistics.override_count,
          .snapshot_bytes = snapshot_bytes,
          .insert_milliseconds = inserted.count(),
          .computed_exact_evaluations = statistics.computed_exact_evaluation_count};
}

}  // namespace

int main(int argc, char* argv[]) {
  using namespace bruce::lhme;

  bool include_full_override_tier = false;
  if (argc == 2 && std::string_view{argv[1]} == "--full-overrides") {
    include_full_override_tier = true;
  } else if (argc != 1) {
    return 1;
  }

  demo::Report report;
  report.heading("FormulaRecipe + Override Storage Comparison");
  report.row("virtual address domain", std::to_string(kVirtualAddressCount));
  report.row("model", "FormulaRecipe base plus sparse BigInt overrides; no KEVIN grid");

  const auto hash = HashValueTable::make(kVirtualAddressCount);
  report.row("one-million hash table container bytes",
             std::to_string(hash.metrics().container_bytes));

  std::vector<Tier> tiers{{"0 percent", 0},
                          {"0.1 percent", 1'000},
                          {"1 percent", 10'000},
                          {"10 percent", 100'000}};
  if (include_full_override_tier) tiers.push_back({"100 percent", kVirtualAddressCount});

  std::optional<std::uint64_t> previous_snapshot_bytes;
  for (const auto tier : tiers) {
    const auto measurement = measure_tier(tier.override_count);
    const auto label = "FormulaWithOverrides " + std::string{tier.name};
    report.row(label + " overrides", std::to_string(measurement.override_count));
    report.row(label + " snapshot bytes", std::to_string(measurement.snapshot_bytes));
    report.row(label + " override insert milliseconds",
               std::to_string(measurement.insert_milliseconds));
    if (!measurement.success) {
      report.row(label + " failure reason", measurement.failure_reason);
    }
    report.expect_true(label + " benchmark completed", measurement.success);
    report.expect_equal(label + " override count", tier.override_count,
                        measurement.override_count);
    report.expect_true(label + " snapshot exists", measurement.snapshot_bytes > 0);
    report.expect_equal(label + " computed exact evaluations", std::uint64_t(0),
                        measurement.computed_exact_evaluations);
    if (previous_snapshot_bytes.has_value()) {
      report.expect_true(label + " snapshot grows with exceptions",
                         measurement.snapshot_bytes > *previous_snapshot_bytes);
    }
    previous_snapshot_bytes = measurement.snapshot_bytes;
  }

  return report.finish();
}
