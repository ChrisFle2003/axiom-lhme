#include "test.hpp"

#include "bruce/lhme/engine.hpp"
#include "bruce/lhme/kernel_sdk.hpp"

#include <atomic>
#include <memory>

namespace {

class AffineKernel final : public bruce::lhme::TechniqueKernel {
 public:
  [[nodiscard]] bruce::lhme::TechniqueIdentity identity() const override {
    return {.id = "example.affine", .version = 1};
  }

  [[nodiscard]] bruce::lhme::EvaluationResult evaluate(
      const bruce::lhme::TechniqueRequest& request) const override {
    received_signal = request.cancellation_signal;
    received_signal_was_set = bruce::lhme::cancellation_requested(request);
    if (received_signal_was_set) {
      return bruce::lhme::cancelled_technique_result(request.mode, "cancelled");
    }
    if (request.identity != identity() ||
        request.parameters != "factor=1:2;offset=1:3" ||
        request.address.index < 0 ||
        request.mode != bruce::lhme::EvaluationMode::Exact) {
      return bruce::lhme::invalid_technique_request_result(request.mode,
                                                            "invalid request");
    }
    return bruce::lhme::confirmed_exact_result(
        bruce::lhme::BigInt{request.address.index} * 2 + 3,
        bruce::lhme::make_trace({}));
  }

  mutable std::shared_ptr<const std::atomic_bool> received_signal;
  mutable bool received_signal_was_set = false;
};

bruce::lhme::TechniqueRequest affine_request(
    bruce::lhme::Budget budget = bruce::lhme::Budget::unlimited()) {
  return {.identity = {.id = "example.affine", .version = 1},
          .parameters = "factor=1:2;offset=1:3",
          .address = {.index = 21},
          .mode = bruce::lhme::EvaluationMode::Exact,
          .budget = budget};
}

}  // namespace

TEST(kernel_sdk_creates_confirmed_exact_results_with_a_base_trace) {
  const auto result = bruce::lhme::confirmed_exact_result(45, nullptr);

  CHECK(result.status == bruce::lhme::Status::Confirmed);
  CHECK(result.mode == bruce::lhme::EvaluationMode::Exact);
  CHECK(result.exact_integer() == 45);
  CHECK(result.trace != nullptr);
  CHECK(result.trace->events.empty());
}

TEST(kernel_sdk_preserves_a_supplied_success_trace) {
  const auto trace = bruce::lhme::make_trace({{
      .scope = bruce::lhme::GateScope::Publish,
      .status = bruce::lhme::Status::Confirmed,
      .reason = "published",
  }});
  const auto result = bruce::lhme::confirmed_exact_result(45, trace);

  CHECK(result.trace == trace);
  CHECK(result.trace->events.size() == 1);
  CHECK(result.trace->events.front().status == bruce::lhme::Status::Confirmed);
  CHECK(result.trace->events.front().reason == "published");
}

TEST(kernel_sdk_creates_traced_invalid_and_cancelled_results) {
  const auto invalid = bruce::lhme::invalid_technique_request_result(
      bruce::lhme::EvaluationMode::Modular, "invalid request");
  const auto cancelled = bruce::lhme::cancelled_technique_result(
      bruce::lhme::EvaluationMode::Metric, "cancelled");

  CHECK(invalid.status == bruce::lhme::Status::InvalidArgument);
  CHECK(invalid.mode == bruce::lhme::EvaluationMode::Modular);
  CHECK(invalid.trace->events.size() == 1);
  CHECK(invalid.trace->events.front().scope == bruce::lhme::GateScope::Branch);
  CHECK(invalid.trace->events.front().status == bruce::lhme::Status::InvalidArgument);
  CHECK(invalid.trace->events.front().reason == "invalid request");
  CHECK(cancelled.status == bruce::lhme::Status::Cancelled);
  CHECK(cancelled.mode == bruce::lhme::EvaluationMode::Metric);
  CHECK(cancelled.trace->events.size() == 1);
  CHECK(cancelled.trace->events.front().scope == bruce::lhme::GateScope::Run);
  CHECK(cancelled.trace->events.front().status == bruce::lhme::Status::Cancelled);
  CHECK(cancelled.trace->events.front().reason == "cancelled");
}

