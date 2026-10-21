#include "test.hpp"

#include "bruce/lhme/fibonacci_kernel.hpp"
#include "bruce/lhme/power_offset_kernel.hpp"

#include <limits>
#include <string>

namespace {

using bruce::lhme::TechniqueIdentity;
using bruce::lhme::FibonacciKernel;
using bruce::lhme::PowerOffsetKernel;
using bruce::lhme::TechniqueRequest;
using bruce::lhme::encode_power_offset_parameters;
using bruce::lhme::valid_technique_identity;

}  // namespace

TEST(technique_identity_requires_ascii_name_and_positive_version) {
  CHECK(valid_technique_identity({.id = "bruce.power_offset", .version = 1}));
  CHECK(!valid_technique_identity({.id = "", .version = 1}));
  CHECK(!valid_technique_identity({.id = "bruce.power_offset", .version = 0}));
  CHECK(!valid_technique_identity(
      {.id = std::string{"bruce."} + static_cast<char>(0xC3), .version = 1}));
}

TEST(power_offset_parameter_encoding_is_unambiguous) {
  CHECK(encode_power_offset_parameters(1, 23) !=
        encode_power_offset_parameters(12, 3));
  CHECK(encode_power_offset_parameters(2, 100) == "base=1:2;scale=3:100");
}

TEST(power_offset_kernel_delegates_exact_math_to_bruce) {
  const PowerOffsetKernel kernel;
  const auto result = kernel.evaluate({
      .identity = kernel.identity(),
      .parameters = encode_power_offset_parameters(2, 100),
      .address = {.index = 10},
      .mode = bruce::lhme::EvaluationMode::Exact,
      .budget = bruce::lhme::Budget::unlimited()});

  CHECK(result.status == bruce::lhme::Status::Confirmed);
  CHECK(result.exact_integer() == 102300);
}

TEST(power_offset_kernel_rejects_modular_mode_without_a_positive_modulus) {
  const PowerOffsetKernel kernel;
  const auto result = kernel.evaluate({
      .identity = kernel.identity(),
      .parameters = encode_power_offset_parameters(2, 100),
      .address = {.index = 10},
      .mode = bruce::lhme::EvaluationMode::Modular,
      .budget = bruce::lhme::Budget::unlimited()});

  CHECK(result.status == bruce::lhme::Status::InvalidArgument);
  CHECK(result.mode == bruce::lhme::EvaluationMode::Modular);
}

TEST(power_offset_kernel_uses_the_generic_modulus_contract) {
  const PowerOffsetKernel kernel;
  const auto modular = kernel.evaluate({
      .identity = kernel.identity(),
      .parameters = encode_power_offset_parameters(2, 100),
      .address = {.index = 10},
      .mode = bruce::lhme::EvaluationMode::Modular,
      .budget = bruce::lhme::Budget::unlimited(),
      .modulus = 17});
  const auto missing = kernel.evaluate({
      .identity = kernel.identity(),
      .parameters = encode_power_offset_parameters(2, 100),
      .address = {.index = 10},
      .mode = bruce::lhme::EvaluationMode::Modular,
      .budget = bruce::lhme::Budget::unlimited()});
  const auto zero = kernel.evaluate({
      .identity = kernel.identity(),
      .parameters = encode_power_offset_parameters(2, 100),
      .address = {.index = 10},
      .mode = bruce::lhme::EvaluationMode::Modular,
      .budget = bruce::lhme::Budget::unlimited(),
      .modulus = 0});
  const auto negative = kernel.evaluate({
      .identity = kernel.identity(),
      .parameters = encode_power_offset_parameters(2, 100),
      .address = {.index = 10},
      .mode = bruce::lhme::EvaluationMode::Modular,
      .budget = bruce::lhme::Budget::unlimited(),
      .modulus = -17});

  CHECK(modular.status == bruce::lhme::Status::Confirmed);
  CHECK(modular.exact_integer() == 11);
  CHECK(missing.status == bruce::lhme::Status::InvalidArgument);
  CHECK(zero.status == bruce::lhme::Status::InvalidArgument);
  CHECK(negative.status == bruce::lhme::Status::InvalidArgument);
}

