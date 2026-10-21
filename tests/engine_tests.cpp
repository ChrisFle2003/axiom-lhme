#include "test.hpp"

#include "bruce/lhme/engine.hpp"
#include "bruce/lhme/fibonacci_kernel.hpp"
#include "bruce/lhme/power_offset_kernel.hpp"
#include "bruce/lhme/trace.hpp"

#include <future>
#include <memory>
#include <stdexcept>
#include <thread>

namespace {

class SignalProbeKernel final : public bruce::lhme::TechniqueKernel {
 public:
  [[nodiscard]] bruce::lhme::TechniqueIdentity identity() const override {
    return {.id = "test.signal_probe", .version = 1};
  }

  [[nodiscard]] bruce::lhme::EvaluationResult evaluate(
      const bruce::lhme::TechniqueRequest& request) const override {
    received_signal = request.cancellation_signal;
    return {.status = request.cancellation_signal != nullptr &&
                      request.cancellation_signal->load()
                  ? bruce::lhme::Status::Cancelled
                  : bruce::lhme::Status::Confirmed,
            .mode = request.mode};
  }

  mutable std::shared_ptr<const std::atomic_bool> received_signal;
};

class OversizedExactKernel final : public bruce::lhme::TechniqueKernel {
 public:
  [[nodiscard]] bruce::lhme::TechniqueIdentity identity() const override {
    return {.id = "test.oversized", .version = 1};
  }

  [[nodiscard]] bruce::lhme::EvaluationResult evaluate(
      const bruce::lhme::TechniqueRequest& request) const override {
    return {.status = bruce::lhme::Status::Confirmed,
            .mode = request.mode,
            .integer = bruce::lhme::BigInt{1024}};
  }
};

class ThrowingIdentityKernel final : public bruce::lhme::TechniqueKernel {
 public:
  [[nodiscard]] bruce::lhme::TechniqueIdentity identity() const override {
    throw std::runtime_error("identity failed");
  }

  [[nodiscard]] bruce::lhme::EvaluationResult evaluate(
      const bruce::lhme::TechniqueRequest&) const override {
    return {};
  }
};

class CancellationTraceKernel final : public bruce::lhme::TechniqueKernel {
 public:
  CancellationTraceKernel()
      : entered_(std::make_shared<std::atomic_bool>(false)) {}

  [[nodiscard]] bruce::lhme::TechniqueIdentity identity() const override {
    return {.id = "test.cancellation_trace", .version = 1};
  }

  [[nodiscard]] bruce::lhme::EvaluationResult evaluate(
      const bruce::lhme::TechniqueRequest& request) const override {
    entered_->store(true);
    while (!request.cancellation_signal->load()) {
      std::this_thread::yield();
    }
    return {.status = bruce::lhme::Status::Cancelled, .mode = request.mode};
  }

  std::shared_ptr<std::atomic_bool> entered_;
};

bruce::lhme::TechniqueRequest power_offset_request(
    bruce::lhme::Budget budget = bruce::lhme::Budget::unlimited()) {
  return {.identity = {.id = "bruce.power_offset", .version = 1},
          .parameters = bruce::lhme::encode_power_offset_parameters(2, 100),
          .address = {.index = 10},
          .mode = bruce::lhme::EvaluationMode::Exact,
          .budget = budget};
}

}  // namespace

TEST(cpp_engine_registers_and_evaluates_reference_formula) {
  auto engine = bruce::lhme::Engine::create(
      {.cache_max_entries = 8, .cache_max_bytes = 4096});
  const auto formula = engine.register_formula(
      bruce::lhme::make_power_offset_spec(2, 100, 1));

  const auto result = engine.evaluate({.spec = formula,
                                       .address = {.index = 10},
                                       .mode = bruce::lhme::EvaluationMode::Exact,
                                       .budget = bruce::lhme::Budget::unlimited()});

  CHECK(result.status == bruce::lhme::Status::Confirmed);
  CHECK(result.exact_integer() == 102300);
}

TEST(cpp_engine_reuses_confirmed_exact_results_from_its_bounded_cache) {
  auto engine = bruce::lhme::Engine::create(
      {.cache_max_entries = 8, .cache_max_bytes = 4096});
  const auto formula = engine.register_formula(
      bruce::lhme::make_power_offset_spec(2, 100, 1));
  const bruce::lhme::EvaluationRequest request{
      .spec = formula,
      .address = {.index = 10},
      .mode = bruce::lhme::EvaluationMode::Exact,
      .budget = bruce::lhme::Budget::unlimited()};

  (void)engine.evaluate(request);
  const auto second = engine.evaluate(request);

  CHECK(second.exact_integer() == 102300);
  CHECK(engine.inspect().resident_entries == 1);
}

