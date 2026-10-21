# KEVIN Memory Store V1.4 Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Add a CPU/RAM-only KEVIN Memory Store that maps collision-free 8×8 paths to registered technique kernels, computes values lazily, and stores only GridSpecs plus sparse exact overrides.

**Architecture:** `KevinMemoryStore` owns validated `GridSpec` records and sparse overrides, while the existing `Engine` remains the sole owner of technique dispatch, ResultCache, budgets, and cancellation. `GridAddress` encodes a bounded 8×8 path into a unique nonnegative `FormulaAddress` index by prefixing the concatenated 6-bit path value with its geometric depth offset.

**Tech Stack:** C++20, CMake 3.25, Boost.Multiprecision `cpp_int`, existing BRUCE LHME Engine and test harness.

**Spec:** `docs/superpowers/specs/2026-10-21-kevin-memory-store-v14-design.md`

## Global Constraints

- CPU and RAM only; do not add persistence, file snapshots, GPU support, semantic routing, or a second result cache.
- Support only hierarchical 8×8 paths of depth `0..10`; reject invalid coordinates and depth `> 10` fail-closed.
- Use the existing `TechniqueKernel`/`Engine::evaluate_technique` path; do not add a parallel direct-`FormulaSpec` evaluator.
- Preserve existing Symbolic, Metric, Modular, Exact, Budget, Trace, ResultCache, and global `Engine::cancel()` semantics.
- Store regular values only in the existing bounded Engine cache; KEVIN owns only GridSpecs and explicit exact BigInt overrides.
- Keep KEVIN tests under `tests/kevin/`, separate from the existing generic test files.
- This workspace is not a Git repository. Do not create commits; record execution evidence in `.superpowers/sdd/2026-10-21-kevin-memory-store-v14/progress.md`.

## Review Focus

- Empty versus all-zero paths must map to different formula indexes; cover this in Task 1.
- A depth-10 path must remain nonnegative and fit `int64_t`; cover this in Task 1.
- An override must be derived correctly in every evaluation mode, including a negative BigInt modulo a positive modulus; cover this in Task 3.
- Replacing a GridSpec must not make a retained override bypass the new depth restriction; cover this in Task 3.
- A cancelled Engine must prevent KEVIN from publishing a computed value; cover this in Task 3.

---

## File Structure

- `include/bruce/lhme/kevin_grid.hpp` — public grid identity, step, address, validation, and `GridSpec` contracts.
- `src/kevin/grid.cpp` — collision-free address packing and GridSpec validation helpers.
- `include/bruce/lhme/kevin_memory_store.hpp` — public store, source provenance, and statistics contract.
- `src/kevin/memory_store.cpp` — sparse Store implementation and override-mode derivation.
- `tests/kevin/kevin_grid_tests.cpp` — isolated address/identity/GridSpec tests.
- `tests/kevin/kevin_memory_store_tests.cpp` — Engine integration, overrides, budgets, cancellation, and inspect tests.
- `demos/lhme/kevin_memory_store_demo.cpp` — observable, non-materializing formula-grid demo.
- `docs/kevin-memory-store.md` — copy-and-adapt API usage and memory-model explanation.
- `docs/architecture.md` — implemented C++ architecture index; replaces the stale missing concept blueprint target.
- `concepts/README.md` — link the concepts index to `docs/architecture.md`.
- `CMakeLists.txt` — add KEVIN sources, isolated KEVIN test target, and the demo CTest target.

### Task 1: Public 8×8 grid identity and collision-free address contract

**Files:**
- Create: `include/bruce/lhme/kevin_grid.hpp`
- Create: `src/kevin/grid.cpp`
- Create: `tests/kevin/kevin_grid_tests.cpp`
- Modify: `CMakeLists.txt`

**Interfaces:**
- Consumes: `FormulaAddress` from `include/bruce/lhme/types.hpp` and `TechniqueIdentity` from `include/bruce/lhme/technique.hpp`.
- Produces: `GridIdentity`, `GridStep`, `GridAddress`, `GridSpec`, `valid_grid_identity`, `valid_grid_spec`, and `GridAddress::formula_address()` for Tasks 2–4.

- [ ] **Step 1: Write failing address and identity contract tests**

  Add tests for valid identity `"bruce.grid"` version `1`; invalid empty, version-zero, and invalid-character identities; valid steps `(0,0)` and `(7,7)`; rejection of coordinate `8`; and depth `11` rejection.

  Add exact mapping assertions:

  ```cpp
  CHECK(GridAddress::from_steps({})->formula_address().index == 0);
  CHECK(GridAddress::from_steps({{0, 0}})->formula_address().index == 1);
  CHECK(GridAddress::from_steps({{0, 0}, {0, 0}})->formula_address().index == 65);
  CHECK(GridAddress::from_steps({{7, 7}})->formula_address().index == 64);
  ```

  Also assert that a depth-10 all-`(7,7)` path converts successfully and is nonnegative.

