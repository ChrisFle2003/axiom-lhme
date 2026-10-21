#include "demo_report.hpp"

#include "bruce/lhme/engine.hpp"
#include "bruce/lhme/fibonacci_kernel.hpp"

#include <memory>

int main() {
  bruce::lhme::demo::Report report;
  report.heading("LHME TechniqueKernel Consumer");

  auto engine = bruce::lhme::Engine::create(
      {.cache_max_entries = 8, .cache_max_bytes = 4096});
  const auto kernel = std::make_shared<bruce::lhme::FibonacciKernel>();
  report.expect_true("register fibonacci", engine.register_technique(kernel));

  const auto request = [&kernel](bruce::lhme::EvaluationMode mode) {
    return bruce::lhme::TechniqueRequest{
        .identity = kernel->identity(),
        .parameters = bruce::lhme::kFibonacciFastDoublingParameters,
        .address = {.index = 10},
        .mode = mode,
        .budget = bruce::lhme::Budget::unlimited()};
  };

  const auto exact = engine.evaluate_technique(
      request(bruce::lhme::EvaluationMode::Exact));
  auto modular_request = request(bruce::lhme::EvaluationMode::Modular);
  modular_request.modulus = 17;
  const auto modular = engine.evaluate_technique(modular_request);
  const auto metric = engine.evaluate_technique(
      request(bruce::lhme::EvaluationMode::Metric));
  auto limited_request = request(bruce::lhme::EvaluationMode::Exact);
  limited_request.budget.max_bit_length = 1;
  const auto limited = engine.evaluate_technique(limited_request);

  report.expect_equal("exact status", bruce::lhme::Status::Confirmed,
                      exact.status);
  report.expect_equal("exact F(10)", bruce::lhme::BigInt{55},
                      exact.integer.value_or(bruce::lhme::BigInt{0}));
  report.expect_equal("modular status", bruce::lhme::Status::Confirmed,
                      modular.status);
  report.expect_equal("modular F(10) mod 17", bruce::lhme::BigInt{4},
                      modular.integer.value_or(bruce::lhme::BigInt{0}));
  report.expect_equal("metric status", bruce::lhme::Status::Confirmed,
                      metric.status);
  report.expect_true("metric has no integer", !metric.integer.has_value());
  report.expect_equal("limited exact status", bruce::lhme::Status::BudgetExceeded,
                      limited.status);
  report.expect_true("limited exact has no integer", !limited.integer.has_value());

  if (exact.integer.has_value()) {
    report.row("exact F(10)", exact.exact_integer().convert_to<std::string>());
  }
  if (metric.metric_bit_length.has_value()) {
    report.row("metric upper bits", std::to_string(*metric.metric_bit_length));
  }
  return report.finish();
}