TEST(cpp_engine_does_not_confuse_formulae_with_ambiguous_decimal_fields) {
  auto engine = bruce::lhme::Engine::create(
      {.cache_max_entries = 8, .cache_max_bytes = 4096});
  const auto first = engine.register_formula(
      bruce::lhme::make_power_offset_spec(1, 23, 1));
  const auto second = engine.register_formula(
      bruce::lhme::make_power_offset_spec(12, 3, 1));

  CHECK(first.specification_hash != second.specification_hash);
  CHECK(engine.evaluate({.spec = first,
                         .address = {.index = 2},
                         .mode = bruce::lhme::EvaluationMode::Exact,
                         .budget = bruce::lhme::Budget::unlimited()})
            .exact_integer() == 0);
  CHECK(engine.evaluate({.spec = second,
                         .address = {.index = 2},
                         .mode = bruce::lhme::EvaluationMode::Exact,
                         .budget = bruce::lhme::Budget::unlimited()})
            .exact_integer() == 429);
}

TEST(cpp_engine_verifies_formula_identity_even_if_a_hash_is_forced_to_collide) {
  auto engine = bruce::lhme::Engine::create(
      {.cache_max_entries = 8, .cache_max_bytes = 4096});
  const auto first = engine.register_formula(
      bruce::lhme::make_power_offset_spec(2, 100, 1));
  auto colliding = bruce::lhme::make_power_offset_spec(3, 100, 1);
  colliding.specification_hash = first.specification_hash;
  const auto second = engine.register_formula(colliding);

  CHECK(engine.evaluate({.spec = first,
                         .address = {.index = 2},
                         .mode = bruce::lhme::EvaluationMode::Exact,
                         .budget = bruce::lhme::Budget::unlimited()})
            .exact_integer() == 300);
  CHECK(engine.evaluate({.spec = second,
                         .address = {.index = 2},
                         .mode = bruce::lhme::EvaluationMode::Exact,
                         .budget = bruce::lhme::Budget::unlimited()})
            .exact_integer() == 800);
}

TEST(cpp_engine_rechecks_the_callers_budget_before_returning_a_cache_hit) {
  auto engine = bruce::lhme::Engine::create(
      {.cache_max_entries = 8, .cache_max_bytes = 4096});
  const auto formula = engine.register_formula(
      bruce::lhme::make_power_offset_spec(2, 100, 1));
  const bruce::lhme::EvaluationRequest unrestricted{
      .spec = formula,
      .address = {.index = 10},
      .mode = bruce::lhme::EvaluationMode::Exact,
      .budget = bruce::lhme::Budget::unlimited()};
  (void)engine.evaluate(unrestricted);

  auto restricted = unrestricted;
  restricted.budget.max_bit_length = 0;
  const auto bit_limited_result = engine.evaluate(restricted);
  restricted.budget.max_bit_length = UINT64_MAX;
  restricted.budget.max_operations = 0;
  const auto operation_limited_result = engine.evaluate(restricted);

  CHECK(bit_limited_result.status == bruce::lhme::Status::BudgetExceeded);
  CHECK(operation_limited_result.status == bruce::lhme::Status::BudgetExceeded);
  CHECK(!bit_limited_result.integer.has_value());
  CHECK(!operation_limited_result.integer.has_value());
}

TEST(cpp_engine_registry_rejects_duplicate_and_unknown_techniques) {
  auto engine = bruce::lhme::Engine::create({});
  const auto kernel = std::make_shared<bruce::lhme::PowerOffsetKernel>();

  CHECK(engine.register_technique(kernel));
  CHECK(!engine.register_technique(kernel));
  CHECK(engine.evaluate_technique({.identity = {.id = "missing", .version = 1}})
            .status == bruce::lhme::Status::InvalidArgument);
}

TEST(cpp_engine_registry_rejects_a_technique_whose_identity_throws) {
  auto engine = bruce::lhme::Engine::create({});

  CHECK(!engine.register_technique(std::make_shared<ThrowingIdentityKernel>()));
  CHECK(engine.register_technique(std::make_shared<bruce::lhme::PowerOffsetKernel>()));
}

TEST(cpp_engine_technique_cache_rechecks_tighter_exact_budget) {
  auto engine = bruce::lhme::Engine::create(
      {.cache_max_entries = 8, .cache_max_bytes = 4096});
  CHECK(engine.register_technique(
      std::make_shared<bruce::lhme::PowerOffsetKernel>()));

  CHECK(engine.evaluate_technique(power_offset_request()).exact_integer() == 102300);
  CHECK(engine.inspect().resident_entries == 1);

  auto restricted = power_offset_request();
  restricted.budget.max_bit_length = 0;
  const auto result = engine.evaluate_technique(restricted);
  CHECK(result.status == bruce::lhme::Status::BudgetExceeded);
  CHECK(!result.integer.has_value());
}

