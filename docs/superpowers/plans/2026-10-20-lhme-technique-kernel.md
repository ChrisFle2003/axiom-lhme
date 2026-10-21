# LHME TechniqueKernel V1 Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Let C++ hosts register deterministic, named and versioned BRUCE computation kernels while preserving Engine cache, SafeStop and trace controls.

**Architecture:** Add a public `TechniqueKernel` contract and `PowerOffsetKernel` reference implementation. Extend `Engine` with a fail-closed registry and technique evaluation path; extend CacheKey with a canonical technique domain so cached results cannot cross identities or parameter bytes. Existing FormulaSpec and C API entry points remain unchanged.

**Tech Stack:** C++20, CMake/CTest, existing BRUCE Engine, ResultCache and TraceRecord.

**Spec:** `docs/superpowers/specs/2026-10-20-lhme-technique-kernel-design.md`

## Global Constraints

- V1 is compile-time C++ extension only; do not load shared libraries or execute arbitrary source/scripts.
- Preserve public FormulaSpec, EvaluationRequest, streaming APIs and C API behavior.
- A kernel identity is a nonempty ASCII ID plus a positive `std::uint32_t` version.
- A TechniqueRequest contains canonical UTF-8 parameter text, address, mode, budget and an Engine-owned cancellation signal.
- Registry rejects null kernels, invalid identities and duplicates without state mutation.
- Cache only Exact and Metric; key material is length-delimited ID, version, parameter bytes, address and mode.
- The project is not a Git worktree; use an SDD ledger instead of commits.

## Review Focus

- Duplicate or malformed identities must not replace a registered kernel; Task 2 tests this.
- Ambiguous parameter encodings must not share cache entries; Task 3 tests this.
- A kernel receives the Engine cancellation signal, not a caller signal; Task 3 tests this.
- A cache hit with a tighter budget cannot publish an exact value; Task 3 tests this.
- FormulaSpec and C API behavior must remain compatible; Task 4 verifies this.

---

## Planned File Structure

```text
include/bruce/lhme/technique.hpp          public identity, request, kernel API
include/bruce/lhme/power_offset_kernel.hpp reference kernel declaration and codec
src/core/power_offset_kernel.cpp          codec and existing-math delegation
include/bruce/lhme/cache.hpp               generic technique cache key
src/cache/result_cache.cpp                 technique key construction
include/bruce/lhme/engine.hpp              registry and public methods
src/engine.cpp                             registry, cache, SafeStop, trace
tests/technique_tests.cpp                  contract and reference-kernel tests
tests/engine_tests.cpp                     registry/cache/cancellation tests
CMakeLists.txt                             source and test registration
```

### Task 1: Public Technique Contract and Parameter Codec

**Files:**
- Create: `include/bruce/lhme/technique.hpp`
- Create: `include/bruce/lhme/power_offset_kernel.hpp`
- Create: `tests/technique_tests.cpp`
- Modify: `CMakeLists.txt`

**Interfaces:**
- Produces `TechniqueIdentity { std::string id; std::uint32_t version; }` and `bool valid_technique_identity(const TechniqueIdentity&)`.
- Produces `TechniqueRequest { TechniqueIdentity identity; std::string parameters; FormulaAddress address; EvaluationMode mode; Budget budget; std::shared_ptr<const std::atomic_bool> cancellation_signal; }`.
- Produces abstract `TechniqueKernel::identity() -> TechniqueIdentity` and `TechniqueKernel::evaluate(const TechniqueRequest&) -> EvaluationResult`.
- Declares `PowerOffsetKernel`, `encode_power_offset_parameters(BigInt, BigInt)` and `parse_power_offset_parameters(std::string_view)`.

- [ ] **Step 1: Write the failing contract tests**

```cpp
TEST(technique_identity_requires_ascii_name_and_positive_version) {
  CHECK(valid_technique_identity({.id = "bruce.power_offset", .version = 1}));
  CHECK(!valid_technique_identity({.id = "", .version = 1}));
  CHECK(!valid_technique_identity({.id = "bruce.power_offset", .version = 0}));
}

TEST(power_offset_parameter_encoding_is_unambiguous) {
  CHECK(encode_power_offset_parameters(1, 23) !=
        encode_power_offset_parameters(12, 3));
}
```

