# LHME Observable Demos Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Add three deterministic terminal demos outside `tests/` that visibly
demonstrate LHME formula modes, CPU reduction, and lazy memory economics while
also acting as CTest regression checks.

**Architecture:** A small header-only reporting helper gives every demo the
same table-like output and PASS/FAIL exit semantics. Each demo is an isolated
executable under `demos/lhme/` which consumes only the public C++ LHME API.
CMake registers every executable as a separate CTest test.

**Tech Stack:** C++20, CMake/CTest, existing `bruce_lhme_core` static library,
and the C++ standard library only.

**Spec:** `docs/superpowers/specs/2026-10-20-lhme-observable-demos-design.md`

## Global Constraints

- Create all new source files below `demos/lhme/`; do not add demo code to
  `tests/`, `include/`, or `src/`.
- Keep all outputs deterministic: no timings, addresses, thread-completion
  order, random values, or platform-dependent memory claims.
- Use the existing public C++ API (`Engine`, `EvaluationRequest`,
  `evaluate_range`) rather than internal implementation headers.
- A failed invariant must print `FAIL <label>: expected=<value> actual=<value>`
  and return exit code `1`; success must print one final `PASS` line and return
  exit code `0`.
- Use `h(x) = 100(2^x - 1)` with `FormulaSpec` version `1` in all three demos.
- `memory_demo` must not request `EvaluationMode::Exact`.
- Do not add GPU, persistence, networking, new formula operations, or a UI.

## Review Focus

- A changed worker completion order must not make `range_demo` report a
  different candidate order; Task 3 compares serial and parallel candidates.
- An expected Safe-Stop must not be treated as a generic demo failure; Task 2
  checks `BudgetExceeded` plus absent exact payload explicitly.
- `memory_demo` must not silently materialize a `BigInt`; Task 4 asserts that
  both requests use non-Exact modes and that no integer payload exists.
- A demo executable must surface a failed invariant to CTest; Task 1 gives the
  reporter a nonzero exit path and Tasks 2-4 route every assertion through it.
- The claimed formula description must not be confused with process RAM; Task
  4 labels `sizeof` and hypothetical value bytes as distinct quantities.

---

## Planned File Structure

```text
CMakeLists.txt                                      # add the three targets/tests
demos/lhme/
├─ support/
│  ├─ demo_report.hpp                               # deterministic terminal reporter
│  └─ reference_values.hpp                          # formula and exact expected values
├─ formula_demo.cpp                                 # modes and bit-budget safe stop
├─ range_demo.cpp                                   # serial/parallel range comparison
└─ memory_demo.cpp                                  # lazy description versus value size
```

## Task 1: Shared Demo Reporter and CMake Registration Pattern

**Files:**
- Create: `demos/lhme/support/demo_report.hpp`
- Create: `demos/lhme/support/reference_values.hpp`
- Modify: `CMakeLists.txt`

**Interfaces:**
- Produces `bruce::lhme::demo::Report` with
  `void heading(std::string_view)`, `void row(std::string_view, std::string_view)`,
  `void expect_equal(std::string_view, const T&, const T&)`,
  `void expect_true(std::string_view, bool)`, and `int finish()`.
- `finish()` prints `PASS` and returns `0` when no expectation failed; otherwise
  prints `FAILURES=<count>` and returns `1`.
- `reference_values.hpp` provides `reference_spec()`, `exact_at_10()`,
  `exact_at_20()`, `modulo_17_at_10()`, `large_metric_index()`, and
  `large_metric_minimum_bytes()`.

- [ ] **Step 1: Write a small reporter probe in `demos/lhme/formula_demo.cpp`**

```cpp
bruce::lhme::demo::Report report;
report.heading("LHME Formula Modes");
report.expect_equal("reporter equality", 1, 1);
return report.finish();
```

- [ ] **Step 2: Add `lhme_formula_demo` to CMake and run it before the helper exists**

Run: `cmake -S . -B build && cmake --build build --target lhme_formula_demo`

Expected: FAIL because `demos/lhme/support/demo_report.hpp` does not exist.

- [ ] **Step 3: Implement the header-only report and reference contracts**

