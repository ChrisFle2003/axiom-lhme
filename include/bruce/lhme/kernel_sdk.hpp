#pragma once

#include <memory>
#include <string>
#include <utility>

#include "bruce/lhme/technique.hpp"
#include "bruce/lhme/trace.hpp"

namespace bruce::lhme {

[[nodiscard]] inline bool cancellation_requested(
    const TechniqueRequest& request) noexcept {
  return request.cancellation_signal != nullptr &&
         request.cancellation_signal->load();
}

[[nodiscard]] inline EvaluationResult confirmed_exact_result(
    BigInt value, std::shared_ptr<const TraceRecord> trace) {
  EvaluationResult result;
  result.status = Status::Confirmed;
  result.mode = EvaluationMode::Exact;
  result.integer = std::move(value);
  result.trace = trace != nullptr ? std::move(trace) : make_trace({});
  return result;
}

[[nodiscard]] inline EvaluationResult invalid_technique_request_result(
    EvaluationMode mode, std::string reason) {
  EvaluationResult result;
  result.status = Status::InvalidArgument;
  result.mode = mode;
  result.trace = make_trace({{.scope = GateScope::Branch,
                              .status = result.status,
                              .reason = std::move(reason)}});
  return result;
}

[[nodiscard]] inline EvaluationResult cancelled_technique_result(
    EvaluationMode mode, std::string reason) {
  EvaluationResult result;
  result.status = Status::Cancelled;
  result.mode = mode;
  result.trace = make_trace({{.scope = GateScope::Run,
                              .status = result.status,
                              .reason = std::move(reason)}});
  return result;
}

}  // namespace bruce::lhme
