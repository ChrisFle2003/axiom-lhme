# BRUCE GGUF Inspector V1 Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Build a deterministic, read-only C++20 GGUF V3 inspector that emits a CPU/RAM loading plan, a trace and stable reports without changing model bytes or tensor ordering.

**Architecture:** A bounded binary reader validates the GGUF container and produces immutable tensor descriptors in data-offset order. A separate planner turns those descriptors into resident/on-demand/deferred decisions under an explicit policy and memory budget; a report layer and a deliberately thin CLI expose the result. Optional pattern scanning is isolated from the parser and can only report evidence, never replace tensor data or create a FormulaRecipe sidecar.

**Tech Stack:** C++20, CMake 3.25, C++ standard library, existing BRUCE LHME `Status` and `TraceRecord` types; no runtime dependencies or network access.

**Spec:** `docs/superpowers/specs/2026-10-21-bruce-gguf-optimizer-design.md`

## Global Constraints

- Support local GGUF version 3 only; reject any other version before interpreting descriptors.
- Version 1 is read-only: do not write, reorder, quantize, dequantize or otherwise alter a GGUF payload.
- Preserve the scalar coordinate order of every tensor; only descriptor and plan presentation may be ordered by file offset then name.
- Enforce supplied file-size, entry-count, dimension-count and work limits before allocation or payload access; overflow, malformed ranges and misalignment fail closed.
- Use a deterministic output order and deterministic FNV-derived input fingerprint; equal input bytes and configuration must produce equal inventory, plan, trace and machine report bytes.
- Do not add inference, a model format, LLM/embedding control, BSGS/chain/password logic, downloads or external libraries.
- `FormulaRecipe` remains future-facing and exact-only: V1 pattern results are candidates, never replacement values.

## Review Focus

- A valid header with a metadata string or array whose declared length crosses EOF must return `Status::Rejected` without allocating its declared length; cover in Task 1.
- A dimension product or tensor data range that overflows `uint64_t` must return `Status::Rejected`, even when the file itself is small; cover in Task 1.
- Tensor descriptors declared in reverse physical order must be presented in ascending data-offset order while their on-disk scalar bytes remain untouched; cover in Task 1.
- A `budgeted` use profile containing an unknown or duplicate tensor name must be rejected rather than silently changing priority; cover in Task 3.
- A pattern scan request that names a valid tensor but exceeds the configured payload-work limit must return a reportable budget result and must leave the source file byte-identical; cover in Task 4.

---

## File structure

- Create `include/bruce/lhme/gguf_inspector.hpp`: public immutable GGUF inventory, inspection limits and inspection result API.
- Create `src/gguf/reader.cpp`: bounded little-endian reader, GGUF V3 metadata skipping, descriptor validation and fingerprinting.
- Create `include/bruce/lhme/gguf_storage_plan.hpp`: public policy, plan item and planner API.
- Create `src/gguf/storage_planner.cpp`: deterministic chunking, priority ordering and budget assignments.
- Create `include/bruce/lhme/gguf_pattern_scan.hpp`: report-only candidate scanner API.
- Create `src/gguf/pattern_scanner.cpp`: bounded raw-byte repetition/constant evidence and exact F32 progression candidate detection for selected chunks.
- Create `include/bruce/lhme/gguf_report.hpp` and `src/gguf/report.cpp`: stable plain-text and JSON serialization shared by library callers and CLI.
- Create `tools/gguf_inspect/command.hpp`, `tools/gguf_inspect/command.cpp` and `tools/gguf_inspect/main.cpp`: testable command boundary, argument parsing and file/report output; no planning logic in `main.cpp`.
- Create `tests/support/gguf_fixture.hpp`: small, generated GGUF V3 fixture writer plus a byte-copy helper used only by tests.
- Create `tests/gguf_reader_tests.cpp`, `tests/gguf_storage_plan_tests.cpp`, `tests/gguf_pattern_scan_tests.cpp` and `tests/gguf_report_tests.cpp`: focused contract tests using generated fixtures.
- Modify `CMakeLists.txt`: compile the four core sources, register four test executables and add the `bruce_gguf_inspect` tool target.
- Modify `README.md` and create `docs/gguf-inspector.md`: user-facing scope, safety guarantees, build/use examples, policy semantics and known V1 limits.

