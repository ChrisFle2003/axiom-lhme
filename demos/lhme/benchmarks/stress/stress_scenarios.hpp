#pragma once

#include "isolated_protocol.hpp"
#include "process_memory.hpp"
#include "reference_values.hpp"

#include "axiom/lhme/streaming.hpp"

#include <chrono>
#include <cstdint>

namespace axiom::lhme::demo::stress {

inline constexpr std::uint64_t kMillionAddressCount = 1'000'001;

[[nodiscard]] inline EvaluationRequest exact_request() {
  auto request = EvaluationRequest{
      .spec = demo::reference_spec(),
      .mode = EvaluationMode::Exact,
      .budget = Budget::unlimited()};
  request.budget.max_candidate_count = kMillionAddressCount;
  return request;
}

[[nodiscard]] inline CandidateFilter never_match_filter() {
  return {.mode = FilterMode::ModularEquals, .modulus = 17, .expected_residue = 2};
}

[[nodiscard]] inline CandidateFilter matching_filter() {
  return {.mode = FilterMode::ModularEquals, .modulus = 17, .expected_residue = 11};
}

[[nodiscard]] inline StreamingOptions million_scan_options(std::uint32_t workers) {
  return {.worker_count = workers,
          .tile_size = 4096,
          .retention_mode = RetentionMode::SummaryOnly,
          .max_retained_candidates = 0,
          .retention_memory_bytes = 0,
          .filter = never_match_filter()};
}

[[nodiscard]] inline StreamingOptions adaptive_options() {
  return {.worker_count = 1,
          .tile_size = 128,
          .retention_mode = RetentionMode::Adaptive,
          .max_retained_candidates = 3,
          .retention_memory_bytes = 4096,
          .filter = matching_filter()};
}

[[nodiscard]] inline bool adaptive_top_k_is_expected(
    const StreamingRangeResult& result) {
  return result.retained_candidates.size() == 3 &&
         result.retained_candidates[0].integer == BigInt(300) &&
         result.retained_candidates[1].integer == BigInt(102300) &&
         result.retained_candidates[2].integer == BigInt(26'214'300);
}

[[nodiscard]] inline bool scenario_is_expected(
    Scenario scenario, const StreamingRangeResult& result) {
  if (result.status != Status::Confirmed) return false;

  switch (scenario) {
    case Scenario::MillionScanOne:
    case Scenario::MillionScanEight:
      return result.summary.checked_count == kMillionAddressCount &&
             result.exact_evaluation_count == 0 &&
             result.filtered_out_count == kMillionAddressCount &&
             result.retained_candidates.empty() &&
             result.effective_retention_mode == RetentionMode::SummaryOnly;
    case Scenario::AdaptiveHits:
      return result.summary.checked_count == 10'001 &&
             result.exact_evaluation_count == 1'250 &&
             result.filtered_out_count == 8'751 &&
             result.effective_retention_mode == RetentionMode::TopK &&
             adaptive_top_k_is_expected(result);
  }
  return false;
}

[[nodiscard]] inline ChildReport execute_scenario(Scenario scenario) {
  const auto request = exact_request();
  const auto start = std::chrono::steady_clock::now();

  StreamingRangeResult result;
  switch (scenario) {
    case Scenario::MillionScanOne:
      result = evaluate_streaming_range(request, {0, 1'000'000},
                                        million_scan_options(1));
      break;
    case Scenario::MillionScanEight:
      result = evaluate_streaming_range(request, {0, 1'000'000},
                                        million_scan_options(8));
      break;
    case Scenario::AdaptiveHits:
      result = evaluate_streaming_range(request, {0, 10'000}, adaptive_options());
      break;
  }

  const auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(
      std::chrono::steady_clock::now() - start);
  const auto peak = process_peak_memory();
  const bool expected = scenario_is_expected(scenario, result);

  return {.scenario = static_cast<std::uint32_t>(scenario),
          .status = static_cast<std::uint32_t>(
              expected ? Status::Confirmed : Status::InternalError),
          .checked_count = result.summary.checked_count,
          .confirmed_count = result.summary.confirmed_count,
          .invalid_argument_count = result.summary.invalid_argument_count,
          .filtered_out_count = result.filtered_out_count,
          .exact_evaluation_count = result.exact_evaluation_count,
          .retention_mode =
              static_cast<std::uint32_t>(result.effective_retention_mode),
          .retained_count = result.retained_candidates.size(),
          .elapsed_milliseconds = elapsed.count(),
          .peak_bytes = peak.available ? peak.bytes : 0};
}

}  // namespace axiom::lhme::demo::stress
