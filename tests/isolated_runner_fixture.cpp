#include "isolated_protocol.hpp"

#include <cstddef>
#include <cstdint>
#include <string_view>

#if defined(__linux__)
#include <unistd.h>
#endif

namespace {

using bruce::lhme::RetentionMode;
using bruce::lhme::Status;
using bruce::lhme::demo::stress::ChildReport;
using bruce::lhme::demo::stress::Scenario;

[[nodiscard]] ChildReport valid_report(Scenario scenario) {
  ChildReport report{};
  report.scenario = static_cast<std::uint32_t>(scenario);
  report.status = static_cast<std::uint32_t>(Status::Confirmed);
  report.retention_mode = static_cast<std::uint32_t>(RetentionMode::SummaryOnly);
  report.peak_bytes = 1;
  return report;
}

#if defined(__linux__)
[[nodiscard]] bool write_bytes(const void* data, std::size_t size) {
  const auto* bytes = reinterpret_cast<const unsigned char*>(data);
  while (size > 0) {
    const auto written = write(STDOUT_FILENO, bytes, size);
    if (written <= 0) return false;
    bytes += written;
    size -= static_cast<std::size_t>(written);
  }
  return true;
}
#endif

}  // namespace

int main(int argc, char* argv[]) {
#if !defined(__linux__)
  static_cast<void>(argc);
  static_cast<void>(argv);
  return 0;
#else
  if (argc != 3 || std::string_view{argv[1]} != "--isolated-child") return 2;

  const auto scenario = bruce::lhme::demo::stress::parse_scenario(argv[2]);
  if (!scenario.has_value()) return 3;
  const auto report = valid_report(*scenario);

  switch (*scenario) {
    case Scenario::MillionScanOne:
      return write_bytes(&report, sizeof(report) - 1) ? 0 : 4;
    case Scenario::MillionScanEight: {
      const std::uint8_t extra_byte = 0;
      return write_bytes(&report, sizeof(report)) &&
                     write_bytes(&extra_byte, sizeof(extra_byte))
                 ? 0
                 : 4;
    }
    case Scenario::AdaptiveHits:
      return write_bytes(&report, sizeof(report)) ? 5 : 4;
  }
  return 6;
#endif
}