### Task 1: Bounded GGUF V3 reader and immutable inventory

**Files:**
- Create: `include/bruce/lhme/gguf_inspector.hpp`
- Create: `src/gguf/reader.cpp`
- Create: `tests/support/gguf_fixture.hpp`
- Create: `tests/gguf_reader_tests.cpp`
- Modify: `CMakeLists.txt`

**Interfaces:**
- Produces `enum class GgufValidationError`, `struct GgufInspectionLimits`, `struct GgufTensor`, `struct GgufInventory`, `struct GgufInspection`, and `inspect_gguf_file(const std::filesystem::path&, const GgufInspectionLimits&) -> GgufInspection` in namespace `bruce::lhme`.
- `GgufInspection` contains `Status status`, `GgufValidationError error`, `std::string diagnostic`, `std::optional<GgufInventory> inventory`, and `std::shared_ptr<const TraceRecord> trace`.
- `GgufTensor` contains tensor name, rank/shape, `ggml_type`, logical element count, relative tensor-data offset, absolute file offset, stored byte count and an optional decoded-RAM estimate.
- Consumes the existing `Status`, `TraceRecord`, `GateScope` and `make_trace` interfaces from `types.hpp` and `trace.hpp`.

- [ ] **Step 1: Write failing reader-contract tests**

Add tests for a generated minimal valid V3 F32 fixture, byte-stable input fingerprint, invalid magic, unsupported version, truncated metadata string/array, dimension-product overflow, tensor range overflow and a non-aligned data offset. Assert `Status::Confirmed` only for the valid fixture and `Status::Rejected` for every malformed fixture.

- [ ] **Step 2: Run the reader tests to verify they fail**

Run: `cmake --build --preset debug --target gguf_reader_tests && ctest --preset debug -R '^gguf_reader_tests$' --output-on-failure`

Expected: FAIL because the GGUF reader and its test target do not exist.

- [ ] **Step 3: Define the public reader API in `include/bruce/lhme/gguf_inspector.hpp`**

Use `std::filesystem::path`, fixed-width unsigned integer types and `std::vector<std::uint64_t>` for dimensions. Set conservative limits in `GgufInspectionLimits` and reject zero limits that would make parsing unsafe. Store inventory tensors in their stable presentation order, not on-disk declaration order.

- [ ] **Step 4: Implement `inspect_gguf_file` in `src/gguf/reader.cpp`**

Read exactly the GGUF V3 header sequence `magic`, `version`, `tensor_count`, `metadata_kv_count` as little-endian values. Skip every legal V3 metadata value type with length checks; read each tensor descriptor as name, rank, dimensions, ggml type and relative offset. Use checked addition and multiplication for all lengths and offsets. Resolve tensor stored ranges from validated tensor offsets in a copy sorted by offset/name; retain the original tensor bytes untouched and publish descriptors in that same stable order. Support V3 F32 stored-size validation in this first task and return `Status::UnsupportedMode` plus `GgufValidationError::UnsupportedTensorType` for any other type rather than guessing.

- [ ] **Step 5: Add the target and run the reader tests to verify they pass**

Run: `cmake --build --preset debug --target gguf_reader_tests && ctest --preset debug -R '^gguf_reader_tests$' --output-on-failure`

Expected: PASS; malformed fixtures fail closed without a payload allocation proportional to an untrusted declared length.

- [ ] **Step 6: Commit the reader slice**

```bash
git add CMakeLists.txt include/bruce/lhme/gguf_inspector.hpp src/gguf/reader.cpp \
  tests/support/gguf_fixture.hpp tests/gguf_reader_tests.cpp
git commit -m "feat: inspect bounded GGUF v3 inventories"
```

### Task 2: Expand supported stored-size types and inventory RAM accounting

**Files:**
- Modify: `include/bruce/lhme/gguf_inspector.hpp`
- Modify: `src/gguf/reader.cpp`
- Modify: `tests/gguf_reader_tests.cpp`

