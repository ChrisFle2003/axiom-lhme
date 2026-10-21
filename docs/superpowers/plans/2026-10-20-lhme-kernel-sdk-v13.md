# LHME Kernel SDK V1.3 Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Deliver an installed, header-only Kernel SDK, a deterministic external
consumer, and contract tests that let a developer safely author a BRUCE
`TechniqueKernel`.

**Architecture:** Keep `TechniqueKernel` and `Engine` unchanged. Add inline
result/cancellation helpers in one public header, prove their semantics through
an in-tree Engine integration test, then prove the same authoring model from a
separate staged-install CMake consumer.

**Tech Stack:** C++20, existing LHME public headers and Engine, CMake 3.25,
CTest, Boost `cpp_int` through existing `BigInt`.

**Spec:** `docs/superpowers/specs/2026-10-20-lhme-kernel-sdk-v13-design.md`

## Global Constraints

- Keep `Engine`, `TechniqueKernel`, existing calculation kernels, and C API
  interfaces unchanged.
- Install `include/bruce/lhme/kernel_sdk.hpp` through the existing public
  header installation rule; add no compiled library source.
- SDK helpers are optional authoring conveniences, not a required base class or
  runtime plug-in mechanism.
- External consumer uses only `find_package(bruce_lhme 0.1 CONFIG REQUIRED)`
  and `bruce_lhme::core`; it may not add a BRUCE source-tree include or link
  path.
- The example identity is `example.affine`, version `1`, canonical parameters
  are `factor=1:2;offset=1:3`, and Exact address `21` confirms BigInt `45`.
- CTest stage/build cleanup is limited to named directories below the active
  binary tree. No network or global package installation.
- The workspace is not a Git repository; record Red/Green evidence in a V1.3
  SDD ledger instead of commits.

## Review Focus

- `confirmed_exact_result` must not produce a successful result without a
  trace when a caller passes a null trace; Task 1 normalizes it to a base trace.
- Invalid and cancelled helper results must preserve the caller's requested
  mode and use Branch/Run respectively; Task 1 tests both directly.
- The Engine must replace a caller-supplied cancellation signal with its own;
  Task 1 records the received signal and tests both Engine injection and a set
  cancellation signal directly.
- A confirmed technique result must still respect the caller's bit budget;
  Task 1 requests `max_bit_length=5` for result `45` and expects
  `BudgetExceeded` from `Engine`.
- A copied external fixture must not acquire BRUCE source paths accidentally;
  Task 2 configures it only with the staged `CMAKE_PREFIX_PATH` package.
- Repeated external CTest runs must not inherit artifacts; Task 2's driver
  removes only its named stage and consumer-build directories before each run.

## Planned File Structure

```text
include/bruce/lhme/kernel_sdk.hpp          inline public authoring helpers
tests/kernel_sdk_contract_tests.cpp        public helper and Engine contract tests
examples/kernel_sdk_consumer/CMakeLists.txt independent package consumer config
examples/kernel_sdk_consumer/main.cpp      AffineKernel consumer example
tests/kernel_sdk_consumer_test.cmake        staged-install integration driver
docs/kernel-sdk.md                          copy-and-adapt author guide
CMakeLists.txt                              test target registration
.superpowers/sdd/.../progress.md            Red/Green and final verification ledger
```

### Task 1: Header-Only SDK Helper Contract

**Files:**
- Create: `include/bruce/lhme/kernel_sdk.hpp`
- Create: `tests/kernel_sdk_contract_tests.cpp`
- Modify: `CMakeLists.txt`

**Interfaces:**
- Produces `cancellation_requested(const TechniqueRequest&) noexcept -> bool`.
- Produces `confirmed_exact_result(BigInt value, std::shared_ptr<const TraceRecord> trace) -> EvaluationResult`.
- Produces `invalid_technique_request_result(EvaluationMode mode, std::string reason) -> EvaluationResult`.
- Produces `cancelled_technique_result(EvaluationMode mode, std::string reason) -> EvaluationResult`.
- The two failure helpers create exactly one standard trace event: Branch for
  invalid and Run for cancelled. `confirmed_exact_result` normalizes a null
  trace to `make_trace({})`.

- [ ] **Step 1: Write the failing SDK contract tests**

  Add `kernel_sdk_contract_tests` using the existing `tests/support/test_main.cpp`.
  Test confirmed Exact BigInt `45` has status `Confirmed`, mode `Exact`, value,
  and a non-null trace even when the helper receives null. Test invalid Modular
  returns `InvalidArgument`, preserves `Modular`, and has Branch trace; test
  cancelled Metric returns `Cancelled`, preserves `Metric`, and has Run trace.
  Test missing/false/set cancellation signals map to false/false/true.

  Add a local `AffineKernel` test double that uses the SDK helpers. Register it
  through `Engine`, confirm `factor=1:2;offset=1:3` at index `21` gives `45`,
  and assert it received a non-null, unset Engine-owned signal even when the
  caller supplied a set signal. Invoke the kernel directly with a set signal
  to confirm its cancellation path, then call `Engine::cancel()` and confirm
  Engine returns `Cancelled`. Request the same result with
  `max_bit_length=5` and expect `BudgetExceeded`. Register the CTest target in
  `CMakeLists.txt` but do not create the SDK header.

- [ ] **Step 2: Verify RED**

  Run: `cmake -S . -B build && cmake --build build --target kernel_sdk_contract_tests`

  Expected: FAIL because `bruce/lhme/kernel_sdk.hpp` does not exist.

- [ ] **Step 3: Implement the public helper header**

  Include `technique.hpp` and `trace.hpp`. Implement all four exact inline
  interfaces from this task. The successful helper sets `Confirmed`, `Exact`,
  `integer`, and a supplied-or-base trace. Failure helpers set their specified
  status and requested mode and call `make_trace` with one event and the
  prescribed scope. The cancellation helper returns true only for a present,
  set `cancellation_signal`.

