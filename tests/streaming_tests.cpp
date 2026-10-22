#include "test.hpp"

#include "axiom/lhme/streaming.hpp"

#include <atomic>
#include <memory>
#include <vector>

namespace {

axiom::lhme::EvaluationRequest exact_request() {
  return {.spec = axiom::lhme::make_power_offset_spec(2, 100, 1),
          .mode = axiom::lhme::EvaluationMode::Exact,
          .budget = axiom::lhme::Budget::unlimited()};
}

axiom::lhme::StreamingOptions retained_options() {
  return {.worker_count = 1,
          .tile_size = 2,
          .retention_mode = axiom::lhme::RetentionMode::FilteredCandidates,
          .max_retained_candidates = 16,
          .retention_memory_bytes = 4096};
}

std::vector<axiom::lhme::BigInt> exact_values(
    const std::vector<axiom::lhme::EvaluationResult>& results) {
  std::vector<axiom::lhme::BigInt> values;
  for (const auto& result : results) values.push_back(result.exact_integer());
  return values;
}

}  // namespace

TEST(streaming_rejects_non_exact_and_invalid_options_before_exact_work) {
  auto metric = exact_request();
  metric.mode = axiom::lhme::EvaluationMode::Metric;
  const auto non_exact = axiom::lhme::evaluate_streaming_range(
      metric, {0, 5}, retained_options());
  CHECK(non_exact.status == axiom::lhme::Status::InvalidArgument);
  CHECK(non_exact.exact_evaluation_count == 0);

  auto no_workers = retained_options();
  no_workers.worker_count = 0;
  const auto invalid_worker = axiom::lhme::evaluate_streaming_range(
      exact_request(), {0, 5}, no_workers);
  CHECK(invalid_worker.status == axiom::lhme::Status::InvalidArgument);
  CHECK(invalid_worker.exact_evaluation_count == 0);

  auto invalid_filter = retained_options();
  invalid_filter.filter = {.mode = axiom::lhme::FilterMode::ModularEquals,
                           .modulus = 0,
                           .expected_residue = 0};
  const auto bad_filter = axiom::lhme::evaluate_streaming_range(
      exact_request(), {0, 5}, invalid_filter);
  CHECK(bad_filter.status == axiom::lhme::Status::InvalidArgument);
  CHECK(bad_filter.exact_evaluation_count == 0);
}

TEST(zero_retention_limits_force_summary_only) {
  auto options = retained_options();
  options.max_retained_candidates = 0;
  const auto result = axiom::lhme::evaluate_streaming_range(
      exact_request(), {0, 5}, options);

  CHECK(result.status == axiom::lhme::Status::Confirmed);
  CHECK(result.effective_retention_mode == axiom::lhme::RetentionMode::SummaryOnly);
  CHECK(result.retained_candidates.empty());
  CHECK(result.summary.checked_count == 6);
  CHECK(result.summary.confirmed_count == 6);
  CHECK(result.exact_evaluation_count == 6);
}

TEST(summary_only_never_retains_candidates_with_nonzero_limits) {
  auto options = retained_options();
  options.retention_mode = axiom::lhme::RetentionMode::SummaryOnly;
  const auto result = axiom::lhme::evaluate_streaming_range(
      exact_request(), {0, 20}, options);

  CHECK(result.status == axiom::lhme::Status::Confirmed);
  CHECK(result.effective_retention_mode == axiom::lhme::RetentionMode::SummaryOnly);
  CHECK(result.retained_candidates.empty());
  CHECK(result.summary.confirmed_count == 21);
}

TEST(streaming_one_and_four_workers_have_same_summary_and_candidates) {
  const auto one = axiom::lhme::evaluate_streaming_range(
      exact_request(), {0, 20}, retained_options());
  auto four_options = retained_options();
  four_options.worker_count = 4;
  const auto four = axiom::lhme::evaluate_streaming_range(
      exact_request(), {0, 20}, four_options);

  CHECK(one.status == axiom::lhme::Status::Confirmed);
  CHECK(one.summary == four.summary);
  CHECK(exact_values(one.retained_candidates) == exact_values(four.retained_candidates));
}

TEST(streaming_shared_cancellation_retains_nothing_after_stop) {
  auto request = exact_request();
  request.cancellation_signal = std::make_shared<std::atomic_bool>(true);
  const auto result = axiom::lhme::evaluate_streaming_range(
      request, {0, 20}, retained_options());

  CHECK(result.status == axiom::lhme::Status::Cancelled);
  CHECK(result.retained_candidates.empty());
}

TEST(modular_prefilter_skips_exact_for_non_matching_addresses) {
  auto options = retained_options();
  options.worker_count = 2;
  options.filter = {.mode = axiom::lhme::FilterMode::ModularEquals,
                    .modulus = 17,
                    .expected_residue = 11};
  const auto result = axiom::lhme::evaluate_streaming_range(
      exact_request(), {0, 10}, options);

  CHECK(result.status == axiom::lhme::Status::Confirmed);
  CHECK(result.filtered_out_count == 9);
  CHECK(result.exact_evaluation_count == 2);
  CHECK(result.summary.confirmed_count == 2);
  CHECK(result.summary.invalid_argument_count == 0);
  CHECK(result.retained_candidates.size() == 2);
  const std::vector<axiom::lhme::BigInt> expected{300, 102300};
  CHECK(exact_values(result.retained_candidates) == expected);
}

TEST(top_k_retains_lowest_addresses_independent_of_workers) {
  auto one_options = retained_options();
  one_options.retention_mode = axiom::lhme::RetentionMode::TopK;
  one_options.max_retained_candidates = 3;
  const auto one = axiom::lhme::evaluate_streaming_range(
      exact_request(), {0, 20}, one_options);
  auto four_options = one_options;
  four_options.worker_count = 4;
  const auto four = axiom::lhme::evaluate_streaming_range(
      exact_request(), {0, 20}, four_options);

  const std::vector<axiom::lhme::BigInt> expected{0, 100, 300};
  CHECK(exact_values(one.retained_candidates) == expected);
  CHECK(exact_values(one.retained_candidates) == exact_values(four.retained_candidates));
}

TEST(adaptive_switches_to_top_k_without_exceeding_limits) {
  auto options = retained_options();
  options.retention_mode = axiom::lhme::RetentionMode::Adaptive;
  options.max_retained_candidates = 3;
  const auto result = axiom::lhme::evaluate_streaming_range(
      exact_request(), {0, 20}, options);

  CHECK(result.status == axiom::lhme::Status::Confirmed);
  CHECK(result.effective_retention_mode == axiom::lhme::RetentionMode::TopK);
  CHECK(result.retained_candidates.size() == 3);
}

TEST(oversized_candidate_is_summarized_but_not_retained) {
  auto options = retained_options();
  options.retention_memory_bytes = 1;
  const auto result = axiom::lhme::evaluate_streaming_range(
      exact_request(), {20, 20}, options);

  CHECK(result.status == axiom::lhme::Status::Confirmed);
  CHECK(result.summary.confirmed_count == 1);
  CHECK(result.retained_candidates.empty());
}
