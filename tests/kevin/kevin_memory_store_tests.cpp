#include "test.hpp"

#include "axiom/lhme/fibonacci_kernel.hpp"
#include "axiom/lhme/kevin_memory_store.hpp"
#include "axiom/lhme/power_offset_kernel.hpp"

#include <atomic>
#include <memory>
#include <thread>
#include <vector>

namespace {
using namespace axiom::lhme;
GridAddress address(std::uint8_t x, std::uint8_t y = 0) {
  const std::vector<GridStep> steps{{.x = x, .y = y}};
  return *GridAddress::from_steps(steps);
}
GridSpec power_spec() {
  return {.identity = {.id = "axiom.grid.power", .version = 1},
          .technique = {.id = "axiom.power_offset", .version = 1},
          .canonical_parameters = encode_power_offset_parameters(2, 100),
          .max_depth = 1};
}
GridSpec fibonacci_spec() {
  return {.identity = {.id = "axiom.grid.fibonacci", .version = 1},
          .technique = {.id = "axiom.fibonacci", .version = 1},
          .canonical_parameters = kFibonacciFastDoublingParameters,
          .max_depth = 1};
}
Engine configured_engine() {
  auto engine = Engine::create({.cache_max_entries = 16, .cache_max_bytes = 4096});
  CHECK(engine.register_technique(std::make_shared<PowerOffsetKernel>()));
  CHECK(engine.register_technique(std::make_shared<FibonacciKernel>()));
  return engine;
}
}

TEST(kevin_store_lazily_dispatches_registered_grids_through_engine) {
  auto engine = configured_engine();
  KevinMemoryStore store{engine};
  const auto power = power_spec();
  const auto fibonacci = fibonacci_spec();
  CHECK(store.register_grid(power));
  CHECK(store.register_grid(fibonacci));

  const auto exact = store.get(power.identity, address(1, 1), EvaluationMode::Exact,
                               Budget::unlimited());
  const auto metric = store.get(fibonacci.identity, address(1, 1), EvaluationMode::Metric,
                                Budget::unlimited());
  const auto modular = store.get(power.identity, address(1, 1), EvaluationMode::Modular,
                                 Budget::unlimited(), BigInt{17});
  CHECK(exact.source == KevinValueSource::Computed);
  CHECK(exact.result.exact_integer() == 102300);
  CHECK(metric.result.metric_bit_length == 7);
  CHECK(modular.result.exact_integer() == 11);
}

TEST(kevin_store_rejects_unknown_grids_and_addresses_beyond_the_grid_depth) {
  auto engine = configured_engine();
  KevinMemoryStore store{engine};
  const auto power = power_spec();
  CHECK(store.register_grid(power));
  const auto unknown = store.get({.id = "missing", .version = 1}, address(2),
                                 EvaluationMode::Exact, Budget::unlimited());
  const std::vector<GridStep> deep_steps{{.x = 1, .y = 1}, {.x = 1, .y = 1}};
  const auto deep = store.get(power.identity, *GridAddress::from_steps(deep_steps),
                              EvaluationMode::Exact, Budget::unlimited());
  CHECK(unknown.result.status == Status::InvalidArgument);
  CHECK(deep.result.status == Status::InvalidArgument);
}

TEST(kevin_store_uses_sparse_overrides_before_formula_evaluation) {
  auto engine = configured_engine();
  KevinMemoryStore store{engine};
  const auto power = power_spec();
  const auto target = address(1, 1);
  CHECK(store.register_grid(power));
  CHECK(store.set_override(power.identity, target, BigInt{-42}) == Status::Confirmed);

  const auto exact = store.get(power.identity, target, EvaluationMode::Exact, Budget::unlimited());
  const auto metric = store.get(power.identity, target, EvaluationMode::Metric, Budget::unlimited());
  const auto modular = store.get(power.identity, target, EvaluationMode::Modular,
                                 Budget::unlimited(), BigInt{17});
  CHECK(exact.source == KevinValueSource::Override);
  CHECK(exact.result.exact_integer() == -42);
  CHECK(metric.result.metric_bit_length == 6);
  CHECK(modular.result.exact_integer() == 9);
  CHECK(store.remove_override(power.identity, target));
  CHECK(store.get(power.identity, target, EvaluationMode::Exact, Budget::unlimited())
            .result.exact_integer() == 102300);
}

TEST(kevin_store_enforces_grid_and_caller_budgets_for_overrides) {
  auto engine = configured_engine();
  KevinMemoryStore store{engine};
  auto power = power_spec();
  power.default_budget.max_bit_length = 6;
  const auto target = address(1, 1);
  CHECK(store.register_grid(power));
  CHECK(store.set_override(power.identity, target, BigInt{64}) == Status::BudgetExceeded);
  CHECK(store.set_override(power.identity, target, BigInt{42}) == Status::Confirmed);
  const auto limited = store.get(power.identity, target, EvaluationMode::Exact,
                                 {.max_bit_length = 5});
  CHECK(limited.result.status == Status::BudgetExceeded);
}

TEST(kevin_store_respects_the_engine_run_gate_for_computed_values) {
  auto engine = configured_engine();
  KevinMemoryStore store{engine};
  const auto power = power_spec();
  CHECK(store.register_grid(power));
  engine.cancel();
  const auto result = store.get(power.identity, address(1, 1),
                                EvaluationMode::Exact, Budget::unlimited());
  CHECK(result.result.status == Status::Cancelled);
  CHECK(!result.result.integer.has_value());
}

TEST(kevin_store_exports_and_replaces_only_logical_snapshot_state) {
  auto engine = configured_engine();
  KevinMemoryStore source{engine};
  const auto power = power_spec();
  const auto target = address(1, 1);
  CHECK(source.register_grid(power));
  CHECK(source.set_override(power.identity, target, BigInt{-42}) == Status::Confirmed);
  const auto snapshot = source.snapshot();
  KevinMemoryStore restored{engine};
  CHECK(restored.replace_snapshot(snapshot) == SnapshotStatus::Confirmed);
  const auto value = restored.get(power.identity, target, EvaluationMode::Exact,
                                  Budget::unlimited());
  CHECK(value.source == KevinValueSource::Override);
  CHECK(value.result.exact_integer() == -42);
}

TEST(kevin_store_keeps_concurrent_exact_reads_correct_and_counts_every_request) {
  auto engine = configured_engine();
  KevinMemoryStore store{engine};
  const auto power = power_spec();
  CHECK(store.register_grid(power));

  constexpr std::uint32_t worker_count = 8;
  constexpr std::uint32_t requests_per_worker = 128;
  std::atomic_bool start = false;
  std::atomic<std::uint32_t> failures = 0;
  std::vector<std::thread> workers;
  workers.reserve(worker_count);
  for (std::uint32_t worker = 0; worker < worker_count; ++worker) {
    workers.emplace_back([&] {
      start.wait(false);
      for (std::uint32_t request = 0; request < requests_per_worker; ++request) {
        const auto value = store.get(power.identity, address(request % 8U),
                                     EvaluationMode::Exact, Budget::unlimited());
        if (value.result.status != Status::Confirmed ||
            value.result.exact_integer() == 0) {
          ++failures;
        }
      }
    });
  }
  start.store(true);
  start.notify_all();
  for (auto& worker : workers) worker.join();

  CHECK(failures.load() == 0);
  CHECK(store.inspect().computed_exact_evaluation_count ==
        static_cast<std::uint64_t>(worker_count) * requests_per_worker);
}
