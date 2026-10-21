# BRUCE Lazy Hierarchical Memory Engine C++20 V1 Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Build a reusable C++20 CPU/RAM library that evaluates versioned
formula addresses lazily, reduces bounded tiles deterministically, and exposes
both C++ and C APIs.

**Architecture:** `bruce_lhme_core` owns immutable contracts and exact
`PowerOffset` evaluation. `bruce_lhme_cpu` schedules bounded address tiles and
merges summaries in canonical tile order. Trace and cache are independent
services; the public C++ facade composes them, while the C ABI wraps the facade
with opaque handles.

**Tech Stack:** C++20, CMake 3.25+, CTest, Boost.Multiprecision `cpp_int`,
standard-library threads, header-only test harness stored in `tests/support/`.

**Spec:** `docs/superpowers/specs/2026-10-20-lazy-hierarchical-memory-engine-cpp20-design.md`

## Global Constraints

- Target C++20; do not require GPU, CUDA, OpenCL, or SIMD libraries.
- Use `boost::multiprecision::cpp_int` for V1 exact integers; do not invent a
  second BigInt implementation.
- Keep `bruce_lhme_core` independent of threads, cache implementation, and
  C ABI code.
- Treat `FormulaSpec` as immutable after registration; include its version and
  canonical hash in every result and cache key.
- Preserve the mode ladder: `Symbolic`, `Metric`, `Modular`, `Exact`.
- Reject an Exact request before BigInt evaluation when the estimated bit
  length exceeds `max_bit_length`.
- Merge reduced data in stable tile-ID order, never completion order.
- Cache only confirmed, verified results and keep cache capacity bounded.
- A branch reject must not stop an unrelated tile; a run cancellation must
  prevent new Exact work and return `Cancelled`.
- Public C functions must be exception-free and use opaque ownership handles.

## Review Focus

- Negative index and malformed `PowerOffset` parameters: reject as
  `InvalidArgument` in Task 1.
- Exact request exactly at versus one bit over its limit: Task 2 proves the
  former succeeds and the latter returns `BudgetExceeded` without value data.
- Parallel completion order differing from tile-ID order: Task 3 compares a
  deliberately delayed multi-worker run to a serial canonical result.
- A cache entry with a mismatched mode or FormulaSpec version: Task 4 verifies
  it is a cache miss, not a reused value.
- Cancellation racing with a queued Exact tile: Task 3 verifies queued work
  becomes `Cancelled` and does not emit a confirmed result.

---

## Planned file structure

```text
CMakeLists.txt
include/bruce/lhme/
├─ types.hpp                 # IDs, status, modes, budgets, result value types
├─ formula.hpp               # immutable FormulaSpec and PowerOffset evaluator
├─ trace.hpp                 # trace event and immutable TraceRecord contracts
├─ reduction.hpp             # summaries and deterministic reducer contracts
├─ cache.hpp                 # bounded verified cache contract
├─ engine.hpp                # public RAII C++ facade
└─ c_api.h                   # stable C ABI declarations
src/
├─ core/formula.cpp
├─ core/trace.cpp
├─ core/reduction.cpp
├─ cpu/scheduler.cpp
├─ cache/result_cache.cpp
├─ engine.cpp
└─ c_api.cpp
tests/
├─ support/test_main.cpp
├─ support/test.hpp
├─ core_formula_tests.cpp
├─ core_budget_trace_tests.cpp
├─ cpu_scheduler_tests.cpp
├─ cache_tests.cpp
├─ engine_tests.cpp
└─ c_api_tests.cpp
```

## Task 1: Build scaffold, core types, and `PowerOffset`

**Files:**
- Create: `CMakeLists.txt`
- Create: `include/bruce/lhme/types.hpp`
- Create: `include/bruce/lhme/formula.hpp`
- Create: `src/core/formula.cpp`
- Create: `tests/support/test_main.cpp`
- Create: `tests/support/test.hpp`
- Create: `tests/core_formula_tests.cpp`

