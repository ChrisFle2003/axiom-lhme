# LHME Streaming Runtime V2 Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Add a deterministic CPU/RAM streaming range API with bounded worker batches, Modular prefiltering, and adaptive candidate retention.

**Architecture:** A new public streaming contract sits beside V1 reduction. Its CPU implementation uses fixed workers for a bounded request, reduces tile results by Tile ID, and retains only Exact candidates selected by a deterministic policy. The existing `evaluate_range` stays unchanged.

**Tech Stack:** C++20, CMake/CTest, Boost.Multiprecision `cpp_int`, standard-library threads and synchronization.

**Spec:** `docs/superpowers/specs/2026-10-20-lhme-streaming-runtime-v2-design.md`

## Global Constraints

- Keep V1 `evaluate_range` and all existing public contracts behaviorally unchanged.
- Expose V2 only through `include/bruce/lhme/streaming.hpp`.
- Stay CPU/RAM-only; do not add GPU, persistence, parsing, networking, or new formula operations.
- Never materialize an unbounded tile queue or full confirmed-candidate list.
- Merge by increasing Tile ID, never completion order.
- A retention count or byte limit of `0` means SummaryOnly.
- Exact runs only after a filter accepts an address.
- Add all V2 tests to `tests/streaming_tests.cpp`.

## Review Focus

- Invalid ModularEquals options must return `InvalidArgument` before scheduling or Exact work; Task 1 tests this.
- A filter miss increments only `filtered_out_count`, not confirmed, invalid, or exact counters; Task 3 tests it.
- One and four workers must produce identical summary and retained candidates; Task 2 tests it.
- An oversized candidate must leave summary correct without exceeding retention memory; Task 4 tests it.
- A shared cancellation signal must fail closed without later retained candidates; Task 2 tests it.

---

## Planned File Structure

```text
CMakeLists.txt
include/bruce/lhme/streaming.hpp       # public V2 types and function
src/cpu/streaming.cpp                  # fixed workers, filter, reducer, retention
tests/streaming_tests.cpp              # V2 test suite
```

## Task 1: Public Contract and Fail-Closed Validation

**Files:**

- Create: `include/bruce/lhme/streaming.hpp`
- Create: `src/cpu/streaming.cpp`
- Create: `tests/streaming_tests.cpp`
- Modify: `CMakeLists.txt`

**Interfaces:**

- Produces `RetentionMode { SummaryOnly, FilteredCandidates, TopK, Adaptive }`.
- Produces `FilterMode { AcceptAll, ModularEquals }` and `CandidateFilter { FilterMode mode; BigInt modulus; BigInt expected_residue; }`.
- Produces `StreamingOptions { worker_count, tile_size, retention_mode, max_retained_candidates, retention_memory_bytes, filter }`.
- Produces `StreamingRangeResult { status, summary, retained_candidates, effective_retention_mode, filtered_out_count, exact_evaluation_count }`.
- Produces `StreamingRangeResult evaluate_streaming_range(const EvaluationRequest&, AddressRange, StreamingOptions)`.

- [ ] **Step 1: Write failing validation tests**

```cpp
TEST(streaming_rejects_non_exact_and_invalid_options_before_exact_work) {
  CHECK(evaluate_streaming_range(metric_request(), {0, 20}, valid_options()).status == Status::InvalidArgument);
  CHECK(evaluate_streaming_range(exact_request(), {0, 20}, {.worker_count = 0}).status == Status::InvalidArgument);
  CHECK(evaluate_streaming_range(exact_request(), {0, 20}, modular_equals(0, 0)).status == Status::InvalidArgument);
}

TEST(zero_retention_limits_force_summary_only) {
  auto result = evaluate_streaming_range(exact_request(), {0, 5}, {.worker_count = 1, .tile_size = 2});
  CHECK(result.status == Status::Confirmed);
  CHECK(result.effective_retention_mode == RetentionMode::SummaryOnly);
  CHECK(result.retained_candidates.empty());
}
```

- [ ] **Step 2: Run the new test target to verify it fails**

Run: `cmake -S . -B build && cmake --build build --target streaming_tests && ctest --test-dir build -R streaming_tests --output-on-failure`

Expected: FAIL because the header, implementation, and CMake target do not exist.

- [ ] **Step 3: Implement public types plus minimal validation**