TEST(cpp_engine_technique_power_offset_uses_the_generic_modulus_contract) {
  auto engine = bruce::lhme::Engine::create({});
  const auto kernel = std::make_shared<bruce::lhme::PowerOffsetKernel>();
  CHECK(engine.register_technique(kernel));

  const auto result = engine.evaluate_technique(
      {.identity = kernel->identity(),
       .parameters = bruce::lhme::encode_power_offset_parameters(2, 100),
       .address = {.index = 10},
       .mode = bruce::lhme::EvaluationMode::Modular,
       .budget = bruce::lhme::Budget::unlimited(),
       .modulus = 17});

  CHECK(result.status == bruce::lhme::Status::Confirmed);
  CHECK(result.exact_integer() == 11);
}

TEST(cpp_engine_caches_exact_fibonacci_but_not_modular_fibonacci) {
  auto engine = bruce::lhme::Engine::create(
      {.cache_max_entries = 8, .cache_max_bytes = 4096});
  const auto kernel = std::make_shared<bruce::lhme::FibonacciKernel>();
  CHECK(engine.register_technique(kernel));

  const bruce::lhme::TechniqueRequest exact{
      .identity = kernel->identity(),
      .parameters = bruce::lhme::kFibonacciFastDoublingParameters,
      .address = {.index = 10},
      .mode = bruce::lhme::EvaluationMode::Exact,
      .budget = bruce::lhme::Budget::unlimited()};
  CHECK(engine.evaluate_technique(exact).exact_integer() == 55);
  CHECK(engine.evaluate_technique(exact).exact_integer() == 55);
  CHECK(engine.inspect().resident_entries == 1);

  auto metric = exact;
  metric.mode = bruce::lhme::EvaluationMode::Metric;
  const auto first_metric = engine.evaluate_technique(metric);
  const auto second_metric = engine.evaluate_technique(metric);
  CHECK(first_metric.status == bruce::lhme::Status::Confirmed);
  CHECK(second_metric.status == bruce::lhme::Status::Confirmed);
  CHECK(second_metric.metric_bit_length == 7);
  CHECK(!second_metric.integer.has_value());
  CHECK(engine.inspect().resident_entries == 2);

  auto modular = exact;
  modular.mode = bruce::lhme::EvaluationMode::Modular;
  modular.modulus = 17;
  const auto modular_result = engine.evaluate_technique(modular);
  CHECK(modular_result.status == bruce::lhme::Status::Confirmed);
  CHECK(modular_result.exact_integer() == 4);
  CHECK(engine.inspect().resident_entries == 2);
}

TEST(cpp_engine_rejects_a_fresh_technique_result_that_exceeds_its_budget) {
  auto engine = bruce::lhme::Engine::create(
      {.cache_max_entries = 8, .cache_max_bytes = 4096});
  CHECK(engine.register_technique(std::make_shared<OversizedExactKernel>()));

  const auto result = engine.evaluate_technique(
      {.identity = {.id = "test.oversized", .version = 1},
       .parameters = "oversized",
       .address = {.index = 1},
       .mode = bruce::lhme::EvaluationMode::Exact,
       .budget = {.max_bit_length = 1}});

  CHECK(result.status == bruce::lhme::Status::BudgetExceeded);
  CHECK(!result.integer.has_value());
  CHECK(engine.inspect().resident_entries == 0);
}

TEST(cpp_engine_replaces_a_technique_callers_cancellation_signal) {
  auto engine = bruce::lhme::Engine::create({});
  const auto kernel = std::make_shared<SignalProbeKernel>();
  const auto caller_signal = std::make_shared<std::atomic_bool>(false);
  CHECK(engine.register_technique(kernel));

  const auto result = engine.evaluate_technique(
      {.identity = kernel->identity(),
       .parameters = "probe",
       .cancellation_signal = caller_signal});
  CHECK(result.status == bruce::lhme::Status::Confirmed);
  CHECK(kernel->received_signal != caller_signal);

  engine.cancel();
  CHECK(engine.evaluate_technique({.identity = kernel->identity(),
                                  .parameters = "probe"})
            .status == bruce::lhme::Status::Cancelled);
}

TEST(cpp_engine_traces_cancellation_that_arrives_during_technique_evaluation) {
  auto engine = bruce::lhme::Engine::create({});
  const auto kernel = std::make_shared<CancellationTraceKernel>();
  CHECK(engine.register_technique(kernel));

  auto pending = std::async(std::launch::async, [&engine, kernel] {
    return engine.evaluate_technique(
        {.identity = kernel->identity(),
         .parameters = "wait",
         .address = {.index = 1},
         .mode = bruce::lhme::EvaluationMode::Metric,
         .budget = bruce::lhme::Budget::unlimited()});
  });
  while (!kernel->entered_->load()) {
    std::this_thread::yield();
  }
  engine.cancel();

  const auto result = pending.get();
  CHECK(result.status == bruce::lhme::Status::Cancelled);
  CHECK(result.trace != nullptr);
  CHECK(result.trace->contains(bruce::lhme::GateScope::Run));
}