**Interfaces:**
- Produces `enum class EvaluationMode`, `enum class Status`, `struct Budget`,
  `struct FormulaAddress`, `struct FormulaSpec`, and
  `StatusOr<EvaluatedValue> evaluate_power_offset(const FormulaSpec&, const FormulaAddress&, EvaluationMode, const Budget&)`.
- `FormulaSpec` represents `PowerOffset { cpp_int base; cpp_int scale; bool nonnegative_index; std::uint32_t version; std::array<std::byte, 32> specification_hash; }`.
- Later tasks consume the types and evaluator without adding thread or cache
  dependencies to this module.

- [ ] **Step 1: Write failing core formula tests**

```cpp
TEST(power_offset_exact_reference_values) {
  auto spec = power_offset_spec(2, 100, 1);
  CHECK(exact(spec, 0).integer == 0);
  CHECK(exact(spec, 1).integer == 100);
  CHECK(exact(spec, 5).integer == 3100);
  CHECK(exact(spec, 10).integer == 102300);
  CHECK(exact(spec, 20).integer == 104857500);
}

TEST(power_offset_rejects_negative_address) {
  CHECK(evaluate_power_offset(power_offset_spec(2, 100, 1), {-1}, Exact, unlimited()).status == InvalidArgument);
}
```

- [ ] **Step 2: Run the core formula test to verify it fails**

Run: `cmake -S . -B build && cmake --build build --target core_formula_tests && ctest --test-dir build -R core_formula_tests --output-on-failure`

Expected: FAIL because the project and public evaluator do not yet exist.

- [ ] **Step 3: Implement the CMake targets and exact core types**

Create static libraries `bruce_lhme_core`, `bruce_lhme_cpu`,
`bruce_lhme_cache`, and `bruce_lhme`, but initially link tests only against
`bruce_lhme_core`. Make Boost.Multiprecision available as a header dependency.
Define `StatusOr<T>` as an owned status/value pair, with a value present only
when `status == Status::Confirmed`.

- [ ] **Step 4: Implement `evaluate_power_offset`**

Implement direct exact calculation for `scale * (base^index - 1)` using
binary exponentiation and `cpp_int`. Implement `Symbolic`, `Metric`, and
`Modular` return shapes too, but defer budget and trace enforcement to Task 2.
Use a canonical byte encoding of operation kind, base, scale, domain, and
version to populate `specification_hash`.

- [ ] **Step 5: Run core formula tests to verify they pass**

Run: `cmake -S . -B build && cmake --build build --target core_formula_tests && ctest --test-dir build -R core_formula_tests --output-on-failure`

Expected: PASS; all five reference values are exact and the negative address
is rejected.

- [ ] **Step 6: Commit the core formula slice**

```bash
git add CMakeLists.txt include/bruce/lhme/types.hpp include/bruce/lhme/formula.hpp src/core/formula.cpp tests/support/test_main.cpp tests/support/test.hpp tests/core_formula_tests.cpp
git commit -m "feat: add LHME core PowerOffset evaluator"
```

## Task 2: Budgets, gates, structured result, and traces

**Files:**
- Create: `include/bruce/lhme/trace.hpp`
- Create: `src/core/trace.cpp`
- Modify: `include/bruce/lhme/types.hpp`
- Modify: `include/bruce/lhme/formula.hpp`
- Modify: `src/core/formula.cpp`
- Create: `tests/core_budget_trace_tests.cpp`

**Interfaces:**
- Consumes `FormulaSpec`, `FormulaAddress`, `StatusOr`, and evaluator modes.
- Produces `enum class GateScope`, `struct GateEvent`, `struct TraceRecord`,
  `struct EvaluationResult`, and `EvaluationResult evaluate(const EvaluationRequest&)`.
- `EvaluationRequest` contains FormulaSpec reference, FormulaAddress, mode,
  Budget, optional positive modulus, and optional cancellation token.

- [ ] **Step 1: Write failing budget, mode, and gate tests**

