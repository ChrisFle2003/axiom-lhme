# LHME Isolated Peak-Memory Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Measure each LHME stress scenario in a fresh Linux child process so its reported peak RAM is isolated and comparable.

**Architecture:** The stress executable becomes a controller plus `--isolated-child` mode. A child computes one named scenario, samples its own Linux `ru_maxrss`, and writes a fixed binary report through a pipe. The controller validates the report and child exit status before displaying it.

**Tech Stack:** C++20, CMake/CTest, Linux `fork`/`exec`/`pipe`/`waitpid`, `/proc/self/exe`, `getrusage`.

**Spec:** `docs/superpowers/specs/2026-10-20-lhme-isolated-peak-memory-design.md`

## Global Constraints

- Modify only the manual CPU/RAM stress demo; do not change public LHME APIs.
- Linux uses a fresh `exec` process for each measured scenario; do not treat fork Copy-on-Write memory as isolated measurement.
- The internal binary pipe report is versioned and fail-closed; it is not a public or persistent format.
- Keep the full million-address controller run outside normal CTest.
- On non-Linux, report isolated measurements as unavailable rather than fabricating a replacement.
- Time values are informational only; counts, statuses, retention mode and report validity are functional assertions.

## Review Focus

- A truncated pipe payload, unknown magic or unknown protocol version must be rejected before any value is displayed; Task 1 owns this test.
- A child with an invalid scenario or abnormal exit must make the controller fail closed; Task 2 owns this test.
- `million-scan-1` and `million-scan-8` must have equal domain counters but independently sampled peaks; Task 3 checks both reports.
- `adaptive-hits` must retain the deterministic lowest three values while reporting `TopK`; Task 3 checks the child report and existing scenario invariants.
- Non-Linux builds must not include Linux process headers or pretend to measure isolated RAM; Task 2 owns the availability test.

---

## Planned File Structure

```text
demos/lhme/benchmarks/stress/
├─ process_memory.hpp             # existing Linux process-memory helper
├─ isolated_protocol.hpp           # fixed child-to-parent record and validation
├─ stress_scenarios.hpp            # named scenario execution and result conversion
├─ isolated_runner.hpp             # Linux pipe/fork/exec/wait controller helper
└─ controlled_stress.cpp           # CLI child mode and human-readable parent controller
tests/
├─ isolated_protocol_tests.cpp     # fast protocol validation checks
└─ isolated_runner_tests.cpp       # fast adaptive child integration check
CMakeLists.txt                     # test and target registration
```

### Task 1: Versioned Internal Result Protocol

**Files:**
- Create: `demos/lhme/benchmarks/stress/isolated_protocol.hpp`
- Create: `tests/isolated_protocol_tests.cpp`
- Modify: `CMakeLists.txt`

**Interfaces:**
- Produces `bruce::lhme::demo::stress::Scenario` with `MillionScanOne`, `MillionScanEight`, `AdaptiveHits`.
- Produces trivially-copyable `ChildReport` containing a magic constant, protocol version, scenario, `Status`, `GlobalSummary` fields, filter/Exact counts, effective `RetentionMode`, retained count, elapsed milliseconds and `peak_bytes`.
- Produces `[[nodiscard]] bool valid_child_report(const ChildReport&)`.

- [ ] **Step 1: Write failing protocol-validation tests**

```cpp
TEST(child_report_rejects_unknown_magic_and_version) {
  auto report = valid_report();
  report.magic = 0;
  CHECK(!valid_child_report(report));
  report = valid_report();
  report.version += 1;
  CHECK(!valid_child_report(report));
}
```

- [ ] **Step 2: Run the test target to verify it fails**

Run: `cmake -S . -B build && cmake --build build --target isolated_protocol_tests && ctest --test-dir build -R isolated_protocol_tests --output-on-failure`

Expected: FAIL because the protocol header and test target do not yet exist.

- [ ] **Step 3: Implement the fixed report contract**

Use fixed-width integer fields only. `valid_child_report` must recognize only the three named scenarios, known `Status` and `RetentionMode` values, the current magic/version, and `peak_bytes > 0` for a completed Linux child.

- [ ] **Step 4: Verify protocol tests pass**

Run: `cmake --build build --target isolated_protocol_tests && ctest --test-dir build -R isolated_protocol_tests --output-on-failure`

Expected: PASS.

- [ ] **Step 5: Record completion**

No Git commit is possible because this workspace is not a Git repository; record the test command in the SDD ledger created during execution.

### Task 2: Isolated Linux Scenario Runner

**Files:**
- Create: `demos/lhme/benchmarks/stress/stress_scenarios.hpp`
- Create: `demos/lhme/benchmarks/stress/isolated_runner.hpp`
- Create: `tests/isolated_runner_tests.cpp`
- Modify: `CMakeLists.txt`