- [ ] **Step 2: Verify RED**

Run: `cmake -S . -B build && cmake --build build --target technique_tests && ctest --test-dir build -R technique_tests --output-on-failure`

Expected: FAIL because the TechniqueKernel headers and test target do not exist.

- [ ] **Step 3: Implement the public contract**

Accept identifier bytes `[A-Za-z0-9._-]`, length 1–128, and version > 0. Use canonical parameters `base=<length>:<decimal>;scale=<length>:<decimal>`; parsing rejects malformed text, negative base and negative scale.

- [ ] **Step 4: Verify GREEN and record it**

Run: `cmake --build build --target technique_tests && ctest --test-dir build -R technique_tests --output-on-failure`

Expected: PASS. Record RED/GREEN commands in `.superpowers/sdd/2026-10-20-lhme-technique-kernel/progress.md`; no commit.

### Task 2: PowerOffsetKernel and Engine Registry

**Files:**
- Create: `src/core/power_offset_kernel.cpp`
- Modify: `include/bruce/lhme/engine.hpp`
- Modify: `src/engine.cpp`
- Modify: `CMakeLists.txt`
- Modify: `tests/technique_tests.cpp`
- Modify: `tests/engine_tests.cpp`

**Interfaces:**
- Produces `PowerOffsetKernel::identity() -> {"bruce.power_offset", 1}`.
- Produces `PowerOffsetKernel::evaluate(const TechniqueRequest&) -> EvaluationResult`.
- Produces `Engine::register_technique(std::shared_ptr<const TechniqueKernel>) -> bool` and `Engine::evaluate_technique(const TechniqueRequest&) -> EvaluationResult`.

- [ ] **Step 1: Write failing reference-kernel and registry tests**

```cpp
TEST(power_offset_kernel_delegates_exact_math_to_bruce) {
  bruce::lhme::PowerOffsetKernel kernel;
  const auto result = kernel.evaluate({.identity = kernel.identity(),
      .parameters = encode_power_offset_parameters(2, 100),
      .address = {.index = 10}, .mode = bruce::lhme::EvaluationMode::Exact,
      .budget = bruce::lhme::Budget::unlimited()});
  CHECK(result.exact_integer() == 102300);
}

TEST(engine_registry_rejects_duplicate_and_unknown_techniques) {
  auto engine = bruce::lhme::Engine::create({});
  const auto kernel = std::make_shared<bruce::lhme::PowerOffsetKernel>();
  CHECK(engine.register_technique(kernel));
  CHECK(!engine.register_technique(kernel));
  CHECK(engine.evaluate_technique({.identity = {.id = "missing", .version = 1}})
            .status == bruce::lhme::Status::InvalidArgument);
}
```

- [ ] **Step 2: Verify RED**

Run: `cmake --build build --target technique_tests engine_tests && ctest --test-dir build -R 'technique_tests|engine_tests' --output-on-failure`

Expected: FAIL because the reference kernel and Engine technique methods are absent.

- [ ] **Step 3: Implement the reference kernel and registry**

PowerOffsetKernel validates its identity/parameters/address, calls `make_power_offset_spec(base, scale, request.identity.version)`, and delegates to existing `bruce::lhme::evaluate`. The Engine stores kernels by a length-delimited `(id, version)` key, rejects null/invalid/duplicate registrations before mutation, replaces the request cancellation signal, turns kernel exceptions into `InternalError`, and traces registry rejection.

- [ ] **Step 4: Verify GREEN and record it**

Run: `cmake --build build --target technique_tests engine_tests && ctest --test-dir build -R 'technique_tests|engine_tests' --output-on-failure`

Expected: PASS. Update the SDD ledger; no commit.

### Task 3: Cache Identity, Budget and SafeStop

**Files:**
- Modify: `include/bruce/lhme/cache.hpp`
- Modify: `src/cache/result_cache.cpp`
- Modify: `src/engine.cpp`
- Modify: `tests/cache_tests.cpp`
- Modify: `tests/engine_tests.cpp`