- [ ] **Step 2: Run the focused test to verify it fails**

  Run: `cmake -S . -B build && cmake --build build --target kevin_grid_tests && ctest --test-dir build -R '^kevin_grid_tests$' --output-on-failure`

  Expected: FAIL because the KEVIN grid header, source, and target do not exist.

- [ ] **Step 3: Implement the immutable public grid types and packing rule**

  In `kevin_grid.hpp`, define:

  ```cpp
  struct GridIdentity { std::string id; std::uint32_t version = 0; };
  struct GridStep { std::uint8_t x = 0; std::uint8_t y = 0; };
  class GridAddress {
   public:
    static std::optional<GridAddress> from_steps(std::span<const GridStep>);
    [[nodiscard]] std::size_t depth() const noexcept;
    [[nodiscard]] const std::vector<GridStep>& steps() const noexcept;
    [[nodiscard]] FormulaAddress formula_address() const noexcept;
  };
  struct GridSpec {
    GridIdentity identity;
    TechniqueIdentity technique;
    std::string canonical_parameters;
    std::uint8_t max_depth = 0;
    Budget default_budget = Budget::unlimited();
  };
  ```

  `GridAddress::from_steps` returns `std::nullopt` for a coordinate above 7 or more than 10 steps. `formula_address()` uses `depth_offset(d) + path_value`, with `path_value` built left-to-right from `(y << 3) | x`. Validate Grid IDs with the same 1–128 character ASCII alphabet as technique IDs; `valid_grid_spec` also requires a valid technique identity, nonempty parameters, and `max_depth <= 10`.

- [ ] **Step 4: Register source files and the isolated test target in CMake**

  Add `src/kevin/grid.cpp` to `bruce_lhme_core`. Create `kevin_grid_tests` from `tests/support/test_main.cpp` and `tests/kevin/kevin_grid_tests.cpp`, link it to `bruce_lhme_core`, and register CTest name `kevin_grid_tests`.

- [ ] **Step 5: Run the focused test to verify it passes**

  Run: `cmake -S . -B build && cmake --build build --target kevin_grid_tests && ctest --test-dir build -R '^kevin_grid_tests$' --output-on-failure`

  Expected: PASS; exact values `0`, `1`, `64`, and `65` prove depth-aware collision freedom.

- [ ] **Step 6: Record Task 1 evidence**

  Create or update `.superpowers/sdd/2026-10-21-kevin-memory-store-v14/progress.md` with the red/green commands and observed result. No Git commit is possible in this workspace.

### Task 2: Lazy Store dispatch through the existing Engine

**Files:**
- Create: `include/bruce/lhme/kevin_memory_store.hpp`
- Create: `src/kevin/memory_store.cpp`
- Create: `tests/kevin/kevin_memory_store_tests.cpp`
- Modify: `CMakeLists.txt`

**Interfaces:**
- Consumes: Task 1 `GridIdentity`, `GridAddress`, `GridSpec`, `valid_grid_spec`; `Engine::evaluate_technique`.
- Produces: `KevinValueSource`, `KevinValue`, `KevinStoreStatistics`, `KevinMemoryStore::register_grid`, `KevinMemoryStore::get`, and `KevinMemoryStore::inspect` for Tasks 3–4.

- [ ] **Step 1: Write failing Engine-integration tests**

  Instantiate an Engine with a finite cache, register `PowerOffsetKernel` and `FibonacciKernel`, then register corresponding GridSpecs. Assert:

  ```cpp
  CHECK(store.get(power_grid, address_10, EvaluationMode::Exact).result.exact_integer()
        == 102300);
  CHECK(store.get(fibonacci_grid, address_10, EvaluationMode::Metric)
            .result.metric_bit_length == 7);
  CHECK(store.get(power_grid, address_10, EvaluationMode::Modular,
                  Budget::unlimited(), BigInt{17})
            .result.exact_integer() == 11);
  ```

  Assert every normal result has `source == KevinValueSource::Computed`, an unknown grid returns `InvalidArgument`, and an address deeper than `GridSpec::max_depth` is rejected before Engine dispatch.

- [ ] **Step 2: Run the focused test to verify it fails**

  Run: `cmake -S . -B build && cmake --build build --target kevin_memory_store_tests && ctest --test-dir build -R '^kevin_memory_store_tests$' --output-on-failure`

  Expected: FAIL because the store API and target do not exist.

