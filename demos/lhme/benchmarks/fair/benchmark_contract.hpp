#pragma once

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <optional>
#include <vector>

namespace bruce::lhme::benchmark::fair {

inline constexpr std::size_t kTimedSampleCount = 11;

enum class WorkloadKind { Sequential, PseudoRandom, RepeatHeavy };

struct FairBenchmarkConfig {
  std::uint64_t record_count = 0;
  std::uint64_t query_count = 0;
  std::uint64_t modulus = 0;
  std::uint64_t override_count = 0;
  std::uint64_t seed = 0;
};

struct Workload {
  std::vector<std::uint64_t> addresses;
  std::uint64_t checksum = 0;
};

struct TimingSummary {
  std::uint64_t minimum_microseconds = 0;
  std::uint64_t median_microseconds = 0;
  std::uint64_t maximum_microseconds = 0;
};

[[nodiscard]] inline bool validate_config(const FairBenchmarkConfig& config) {
  return config.record_count != 0 && config.query_count != 0 && config.modulus != 0 &&
         config.override_count <= config.record_count;
}

[[nodiscard]] inline std::uint64_t mix(std::uint64_t value) {
  value += 0x9e3779b97f4a7c15ULL;
  value = (value ^ (value >> 30U)) * 0xbf58476d1ce4e5b9ULL;
  value = (value ^ (value >> 27U)) * 0x94d049bb133111ebULL;
  return value ^ (value >> 31U);
}

[[nodiscard]] inline std::uint64_t workload_checksum(
    const std::vector<std::uint64_t>& addresses) {
  auto checksum = 0xcbf29ce484222325ULL;
  for (const auto address : addresses) {
    checksum ^= mix(address);
    checksum *= 0x100000001b3ULL;
  }
  return checksum;
}

[[nodiscard]] inline std::optional<Workload> make_workload(
    const FairBenchmarkConfig& config, WorkloadKind kind) {
  if (!validate_config(config)) return std::nullopt;

  Workload output;
  output.addresses.reserve(static_cast<std::size_t>(config.query_count));
  for (std::uint64_t query = 0; query < config.query_count; ++query) {
    std::uint64_t address = 0;
    switch (kind) {
      case WorkloadKind::Sequential:
        address = query % config.record_count;
        break;
      case WorkloadKind::PseudoRandom:
        address = mix(config.seed + query) % config.record_count;
        break;
      case WorkloadKind::RepeatHeavy: {
        const auto hot_count = std::min<std::uint64_t>(config.record_count, 16);
        const auto candidate = mix(config.seed + query);
        address = query % 8U == 0 ? candidate % config.record_count
                                  : candidate % hot_count;
        break;
      }
    }
    output.addresses.push_back(address);
  }
  output.checksum = workload_checksum(output.addresses);
  return output;
}

[[nodiscard]] inline std::optional<TimingSummary> summarize_samples(
    const std::vector<std::uint64_t>& samples) {
  if (samples.size() != kTimedSampleCount) return std::nullopt;
  auto ordered = samples;
  std::sort(ordered.begin(), ordered.end());
  return TimingSummary{.minimum_microseconds = ordered.front(),
                       .median_microseconds = ordered[kTimedSampleCount / 2U],
                       .maximum_microseconds = ordered.back()};
}

}  // namespace bruce::lhme::benchmark::fair