```cpp
TEST(exact_bit_limit_is_checked_before_expansion) {
  auto request = exact_request(power_offset_spec(2, 100, 1), 20, {.max_bit_length = 20});
  auto result = evaluate(request);
  CHECK(result.status == Status::BudgetExceeded);
  CHECK_FALSE(result.value.has_value());
}

TEST(metric_and_modular_do_not_claim_exact_value) {
  CHECK(evaluate(metric_request(spec, 10)).mode == EvaluationMode::Metric);
  CHECK(evaluate(modular_request(spec, 10, 17)).mode == EvaluationMode::Modular);
}

TEST(branch_reject_emits_trace_without_run_cancellation) {
  auto result = evaluate(invalid_address_request(spec, -1));
  CHECK(result.status == Status::InvalidArgument);
  CHECK(result.trace.contains(GateScope::Branch));
}
```

- [ ] **Step 2: Run the budget and trace tests to verify they fail**

Run: `cmake --build build --target core_budget_trace_tests && ctest --test-dir build -R core_budget_trace_tests --output-on-failure`

Expected: FAIL because request-level evaluation, trace records, and budget
checks do not exist.

- [ ] **Step 3: Implement `EvaluationRequest`, `EvaluationResult`, and `TraceRecord`**

Represent result payload as a tagged variant of symbolic reference, metric,
modular `cpp_int`, and exact `cpp_int`. Give each trace a stable ID computed
from canonical input and events, not wall-clock time.

- [ ] **Step 4: Enforce pre-expansion budgets and gate semantics**

Estimate `PowerOffset` bit length before exact exponentiation. Return
`BudgetExceeded` with a Metric payload when over the limit. Emit branch-level
trace events for invalid addresses and `Publish` gate events whenever a result
is non-confirmed. Validate that a modular request has a positive modulus.

- [ ] **Step 5: Run budget and trace tests to verify they pass**

Run: `cmake --build build --target core_budget_trace_tests && ctest --test-dir build -R core_budget_trace_tests --output-on-failure`

Expected: PASS; no rejected or over-budget result carries an exact value.

- [ ] **Step 6: Commit budgets, gates, and traces**

```bash
git add include/bruce/lhme/types.hpp include/bruce/lhme/formula.hpp include/bruce/lhme/trace.hpp src/core/formula.cpp src/core/trace.cpp tests/core_budget_trace_tests.cpp
git commit -m "feat: add LHME budgets gates and traces"
```

## Task 3: CPU tiles and deterministic hierarchical reduction

**Files:**
- Create: `include/bruce/lhme/reduction.hpp`
- Create: `src/core/reduction.cpp`
- Create: `src/cpu/scheduler.cpp`
- Modify: `CMakeLists.txt`
- Create: `tests/cpu_scheduler_tests.cpp`

**Interfaces:**
- Consumes `EvaluationRequest`, `EvaluationResult`, Budget, and TraceRecord.
- Produces `struct WorkTile`, `struct LocalSummary`, `struct GroupSummary`,
  `struct GlobalSummary`, and
  `RangeResult evaluate_range(const EvaluationRequest&, AddressRange, CpuExecutionOptions)`.
- `CpuExecutionOptions` contains `worker_count`, `tile_size`, and a test-only
  deterministic delay hook keyed by tile ID.

- [ ] **Step 1: Write failing CPU range tests**

```cpp
TEST(serial_and_parallel_range_results_are_canonically_equal) {
  auto serial = evaluate_range(request, {0, 20}, {.worker_count = 1, .tile_size = 4});
  auto parallel = evaluate_range(request, {0, 20}, {.worker_count = 4, .tile_size = 4});
  CHECK(serial.global_summary == parallel.global_summary);
  CHECK(serial.candidates == parallel.candidates);
}

TEST(branch_failure_does_not_stop_other_tile) {
  auto result = evaluate_range(exact_request(spec), {-1, 5}, {.worker_count = 2, .tile_size = 1});
  CHECK(result.summary.rejections.invalid_argument == 1);
  CHECK(result.summary.confirmed_count == 6);
}

TEST_cancelled_queued_tile_never_confirms_exact_value) {
  CHECK(cancel_before_tile_execution().status == Status::Cancelled);
}
```

- [ ] **Step 2: Run CPU range tests to verify they fail**

Run: `cmake --build build --target cpu_scheduler_tests && ctest --test-dir build -R cpu_scheduler_tests --output-on-failure`

