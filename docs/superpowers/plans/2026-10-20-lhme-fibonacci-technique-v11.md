# LHME Fibonacci Technique V1.1 Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Add a lazy, deterministic Fibonacci TechniqueKernel with a generic
modulus contract and a public-header consumer demo.

**Architecture:** Extend `TechniqueRequest` with an optional modulus and have
the existing PowerOffset kernel forward it to FormulaSpec evaluation. Add an
independent Fibonacci fast-doubling kernel that owns only domain computation;
Engine continues to own registration, cancellation, result publication and
Exact/Metric cache behavior. Build a demo solely against public headers to
prove ordinary C++ hosts can consume the library.

**Tech Stack:** C++20, Boost.Multiprecision `cpp_int`, CMake/CTest, existing
BRUCE Engine/Trace/ResultCache.

**Spec:** `docs/superpowers/specs/2026-10-20-lhme-fibonacci-technique-v11-design.md`

## Global Constraints

- V1.1 remains a CPU/RAM, compile-time C++ library; no GPU, dynamic-library,
  foreign-language ABI or C API work.
- Preserve FormulaSpec, EvaluationRequest, streaming and C API behavior.
- Modular results are not cacheable; Exact and Metric retain existing cache
  behavior.
- Fibonacci identity is exactly `bruce.fibonacci`, version `1`; its sole
  valid parameter string is `algorithm=8:fast_dbl`.
- Fibonacci addresses are nonnegative; Modular requires a positive modulus.
- Exact Fibonacci work uses iterative Fast-Doubling and Engine cancellation.
- The repository has no Git metadata; record Red/Green evidence in the SDD
  ledger rather than creating commits.

## Review Focus

- A missing, zero or negative modulus must fail closed, never silently select a
  default modulus; Task 1 and Task 2 test this.
- A noncanonical Fibonacci parameter string or negative address must not begin
  computation; Task 2 tests this.
- A huge Metric address must return a bounded, saturating size estimate without
  an integer; Task 2 tests this.
- Exact Fibonacci must stop before publication when bit, work, temporary-memory
  or cancellation controls reject it; Task 2 tests each path.
- Modular requests must not reuse Exact/Metric cache entries; Task 2 tests the
  cache remains unchanged after a Modular call.

## Planned File Structure

```text
include/bruce/lhme/technique.hpp          add optional generic modulus
include/bruce/lhme/power_offset_kernel.hpp unchanged public class declaration
include/bruce/lhme/fibonacci_kernel.hpp   public Fibonacci identity/parameter/kernel API
src/core/power_offset_kernel.cpp          forward Modular modulus to FormulaSpec
src/core/fibonacci_kernel.cpp              Fast-Doubling and budget/cancellation logic
tests/technique_tests.cpp                  direct kernel contract and math tests
tests/engine_tests.cpp                     Engine request, cache and registry integration
demos/lhme/technique_demo.cpp              public-header consuming executable
CMakeLists.txt                             source, demo target and CTest registration
docs/superpowers/specs/...                 approved design
.superpowers/sdd/.../progress.md           Red/Green and final verification ledger
```

### Task 1: Generic Modulus Contract and PowerOffset Modular Support

**Files:**
- Modify: `include/bruce/lhme/technique.hpp`
- Modify: `src/core/power_offset_kernel.cpp`
- Modify: `tests/technique_tests.cpp`
- Modify: `tests/engine_tests.cpp`

**Interfaces:**
- Produces `TechniqueRequest::modulus` as
  `std::optional<BigInt>` immediately after `budget`.
- Produces PowerOffset Modular forwarding to `EvaluationRequest::modulus`.
- Consumes existing `EvaluationRequest` modular semantics: `h(10) mod 17 == 11`.

- [ ] **Step 1: Write failing modular contract tests**

  Add direct PowerOffset tests asserting a request with modulus `17` confirms
  residue `11`, while missing, zero and negative moduli return
  `Status::InvalidArgument`. Add an Engine integration test for the confirmed
  positive-modulus call.

- [ ] **Step 2: Verify RED**

  Run: `cmake --build build --target technique_tests engine_tests && ctest --test-dir build -R 'technique_tests|engine_tests' --output-on-failure`

  Expected: the new positive-modulus test fails because TechniqueRequest has no
  modulus and PowerOffset currently returns `UnsupportedMode`.

- [ ] **Step 3: Add the optional modulus and forward it**

  Add `std::optional<BigInt> modulus` to `TechniqueRequest`. In
  `PowerOffsetKernel::evaluate`, pass it into the delegated `EvaluationRequest`.
  Keep malformed identity, parameters and address validation before delegation;
  let the existing formula evaluator produce the Modular validation status.

- [ ] **Step 4: Verify GREEN and record it**

  Re-run the Step 2 command. It must pass, including the new invalid-modulus
  cases. Record the exact Red/Green commands in the SDD ledger.

### Task 2: FibonacciKernel Domain Computation

**Files:**
- Create: `include/bruce/lhme/fibonacci_kernel.hpp`
- Create: `src/core/fibonacci_kernel.cpp`
- Modify: `tests/technique_tests.cpp`
- Modify: `tests/engine_tests.cpp`
- Modify: `CMakeLists.txt`

**Interfaces:**
- Produces `inline constexpr char kFibonacciFastDoublingParameters[] = "algorithm=8:fast_dbl"`.
- Produces `class FibonacciKernel final : public TechniqueKernel` with
  `identity() -> {"bruce.fibonacci", 1}` and
  `evaluate(const TechniqueRequest&) -> EvaluationResult`.