Use `std::ostream` output, an internal failure counter, and a simple stable
`label | value` row format. `reference_spec()` must return
`make_power_offset_spec(2, 100, 1)`. Define the reference values as named
constants: `102300`, `104857500`, `11`, index `1'000'000`, and minimum bytes
`125001`.

- [ ] **Step 4: Register the first executable as a CTest test**

Add an executable target that links `bruce_lhme_core`, includes `include` and
`demos/lhme/support`, and register `add_test(NAME lhme_formula_demo COMMAND lhme_formula_demo)`.

- [ ] **Step 5: Run the probe to verify report formatting and CTest wiring**

Run: `cmake -S . -B build && cmake --build build --target lhme_formula_demo && ctest --test-dir build -R lhme_formula_demo --output-on-failure`

Expected: PASS; the program manually prints its heading and a final `PASS`.

- [ ] **Step 6: Commit the shared demo foundation**

```bash
git add CMakeLists.txt demos/lhme/support demos/lhme/formula_demo.cpp
git commit -m "feat: add LHME demo reporting foundation"
```

## Task 2: Formula Modes and Safe-Stop Demo

**Files:**
- Modify: `demos/lhme/formula_demo.cpp`
- Modify: `demos/lhme/support/reference_values.hpp`

**Interfaces:**
- Consumes `demo::Report`, `reference_spec()`, and `Engine::evaluate`.
- Produces the `lhme_formula_demo` end-to-end report.

- [ ] **Step 1: Add failing mode assertions to `formula_demo.cpp`**

```cpp
report.expect_true("symbolic has no integer", !symbolic.integer.has_value());
report.expect_true("metric has no integer", !metric.integer.has_value());
report.expect_equal("modular residue", modular.exact_integer(), BigInt(11));
report.expect_equal("exact value", exact.exact_integer(), BigInt(102300));
report.expect_equal("tiny exact budget status", limited.status, Status::BudgetExceeded);
report.expect_true("tiny exact budget has no integer", !limited.integer.has_value());
```

- [ ] **Step 2: Run the demo to verify it fails before mode requests are implemented**

Run: `cmake --build build --target lhme_formula_demo && ctest --test-dir build -R lhme_formula_demo --output-on-failure`

Expected: FAIL because the probe does not evaluate all requested modes.

- [ ] **Step 3: Implement four same-address requests and one restricted Exact request**

Create an `Engine`, register `reference_spec()`, and evaluate `x=10` in
`Symbolic`, `Metric`, `Modular` with modulus `17`, and `Exact`. Evaluate a
second Exact request whose `max_bit_length` is `0`. Print a row for each mode
with its status and value or metric, then route all invariants through Report.

- [ ] **Step 4: Run the formula demo manually and under CTest**

Run: `cmake --build build --target lhme_formula_demo && build/lhme_formula_demo && ctest --test-dir build -R lhme_formula_demo --output-on-failure`

Expected: terminal report ends in `PASS`; CTest passes.

- [ ] **Step 5: Commit the formula-mode demo**

```bash
git add demos/lhme/formula_demo.cpp demos/lhme/support/reference_values.hpp
git commit -m "feat: demonstrate LHME formula modes and safe stop"
```

## Task 3: Deterministic CPU Range Demo

**Files:**
- Create: `demos/lhme/range_demo.cpp`
- Modify: `CMakeLists.txt`

**Interfaces:**
- Consumes `demo::Report`, `reference_spec()`, `exact_at_20()`, and
  `Engine::evaluate_range`.
- Produces the `lhme_range_demo` executable and CTest registration.

- [ ] **Step 1: Write the range comparison assertions**

```cpp
report.expect_equal("summary equality", serial.summary, parallel.summary);
report.expect_equal("candidate count", serial.candidates.size(), parallel.candidates.size());
report.expect_equal("last exact candidate", parallel.candidates.back().exact_integer(), BigInt(104857500));
report.expect_equal("branch invalid count", branch.summary.invalid_argument_count, 1U);
report.expect_equal("branch confirmed count", branch.summary.confirmed_count, 6U);
```

- [ ] **Step 2: Add the target and run it before range evaluation is implemented**

Run: `cmake -S . -B build && cmake --build build --target lhme_range_demo && ctest --test-dir build -R lhme_range_demo --output-on-failure`

Expected: FAIL because `range_demo.cpp` initially contains no range result logic.

