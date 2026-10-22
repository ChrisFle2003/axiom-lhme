#pragma once

#include "algorithm_result.hpp"

namespace axiom::lhme::benchmark::algorithm {

inline Result modular_prefilter(const EvaluationRequest& exact_request,
                                AddressRange range, const CandidateFilter& filter) {
  Result output;
  if (exact_request.mode != EvaluationMode::Exact || range.end < range.start ||
      filter.mode != FilterMode::ModularEquals || filter.modulus <= 0 ||
      filter.expected_residue < 0 || filter.expected_residue >= filter.modulus) {
    output.status = Status::InvalidArgument;
    return output;
  }
  for (std::int64_t index = range.start;; ++index) {
    if (cancelled(exact_request)) {
      output.status = Status::Cancelled;
      output.retained_candidates.clear();
      return output;
    }
    ++output.summary.checked_count;
    auto request = exact_request;
    request.address.index = index;
    request.mode = EvaluationMode::Modular;
    request.modulus = filter.modulus;
    const auto modular = evaluate(request);
    if (modular.status == Status::InvalidArgument) {
      ++output.summary.invalid_argument_count;
      if (index == range.end) break;
      continue;
    }
    if (modular.status != Status::Confirmed || !modular.integer.has_value()) {
      output.status = modular.status;
      output.retained_candidates.clear();
      return output;
    }
    if (*modular.integer != filter.expected_residue) {
      ++output.filtered_out_count;
      if (index == range.end) break;
      continue;
    }
    request.mode = EvaluationMode::Exact;
    request.modulus.reset();
    ++output.exact_evaluation_count;
    auto exact = evaluate(request);
    if (exact.status == Status::Confirmed) {
      ++output.summary.confirmed_count;
      output.retained_candidates.push_back(std::move(exact));
    } else if (exact.status == Status::InvalidArgument) {
      ++output.summary.invalid_argument_count;
    } else {
      output.status = exact.status;
      output.retained_candidates.clear();
      return output;
    }
    if (index == range.end) break;
  }
  output.status = Status::Confirmed;
  return output;
}

}  // namespace axiom::lhme::benchmark::algorithm