Expected: FAIL because tiles, range execution, and reduction are absent.

- [ ] **Step 3: Implement summary contracts and canonical reducers**

Define `LocalSummary` with checked/confirmed counts, rejection counters,
budget totals, bounded candidate list, and trace root references. Merge
summaries strictly by ascending `tile_id`; order candidate ties by address
then specification hash.

- [ ] **Step 4: Implement `evaluate_range` in the CPU backend**

Partition the requested inclusive range before launching workers. Allow work
stealing internally, but collect tile results in preassigned slots and merge
only after all reachable tiles finish. Check cancellation before every new
address and before Exact evaluation. Record Tile and Run gate events.

- [ ] **Step 5: Run CPU range tests to verify they pass**

Run: `cmake --build build --target cpu_scheduler_tests && ctest --test-dir build -R cpu_scheduler_tests --output-on-failure`

Expected: PASS; delayed completion cannot change canonical results.

- [ ] **Step 6: Commit CPU tiles and reducers**

```bash
git add CMakeLists.txt include/bruce/lhme/reduction.hpp src/core/reduction.cpp src/cpu/scheduler.cpp tests/cpu_scheduler_tests.cpp
git commit -m "feat: add LHME CPU tiles and deterministic reduction"
```

## Task 4: Bounded verified result cache

**Files:**
- Create: `include/bruce/lhme/cache.hpp`
- Create: `src/cache/result_cache.cpp`
- Modify: `CMakeLists.txt`
- Create: `tests/cache_tests.cpp`

**Interfaces:**
- Consumes `EvaluationRequest`, `EvaluationResult`, FormulaSpec hash, and
  algorithm version.
- Produces `struct CacheKey`, `enum class CacheStatus`, and
  `class ResultCache` with `lookup(const CacheKey&)` and
  `admit(const CacheKey&, const EvaluationResult&)`.
- Cache uses configuration `max_entries` and `max_bytes`.

- [ ] **Step 1: Write failing cache tests**

```cpp
TEST(cache_round_trip_preserves_confirmed_result) {
  auto key = CacheKey::for_exact(spec, 10);
  auto original = confirmed_exact_result(spec, 10, 102300);
  cache.admit(key, original);
  auto cached = cache.lookup(key);
  CHECK(cached->value == original.value);
  CHECK(cached->status == original.status);
}

TEST(mode_or_version_mismatch_is_a_cache_miss) {
  cache.admit(CacheKey::for_exact(spec, 10), confirmed_exact_result(spec, 10, 102300));
  CHECK_FALSE(cache.lookup(CacheKey::for_metric(spec, 10)).has_value());
  CHECK_FALSE(cache.lookup(CacheKey::for_exact(spec_with_version(2), 10)).has_value());
}

TEST(cache_never_exceeds_configured_byte_limit) {
  CHECK(cache.statistics().resident_bytes <= 256);
}
```

- [ ] **Step 2: Run cache tests to verify they fail**

Run: `cmake --build build --target cache_tests && ctest --test-dir build -R cache_tests --output-on-failure`

Expected: FAIL because no cache service or cache statistics exist.

- [ ] **Step 3: Implement `CacheKey` and bounded `ResultCache`**

Key by specification hash, address, mode, optional modulus, and algorithm
version. Admit only `Confirmed` results with verified traces. Use a
deterministic eviction comparison: lower verified relevance, then larger byte
cost, then older access sequence, then lexicographic cache key.

- [ ] **Step 4: Implement admission, lookup, and deterministic eviction**

Admit only `Confirmed` results with verified traces. Store an explicit cache
decision field with every copied result. The public evaluation-path integration
is deliberately deferred to Task 5, when `Engine` exists.

- [ ] **Step 5: Run cache tests to verify they pass**

Run: `cmake --build build --target cache_tests && ctest --test-dir build -R cache_tests --output-on-failure`

Expected: PASS; no invalid or over-budget result is admitted.

- [ ] **Step 6: Commit the bounded cache**

```bash
git add CMakeLists.txt include/bruce/lhme/cache.hpp src/cache/result_cache.cpp tests/cache_tests.cpp
git commit -m "feat: add bounded verified LHME result cache"
```