**Interfaces:**
- Consumes `Scenario`, `ChildReport`, `process_peak_memory()` and `evaluate_streaming_range`.
- Produces `[[nodiscard]] ChildReport execute_scenario(Scenario)` for child mode.
- Produces `[[nodiscard]] IsolatedRunResult run_isolated(std::string_view executable, Scenario)` with availability, child exit validity and one validated `ChildReport`.

- [ ] **Step 1: Write failing runner integration tests**

```cpp
TEST(adaptive_child_returns_a_valid_isolated_report) {
  const auto run = run_isolated(stress_executable_path(), Scenario::AdaptiveHits);
  CHECK(run.available);
  CHECK(run.success);
  CHECK(run.report.status == Status::Confirmed);
  CHECK(run.report.exact_evaluation_count == 1250);
  CHECK(run.report.peak_bytes > 0);
}
```

Compile the test with the stress executable path supplied by CMake and make
the test target depend on `lhme_controlled_stress`.

- [ ] **Step 2: Run it to verify it fails**

Run: `cmake --build build --target isolated_runner_tests && ctest --test-dir build -R isolated_runner_tests --output-on-failure`

Expected: FAIL because the child mode and runner do not exist.

- [ ] **Step 3: Implement scenarios and Linux runner**

`execute_scenario` must retain the existing exact ranges and invariants. On
Linux, create a pipe, fork, redirect the child's result stream, and `execv`
the supplied executable with `--isolated-child <scenario>`. The parent must
read exactly one `ChildReport`, wait for a normal zero exit, and reject every
other outcome. On non-Linux, return `available=false` without invoking a
process API.

- [ ] **Step 4: Verify integration and existing stress behavior**

Run: `cmake --build build --target isolated_runner_tests lhme_controlled_stress && ctest --test-dir build -R isolated_runner_tests --output-on-failure`

Expected: PASS; the short adaptive child reports a nonzero isolated peak.

- [ ] **Step 5: Record completion**

No Git commit is possible; update the execution ledger with the passing test.

### Task 3: Parent Controller and Isolated Report

**Files:**
- Modify: `demos/lhme/benchmarks/stress/controlled_stress.cpp`
- Modify: `CMakeLists.txt` only if target dependencies need adjustment

**Interfaces:**
- Consumes `run_isolated(argv[0], scenario)` and the validated `ChildReport`.
- Produces parent-mode report rows for each isolated scenario; child mode writes only its binary report and exits.

- [ ] **Step 1: Write failing controller assertions**

Add parent-mode expectations for `MillionScanOne`, `MillionScanEight` and
`AdaptiveHits`: each report has `peak_bytes > 0`; both million reports have
1,000,001 checked / 0 Exact / 0 retained; Adaptive has 1,250 Exact, 8,751
misses, `TopK`, and three retained values.

- [ ] **Step 2: Run controller to verify it fails**

Run: `cmake --build build --target lhme_controlled_stress && ./build/lhme_controlled_stress`

Expected: FAIL until parent mode invokes isolated children and validates their reports.

- [ ] **Step 3: Implement controller/child dispatch**

Change `main` to parse only `--isolated-child <scenario>` for child mode;
all other direct invocation runs the Linux parent controller. Keep the
pre-cancel SafeStop assertion locally in parent mode. Display each scenario's
elapsed milliseconds and `isolated process peak RAM bytes` only after report
validation succeeds.

- [ ] **Step 4: Verify manual stress and complete project**

Run: `cmake -S . -B build && cmake --build build && ./build/lhme_controlled_stress && ctest --test-dir build --output-on-failure && cmake --install build --prefix build/install`

Expected: three independently measured peak values print, the stress target
prints `PASS`, and the normal CTest suite remains quick and green.

- [ ] **Step 5: Record completion**

No Git commit is possible; include manual stress output and the full
verification command in the execution ledger.

## Plan Self-Review

**Spec coverage:** Task 1 provides the versioned fail-closed wire record; Task 2 supplies fresh Linux child execution plus the unavailable fallback; Task 3 integrates every named scenario, output and manual verification.

**Type consistency:** `Scenario` and `ChildReport` originate in Task 1 and are consumed unchanged by Task 2 and Task 3. Only validated `ChildReport` values reach the controller report.

**Review focus coverage:** malformed protocol is Task 1; abnormal child and non-Linux availability are Task 2; deterministic counters, TopK, and isolated peaks are Task 3.

**Scope:** The plan changes only the manual stress instrumentation and two focused test targets. It does not change LHME library behavior, CTest's million-run policy, or public headers.