**Interfaces:**
- Consumes `GgufTensor` and `GgufInventory` from Task 1.
- Produces exact stored-byte validation for the documented GGUF V3 `F32`, `F16`, `Q4_0`, `Q4_1`, `Q5_0`, `Q5_1`, `Q8_0`, `Q8_1`, `Q2_K`, `Q3_K`, `Q4_K`, `Q5_K`, `Q6_K` and `Q8_K` block layouts, plus a decoded-RAM estimate when a tensor's logical element count is known.

- [ ] **Step 1: Write failing type-accounting tests**

Extend the fixture builder and tests with F16 and one block-quantized tensor. Assert the expected stored bytes, logical element count and F32-decoded RAM estimate. Add a fixture whose stored span disagrees with its type/block calculation and assert `Status::Rejected`.

- [ ] **Step 2: Run the reader tests to verify they fail**

Run: `cmake --build --preset debug --target gguf_reader_tests && ctest --preset debug -R '^gguf_reader_tests$' --output-on-failure`

Expected: FAIL because non-F32 GGML type accounting is not implemented.

- [ ] **Step 3: Implement the immutable GGML V3 type table in `src/gguf/reader.cpp`**

Represent each supported type by its exact V3 identifier, elements per block and stored bytes per block:

```text
F32  = 0 /   1 /   4       F16  = 1 /   1 /   2
Q4_0 = 2 /  32 /  18      Q4_1 = 3 /  32 /  20
Q5_0 = 6 /  32 /  22      Q5_1 = 7 /  32 /  24
Q8_0 = 8 /  32 /  34      Q8_1 = 9 /  32 /  40
Q2_K =10 / 256 /  84      Q3_K =11 / 256 / 110
Q4_K =12 / 256 / 144      Q5_K =13 / 256 / 176
Q6_K =14 / 256 / 210      Q8_K =15 / 256 / 292
```

Require a logical element count divisible by the block size for block-quantized types; use checked multiplication for blocks and bytes. Calculate decoded RAM as `logical_element_count * sizeof(float)` with overflow protection. Keep unknown types explicit and return `Status::UnsupportedMode`.

- [ ] **Step 4: Run the reader tests to verify they pass**

Run: `cmake --build --preset debug --target gguf_reader_tests && ctest --preset debug -R '^gguf_reader_tests$' --output-on-failure`

Expected: PASS; all supported types are range-validated and mismatched spans are rejected.

- [ ] **Step 5: Commit the type-accounting slice**

```bash
git add include/bruce/lhme/gguf_inspector.hpp src/gguf/reader.cpp tests/gguf_reader_tests.cpp
git commit -m "feat: account for common GGUF tensor storage types"
```

### Task 3: Deterministic CPU/RAM storage planner

**Files:**
- Create: `include/bruce/lhme/gguf_storage_plan.hpp`
- Create: `src/gguf/storage_planner.cpp`
- Create: `tests/gguf_storage_plan_tests.cpp`
- Modify: `CMakeLists.txt`

**Interfaces:**
- Consumes `const GgufInventory&` from Task 1/2.
- Produces `enum class GgufLoadingPolicy { AllResident, Budgeted, MetadataOnly }`, `enum class GgufPlacement { Resident, OnDemand, Deferred, Rejected }`, `struct GgufPlanConfig`, `struct GgufPlanItem`, `struct GgufStoragePlan`, and `make_gguf_storage_plan(const GgufInventory&, const GgufPlanConfig&) -> GgufStoragePlan`.
- `GgufPlanConfig` contains `memory_budget_bytes`, nonzero `chunk_size_bytes`, policy and `std::vector<std::string> ordered_tensor_use_profile`.
- `GgufStoragePlan` contains a `Status`, the normalized plan items, assigned resident bytes, optional overflow/rejection diagnostic and a trace.

- [ ] **Step 1: Write failing planner-contract tests**

Build a fixture inventory with three distinct spans. Assert: `all-resident` rejects all items if decoded RAM exceeds the budget; `budgeted` fills profile-listed tensor chunks first then offset/name fallback chunks; `metadata-only` defers all chunks; equal input/configuration produces equal item order and trace ID; a duplicate or unknown profile name is invalid.

