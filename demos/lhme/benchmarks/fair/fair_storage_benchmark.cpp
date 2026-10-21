#include "demo_report.hpp"

#include "benchmark_contract.hpp"
#include "fair_isolated_memory.hpp"
#include "fair_storage.hpp"

#include <array>
#include <chrono>
#include <cstdint>
#include <string_view>
#include <vector>

namespace {

using namespace bruce::lhme;
using namespace bruce::lhme::benchmark::fair;

[[nodiscard]] const char* workload_name(WorkloadKind kind) {
  switch (kind) {
    case WorkloadKind::Sequential: return "sequential";
    case WorkloadKind::PseudoRandom: return "pseudo-random";
    case WorkloadKind::RepeatHeavy: return "repeat-heavy";
  }
  return "unknown";
}

[[nodiscard]] bool run_case(demo::Report& report, FairBenchmarkConfig config,
                            StorageStrategyKind strategy, WorkloadKind kind) {
  const auto workload = make_workload(config, kind);
  if (!workload.has_value()) return false;
  const auto build_started = std::chrono::steady_clock::now();
  const auto storage = build_storage_case(config, strategy);
  const auto build_elapsed = std::chrono::duration_cast<std::chrono::microseconds>(
      std::chrono::steady_clock::now() - build_started).count();
  if (!storage.has_value() || !verify_workload(*storage, *workload)) return false;
  if (!verify_workload(*storage, *workload)) return false;  // excluded warm-up

  std::vector<std::uint64_t> samples;
  samples.reserve(kTimedSampleCount);
  for (std::size_t sample = 0; sample < kTimedSampleCount; ++sample) {
    const auto started = std::chrono::steady_clock::now();
    std::uint64_t checksum = 0;
    for (const auto address : workload->addresses) {
      const auto value = lookup_scalar(*storage, address);
      if (!value.has_value()) return false;
      checksum ^= mix(address ^ *value);
    }
    if (checksum == 0 && !workload->addresses.empty()) return false;
    samples.push_back(static_cast<std::uint64_t>(
        std::chrono::duration_cast<std::chrono::microseconds>(
            std::chrono::steady_clock::now() - started).count()));
  }
  const auto summary = summarize_samples(samples);
  const auto peak = measure_storage_peak(config, strategy, *workload);
  if (!summary.has_value() || !peak.success) return false;

  const auto label = storage_case_label(*storage) + " " + workload_name(kind);
  report.row(label + " domain", std::to_string(config.record_count));
  report.row(label + " seed", std::to_string(config.seed));
  report.row(label + " build microseconds", std::to_string(build_elapsed));
  report.row(label + " lookup minimum microseconds", std::to_string(summary->minimum_microseconds));
  report.row(label + " lookup median microseconds", std::to_string(summary->median_microseconds));
  report.row(label + " lookup maximum microseconds", std::to_string(summary->maximum_microseconds));
  report.row(label + (peak.available ? " peak RAM bytes" : " peak RAM unavailable"),
             peak.available ? std::to_string(peak.bytes) : "unavailable");
  return true;
}

}  // namespace

int main(int argc, char* argv[]) {
  const bool full = argc == 2 && std::string_view{argv[1]} == "--full";
  const bool quick = argc == 2 && std::string_view{argv[1]} == "--quick";
  if (argc != 1 && !full && !quick) return 1;

  demo::Report report;
  report.heading("LHME Fair Storage Comparison");
  report.row("comparison rule", fair_storage_report_contract());
  report.row("modulus", "2305843009213693951");

  const std::array<std::uint64_t, 3> domains = full
      ? std::array<std::uint64_t, 3>{1'024, 65'536, 1'000'000}
      : std::array<std::uint64_t, 3>{1'024, 0, 0};
  const std::vector<std::uint64_t> densities = full
      ? std::vector<std::uint64_t>{0, 1, 10, 100, 1'000}
      : std::vector<std::uint64_t>{0};
  const std::array strategies{StorageStrategyKind::FormulaRecipe,
                              StorageStrategyKind::FormulaRecipeWithOverrides,
                              StorageStrategyKind::DenseVector,
                              StorageStrategyKind::SortedPairs,
                              StorageStrategyKind::HashTable};
  const std::array workloads{WorkloadKind::Sequential, WorkloadKind::PseudoRandom,
                             WorkloadKind::RepeatHeavy};

  for (const auto domain : domains) {
    if (domain == 0) continue;
    for (const auto density_marker : densities) {
      const auto overrides = full ? domain * density_marker / 1'000U : 0U;
      for (const auto strategy : strategies) {
        if (strategy == StorageStrategyKind::FormulaRecipe && overrides != 0) continue;
        const FairBenchmarkConfig config{.record_count = domain,
                                         .query_count = full ? domain : 257,
                                         .modulus = 2'305'843'009'213'693'951ULL,
                                         .override_count = overrides,
                                         .seed = 0x4252554345ULL};
        for (const auto workload : workloads) {
          report.expect_true("fair storage case", run_case(report, config, strategy, workload));
        }
      }
    }
  }
  return report.finish();
}