TEST(fibonacci_kernel_returns_known_exact_values) {
  const FibonacciKernel kernel;
  const auto exact = [&kernel](std::int64_t index) {
    return kernel.evaluate({
        .identity = kernel.identity(),
        .parameters = bruce::lhme::kFibonacciFastDoublingParameters,
        .address = {.index = index},
        .mode = bruce::lhme::EvaluationMode::Exact,
        .budget = bruce::lhme::Budget::unlimited()});
  };

  CHECK(exact(0).exact_integer() == 0);
  CHECK(exact(1).exact_integer() == 1);
  CHECK(exact(10).exact_integer() == 55);
  CHECK(exact(100).exact_integer() ==
        bruce::lhme::BigInt{"354224848179261915075"});
}

TEST(fibonacci_kernel_keeps_symbolic_and_metric_requests_lazy) {
  const FibonacciKernel kernel;
  const auto symbolic = kernel.evaluate({
      .identity = kernel.identity(),
      .parameters = bruce::lhme::kFibonacciFastDoublingParameters,
      .address = {.index = 10},
      .mode = bruce::lhme::EvaluationMode::Symbolic,
      .budget = bruce::lhme::Budget::unlimited()});
  const auto metric = kernel.evaluate({
      .identity = kernel.identity(),
      .parameters = bruce::lhme::kFibonacciFastDoublingParameters,
      .address = {.index = 10},
      .mode = bruce::lhme::EvaluationMode::Metric,
      .budget = bruce::lhme::Budget::unlimited()});
  const auto million_metric = kernel.evaluate({
      .identity = kernel.identity(),
      .parameters = bruce::lhme::kFibonacciFastDoublingParameters,
      .address = {.index = 1'000'000},
      .mode = bruce::lhme::EvaluationMode::Metric,
      .budget = bruce::lhme::Budget::unlimited()});
  const auto maximum_metric = kernel.evaluate({
      .identity = kernel.identity(),
      .parameters = bruce::lhme::kFibonacciFastDoublingParameters,
      .address = {.index = std::numeric_limits<std::int64_t>::max()},
      .mode = bruce::lhme::EvaluationMode::Metric,
      .budget = bruce::lhme::Budget::unlimited()});

  CHECK(symbolic.status == bruce::lhme::Status::Confirmed);
  CHECK(symbolic.symbolic_expression == "F(10)");
  CHECK(!symbolic.integer.has_value());
  CHECK(metric.status == bruce::lhme::Status::Confirmed);
  CHECK(metric.metric_bit_length == 7);
  CHECK(!metric.integer.has_value());
  CHECK(million_metric.status == bruce::lhme::Status::Confirmed);
  CHECK(million_metric.metric_bit_length == 700000);
  CHECK(!million_metric.integer.has_value());
  CHECK(maximum_metric.status == bruce::lhme::Status::Confirmed);
  CHECK(maximum_metric.metric_bit_length == 6456360425798343065ULL);
  CHECK(!maximum_metric.integer.has_value());
}

