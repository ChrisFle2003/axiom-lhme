#include "test.hpp"

#include "axiom/lhme/reduction.hpp"

namespace {

axiom::lhme::EvaluationRequest request() {
  return {.spec = axiom::lhme::make_power_offset_spec(2, 100, 1),
          .mode = axiom::lhme::EvaluationMode::Exact,
          .budget = axiom::lhme::Budget::unlimited()};
}

}  // namespace

TEST(serial_and_parallel_range_results_are_canonically_equal) {
  const auto serial = axiom::lhme::evaluate_range(
      request(), {0, 20}, {.worker_count = 1, .tile_size = 4});
  const auto parallel = axiom::lhme::evaluate_range(
      request(), {0, 20}, {.worker_count = 4, .tile_size = 4});

  CHECK(serial.summary == parallel.summary);
  CHECK(serial.candidates.size() == parallel.candidates.size());
  CHECK(serial.candidates.back().exact_integer() == 104857500);
}

TEST(branch_failure_does_not_stop_other_tile) {
  const auto result = axiom::lhme::evaluate_range(
      request(), {-1, 5}, {.worker_count = 2, .tile_size = 1});

  CHECK(result.summary.invalid_argument_count == 1);
  CHECK(result.summary.confirmed_count == 6);
}

TEST(cancelled_range_never_confirms_exact_value) {
  auto cancelled = request();
  cancelled.cancellation_requested = true;
  const auto result = axiom::lhme::evaluate_range(
      cancelled, {0, 20}, {.worker_count = 2, .tile_size = 4});

  CHECK(result.status == axiom::lhme::Status::Cancelled);
  CHECK(result.candidates.empty());
}

TEST(range_rejects_work_that_exceeds_the_candidate_budget_before_scheduling) {
  auto limited = request();
  limited.budget.max_candidate_count = 2;

  const auto result = axiom::lhme::evaluate_range(
      limited, {0, 20}, {.worker_count = 2, .tile_size = 4});

  CHECK(result.status == axiom::lhme::Status::BudgetExceeded);
  CHECK(result.candidates.empty());
}

TEST(range_rejects_nonrepresentable_tile_width_without_overflowing) {
  const auto result = axiom::lhme::evaluate_range(
      request(), {0, 0}, {.worker_count = 1,
                          .tile_size = UINT64_MAX});

  CHECK(result.status == axiom::lhme::Status::InvalidArgument);
}
