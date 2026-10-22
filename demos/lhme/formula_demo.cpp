#include "demo_report.hpp"
#include "reference_values.hpp"

#include "axiom/lhme/engine.hpp"

int main() {
  axiom::lhme::demo::Report report;
  report.heading("LHME Formula Modes");
  auto engine = axiom::lhme::Engine::create({});
  const auto formula = engine.register_formula(axiom::lhme::demo::reference_spec());
  const auto request = [&formula](axiom::lhme::EvaluationMode mode) {
    return axiom::lhme::EvaluationRequest{.spec = formula,
                                           .address = {.index = 10},
                                           .mode = mode,
                                           .budget = axiom::lhme::Budget::unlimited()};
  };

  const auto symbolic = engine.evaluate(request(axiom::lhme::EvaluationMode::Symbolic));
  const auto metric = engine.evaluate(request(axiom::lhme::EvaluationMode::Metric));
  auto modular_request = request(axiom::lhme::EvaluationMode::Modular);
  modular_request.modulus = 17;
  const auto modular = engine.evaluate(modular_request);
  const auto exact = engine.evaluate(request(axiom::lhme::EvaluationMode::Exact));
  auto limited_request = request(axiom::lhme::EvaluationMode::Exact);
  limited_request.budget.max_bit_length = 0;
  const auto limited = engine.evaluate(limited_request);

  report.expect_equal("symbolic status", axiom::lhme::Status::Confirmed,
                      symbolic.status);
  report.expect_equal("metric status", axiom::lhme::Status::Confirmed,
                      metric.status);
  report.expect_equal("modular status", axiom::lhme::Status::Confirmed,
                      modular.status);
  report.expect_equal("exact status", axiom::lhme::Status::Confirmed,
                      exact.status);
  report.expect_true("symbolic has no integer", !symbolic.integer.has_value());
  report.expect_true("metric has no integer", !metric.integer.has_value());
  report.expect_true("symbolic expression exists", symbolic.symbolic_expression.has_value());
  report.expect_true("metric bit length exists", metric.metric_bit_length.has_value());
  report.expect_true("modular residue exists", modular.integer.has_value());
  report.expect_true("exact value exists", exact.integer.has_value());
  if (symbolic.symbolic_expression.has_value()) {
    report.expect_equal("symbolic expression", std::string("100*(2^10-1)"),
                        *symbolic.symbolic_expression);
    report.row("symbolic", *symbolic.symbolic_expression);
  }
  if (metric.metric_bit_length.has_value()) {
    report.expect_equal("metric bit length", std::uint64_t(17),
                        *metric.metric_bit_length);
    report.row("metric bit length", std::to_string(*metric.metric_bit_length));
  }
  if (modular.integer.has_value()) {
    report.row("modular residue", modular.exact_integer().convert_to<std::string>());
  }
  if (exact.integer.has_value()) {
    report.row("exact value", exact.exact_integer().convert_to<std::string>());
  }
  report.row("tiny exact budget status", axiom::lhme::demo::status_name(limited.status));
  report.expect_equal("modular residue", axiom::lhme::demo::modulo_17_at_10(),
                      modular.integer.value_or(axiom::lhme::BigInt(0)));
  report.expect_equal("exact value", axiom::lhme::demo::exact_at_10(),
                      exact.integer.value_or(axiom::lhme::BigInt(0)));
  report.expect_equal("tiny exact budget status", axiom::lhme::Status::BudgetExceeded,
                      limited.status);
  report.expect_true("tiny exact budget has no integer", !limited.integer.has_value());
  return report.finish();
}