**Interfaces:**
- Produces `CacheKey::technique(const TechniqueIdentity&, std::string_view parameters, std::int64_t address, EvaluationMode mode) -> CacheKey`.
- Produces Exact/Metric cache integration in `Engine::evaluate_technique`.

- [ ] **Step 1: Write failing cache, budget and cancellation tests**

```cpp
TEST(technique_cache_key_separates_length_ambiguous_parameters) {
  const auto first = CacheKey::technique({"example", 1}, "base=1:1;scale=2:23",
                                         10, EvaluationMode::Exact);
  const auto second = CacheKey::technique({"example", 1}, "base=2:12;scale=1:3",
                                          10, EvaluationMode::Exact);
  CHECK(first != second);
}

TEST(engine_technique_cache_rechecks_tighter_exact_budget) {
  // Fill PowerOffsetKernel Exact cache at index 10; a later max_bit_length=0
  // request returns BudgetExceeded and no integer.
}
```

Add a test kernel that records its incoming signal; after `engine.cancel()` it must observe true and return `Cancelled`.

- [ ] **Step 2: Verify RED**

Run: `cmake --build build --target cache_tests engine_tests && ctest --test-dir build -R 'cache_tests|engine_tests' --output-on-failure`

Expected: FAIL because generic technique keys and technique cache/SafeStop handling are absent.

- [ ] **Step 3: Extend cache and technique evaluation**

Add `domain`, technique identity and parameter fields to CacheKey. Existing FormulaSpec constructors use domain `formula`; `CacheKey::technique` uses domain `technique` and length-delimited inputs. Cache confirmed Exact/Metric only. Reuse existing exact cache-budget recheck before publishing. Add cache hit/miss events and refuse to publish a confirmed result if the Engine cancellation signal is set after kernel return.

- [ ] **Step 4: Verify GREEN and record it**

Run: `cmake --build build --target cache_tests engine_tests && ctest --test-dir build -R 'cache_tests|engine_tests' --output-on-failure`

Expected: PASS. Update the SDD ledger; no commit.

### Task 4: Compatibility and Final Verification

**Files:**
- Modify: `CMakeLists.txt`
- Modify: `.superpowers/sdd/2026-10-20-lhme-technique-kernel/progress.md`

**Interfaces:**
- Verifies Task 1–3 interfaces without changing the FormulaSpec C API.

- [ ] **Step 1: Run targeted legacy compatibility checks**

Run: `cmake --build build --target c_api_tests c_api_header_smoke engine_tests technique_tests && ctest --test-dir build -R 'c_api_tests|c_api_header_smoke|engine_tests|technique_tests' --output-on-failure`

Expected: PASS; the existing C API exact power-offset request at index 10 still returns `102300`.

- [ ] **Step 2: Run final verification**

Run: `cmake -S . -B build && cmake --build build && ctest --test-dir build --output-on-failure && cmake --install build --prefix build/install`

Expected: build and installation succeed; all tests pass. The million-address controlled stress executable remains manual.

- [ ] **Step 3: Record completion**

Add final evidence to the SDD ledger. No commit is possible in this workspace.

## Plan Self-Review

**Spec coverage:** Task 1 defines the public contract; Task 2 provides trusted C++ registration and PowerOffset reference behavior; Task 3 owns canonical cache identity, budget and SafeStop; Task 4 protects legacy interfaces.

**Type consistency:** Task 1 defines TechniqueIdentity, TechniqueRequest and TechniqueKernel; Tasks 2–3 consume them unchanged. CacheKey::technique accepts the same identity and parameter types. The Engine owns `shared_ptr<const TechniqueKernel>` and supplies the cancellation signal declared in TechniqueRequest.

**Review focus coverage:** Task 2 tests malformed/duplicate registry input. Task 3 tests parameter separation, tighter cache budgets and Engine-owned cancellation. Task 4 tests FormulaSpec/C API compatibility.

**Scope:** Four independently testable tasks implement only V1 compile-time C++ kernels. Dynamic libraries, arbitrary code and generic C bindings remain excluded.