- [ ] **Step 3: Implement serial, parallel, and local-branch runs**

Register the reference formula once. Use exact requests over `0..20` with
`worker_count=1, tile_size=4` and `worker_count=4, tile_size=4`. Run `-1..5`
with two workers and tile size one. Print each run's counts and the final
candidate value. Compare candidate values in their returned order, not future
completion order.

- [ ] **Step 4: Register and run the demo**

Add `lhme_range_demo`, link it to `bruce_lhme_core`, and register it with
CTest. Then run:

Run: `cmake -S . -B build && cmake --build build --target lhme_range_demo && build/lhme_range_demo && ctest --test-dir build -R lhme_range_demo --output-on-failure`

Expected: serial and parallel summaries/candidates agree; local invalid branch
does not suppress the six valid results; CTest passes.

- [ ] **Step 5: Commit the range demo**

```bash
git add CMakeLists.txt demos/lhme/range_demo.cpp
git commit -m "feat: demonstrate deterministic LHME CPU reduction"
```

## Task 4: Lazy Memory Economics Demo and Full Verification

**Files:**
- Create: `demos/lhme/memory_demo.cpp`
- Modify: `CMakeLists.txt`

**Interfaces:**
- Consumes `demo::Report`, `reference_spec()`, `large_metric_index()`, and
  `large_metric_minimum_bytes()`.
- Produces the `lhme_memory_demo` executable and CTest registration.

- [ ] **Step 1: Write memory-demo invariants before evaluation code**

```cpp
report.expect_true("symbolic avoids exact integer", !symbolic.integer.has_value());
report.expect_true("metric avoids exact integer", !metric.integer.has_value());
report.expect_equal("metric bit length", *metric.metric_bit_length, 1'000'007U);
report.expect_equal("hypothetical minimum bytes", minimum_bytes, 125001U);
```

- [ ] **Step 2: Add the target and run it to verify it fails**

Run: `cmake -S . -B build && cmake --build build --target lhme_memory_demo && ctest --test-dir build -R lhme_memory_demo --output-on-failure`

Expected: FAIL because `memory_demo.cpp` has not yet made the Symbolic and
Metric calls or computed its derived byte count.

- [ ] **Step 3: Implement non-materializing Symbolic and Metric reporting**

Register the reference formula and request only `Symbolic` and `Metric` for
index `1'000'000`. Print the formula address, `sizeof(FormulaSpec)`,
`sizeof(FormulaAddress)`, metric bit length, and
`(metric_bits + 7) / 8` labelled as *hypothetical exact-value minimum bytes*.
Do not create any Exact request in this file.

- [ ] **Step 4: Register the target and verify every demo plus the library suite**

Add and register `lhme_memory_demo`. Then run:

Run: `cmake -S . -B build && cmake --build build && build/lhme_formula_demo && build/lhme_range_demo && build/lhme_memory_demo && ctest --test-dir build --output-on-failure && cmake --install build --prefix build/install`

Expected: all three reports end in `PASS`; full CTest includes the three
demos and the existing library tests with zero failures; install succeeds.

- [ ] **Step 5: Commit the observable demo suite**

```bash
git add CMakeLists.txt demos/lhme
git commit -m "feat: add observable LHME demo suite"
```

## Plan Self-Review

**Spec coverage:** Task 1 establishes the separate folder, deterministic
reporting, and CTest convention. Task 2 covers all four modes plus the expected
budget safe stop. Task 3 covers deterministic CPU reduction and local branch
rejection. Task 4 covers formula/address versus a hypothetical huge value,
never requests Exact, and verifies all demos in CTest.

**Type consistency:** Every demo uses existing `FormulaSpec`, `FormulaAddress`,
`Engine`, `EvaluationRequest`, `EvaluationResult`, `RangeResult`, and
`EvaluationMode` contracts. Task 1 defines the `Report` and reference helpers
before Tasks 2-4 consume them.

**Review focus coverage:** The serial/parallel comparison is Task 3; expected
Safe-Stop and non-Exact mode payload checks are Task 2; non-materialization and
proper memory labels are Task 4; reporter exit behavior is Task 1.

**Scope:** The plan adds only demo executables and CMake registration. It does
not alter library behavior or public LHME interfaces.
