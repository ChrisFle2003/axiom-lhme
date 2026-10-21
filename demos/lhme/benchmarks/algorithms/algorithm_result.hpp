#pragma once

#include <cstdint>
#include <utility>
#include <vector>

#include "bruce/lhme/streaming.hpp"

namespace bruce::lhme::benchmark::algorithm {

struct Result {
  Status status = Status::InternalError;
  GlobalSummary summary;
  std::uint64_t filtered_out_count = 0;
  std::uint64_t exact_evaluation_count = 0;
  std::vector<EvaluationResult> retained_candidates;
};

inline bool cancelled(const EvaluationRequest& request) {
  return request.cancellation_requested ||
         (request.cancellation_signal != nullptr &&
          request.cancellation_signal->load());
}

}  // namespace bruce::lhme::benchmark::algorithm
