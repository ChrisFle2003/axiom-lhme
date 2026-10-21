#pragma once

#include "bruce/lhme/streaming.hpp"

#include <cstdint>
#include <optional>
#include <string_view>
#include <type_traits>

namespace bruce::lhme::demo::stress {

inline constexpr std::uint32_t kChildReportMagic = 0x4c484d45U;  // "LHME"
inline constexpr std::uint32_t kChildReportVersion = 1;

enum class Scenario : std::uint32_t {
  MillionScanOne = 1,
  MillionScanEight = 2,
  AdaptiveHits = 3,
};

[[nodiscard]] inline constexpr std::string_view scenario_name(Scenario scenario) {
  switch (scenario) {
    case Scenario::MillionScanOne:
      return "million-scan-1";
    case Scenario::MillionScanEight:
      return "million-scan-8";
    case Scenario::AdaptiveHits:
      return "adaptive-hits";
  }
  return "unknown";
}

[[nodiscard]] inline constexpr std::optional<Scenario> parse_scenario(
    std::string_view name) {
  if (name == "million-scan-1") {
    return Scenario::MillionScanOne;
  }
  if (name == "million-scan-8") {
    return Scenario::MillionScanEight;
  }
  if (name == "adaptive-hits") {
    return Scenario::AdaptiveHits;
  }
  return std::nullopt;
}

struct ChildReport {
  std::uint32_t magic = kChildReportMagic;
  std::uint32_t version = kChildReportVersion;
  std::uint32_t scenario = 0;
  std::uint32_t status = static_cast<std::uint32_t>(Status::InternalError);
  std::uint64_t checked_count = 0;
  std::uint64_t confirmed_count = 0;
  std::uint64_t invalid_argument_count = 0;
  std::uint64_t filtered_out_count = 0;
  std::uint64_t exact_evaluation_count = 0;
  std::uint32_t retention_mode =
      static_cast<std::uint32_t>(RetentionMode::SummaryOnly);
  std::uint32_t reserved = 0;
  std::uint64_t retained_count = 0;
  std::int64_t elapsed_milliseconds = 0;
  std::uint64_t peak_bytes = 0;
};

static_assert(std::is_trivially_copyable_v<ChildReport>);

[[nodiscard]] inline constexpr bool is_known_scenario(std::uint32_t value) {
  return value == static_cast<std::uint32_t>(Scenario::MillionScanOne) ||
         value == static_cast<std::uint32_t>(Scenario::MillionScanEight) ||
         value == static_cast<std::uint32_t>(Scenario::AdaptiveHits);
}

[[nodiscard]] inline constexpr bool is_known_status(std::uint32_t value) {
  switch (static_cast<Status>(value)) {
    case Status::Confirmed:
    case Status::Rejected:
    case Status::BudgetExceeded:
    case Status::Cancelled:
    case Status::InvalidArgument:
    case Status::UnsupportedMode:
    case Status::InternalError:
      return true;
  }
  return false;
}

[[nodiscard]] inline constexpr bool is_known_retention_mode(std::uint32_t value) {
  switch (static_cast<RetentionMode>(value)) {
    case RetentionMode::SummaryOnly:
    case RetentionMode::FilteredCandidates:
    case RetentionMode::TopK:
    case RetentionMode::Adaptive:
      return true;
  }
  return false;
}

[[nodiscard]] inline constexpr bool valid_child_report(const ChildReport& report) {
  return report.magic == kChildReportMagic &&
         report.version == kChildReportVersion &&
         is_known_scenario(report.scenario) && is_known_status(report.status) &&
         is_known_retention_mode(report.retention_mode) && report.reserved == 0 &&
         report.peak_bytes > 0;
}

}  // namespace bruce::lhme::demo::stress