## Task 5: Public C++ facade and C ABI

**Files:**
- Create: `include/bruce/lhme/engine.hpp`
- Create: `src/engine.cpp`
- Create: `include/bruce/lhme/c_api.h`
- Create: `src/c_api.cpp`
- Modify: `CMakeLists.txt`
- Create: `tests/engine_tests.cpp`
- Create: `tests/c_api_tests.cpp`

**Interfaces:**
- Consumes core evaluator, CPU scheduler, trace records, and result cache.
- Produces `class Engine`, `Engine::create`, `register_formula`, `evaluate`,
  `evaluate_range`, `inspect`, `cancel`; and C functions
  `lhme_engine_create`, `lhme_engine_destroy`, `lhme_formula_register`,
  `lhme_evaluate`, `lhme_result_inspect`, `lhme_result_destroy`,
  `lhme_cancel`.

- [ ] **Step 1: Write failing C++ facade and C ABI tests**

```cpp
TEST(cpp_engine_registers_and_evaluates_reference_formula) {
  auto engine = Engine::create({.cache_max_entries = 8, .cache_max_bytes = 4096});
  auto formula = engine.register_formula(power_offset_spec(2, 100, 1));
  CHECK(engine.evaluate(exact_request(formula, 10)).exact_integer() == 102300);
}

TEST(c_api_uses_opaque_handles_and_releases_everything) {
  lhme_engine* engine = lhme_engine_create(&config);
  lhme_result* result = lhme_evaluate(engine, &request);
  CHECK(lhme_result_exact_decimal(result) == "102300");
  lhme_result_destroy(result);
  lhme_engine_destroy(engine);
}
```

- [ ] **Step 2: Run facade and C ABI tests to verify they fail**

Run: `cmake --build build --target engine_tests c_api_tests && ctest --test-dir build -R "engine_tests|c_api_tests" --output-on-failure`

Expected: FAIL because the facade and ABI symbols do not exist.

- [ ] **Step 3: Implement the RAII C++ facade**

`Engine` owns configuration, registered immutable FormulaSpecs, ResultCache,
and CPU backend. Return owned result objects. Do not expose worker, cache, or
raw mutable internal state in public headers. After request validation, the
facade performs cache lookup before CPU evaluation and cache admission only
after a confirmed, verified result; add an explicit cache-decision trace event
on both paths.

- [ ] **Step 4: Implement the exception-free C ABI**

Use opaque `lhme_engine`, `lhme_formula`, and `lhme_result` handles. Wrap every
C entry point in a catch-all boundary that translates failures into documented
status codes. Returned text views remain valid until their result handle is
destroyed.

- [ ] **Step 5: Run all tests and install/export verification**

Run: `cmake -S . -B build && cmake --build build && ctest --test-dir build --output-on-failure && cmake --install build --prefix build/install`

Expected: PASS; installed headers and library support the documented C++ and
C API smoke tests.

- [ ] **Step 6: Commit the public library interfaces**

```bash
git add CMakeLists.txt include/bruce/lhme/engine.hpp include/bruce/lhme/c_api.h src/engine.cpp src/c_api.cpp tests/engine_tests.cpp tests/c_api_tests.cpp
git commit -m "feat: expose LHME C++ and C library APIs"
```

## Plan self-review

**Spec coverage:** Tasks 1–2 implement formulas, modes, budgets, gates, and
traces; Task 3 implements CPU tiles and reductions; Task 4 implements bounded
verified caching; Task 5 implements C++ and C API boundaries. GPU is excluded
as required.

**Type consistency:** `FormulaSpec`, `FormulaAddress`, `EvaluationRequest`,
`EvaluationResult`, `WorkTile`, `LocalSummary`, `RangeResult`, `CacheKey`, and
`Engine` are introduced before later tasks consume them.

**Review-focus coverage:** The five failure classes above are assigned to
Tasks 1–4 with named tests.

**Scope:** Each task produces a separately testable library slice and avoids
adding GPU, persistent storage, generic parsing, or learning features.
