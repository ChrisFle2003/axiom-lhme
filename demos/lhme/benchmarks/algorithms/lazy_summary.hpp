#pragma once

#include "algorithm_result.hpp"

namespace bruce::lhme::benchmark::algorithm {

inline Result lazy_summary(const EvaluationRequest& exact_request,
                           AddressRange range) {
  Result output;
  if (exact_request.mode != EvaluationMode::Exact || range.end < range.start) {
    output.status = Status::InvalidArgument;
    return output;
  }
  for (std::int64_t index = range.start;; ++index) {
    if (cancelled(exact_request)) {
      output.status = Status::Cancelled;
      return output;
    }
    ++output.summary.checked_count;
    auto request = exact_request;
    request.address.index = index;
    ++output.exact_evaluation_count;
    const auto value = evaluate(request);
    if (value.status == Status::Confirmed) {
      ++output.summary.confirmed_count;
    } else if (value.status == Status::InvalidArgument) {
      ++output.summary.invalid_argument_count;
    } else {
      output.status = value.status;
      return output;
    }
    if (index == range.end) break;
  }
  output.status = Status::Confirmed;
  return output;
}

}  // namespace bruce::lhme::benchmark::algorithm
