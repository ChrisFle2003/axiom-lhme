#include "demo_report.hpp"
#include "reference_values.hpp"

#include "algorithms/eager_range.hpp"
#include "algorithms/lazy_summary.hpp"
#include "algorithms/modular_prefilter.hpp"

#include "bruce/lhme/streaming.hpp"

#include <chrono>
#include <cstdint>
#include <string>
#include <utility>

namespace {

using Clock = std::chrono::steady_clock;

bruce::lhme::EvaluationRequest exact_request() {
  return {.spec = bruce::lhme::demo::reference_spec(),
          .mode = bruce::lhme::EvaluationMode::Exact,
          .budget = bruce::lhme::Budget::unlimited()};
}

bruce::lhme::StreamingOptions summary_options(std::uint32_t workers) {
  return {.worker_count = workers,
          .tile_size = 4,
          .retention_mode = bruce::lhme::RetentionMode::SummaryOnly,
          .max_retained_candidates = 3,
          .retention_memory_bytes = 4096};
}

bruce::lhme::CandidateFilter modular_filter() {
  return {.mode = bruce::lhme::FilterMode::ModularEquals,
          .modulus = 17,
          .expected_residue = 11};
}

bruce::lhme::StreamingOptions filtered_options() {
  return {.worker_count = 1,
          .tile_size = 4,
          .retention_mode = bruce::lhme::RetentionMode::Adaptive,
          .max_retained_candidates = 3,
          .retention_memory_bytes = 4096,
          .filter = modular_filter()};
}

template <typename Callable>
auto measure(Callable callable) {
  const auto start = Clock::now();
  auto result = callable();
  const auto elapsed = std::chrono::duration_cast<std::chrono::microseconds>(
      Clock::now() - start);
  return std::pair{std::move(result), elapsed.count()};
}

void row(bruce::lhme::demo::Report& report, std::string name,
         std::uint32_t workers, std::uint64_t exact_count,
         std::uint64_t filtered_count, std::size_t retained_count,
         std::int64_t elapsed_microseconds) {
  report.row(name + " workers", std::to_string(workers));
  report.row(name + " exact evaluations", std::to_string(exact_count));
  report.row(name + " filtered out", std::to_string(filtered_count));
  report.row(name + " retained candidates", std::to_string(retained_count));
  report.row(name + " elapsed microseconds", std::to_string(elapsed_microseconds));
}

bool same_exact_prefix(const std::vector<bruce::lhme::EvaluationResult>& reference,
                       const std::vector<bruce::lhme::EvaluationResult>& prefix) {
  if (prefix.size() > reference.size()) return false;
  for (std::size_t index = 0; index < prefix.size(); ++index) {
    if (reference[index].integer != prefix[index].integer) return false;
  }
  return true;
}

}  // namespace

