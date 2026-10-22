#pragma once

#include "algorithm_result.hpp"

namespace axiom::lhme::benchmark::algorithm {

inline Result eager_range(const EvaluationRequest& exact_request,
                          AddressRange range) {
  Result output;
  if (exact_request.mode != EvaluationMode::Exact || range.end < range.start) {
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
    ++output.exact_evaluation_count;
    auto value = evaluate(request);
    if (value.status == Status::Confirmed) {
      ++output.summary.confirmed_count;
      output.retained_candidates.push_back(std::move(value));
    } else if (value.status == Status::InvalidArgument) {
      ++output.summary.invalid_argument_count;
    } else {
      output.status = value.status;
      output.retained_candidates.clear();
      return output;
    }
    if (index == range.end) break;
  }
  output.status = Status::Confirmed;
  return output;
}

}  // namespace axiom::lhme::benchmark::algorithm