- [ ] **Step 2: Run the planner tests to verify they fail**

Run: `cmake --build --preset debug --target gguf_storage_plan_tests && ctest --preset debug -R '^gguf_storage_plan_tests$' --output-on-failure`

Expected: FAIL because the storage planner target and API do not exist.

- [ ] **Step 3: Define the planner API in `include/bruce/lhme/gguf_storage_plan.hpp`**

Define a plan item as one contiguous tensor chunk with tensor name, chunk index, file offset, stored bytes, decoded-RAM estimate, placement and reason. Do not expose mutable references to the inventory or tensor payload.

- [ ] **Step 4: Implement `make_gguf_storage_plan` in `src/gguf/storage_planner.cpp`**

Split every stored tensor span into nonzero chunks no larger than `chunk_size_bytes`. Validate the profile is a duplicate-free subset of inventory names. For `Budgeted`, prioritise profile order and use inventory offset/name order for the rest; assign `Resident` while each chunk's decoded-RAM estimate fits the remaining budget and `OnDemand` otherwise. For `MetadataOnly`, assign `Deferred`. For `AllResident`, assign every chunk `Resident` only if the whole planned decoded-RAM total fits; otherwise assign `Rejected` and return `Status::BudgetExceeded`. Emit deterministic per-item trace reasons.

- [ ] **Step 5: Run the planner tests to verify they pass**

Run: `cmake --build --preset debug --target gguf_storage_plan_tests && ctest --preset debug -R '^gguf_storage_plan_tests$' --output-on-failure`

Expected: PASS; no policy silently bypasses a configured memory budget.

- [ ] **Step 6: Commit the planner slice**

```bash
git add CMakeLists.txt include/bruce/lhme/gguf_storage_plan.hpp \
  src/gguf/storage_planner.cpp tests/gguf_storage_plan_tests.cpp
git commit -m "feat: plan GGUF tensor residency under RAM budgets"
```

### Task 4: Report-only exact-pattern candidates

**Files:**
- Create: `include/bruce/lhme/gguf_pattern_scan.hpp`
- Create: `src/gguf/pattern_scanner.cpp`
- Create: `tests/gguf_pattern_scan_tests.cpp`
- Modify: `CMakeLists.txt`

**Interfaces:**
- Consumes a local path, `const GgufInventory&` and selected chunks from `GgufPlanItem`.
- Produces `enum class GgufPatternKind { ConstantBytes, RepeatedBytes, ExactF32ArithmeticProgression }`, `struct GgufPatternScanConfig`, `struct GgufPatternCandidate`, `struct GgufPatternScanResult`, and `scan_gguf_pattern_candidates(const std::filesystem::path&, const GgufInventory&, std::span<const GgufPlanItem>, const GgufPatternScanConfig&) -> GgufPatternScanResult`.
- `GgufPatternCandidate::exactly_verified` is always `false` in V1 and no scanner interface returns `FormulaSpec` or writes a sidecar.

- [ ] **Step 1: Write failing scanner-contract tests**

Generate selected F32 chunks containing a constant sequence, repeating raw bytes and a finite F32 arithmetic progression. Assert candidates are deterministic, marked `exactly_verified == false`, and include only requested tensor chunks. Add a work-limit case and compare source bytes before/after scan to prove no modification.

- [ ] **Step 2: Run the scanner tests to verify they fail**

Run: `cmake --build --preset debug --target gguf_pattern_scan_tests && ctest --preset debug -R '^gguf_pattern_scan_tests$' --output-on-failure`

Expected: FAIL because no candidate scanner exists.

- [ ] **Step 3: Implement bounded report-only scanning in `src/gguf/pattern_scanner.cpp`**

Open only validated selected ranges and stop before the configured `max_payload_bytes` or `max_candidates` is exceeded. Detect byte-constant and exact byte-repeated blocks for every supported stored layout. For F32 ranges, detect an arithmetic progression only when IEEE-754 operations reproduce every selected element bit-for-bit; label the result as a candidate, not a verified FormulaRecipe. Return `Status::BudgetExceeded` with a deterministic trace when limits are exceeded and never mutate the input file.