- [ ] **Step 3: Implement store registration, computed reads, and inspection**

  In `kevin_memory_store.hpp`, define:

  ```cpp
  enum class KevinValueSource { Override, Computed };
  struct KevinValue { EvaluationResult result; KevinValueSource source; };
  struct KevinStoreStatistics {
    std::size_t registered_grid_count = 0;
    std::size_t override_count = 0;
    std::uint64_t computed_exact_evaluation_count = 0;
    CacheStatistics engine_cache;
  };
  class KevinMemoryStore {
   public:
    explicit KevinMemoryStore(Engine& engine);
    bool register_grid(GridSpec spec);
    KevinValue get(const GridIdentity&, const GridAddress&, EvaluationMode,
                   Budget, std::optional<BigInt> modulus = std::nullopt);
    [[nodiscard]] KevinStoreStatistics inspect() const;
  };
  ```

  Use an ordered map keyed by a length-delimited `GridIdentity` key and preserve each `GridSpec` verbatim. `register_grid` returns false for an invalid spec or a technique that the Engine has not registered; otherwise replacement of the same identity is allowed. `get` validates identity and address depth, combines every `Budget` limit by `min`, and constructs one `TechniqueRequest` using `address.formula_address()`. It increments `computed_exact_evaluation_count` only when dispatching an Exact request without an override, and returns the Engine result without adding a second cache.

- [ ] **Step 4: Add the store source and test target to CMake**

  Add `src/kevin/memory_store.cpp` to `bruce_lhme_core`. Create and register `kevin_memory_store_tests` from the test support main and the Task 2 test source.

- [ ] **Step 5: Run focused and compatibility tests**

  Run: `cmake -S . -B build && cmake --build build --target kevin_memory_store_tests technique_tests engine_tests && ctest --test-dir build -R '^(kevin_memory_store_tests|technique_tests|engine_tests)$' --output-on-failure`

  Expected: PASS; KEVIN uses the public Engine kernel path without changing the existing technique or Engine behavior.

- [ ] **Step 6: Record Task 2 evidence**

  Append the commands and results to the V1.4 progress ledger.

### Task 3: Sparse overrides, derived modes, and fail-closed resource behavior

**Files:**
- Modify: `include/bruce/lhme/kevin_memory_store.hpp`
- Modify: `src/kevin/memory_store.cpp`
- Modify: `tests/kevin/kevin_memory_store_tests.cpp`

**Interfaces:**
- Consumes: Task 2 store registration, lookup, budget combination, and provenance types.
- Produces: `KevinMemoryStore::set_override` and `remove_override`; complete V1.4 override behavior for Task 4.

- [ ] **Step 1: Write failing override and resource-contract tests**

  Register a power grid, set override `-42` for one valid address, and prove the result is `Override` in all modes:

  ```cpp
  CHECK(exact.result.exact_integer() == -42);
  CHECK(metric.result.metric_bit_length == 6);
  CHECK(modular.result.exact_integer() == 9); // -42 mod 17
  CHECK(symbolic.result.symbolic_expression == "override(-42)");
  ```

  Test that `remove_override` restores the computed value. Test that a default GridSpec bit limit rejects an oversized `set_override`; that a stricter caller Exact budget rejects an otherwise stored override with `BudgetExceeded`; that replacing a GridSpec with a smaller `max_depth` makes an old deeper override inaccessible; and that `Engine::cancel()` produces `Cancelled` for a computed `get` with no payload.

- [ ] **Step 2: Run the focused test to verify it fails**

  Run: `cmake --build build --target kevin_memory_store_tests && ctest --test-dir build -R '^kevin_memory_store_tests$' --output-on-failure`

  Expected: FAIL because override methods and source-specific derivation do not exist.

- [ ] **Step 3: Implement sparse exact overrides without a second result cache**

  Add:

  ```cpp
  Status set_override(const GridIdentity&, const GridAddress&, BigInt);
  bool remove_override(const GridIdentity&, const GridAddress&);
  ```

  Key overrides by the same canonical Grid identity key plus packed formula index. Reject unknown grids, addresses beyond the active `max_depth`, and values whose bit length exceeds the GridSpec default bit limit. In `get`, check the override map before creating `TechniqueRequest`.

  For an override, derive Symbolic as `override(<decimal value>)`, Metric as the absolute-value bit length (`0` for zero), Modular as a nonnegative remainder for a required positive modulus, and Exact as the original BigInt. Apply the effective caller/Grid bit and temporary-byte limits to Exact before publishing. Each override result must carry a `Branch` trace event with reason `"kevin override applied"`. Never insert a derived override result into Engine cache.