- [ ] **Step 4: Verify GREEN**

  Run: `cmake -S . -B build && cmake --build build --target kernel_sdk_contract_tests && ctest --test-dir build -R kernel_sdk_contract_tests --output-on-failure`

  Expected: target builds and 1/1 contract test passes.

### Task 2: Staged External SDK Consumer

**Files:**
- Create: `examples/kernel_sdk_consumer/CMakeLists.txt`
- Create: `examples/kernel_sdk_consumer/main.cpp`
- Create: `tests/kernel_sdk_consumer_test.cmake`
- Modify: `CMakeLists.txt`

**Interfaces:**
- Consumes Task 1's `kernel_sdk.hpp` from the exported `bruce_lhme::core`.
- Produces CTest target `lhme_kernel_sdk_consumer`.
- Produces executable `bruce_lhme_kernel_sdk_consumer`, whose zero exit code
  means Engine confirmed BigInt `45` for `example.affine` at index `21`.

- [ ] **Step 1: Write the failing external fixture and driver**

  Add the independent fixture CMake project with only config-mode
  `find_package(bruce_lhme 0.1 CONFIG REQUIRED)`, one executable and target
  link to `bruce_lhme::core`; leave `main.cpp` absent. Add the CTest driver
  that removes only `${CMAKE_BINARY_DIR}/kernel_sdk_consumer_stage` and
  `${CMAKE_BINARY_DIR}/kernel_sdk_consumer_build`, stages an install, configures
  with `CMAKE_PREFIX_PATH`, builds and runs the executable. Each
  `execute_process` failure must be a `FATAL_ERROR` with captured stdout and
  stderr. Register `lhme_kernel_sdk_consumer` in `CMakeLists.txt`.

- [ ] **Step 2: Verify RED**

  Run: `cmake -S . -B build && cmake --build build && ctest --test-dir build -R lhme_kernel_sdk_consumer --output-on-failure && ctest --test-dir build -R lhme_kernel_sdk_consumer --output-on-failure`

  Expected: FAIL at the external fixture configuration because `main.cpp` is
  absent; this proves the staged package flow is reached.

- [ ] **Step 3: Implement `AffineKernel` consumer**

  Implement `AffineKernel` in fixture `main.cpp` using the SDK helpers. It
  returns identity `example.affine` version `1`, validates the exact canonical
  parameters, nonnegative address, and Exact mode. It checks cancellation
  before computing `BigInt{address.index} * 2 + 3`, returns a base trace, and
  registers through `Engine`. Main exits zero only for confirmed integer `45`.
  Its own CMake file adds no BRUCE source relative include/link directory.

- [ ] **Step 4: Verify GREEN**

  Run: `cmake -S . -B build && cmake --build build && ctest --test-dir build -R lhme_kernel_sdk_consumer --output-on-failure`

  Expected: staged install, package discovery, compile, link and execution all
  pass as 1/1 CTest test on both runs, proving the test-only directories are
  recreated cleanly.

### Task 3: Kernel Author Guide and Compatibility Verification

**Files:**
- Create: `docs/kernel-sdk.md`
- Modify: `.superpowers/sdd/2026-10-20-lhme-kernel-sdk-v13/progress.md`

**Interfaces:**
- Documents the exact consumer API from Tasks 1–2 and the distinction between
  compile-time integration and dynamic plug-ins.
- Verifies all existing public package, C API, technique, and new SDK tests.

- [ ] **Step 1: Write the developer guide**

  Document the `find_package` / `target_link_libraries` snippet, a concise
  `TechniqueKernel` authoring checklist, the exact affine parameter contract,
  cancellation and result helper semantics, and commands for the two SDK
  tests. State plainly that V1.3 is compile-time C++ integration, not a
  runtime plug-in loader.

- [ ] **Step 2: Run targeted public-API compatibility verification**

  Run: `cmake --build build --target c_api_tests c_api_header_smoke technique_tests kernel_sdk_contract_tests && ctest --test-dir build -R 'c_api_tests|c_api_header_smoke|technique_tests|kernel_sdk_contract_tests|lhme_kernel_sdk_consumer' --output-on-failure`

  Expected: all five named CTest tests pass.

- [ ] **Step 3: Run full fresh-build and installation verification**

  Run: `cmake -S . -B build/fresh_v13_sdk && cmake --build build/fresh_v13_sdk && ctest --test-dir build/fresh_v13_sdk --output-on-failure && cmake --install build/fresh_v13_sdk --prefix build/fresh_v13_sdk_install`

  Expected: all existing and new CTest tests pass from a fresh producer build;
  the installation contains `include/bruce/lhme/kernel_sdk.hpp`, static library,
  and all three CMake package files. Record exact Red/Green and final evidence
  in the V1.3 SDD ledger.

## Plan Self-Review

**Spec coverage:** Task 1 provides the optional header-only SDK and validates
result/cancellation semantics through real Engine interaction. Task 2 proves a
separate CMake consumer can author the affine technique and obtain `45`.
Task 3 supplies the user guide and checks compatibility from a fresh build.

**Type consistency:** Task 1 defines all helper names and result types. Task 2
consumes the installed header and package target; Task 3 runs the exact CTest
names created in Tasks 1–2.

**Review focus coverage:** Task 1 covers null trace, modes/traces, Engine
signal injection, cancellation, and budget enforcement. Task 2 covers source
isolation and named staged-directory cleanup. The full fresh-build check
protects installed-header visibility.

**Scope:** V1.3 adds authoring guidance and deterministic SDK conveniences;
it deliberately excludes plug-in loading, Engine redesign, GPU work, and
package publication.
