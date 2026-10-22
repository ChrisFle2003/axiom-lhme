#include "test.hpp"

#include "axiom/lhme/engine.hpp"
#include "axiom/lhme/kernel_sdk.hpp"

#include <atomic>
#include <memory>

namespace {

class AffineKernel final : public axiom::lhme::TechniqueKernel {
 public:
  [[nodiscard]] axiom::lhme::TechniqueIdentity identity() const override {
    return {.id = "example.affine", .version = 1};
  }

  [[nodiscard]] axiom::lhme::EvaluationResult evaluate(
      const axiom::lhme::TechniqueRequest& request) const override {
    received_signal = request.cancellation_signal;
    received_signal_was_set = axiom::lhme::cancellation_requested(request);
    if (received_signal_was_set) {
      return axiom::lhme::cancelled_technique_result(request.mode, "cancelled");
    }
    if (request.identity != identity() ||
        request.parameters != "factor=1:2;offset=1:3" ||
        request.address.index < 0 ||
        request.mode != axiom::lhme::EvaluationMode::Exact) {
      return axiom::lhme::invalid_technique_request_result(request.mode,
                                                            "invalid request");
    }
    return axiom::lhme::confirmed_exact_result(
        axiom::lhme::BigInt{request.address.index} * 2 + 3,
        axiom::lhme::make_trace({}));
  }

  mutable std::shared_ptr<const std::atomic_bool> received_signal;
  mutable bool received_signal_was_set = false;
};

axiom::lhme::TechniqueRequest affine_request(
    axiom::lhme::Budget budget = axiom::lhme::Budget::unlimited()) {
  return {.identity = {.id = "example.affine", .version = 1},
          .parameters = "factor=1:2;offset=1:3",
          .address = {.index = 21},
          .mode = axiom::lhme::EvaluationMode::Exact,
          .budget = budget};
}

}  // namespace

TEST(kernel_sdk_creates_confirmed_exact_results_with_a_base_trace) {
  const auto result = axiom::lhme::confirmed_exact_result(45, nullptr);

  CHECK(result.status == axiom::lhme::Status::Confirmed);
  CHECK(result.mode == axiom::lhme::EvaluationMode::Exact);
  CHECK(result.exact_integer() == 45);
  CHECK(result.trace != nullptr);
  CHECK(result.trace->events.empty());
}

TEST(kernel_sdk_preserves_a_supplied_success_trace) {
  const auto trace = axiom::lhme::make_trace({{
      .scope = axiom::lhme::GateScope::Publish,
      .status = axiom::lhme::Status::Confirmed,
      .reason = "published",
  }});
  const auto result = axiom::lhme::confirmed_exact_result(45, trace);

  CHECK(result.trace == trace);
  CHECK(result.trace->events.size() == 1);
  CHECK(result.trace->events.front().status == axiom::lhme::Status::Confirmed);
  CHECK(result.trace->events.front().reason == "published");
}

TEST(kernel_sdk_creates_traced_invalid_and_cancelled_results) {
  const auto invalid = axiom::lhme::invalid_technique_request_result(
      axiom::lhme::EvaluationMode::Modular, "invalid request");
  const auto cancelled = axiom::lhme::cancelled_technique_result(
      axiom::lhme::EvaluationMode::Metric, "cancelled");

  CHECK(invalid.status == axiom::lhme::Status::InvalidArgument);
  CHECK(invalid.mode == axiom::lhme::EvaluationMode::Modular);
  CHECK(invalid.trace->events.size() == 1);
  CHECK(invalid.trace->events.front().scope == axiom::lhme::GateScope::Branch);
  CHECK(invalid.trace->events.front().status == axiom::lhme::Status::InvalidArgument);
  CHECK(invalid.trace->events.front().reason == "invalid request");
  CHECK(cancelled.status == axiom::lhme::Status::Cancelled);
  CHECK(cancelled.mode == axiom::lhme::EvaluationMode::Metric);
  CHECK(cancelled.trace->events.size() == 1);
  CHECK(cancelled.trace->events.front().scope == axiom::lhme::GateScope::Run);
  CHECK(cancelled.trace->events.front().status == axiom::lhme::Status::Cancelled);
  CHECK(cancelled.trace->events.front().reason == "cancelled");
}