- [ ] **Step 4: Run focused tests to verify they pass**

  Run: `cmake --build build --target kevin_memory_store_tests && ctest --test-dir build -R '^kevin_memory_store_tests$' --output-on-failure`

  Expected: PASS; overrides win only for their exact address and all derived modes are deterministic.

- [ ] **Step 5: Run the full regression suite**

  Run: `cmake -S . -B build && cmake --build build && ctest --test-dir build --output-on-failure`

  Expected: PASS; all legacy tests and both KEVIN test targets pass.

- [ ] **Step 6: Record Task 3 evidence**

  Append red/green and full-suite evidence to the V1.4 progress ledger.

### Task 4: Observable demo, public documentation, and corrected architecture index

**Files:**
- Create: `demos/lhme/kevin_memory_store_demo.cpp`
- Create: `docs/kevin-memory-store.md`
- Create: `docs/architecture.md`
- Modify: `concepts/README.md`
- Modify: `CMakeLists.txt`

**Interfaces:**
- Consumes: complete Task 3 public KEVIN API.
- Produces: a runnable proof that a deep formula grid does not materialize values until read, plus current documentation.

- [ ] **Step 1: Write a failing demo assertion around non-materialization**

  Follow the existing `demos/lhme/*_demo.cpp` Report pattern. Before calling `get`, register one depth-10 Fibonacci or PowerOffset GridSpec and assert:

  ```cpp
  CHECK(store.inspect().registered_grid_count == 1);
  CHECK(store.inspect().override_count == 0);
  CHECK(store.inspect().engine_cache.resident_entries == 0);
  ```

  Then call Metric and Exact for one address, report source, status, index, cache statistics, and exact-evaluation count separately. Do not claim a universal byte-level compression ratio.

- [ ] **Step 2: Run the demo target to verify it fails**

  Run: `cmake -S . -B build && cmake --build build --target lhme_kevin_memory_store_demo && ctest --test-dir build -R '^lhme_kevin_memory_store_demo$' --output-on-failure`

  Expected: FAIL because the demo target and source do not exist.

- [ ] **Step 3: Implement the observable demo and register it in CMake**

  Add `lhme_kevin_memory_store_demo`, link it to `bruce_lhme_core`, and register CTest `lhme_kevin_memory_store_demo`. Use the public KEVIN header only. The demo must show a depth-10 address, whether it was computed or overridden, and bounded cache statistics; it must not allocate a reference table or print ABI `sizeof` figures as memory evidence.

- [ ] **Step 4: Document the API and current architecture boundary**

  In `docs/kevin-memory-store.md`, include build-independent example code for registration, one lazy `get`, one override, and the formula-versus-override memory model. State the conditions for deterministic reconstruction: Grid identity/version, registered technique identity/version, canonical parameters, complete path, and semantic algorithm stability.

  In `docs/architecture.md`, list the implemented C++ components (Engine, kernels, cache, streaming, KEVIN) separately from concepts (NanoCell, semantic routing, AnswerPoints, persistence, GPU). Update `concepts/README.md` to link to this real document rather than the missing `architecture/03-blueprint-consolidation.md`.

- [ ] **Step 5: Run the demo, full suite, and staged install check**

  Run: `cmake -S . -B build && cmake --build build && ./build/lhme_kevin_memory_store_demo && ctest --test-dir build --output-on-failure && cmake --install build --prefix build/install`

  Expected: the demo reports no cache entries before its first read, CTest passes, and installed public headers include both KEVIN headers.

- [ ] **Step 6: Record Task 4 evidence**

  Add final verification output, the installed header check, and deferred scope (persistence, 64-bit filters, GPU) to the V1.4 progress ledger.

## Plan Self-Review

- **Spec coverage:** Tasks 1–3 implement every public model, lazy evaluation, error, budget, trace, deterministic-address, sparse-storage, and RUN_GATE requirement. Task 4 covers the benchmark/demo and documentation requirements. Persistence, 64-bit filters, GPU, and semantic components remain explicitly excluded.
- **Step scan:** Every test uses fixed values and exact expected behavior; every code step names the concrete public type or method it introduces. No task asks for unspecified “edge cases.”
- **Type consistency:** `GridIdentity`, `GridAddress`, `GridSpec`, `KevinValue`, `KevinStoreStatistics`, and all Store method names originate in Tasks 1–2 and are reused consistently later.
- **Review focus:** Each listed risk is covered by the named Task 1 or Task 3 tests.
- **Proportion:** The plan defines boundaries and verification rather than reproducing implementation bodies. The four tasks are sequential because each one exposes the contracts used by the next.