TEST(fibonacci_kernel_validates_its_request_contract) {
  const FibonacciKernel kernel;
  const auto invalid_parameters = kernel.evaluate({
      .identity = kernel.identity(),
      .parameters = "algorithm=5:other",
      .address = {.index = 10},
      .mode = bruce::lhme::EvaluationMode::Exact,
      .budget = bruce::lhme::Budget::unlimited()});
  const auto negative_address = kernel.evaluate({
      .identity = kernel.identity(),
      .parameters = bruce::lhme::kFibonacciFastDoublingParameters,
      .address = {.index = -1},
      .mode = bruce::lhme::EvaluationMode::Exact,
      .budget = bruce::lhme::Budget::unlimited()});
  const auto missing_modulus = kernel.evaluate({
      .identity = kernel.identity(),
      .parameters = bruce::lhme::kFibonacciFastDoublingParameters,
      .address = {.index = 10},
      .mode = bruce::lhme::EvaluationMode::Modular,
      .budget = bruce::lhme::Budget::unlimited()});
  const auto zero_modulus = kernel.evaluate({
      .identity = kernel.identity(),
      .parameters = bruce::lhme::kFibonacciFastDoublingParameters,
      .address = {.index = 10},
      .mode = bruce::lhme::EvaluationMode::Modular,
      .budget = bruce::lhme::Budget::unlimited(),
      .modulus = 0});
  const auto negative_modulus = kernel.evaluate({
      .identity = kernel.identity(),
      .parameters = bruce::lhme::kFibonacciFastDoublingParameters,
      .address = {.index = 10},
      .mode = bruce::lhme::EvaluationMode::Modular,
      .budget = bruce::lhme::Budget::unlimited(),
      .modulus = -17});

  CHECK(invalid_parameters.status == bruce::lhme::Status::InvalidArgument);
  CHECK(negative_address.status == bruce::lhme::Status::InvalidArgument);
  CHECK(missing_modulus.status == bruce::lhme::Status::InvalidArgument);
  CHECK(zero_modulus.status == bruce::lhme::Status::InvalidArgument);
  CHECK(negative_modulus.status == bruce::lhme::Status::InvalidArgument);
}

TEST(fibonacci_kernel_uses_modular_fast_doubling) {
  const FibonacciKernel kernel;
  const auto result = kernel.evaluate({
      .identity = kernel.identity(),
      .parameters = bruce::lhme::kFibonacciFastDoublingParameters,
      .address = {.index = 10},
      .mode = bruce::lhme::EvaluationMode::Modular,
      .budget = bruce::lhme::Budget::unlimited(),
      .modulus = 17});

  CHECK(result.status == bruce::lhme::Status::Confirmed);
  CHECK(result.exact_integer() == 4);
}

TEST(fibonacci_kernel_rejects_exact_work_before_materializing_a_value) {
  const FibonacciKernel kernel;
  const auto limited = [&kernel](bruce::lhme::Budget budget) {
    return kernel.evaluate({
        .identity = kernel.identity(),
        .parameters = bruce::lhme::kFibonacciFastDoublingParameters,
        .address = {.index = 100},
        .mode = bruce::lhme::EvaluationMode::Exact,
        .budget = budget});
  };

  const auto bit_limited = limited({.max_bit_length = 69});
  const auto operation_limited = limited({.max_operations = 13});
  const auto temporary_limited = limited({.max_temporary_bytes = 71});

  CHECK(bit_limited.status == bruce::lhme::Status::BudgetExceeded);
  CHECK(operation_limited.status == bruce::lhme::Status::BudgetExceeded);
  CHECK(temporary_limited.status == bruce::lhme::Status::BudgetExceeded);
  CHECK(!bit_limited.integer.has_value());
  CHECK(!operation_limited.integer.has_value());
  CHECK(!temporary_limited.integer.has_value());
}

TEST(fibonacci_kernel_stops_when_its_engine_signal_is_set) {
  const FibonacciKernel kernel;
  const auto signal = std::make_shared<std::atomic_bool>(true);
  const auto result = kernel.evaluate({
      .identity = kernel.identity(),
      .parameters = bruce::lhme::kFibonacciFastDoublingParameters,
      .address = {.index = 100},
      .mode = bruce::lhme::EvaluationMode::Exact,
      .budget = bruce::lhme::Budget::unlimited(),
      .cancellation_signal = signal});

  CHECK(result.status == bruce::lhme::Status::Cancelled);
  CHECK(!result.integer.has_value());
}