TEST(kernel_sdk_reads_only_a_present_set_cancellation_signal) {
  axiom::lhme::TechniqueRequest request;
  const auto false_signal = std::make_shared<std::atomic_bool>(false);
  const auto true_signal = std::make_shared<std::atomic_bool>(true);

  CHECK(!axiom::lhme::cancellation_requested(request));
  request.cancellation_signal = false_signal;
  CHECK(!axiom::lhme::cancellation_requested(request));
  request.cancellation_signal = true_signal;
  CHECK(axiom::lhme::cancellation_requested(request));
}

TEST(kernel_sdk_affine_kernel_runs_through_engine_contracts) {
  auto engine = axiom::lhme::Engine::create({});
  const auto kernel = std::make_shared<AffineKernel>();
  const auto caller_signal = std::make_shared<std::atomic_bool>(true);
  auto request = affine_request();
  request.cancellation_signal = caller_signal;

  CHECK(engine.register_technique(kernel));
  const auto result = engine.evaluate_technique(request);
  CHECK(result.status == axiom::lhme::Status::Confirmed);
  CHECK(result.exact_integer() == 45);
  CHECK(kernel->received_signal != nullptr);
  CHECK(!kernel->received_signal_was_set);

  const auto directly_cancelled = kernel->evaluate({
      .identity = kernel->identity(),
      .parameters = "factor=1:2;offset=1:3",
      .address = {.index = 21},
      .mode = axiom::lhme::EvaluationMode::Exact,
      .budget = axiom::lhme::Budget::unlimited(),
      .cancellation_signal = caller_signal,
  });
  CHECK(directly_cancelled.status == axiom::lhme::Status::Cancelled);

  auto too_small = affine_request();
  too_small.budget.max_bit_length = 5;
  CHECK(engine.evaluate_technique(too_small).status == axiom::lhme::Status::BudgetExceeded);

  engine.cancel();
  CHECK(engine.evaluate_technique(affine_request()).status == axiom::lhme::Status::Cancelled);
}

TEST(kernel_sdk_affine_kernel_rejects_invalid_requests_through_engine) {
  auto engine = axiom::lhme::Engine::create({});
  const auto kernel = std::make_shared<AffineKernel>();
  CHECK(engine.register_technique(kernel));

  auto invalid_parameters = affine_request();
  invalid_parameters.parameters = "factor=bad";
  auto negative_address = affine_request();
  negative_address.address.index = -1;
  auto unsupported_mode = affine_request();
  unsupported_mode.mode = axiom::lhme::EvaluationMode::Metric;

  const auto check_invalid = [](const axiom::lhme::EvaluationResult& result,
                                axiom::lhme::EvaluationMode mode) {
    CHECK(result.status == axiom::lhme::Status::InvalidArgument);
    CHECK(result.mode == mode);
    CHECK(!result.integer.has_value());
    CHECK(result.trace != nullptr);
    CHECK(result.trace->contains(axiom::lhme::GateScope::Branch));
  };
  check_invalid(engine.evaluate_technique(invalid_parameters),
                axiom::lhme::EvaluationMode::Exact);
  check_invalid(engine.evaluate_technique(negative_address),
                axiom::lhme::EvaluationMode::Exact);
  check_invalid(engine.evaluate_technique(unsupported_mode),
                axiom::lhme::EvaluationMode::Metric);

  auto wrong_identity = affine_request();
  wrong_identity.identity = {.id = "example.other", .version = 1};
  check_invalid(kernel->evaluate(wrong_identity), axiom::lhme::EvaluationMode::Exact);
}
