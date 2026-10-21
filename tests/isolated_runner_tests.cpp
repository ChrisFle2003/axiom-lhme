#include "test.hpp"

#include "isolated_runner.hpp"

#include <string_view>

namespace {

using bruce::lhme::Status;
using bruce::lhme::demo::stress::Scenario;
using bruce::lhme::demo::stress::run_isolated;

[[nodiscard]] std::string_view stress_executable_path() {
  return LHME_STRESS_EXECUTABLE;
}

[[nodiscard]] std::string_view fixture_executable_path() {
  return LHME_ISOLATED_FIXTURE_EXECUTABLE;
}

}  // namespace

TEST(adaptive_child_returns_a_valid_isolated_report) {
  const auto run = run_isolated(stress_executable_path(), Scenario::AdaptiveHits);
#if defined(__linux__)
  CHECK(run.available);
  CHECK(run.success);
  CHECK(run.report.status == static_cast<std::uint32_t>(Status::Confirmed));
  CHECK(run.report.exact_evaluation_count == 1'250);
  CHECK(run.report.peak_bytes > 0);
#else
  CHECK(!run.available);
#endif
}

TEST(truncated_extra_and_abnormal_child_reports_fail_closed) {
#if defined(__linux__)
  const auto truncated =
      run_isolated(fixture_executable_path(), Scenario::MillionScanOne);
  CHECK(truncated.available);
  CHECK(!truncated.success);

  const auto extra = run_isolated(fixture_executable_path(), Scenario::MillionScanEight);
  CHECK(extra.available);
  CHECK(!extra.success);

  const auto nonzero = run_isolated(fixture_executable_path(), Scenario::AdaptiveHits);
  CHECK(nonzero.available);
  CHECK(!nonzero.success);
#endif
}
