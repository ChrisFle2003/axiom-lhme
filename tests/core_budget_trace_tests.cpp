#include "test.hpp"

#include "bruce/lhme/formula.hpp"
#include "bruce/lhme/trace.hpp"

#include <atomic>
#include <memory>

namespace {

bruce::lhme::FormulaSpec reference_spec() {
  return bruce::lhme::make_power_offset_spec(2, 100, 1);
}

bruce::lhme::EvaluationRequest request(bruce::lhme::EvaluationMode mode,
                                       std::int64_t index) {
  return {.spec = reference_spec(),
          .address = {.index = index},
          .mode = mode,
          .budget = bruce::lhme::Budget::unlimited()};
}

}  // namespace

TEST(exact_bit_limit_is_checked_before_expansion) {
  auto limited = request(bruce::lhme::EvaluationMode::Exact, 20);
  limited.budget.max_bit_length = 20;

  const auto result = bruce::lhme::evaluate(limited);

  CHECK(result.status == bruce::lhme::Status::BudgetExceeded);
  CHECK(!result.integer.has_value());
}

TEST(metric_and_modular_do_not_claim_exact_value) {
  const auto metric = bruce::lhme::evaluate(
      request(bruce::lhme::EvaluationMode::Metric, 10));
  auto modular_request = request(bruce::lhme::EvaluationMode::Modular, 10);
  modular_request.modulus = 17;
  const auto modular = bruce::lhme::evaluate(modular_request);

  CHECK(metric.status == bruce::lhme::Status::Confirmed);
  CHECK(metric.metric_bit_length.has_value());
  CHECK(!metric.integer.has_value());
  CHECK(modular.status == bruce::lhme::Status::Confirmed);
  CHECK(modular.integer == 11);
}

TEST(branch_reject_emits_trace_without_run_cancellation) {
  const auto result = bruce::lhme::evaluate(
      request(bruce::lhme::EvaluationMode::Exact, -1));

  CHECK(result.status == bruce::lhme::Status::InvalidArgument);
  CHECK(result.trace != nullptr);
  CHECK(result.trace->contains(bruce::lhme::GateScope::Branch));
  CHECK(!result.trace->contains(bruce::lhme::GateScope::Run));
}

TEST(request_cancellation_prevents_exact_expansion) {
  auto cancelled = request(bruce::lhme::EvaluationMode::Exact, 20);
  cancelled.cancellation_requested = true;

  const auto result = bruce::lhme::evaluate(cancelled);

  CHECK(result.status == bruce::lhme::Status::Cancelled);
  CHECK(!result.integer.has_value());
  CHECK(result.trace->contains(bruce::lhme::GateScope::Run));
}

TEST(constant_zero_power_offset_fits_a_zero_bit_budget) {
  const auto result = bruce::lhme::evaluate({
      .spec = bruce::lhme::make_power_offset_spec(1, 100, 1),
      .address = {.index = 1000},
      .mode = bruce::lhme::EvaluationMode::Exact,
      .budget = {.max_bit_length = 0}});

  CHECK(result.status == bruce::lhme::Status::Confirmed);
  CHECK(result.exact_integer() == 0);
}

TEST(modular_evaluation_validates_the_formula_before_calculation) {
  auto invalid = request(bruce::lhme::EvaluationMode::Modular, 10);
  invalid.spec.base = 0;
  invalid.modulus = 17;

  const auto result = bruce::lhme::evaluate(invalid);

  CHECK(result.status == bruce::lhme::Status::InvalidArgument);
}

TEST(shared_cancellation_signal_prevents_exact_expansion) {
  auto signal = std::make_shared<std::atomic_bool>(true);
  auto cancelled = request(bruce::lhme::EvaluationMode::Exact, 20);
  cancelled.cancellation_signal = signal;

  const auto result = bruce::lhme::evaluate(cancelled);

  CHECK(result.status == bruce::lhme::Status::Cancelled);
  CHECK(!result.integer.has_value());
}

TEST(exact_expansion_respects_operation_and_temporary_memory_budgets) {
  auto operation_limited = request(bruce::lhme::EvaluationMode::Exact, 20);
  operation_limited.budget.max_operations = 0;
  auto memory_limited = request(bruce::lhme::EvaluationMode::Exact, 20);
  memory_limited.budget.max_temporary_bytes = 0;

  const auto operation_result = bruce::lhme::evaluate(operation_limited);
  const auto memory_result = bruce::lhme::evaluate(memory_limited);

  CHECK(operation_result.status == bruce::lhme::Status::BudgetExceeded);
  CHECK(memory_result.status == bruce::lhme::Status::BudgetExceeded);
  CHECK(!operation_result.integer.has_value());
  CHECK(!memory_result.integer.has_value());
}

TEST(trace_budget_can_stop_evaluation_before_trace_allocation) {
  auto trace_limited = request(bruce::lhme::EvaluationMode::Exact, 10);
  trace_limited.budget.max_trace_bytes = 0;

  const auto result = bruce::lhme::evaluate(trace_limited);

  CHECK(result.status == bruce::lhme::Status::BudgetExceeded);
  CHECK(result.trace == nullptr);
}