- [ ] **Step 4: Run the scanner tests to verify they pass**

Run: `cmake --build --preset debug --target gguf_pattern_scan_tests && ctest --preset debug -R '^gguf_pattern_scan_tests$' --output-on-failure`

Expected: PASS; candidate evidence is bounded, deterministic and cannot change tensor data.

- [ ] **Step 5: Commit the scanner slice**

```bash
git add CMakeLists.txt include/bruce/lhme/gguf_pattern_scan.hpp \
  src/gguf/pattern_scanner.cpp tests/gguf_pattern_scan_tests.cpp
git commit -m "feat: report bounded GGUF pattern candidates"
```

### Task 5: Stable reports, CLI and public documentation

**Files:**
- Create: `include/bruce/lhme/gguf_report.hpp`
- Create: `src/gguf/report.cpp`
- Create: `tools/gguf_inspect/command.hpp`
- Create: `tools/gguf_inspect/command.cpp`
- Create: `tools/gguf_inspect/main.cpp`
- Create: `tests/gguf_report_tests.cpp`
- Create: `docs/gguf-inspector.md`
- Modify: `CMakeLists.txt`
- Modify: `README.md`

**Interfaces:**
- Consumes `GgufInspection`, `GgufStoragePlan` and optional `GgufPatternScanResult`.
- Produces `render_gguf_text_report(...) -> std::string` and `render_gguf_json_plan(...) -> std::string`; their byte order and field order are stable.
- Produces `run_gguf_inspect(std::span<const std::string_view> arguments, std::istream& input, std::ostream& output, std::ostream& error) -> int` in the private `tools/gguf_inspect/command.hpp`; `main.cpp` only forwards process arguments and streams to it.
- CLI syntax: `bruce_gguf_inspect MODEL.gguf --memory-budget BYTES --chunk-size BYTES --policy all-resident|budgeted|metadata-only [--use-profile FILE] [--json-output FILE] [--scan]`.

- [ ] **Step 1: Write failing report and command-contract tests**

Use a generated fixture to assert that text and JSON reports include input fingerprint, supported parser version, policy, budget, each assignment and trace ID in stable order. Assert JSON generated twice is byte-identical; invalid arguments produce a nonzero CLI exit status through a testable `run_gguf_inspect(arguments, streams)` function.

- [ ] **Step 2: Run the report tests to verify they fail**

Run: `cmake --build --preset debug --target gguf_report_tests && ctest --preset debug -R '^gguf_report_tests$' --output-on-failure`

Expected: FAIL because the report functions and CLI command boundary do not exist.

- [ ] **Step 3: Implement stable report rendering and a thin CLI**

Serialize strings with JSON escaping and fixed field order using only the standard library. Let the CLI parse only explicit flags, read an optional newline-delimited use profile, call the public library APIs and either write the requested JSON file or print the text report. Return distinct nonzero exits for invalid arguments, rejected input and budget exhaustion; do not download files or write the inspected GGUF.

- [ ] **Step 4: Document the actual V1 contract**

In `docs/gguf-inspector.md`, explain supported GGUF version/type coverage, read-only semantics, policy decisions, candidate-versus-exact distinction, memory estimates and invocation examples. Link it from `README.md`; explicitly state that tensor scalar ordering is never sorted and ordinary quantized weights are not presumed formula-compressible.

- [ ] **Step 5: Run the complete verification suite**

Run: `cmake --preset debug && cmake --build --preset debug && ctest --preset debug --output-on-failure && cmake --preset release && cmake --build --preset release && ctest --preset release --output-on-failure`

Expected: PASS; the library package, existing LHME tests and all four GGUF test targets remain green.

- [ ] **Step 6: Commit the delivery slice**

```bash
git add CMakeLists.txt README.md docs/gguf-inspector.md \
  include/bruce/lhme/gguf_report.hpp src/gguf/report.cpp \
  tools/gguf_inspect/command.hpp tools/gguf_inspect/command.cpp \
  tools/gguf_inspect/main.cpp tests/gguf_report_tests.cpp
git commit -m "feat: expose deterministic GGUF inspection reports"
```

