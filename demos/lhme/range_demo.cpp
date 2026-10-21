#include "demo_report.hpp"
#include "reference_values.hpp"

#include "bruce/lhme/engine.hpp"

namespace {

bool same_candidates(const bruce::lhme::RangeResult& left,
                     const bruce::lhme::RangeResult& right) {
  if (left.status != right.status) return false;
  if (left.candidates.size() != right.candidates.size()) return false;
  for (std::size_t index = 0; index < left.candidates.size(); ++index) {
    const auto& left_candidate = left.candidates[index];
    const auto& right_candidate = right.candidates[index];
    if (left_candidate.status != right_candidate.status ||
        left_candidate.mode != right_candidate.mode ||
        left_candidate.integer != right_candidate.integer ||
        left_candidate.metric_bit_length != right_candidate.metric_bit_length ||
        left_candidate.symbolic_expression != right_candidate.symbolic_expression) {
      return false;
    }
  }
  return true;
}

bruce::lhme::EvaluationRequest exact_request(
    const bruce::lhme::FormulaSpec& formula) {
  return {.spec = formula,
          .mode = bruce::lhme::EvaluationMode::Exact,
          .budget = bruce::lhme::Budget::unlimited()};
}

}  // namespace

int main() {
  bruce::lhme::demo::Report report;
  report.heading("LHME CPU Range Reduction");
  auto serial_engine = bruce::lhme::Engine::create(
      {.worker_count = 1, .tile_size = 4});
  auto parallel_engine = bruce::lhme::Engine::create(
      {.worker_count = 4, .tile_size = 4});
  auto branch_engine = bruce::lhme::Engine::create(
      {.worker_count = 2, .tile_size = 1});
  const auto serial_formula = serial_engine.register_formula(
      bruce::lhme::demo::reference_spec());
  const auto parallel_formula = parallel_engine.register_formula(
      bruce::lhme::demo::reference_spec());
  const auto branch_formula = branch_engine.register_formula(
      bruce::lhme::demo::reference_spec());

  const auto serial = serial_engine.evaluate_range(exact_request(serial_formula), {0, 20});
  const auto parallel = parallel_engine.evaluate_range(exact_request(parallel_formula), {0, 20});
  const auto branch = branch_engine.evaluate_range(exact_request(branch_formula), {-1, 5});

  report.expect_equal("serial status", bruce::lhme::Status::Confirmed, serial.status);
  report.expect_equal("parallel status", bruce::lhme::Status::Confirmed, parallel.status);
  report.expect_equal("branch status", bruce::lhme::Status::Confirmed, branch.status);
  report.row("serial candidates", std::to_string(serial.candidates.size()));
  report.row("parallel candidates", std::to_string(parallel.candidates.size()));
  if (!parallel.candidates.empty() && parallel.candidates.back().integer.has_value()) {
    report.row("parallel last exact",
               parallel.candidates.back().exact_integer().convert_to<std::string>());
  }
  report.row("branch invalid", std::to_string(branch.summary.invalid_argument_count));
  report.row("branch confirmed", std::to_string(branch.summary.confirmed_count));

  report.expect_true("summary equality", serial.summary == parallel.summary);
  report.expect_true("candidate order equality", same_candidates(serial, parallel));
  report.expect_equal("candidate count", std::size_t(21), parallel.candidates.size());
  report.expect_true("last candidate exists", !parallel.candidates.empty());
  if (!parallel.candidates.empty()) {
    report.expect_equal("last candidate status", bruce::lhme::Status::Confirmed,
                        parallel.candidates.back().status);
    report.expect_equal("last exact candidate", bruce::lhme::demo::exact_at_20(),
                        parallel.candidates.back().integer.value_or(bruce::lhme::BigInt(0)));
  }
  report.expect_equal("branch invalid count", std::uint64_t(1),
                      branch.summary.invalid_argument_count);
  report.expect_equal("branch confirmed count", std::uint64_t(6),
                      branch.summary.confirmed_count);
  return report.finish();
}