Add `streaming.hpp`, its CMake source/test registration, and a validating implementation. Require an Exact request, valid range, nonzero worker/tile values, a positive modulus, and normalized residue. Return SummaryOnly with no retained candidates when either retention limit is zero.

- [ ] **Step 4: Run validation tests to verify they pass**

Run: `cmake --build build --target streaming_tests && ctest --test-dir build -R streaming_tests --output-on-failure`

Expected: PASS; invalid calls report `InvalidArgument` before Exact work.

- [ ] **Step 5: Commit the V2 contracts**

```bash
git add CMakeLists.txt include/bruce/lhme/streaming.hpp src/cpu/streaming.cpp tests/streaming_tests.cpp
git commit -m "feat: add LHME streaming contracts"
```

## Task 2: Fixed Workers and Canonical Streaming Reduction

**Files:**

- Modify: `src/cpu/streaming.cpp`
- Modify: `tests/streaming_tests.cpp`

**Interfaces:**

- Consumes Task 1 contracts, `EvaluationRequest`, `evaluate`, `AddressRange`, and `GlobalSummary`.
- Produces confirmed streaming results for AcceptAll with deterministic tile reduction.

- [ ] **Step 1: Write failing worker-determinism and cancellation tests**

```cpp
TEST(streaming_one_and_four_workers_have_same_summary_and_candidates) {
  auto one = evaluate_streaming_range(exact_request(), {0, 20}, filtered_options(1));
  auto four = evaluate_streaming_range(exact_request(), {0, 20}, filtered_options(4));
  CHECK(one.summary == four.summary);
  CHECK(exact_values(one.retained_candidates) == exact_values(four.retained_candidates));
}

TEST(streaming_shared_cancellation_retains_nothing_after_stop) {
  auto signal = std::make_shared<std::atomic_bool>(true);
  auto request = exact_request();
  request.cancellation_signal = signal;
  auto result = evaluate_streaming_range(request, {0, 20}, filtered_options(2));
  CHECK(result.status == Status::Cancelled);
  CHECK(result.retained_candidates.empty());
}
```

- [ ] **Step 2: Run the tests to verify they fail**

Run: `cmake --build build --target streaming_tests && ctest --test-dir build -R streaming_tests --output-on-failure`

Expected: FAIL because Task 1 does not stream or retain results.

- [ ] **Step 3: Implement fixed workers and bounded batch reduction**

Implement an internal pool with exactly `worker_count` threads for one request. Submit only one bounded batch at a time. Each task returns local counts and candidates. Consume every batch by assigned Tile ID. Check cancellation before submission, each address, and retention; a stop clears retained candidates produced after the stop and returns `Cancelled`.

- [ ] **Step 4: Verify deterministic reduction**

Run: `cmake --build build --target streaming_tests && ctest --test-dir build -R streaming_tests --output-on-failure`

Expected: PASS; worker count cannot change summary or retained values.

- [ ] **Step 5: Commit fixed streaming workers**

```bash
git add src/cpu/streaming.cpp tests/streaming_tests.cpp
git commit -m "feat: add deterministic LHME streaming reduction"
```

## Task 3: Modular Prefilter Before Exact

**Files:**

- Modify: `src/cpu/streaming.cpp`
- Modify: `tests/streaming_tests.cpp`

**Interfaces:**

- Consumes Task 2 tile work and `CandidateFilter`.
- Produces exact-work and filter-miss counts.

- [ ] **Step 1: Write a failing ModularEquals test**

```cpp
TEST(modular_prefilter_skips_exact_for_non_matching_addresses) {
  auto result = evaluate_streaming_range(
      exact_request(), {0, 10},
      {.worker_count = 2, .tile_size = 2,
       .retention_mode = RetentionMode::FilteredCandidates,
       .max_retained_candidates = 11, .retention_memory_bytes = 4096,
       .filter = {.mode = FilterMode::ModularEquals, .modulus = 17, .expected_residue = 11}});
  CHECK(result.filtered_out_count == 9);
  CHECK(result.exact_evaluation_count == 2);
  CHECK(result.retained_candidates.size() == 2);
  CHECK(exact_values(result.retained_candidates) == std::vector<BigInt>{300, 102300});
}
```

- [ ] **Step 2: Run the prefilter test to verify it fails**

Run: `cmake --build build --target streaming_tests && ctest --test-dir build -R streaming_tests --output-on-failure`

