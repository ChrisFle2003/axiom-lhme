#include "test.hpp"

#include "isolated_protocol.hpp"

namespace {

using bruce::lhme::Status;
using bruce::lhme::demo::stress::ChildReport;
using bruce::lhme::demo::stress::Scenario;
using bruce::lhme::demo::stress::valid_child_report;

ChildReport valid_report() {
  ChildReport report{};
  report.scenario = static_cast<std::uint32_t>(Scenario::AdaptiveHits);
  report.status = static_cast<std::uint32_t>(Status::Confirmed);
  report.retention_mode = static_cast<std::uint32_t>(bruce::lhme::RetentionMode::TopK);
  report.peak_bytes = 1;
  return report;
}

}  // namespace

TEST(child_report_rejects_unknown_magic_and_version) {
  auto report = valid_report();
  report.magic = 0;
  CHECK(!valid_child_report(report));

  report = valid_report();
  ++report.version;
  CHECK(!valid_child_report(report));
}

TEST(child_report_rejects_unknown_fields_and_zero_peak) {
  auto report = valid_report();
  report.scenario = 99;
  CHECK(!valid_child_report(report));

  report = valid_report();
  report.status = 99;
  CHECK(!valid_child_report(report));

  report = valid_report();
  report.peak_bytes = 0;
  CHECK(!valid_child_report(report));
}
