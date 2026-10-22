#include "demo_report.hpp"

#include "isolated_runner.hpp"
#include "stress_scenarios.hpp"

#include <atomic>
#include <cstdint>
#include <memory>
#include <string>
#include <string_view>

#if defined(__linux__)
#include <cerrno>
#include <unistd.h>
#endif

namespace {

using axiom::lhme::RetentionMode;
using axiom::lhme::Status;
using axiom::lhme::demo::Report;
using axiom::lhme::demo::stress::ChildReport;
using axiom::lhme::demo::stress::IsolatedRunResult;
using axiom::lhme::demo::stress::Scenario;

#if defined(__linux__)
[[nodiscard]] bool write_child_report(const ChildReport& report) {
  const auto* input = reinterpret_cast<const unsigned char*>(&report);
  std::size_t remaining = sizeof(report);
  while (remaining > 0) {
    const auto written = write(STDOUT_FILENO, input, remaining);
    if (written < 0) {
      if (errno == EINTR) continue;
      return false;
    }
    if (written == 0) return false;
    input += written;
    remaining -= static_cast<std::size_t>(written);
  }
  return true;
}

[[nodiscard]] int run_isolated_child(Scenario scenario) {
  const auto child_report = axiom::lhme::demo::stress::execute_scenario(scenario);
  if (!axiom::lhme::demo::stress::valid_child_report(child_report) ||
      child_report.status != static_cast<std::uint32_t>(Status::Confirmed)) {
    return 2;
  }
  return write_child_report(child_report) ? 0 : 3;
}
#endif

void report_million_expectations(Report& report, std::string_view name,
                                 const ChildReport& child_report) {
  const auto prefix = std::string{name} + " ";
  report.expect_equal(prefix + "status",
                      static_cast<std::uint32_t>(Status::Confirmed),
                      child_report.status);
  report.expect_equal(prefix + "checked", std::uint64_t(1'000'001),
                      child_report.checked_count);
  report.expect_equal(prefix + "exact", std::uint64_t(0),
                      child_report.exact_evaluation_count);
  report.expect_equal(prefix + "filtered", std::uint64_t(1'000'001),
                      child_report.filtered_out_count);
  report.expect_equal(prefix + "retained", std::uint64_t(0),
                      child_report.retained_count);
  report.expect_equal(prefix + "retention mode",
                      static_cast<std::uint32_t>(RetentionMode::SummaryOnly),
                      child_report.retention_mode);
}

void report_adaptive_expectations(Report& report, const ChildReport& child_report) {
  report.expect_equal("adaptive status", static_cast<std::uint32_t>(Status::Confirmed),
                      child_report.status);
  report.expect_equal("adaptive checked", std::uint64_t(10'001),
                      child_report.checked_count);
  report.expect_equal("adaptive exact", std::uint64_t(1'250),
                      child_report.exact_evaluation_count);
  report.expect_equal("adaptive filtered", std::uint64_t(8'751),
                      child_report.filtered_out_count);
  report.expect_equal("adaptive retained", std::uint64_t(3),
                      child_report.retained_count);
  report.expect_equal("adaptive effective mode",
                      static_cast<std::uint32_t>(RetentionMode::TopK),
                      child_report.retention_mode);
}

void report_isolated_run(Report& report, Scenario scenario,
                         const IsolatedRunResult& run) {
  const auto name = axiom::lhme::demo::stress::scenario_name(scenario);
  report.expect_true(std::string{name} + " isolated measurement available",
                     run.available);
  report.expect_true(std::string{name} + " isolated child valid", run.success);
  if (!run.success) return;

  report.row(std::string{name} + " milliseconds",
             std::to_string(run.report.elapsed_milliseconds));
  report.row(std::string{name} + " isolated process peak RAM bytes",
             std::to_string(run.report.peak_bytes));
  report.expect_true(std::string{name} + " positive peak", run.report.peak_bytes > 0);

  if (scenario == Scenario::AdaptiveHits) {
    report_adaptive_expectations(report, run.report);
  } else {
    report_million_expectations(report, name, run.report);
  }
}

void report_safe_stop(Report& report) {
  auto request = axiom::lhme::demo::stress::exact_request();
  request.cancellation_signal = std::make_shared<std::atomic_bool>(true);
  const auto cancelled = axiom::lhme::evaluate_streaming_range(
      request, {0, 1'000'000}, axiom::lhme::demo::stress::million_scan_options(4));
  report.expect_equal("safe stop status", Status::Cancelled, cancelled.status);
  report.expect_equal("safe stop checked", std::uint64_t(0),
                      cancelled.summary.checked_count);
  report.expect_equal("safe stop exact", std::uint64_t(0),
                      cancelled.exact_evaluation_count);
  report.expect_equal("safe stop filtered", std::uint64_t(0),
                      cancelled.filtered_out_count);
  report.expect_true("safe stop retains nothing", cancelled.retained_candidates.empty());
}

[[nodiscard]] int run_parent_controller() {
  Report report;
  report.heading("LHME Isolated CPU and Bounded-Retention Stress Test");

#if defined(__linux__)
  const auto first = axiom::lhme::demo::stress::run_isolated(
      "/proc/self/exe", Scenario::MillionScanOne);
  const auto eighth = axiom::lhme::demo::stress::run_isolated(
      "/proc/self/exe", Scenario::MillionScanEight);
  const auto adaptive = axiom::lhme::demo::stress::run_isolated(
      "/proc/self/exe", Scenario::AdaptiveHits);
  report_isolated_run(report, Scenario::MillionScanOne, first);
  report_isolated_run(report, Scenario::MillionScanEight, eighth);
  report_isolated_run(report, Scenario::AdaptiveHits, adaptive);
  if (first.success && eighth.success) {
    report.expect_equal("million scans checked deterministic", first.report.checked_count,
                        eighth.report.checked_count);
    report.expect_equal("million scans filtered deterministic",
                        first.report.filtered_out_count,
                        eighth.report.filtered_out_count);
  }
#else
  report.row("isolated process peak RAM bytes", "unavailable (Linux only)");
#endif

  report_safe_stop(report);
  return report.finish();
}

}  // namespace

int main(int argc, char* argv[]) {
  if (argc == 3 && std::string_view{argv[1]} == "--isolated-child") {
    const auto scenario = axiom::lhme::demo::stress::parse_scenario(argv[2]);
    if (!scenario.has_value()) return 1;
#if defined(__linux__)
    return run_isolated_child(*scenario);
#else
    return 1;
#endif
  }
  if (argc != 1) return 1;
  return run_parent_controller();
}