Expected: FAIL because AcceptAll is the only implemented filter behavior.

- [ ] **Step 3: Implement ModularEquals with the existing Modular evaluator**

Create an internal Modular request per valid address and compare its normalized residue. A miss increments only `filtered_out_count`, carries no Modular payload forward, and never reaches Exact. A hit performs the original Exact request.

- [ ] **Step 4: Verify the prefilter**

Run: `cmake --build build --target streaming_tests && ctest --test-dir build -R streaming_tests --output-on-failure`

Expected: PASS; x=2 and x=10 in 0..10 have remainder 11 modulo 17 and are the only Exact evaluations.

- [ ] **Step 5: Commit the Modular prefilter**

```bash
git add src/cpu/streaming.cpp tests/streaming_tests.cpp
git commit -m "feat: prefilter LHME streaming candidates modularly"
```

## Task 4: Adaptive Retention and Final Verification

**Files:**

- Modify: `src/cpu/streaming.cpp`
- Modify: `tests/streaming_tests.cpp`

**Interfaces:**

- Consumes Task 3's Exact candidates in canonical address order.
- Produces all retention modes with `effective_retention_mode` and bounded bytes.

- [ ] **Step 1: Write failing retention tests**

```cpp
TEST(top_k_retains_lowest_addresses_independent_of_workers) {
  auto one = evaluate_streaming_range(exact_request(), {0, 20}, top_k_options(1, 3));
  auto four = evaluate_streaming_range(exact_request(), {0, 20}, top_k_options(4, 3));
  CHECK(exact_values(one.retained_candidates) == std::vector<BigInt>{0, 100, 300});
  CHECK(exact_values(one.retained_candidates) == exact_values(four.retained_candidates));
}

TEST(adaptive_switches_to_top_k_without_exceeding_limits) {
  auto result = evaluate_streaming_range(exact_request(), {0, 20}, adaptive_options(3));
  CHECK(result.effective_retention_mode == RetentionMode::TopK);
  CHECK(result.retained_candidates.size() == 3);
  CHECK(estimated_retained_bytes(result.retained_candidates) <= adaptive_options(3).retention_memory_bytes);
}

TEST(oversized_candidate_is_summarized_but_not_retained) {
  auto result = evaluate_streaming_range(exact_request(), {20, 20}, filtered_options_with_bytes(1));
  CHECK(result.summary.confirmed_count == 1);
  CHECK(result.retained_candidates.empty());
}
```

- [ ] **Step 2: Run retention tests to verify they fail**

Run: `cmake --build build --target streaming_tests && ctest --test-dir build -R streaming_tests --output-on-failure`

Expected: FAIL because Task 2 retains only the simple initial policy.

- [ ] **Step 3: Implement retention accounting and policies**

Use `sizeof(EvaluationResult)` plus exact decimal digits and trace event text as a conservative candidate estimate. FilteredCandidates retains address-ordered candidates while both limits permit. TopK retains lowest addresses under both limits. Adaptive begins FilteredCandidates and permanently switches to TopK at the first retention rejection. Either zero limit immediately forces SummaryOnly.

- [ ] **Step 4: Run full build, tests, and install verification**

Run: `cmake -S . -B build && cmake --build build && ctest --test-dir build --output-on-failure && cmake --install build --prefix build/install`

Expected: PASS; V1 tests, demos, C API smoke test, and V2 streaming tests all pass.

- [ ] **Step 5: Commit V2 adaptive retention**

```bash
git add CMakeLists.txt include/bruce/lhme/streaming.hpp src/cpu/streaming.cpp tests/streaming_tests.cpp
git commit -m "feat: add adaptive LHME streaming retention"
```

## Plan Self-Review

**Spec coverage:** Task 1 defines validation and contracts; Task 2 supplies bounded workers and canonical reduction; Task 3 applies the Modular-before-Exact rule; Task 4 implements every retention policy and the final verification.

**Type consistency:** The Task 1 names and `evaluate_streaming_range` signature remain unchanged in Tasks 2–4. Existing V1 contracts are consumed, not modified.

**Review focus coverage:** invalid options are Task 1, determinism/cancellation Task 2, filter misses Task 3, and oversized/adaptive retention Task 4.

**Scope:** This plan adds exactly one V2 API path and its tests. V1 range behavior, GPU work, the existing demos, and external Scenario files remain unchanged.