int main() {
  using namespace bruce::lhme;

  demo::Report report;
  report.heading("LHME Algorithm Comparison Benchmark");
  constexpr AddressRange range{0, 30};
  const auto request = exact_request();

  const auto [eager, eager_elapsed] = measure([&] {
    return benchmark::algorithm::eager_range(request, range);
  });
  const auto [lazy, lazy_elapsed] = measure([&] {
    return benchmark::algorithm::lazy_summary(request, range);
  });
  const auto [prefilter, prefilter_elapsed] = measure([&] {
    return benchmark::algorithm::modular_prefilter(request, range, modular_filter());
  });
  const auto [summary_one, summary_one_elapsed] = measure([&] {
    return evaluate_streaming_range(request, range, summary_options(1));
  });
  const auto [summary_four, summary_four_elapsed] = measure([&] {
    return evaluate_streaming_range(request, range, summary_options(4));
  });
  const auto [filtered, filtered_elapsed] = measure([&] {
    return evaluate_streaming_range(request, range, filtered_options());
  });

  row(report, "Reference eager", 1, eager.exact_evaluation_count,
      eager.filtered_out_count, eager.retained_candidates.size(), eager_elapsed);
  row(report, "Reference lazy", 1, lazy.exact_evaluation_count,
      lazy.filtered_out_count, lazy.retained_candidates.size(), lazy_elapsed);
  row(report, "Reference prefilter", 1, prefilter.exact_evaluation_count,
      prefilter.filtered_out_count, prefilter.retained_candidates.size(),
      prefilter_elapsed);
  row(report, "V2 summary", 1, summary_one.exact_evaluation_count,
      summary_one.filtered_out_count, summary_one.retained_candidates.size(),
      summary_one_elapsed);
  row(report, "V2 summary", 4, summary_four.exact_evaluation_count,
      summary_four.filtered_out_count, summary_four.retained_candidates.size(),
      summary_four_elapsed);
  row(report, "V2 filtered adaptive", 1, filtered.exact_evaluation_count,
      filtered.filtered_out_count, filtered.retained_candidates.size(),
      filtered_elapsed);

  report.expect_equal("reference eager status", Status::Confirmed, eager.status);
  report.expect_equal("reference eager materialized candidates", std::size_t(31),
                      eager.retained_candidates.size());
  report.expect_equal("reference lazy status", Status::Confirmed, lazy.status);
  report.expect_true("reference lazy retains nothing",
                     lazy.retained_candidates.empty());
  report.expect_equal("reference prefilter status", Status::Confirmed,
                      prefilter.status);
  report.expect_equal("reference prefilter exact evaluations", std::uint64_t(4),
                      prefilter.exact_evaluation_count);
  report.expect_equal("reference prefilter misses", std::uint64_t(27),
                      prefilter.filtered_out_count);
  report.expect_equal("reference prefilter retained candidates", std::size_t(4),
                      prefilter.retained_candidates.size());
  report.expect_equal("V2 summary status", Status::Confirmed, summary_one.status);
  report.expect_equal("V2 one/four checked", summary_one.summary.checked_count,
                      summary_four.summary.checked_count);
  report.expect_equal("V2 one/four confirmed", summary_one.summary.confirmed_count,
                      summary_four.summary.confirmed_count);
  report.expect_equal("V2 one/four invalid", summary_one.summary.invalid_argument_count,
                      summary_four.summary.invalid_argument_count);
  report.expect_equal("V2 one/four exact count", summary_one.exact_evaluation_count,
                      summary_four.exact_evaluation_count);
  report.expect_equal("V2 summary exact evaluations", std::uint64_t(31),
                      summary_one.exact_evaluation_count);
  report.expect_true("V2 summary retains nothing",
                     summary_one.retained_candidates.empty() &&
                         summary_four.retained_candidates.empty());
  report.expect_equal("V2 filtered status", Status::Confirmed, filtered.status);
  report.expect_equal("V2 filter exact evaluations", std::uint64_t(4),
                      filtered.exact_evaluation_count);
  report.expect_true("V2 filter reduces exact work",
                     filtered.exact_evaluation_count <
                         summary_one.exact_evaluation_count);
  report.expect_equal("V2 filter misses", std::uint64_t(27),
                      filtered.filtered_out_count);
  report.expect_equal("V2 filtered retained candidates", std::size_t(3),
                      filtered.retained_candidates.size());
  report.expect_equal("V2 adaptive fallback", RetentionMode::TopK,
                      filtered.effective_retention_mode);
  report.expect_equal("BRUCE/lazy checked", lazy.summary.checked_count,
                      summary_one.summary.checked_count);
  report.expect_equal("BRUCE/lazy confirmed", lazy.summary.confirmed_count,
                      summary_one.summary.confirmed_count);
  report.expect_equal("BRUCE/lazy invalid", lazy.summary.invalid_argument_count,
                      summary_one.summary.invalid_argument_count);
  report.expect_true("BRUCE adaptive values match reference prefix",
                     same_exact_prefix(prefilter.retained_candidates,
                                       filtered.retained_candidates));
  return report.finish();
}