- Consumes `TechniqueRequest::modulus`, `Budget`, `FormulaAddress` and the
  Engine-provided cancellation signal from Task 1.

- [ ] **Step 1: Write failing Fibonacci kernel tests**

  Add direct tests that assert:

  ```cpp
  CHECK(exact(0).exact_integer() == 0);
  CHECK(exact(1).exact_integer() == 1);
  CHECK(exact(10).exact_integer() == 55);
  CHECK(exact(100).exact_integer() == BigInt{"354224848179261915075"});
  CHECK(modular(10, 17).exact_integer() == 4);
  ```

  Also assert that Symbolic returns `F(10)` without an integer; Metric returns
  no integer and an estimate at least six bits for `F(10)`; invalid parameters,
  negative address, and missing/zero/negative modulus reject; a pre-set signal
  returns `Cancelled`; and restrictive bit, operation and temporary budgets
  return `BudgetExceeded`. Add a million-index Metric test that has no integer
  and returns `700000`.

  Add Engine integration assertions in `engine_tests.cpp`: register
  FibonacciKernel, observe Exact `F(10) == 55`, observe one Exact cache entry
  after a repeat call, then observe Modular `F(10) mod 17 == 4` without another
  cache entry.

- [ ] **Step 2: Verify RED**

  Run: `cmake --build build --target technique_tests engine_tests && ctest --test-dir build -R 'technique_tests|engine_tests' --output-on-failure`

  Expected: FAIL because FibonacciKernel/header and CMake source registration
  do not exist.

- [ ] **Step 3: Implement FibonacciKernel**

  Implement the exact public declarations above. Require the one canonical
  parameter string and `address.index >= 0`. Use unsigned, saturating helpers
  for `ceil(7*n/10)`, estimated `2 * bit_length(n)` operations and eight times
  rounded-up metric bytes. Build `(F(k), F(k+1))` by iterating over the
  significant bits of `n`; check cancellation before every step. For Modular,
  reject nonpositive/missing modulus and reduce each intermediate value. Add an
  empty base trace to every non-early result so Engine cache admission remains
  possible.

- [ ] **Step 4: Verify GREEN and record it**

  Re-run the Step 2 command. All Fibonacci direct and Engine integration tests
  pass; record the commands and result in the SDD ledger.

### Task 3: Public-Header Consumer Demo and Compatibility Verification

**Files:**
- Create: `demos/lhme/technique_demo.cpp`
- Modify: `CMakeLists.txt`
- Modify: `.superpowers/sdd/2026-10-20-lhme-technique-kernel/progress.md`

**Interfaces:**
- Produces CTest target `lhme_technique_demo` linking `bruce_lhme_core` and
  including only `bruce/lhme/engine.hpp` and `bruce/lhme/fibonacci_kernel.hpp`
  plus existing demo support headers.

- [ ] **Step 1: Write the failing demo build registration**

  Register `lhme_technique_demo` in CMake with a source path
  `demos/lhme/technique_demo.cpp` that does not exist yet, and register the
  target with CTest. This is the executable-level failing check.

- [ ] **Step 2: Verify RED**

  Run: `cmake -S . -B build && cmake --build build --target lhme_technique_demo && ctest --test-dir build -R lhme_technique_demo --output-on-failure`

  Expected: FAIL because `demos/lhme/technique_demo.cpp` is missing.

- [ ] **Step 3: Complete the demo and CTest registration**

  Create the demo source using the existing `demo::Report`. It must include
  only public BRUCE headers, register FibonacciKernel, and assert Exact
  `F(10)=55`, Modular `F(10) mod 17=4`, a Metric result without an integer,
  and `BudgetExceeded` under `max_bit_length=1`. Link the target only to
  `bruce_lhme_core` and public/demo-support include paths.

- [ ] **Step 4: Verify targeted compatibility**

  Run: `cmake --build build --target c_api_tests c_api_header_smoke technique_tests engine_tests lhme_technique_demo && ctest --test-dir build -R 'c_api_tests|c_api_header_smoke|technique_tests|engine_tests|lhme_technique_demo' --output-on-failure`

  Expected: all five targets pass. Record evidence in the SDD ledger.

- [ ] **Step 5: Verify full build, tests and installation**

  Run: `cmake -S . -B build && cmake --build build && ctest --test-dir build --output-on-failure && cmake --install build --prefix build/install`

  Expected: build and installation succeed and all CTest targets pass. Record
  final evidence in the SDD ledger; do not create a commit because this
  workspace is not a Git repository.

## Plan Self-Review

**Spec coverage:** Task 1 implements the generic Modular contract and
PowerOffset behavior. Task 2 implements Fibonacci identity, canonical
parameters, lazy modes, Fast-Doubling, budget checks, cancellation and Engine
cache/registry integration. Task 3 proves public consumption and legacy C API
compatibility.

**Type consistency:** Every task uses `TechniqueRequest::modulus` as
`std::optional<BigInt>`. Fibonacci exposes `FibonacciKernel` and
`kFibonacciFastDoublingParameters`, which Task 2 tests and Task 3 consumes.

**Review focus coverage:** Task 1 tests invalid modulus for PowerOffset; Task
2 tests invalid modulus/parameters/address, saturated Metric, all exact budget
limits, cancellation and non-cached Modular Engine execution.

**Scope:** The plan adds one public request field, one reference technique and
one demo. It deliberately excludes GPU work, generic technique range execution
and runtime plugins.
