#include "demo_report.hpp"
#include "reference_values.hpp"

#include "bruce/lhme/engine.hpp"

int main() {
  bruce::lhme::demo::Report report;
  report.heading("LHME Lazy Memory Economics");
  auto engine = bruce::lhme::Engine::create({});
  const auto formula = engine.register_formula(bruce::lhme::demo::reference_spec());
  const auto request = [&formula](bruce::lhme::EvaluationMode mode) {
    return bruce::lhme::EvaluationRequest{
        .spec = formula,
        .address = {.index = bruce::lhme::demo::large_metric_index()},
        .mode = mode,
        .budget = bruce::lhme::Budget::unlimited()};
  };
  const auto symbolic = engine.evaluate(request(bruce::lhme::EvaluationMode::Symbolic));
  const auto metric = engine.evaluate(request(bruce::lhme::EvaluationMode::Metric));
  report.row("formula address", std::to_string(bruce::lhme::demo::large_metric_index()));
  report.row("stored description", "FormulaSpec + FormulaAddress");

  report.expect_equal("symbolic status", bruce::lhme::Status::Confirmed,
                      symbolic.status);
  report.expect_equal("metric status", bruce::lhme::Status::Confirmed,
                      metric.status);
  report.expect_true("symbolic avoids exact integer", !symbolic.integer.has_value());
  report.expect_true("metric avoids exact integer", !metric.integer.has_value());
  report.expect_true("symbolic expression exists", symbolic.symbolic_expression.has_value());
  report.expect_true("metric bit length exists", metric.metric_bit_length.has_value());
  if (metric.metric_bit_length.has_value()) {
    const auto metric_bits = *metric.metric_bit_length;
    const auto minimum_bytes = (metric_bits + 7U) / 8U;
    report.row("metric bit length", std::to_string(metric_bits));
    report.row("hypothetical exact-value minimum bytes", std::to_string(minimum_bytes));
    report.expect_equal("metric bit length",
                        bruce::lhme::demo::large_metric_bit_length(), metric_bits);
    report.expect_equal("hypothetical minimum bytes",
                        bruce::lhme::demo::large_metric_minimum_bytes(), minimum_bytes);
  }
  return report.finish();
}