TEST(kernel_sdk_reads_only_a_present_set_cancellation_signal) {
  bruce::lhme::TechniqueRequest request;
  const auto false_signal = std::make_shared<std::atomic_bool>(false);
  const auto true_signal = std::make_shared<std::atomic_bool>(true);

  CHECK(!bruce::lhme::cancellation_requested(request));
  request.cancellation_signal = false_signal;
  CHECK(!bruce::lhme::cancellation_requested(request));
  request.cancellation_signal = true_signal;
  CHECK(bruce::lhme::cancellation_requested(request));
}

TEST(kernel_sdk_affine_kernel_runs_through_engine_contracts) {
  auto engine = bruce::lhme::Engine::create({});
  const auto kernel = std::make_shared<AffineKernel>();
  const auto caller_signal = std::make_shared<std::atomic_bool>(true);
  auto request = affine_request();
  request.cancellation_signal = caller_signal;

  CHECK(engine.register_technique(kernel));
  const auto result = engine.evaluate_technique(request);
  CHECK(result.status == bruce::lhme::Status::Confirmed);
  CHECK(result.exact_integer() == 45);
  CHECK(kernel->received_signal != nullptr);
  CHECK(!kernel->received_signal_was_set);

  const auto directly_cancelled = kernel->evaluate({
      .identity = kernel->identity(),
      .parameters = "factor=1:2;offset=1:3",
      .address = {.index = 21},
      .mode = bruce::lhme::EvaluationMode::Exact,
      .budget = bruce::lhme::Budget::unlimited(),
      .cancellation_signal = caller_signal,
  });
  CHECK(directly_cancelled.status == bruce::lhme::Status::Cancelled);

  auto too_small = affine_request();
  too_small.budget.max_bit_length = 5;
  CHECK(engine.evaluate_technique(too_small).status == bruce::lhme::Status::BudgetExceeded);

  engine.cancel();
  CHECK(engine.evaluate_technique(affine_request()).status == bruce::lhme::Status::Cancelled);
}

TEST(kernel_sdk_affine_kernel_rejects_invalid_requests_through_engine) {
  auto engine = bruce::lhme::Engine::create({});
  const auto kernel = std::make_shared<AffineKernel>();
  CHECK(engine.register_technique(kernel));

  auto invalid_parameters = affine_request();
  invalid_parameters.parameters = "factor=bad";
  auto negative_address = affine_request();
  negative_address.address.index = -1;
  auto unsupported_mode = affine_request();
  unsupported_mode.mode = bruce::lhme::EvaluationMode::Metric;

  const auto check_invalid = [](const bruce::lhme::EvaluationResult& result,
                                bruce::lhme::EvaluationMode mode) {
    CHECK(result.status == bruce::lhme::Status::InvalidArgument);
    CHECK(result.mode == mode);
    CHECK(!result.integer.has_value());
    CHECK(result.trace != nullptr);
    CHECK(result.trace->contains(bruce::lhme::GateScope::Branch));
  };
  check_invalid(engine.evaluate_technique(invalid_parameters),
                bruce::lhme::EvaluationMode::Exact);
  check_invalid(engine.evaluate_technique(negative_address),
                bruce::lhme::EvaluationMode::Exact);
  check_invalid(engine.evaluate_technique(unsupported_mode),
                bruce::lhme::EvaluationMode::Metric);

  auto wrong_identity = affine_request();
  wrong_identity.identity = {.id = "example.other", .version = 1};
  check_invalid(kernel->evaluate(wrong_identity), bruce::lhme::EvaluationMode::Exact);
}
