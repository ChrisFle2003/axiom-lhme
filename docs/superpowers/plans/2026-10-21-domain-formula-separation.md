# Domain–Formula Separation Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Separate BRUCE's formula-value layer, KEVIN 8×8 domain layer, sparse overrides and storage-policy vocabulary without changing the C Formula API.

**Architecture:** `FormulaSpec`/`TechniqueKernel` and `FormulaAddress` remain the only value generator. KEVIN becomes a domain coordinate and filter layer; an explicit versioned projection maps a `GridAddress` to a formula address only when needed. A new formula-bound override store replaces KEVIN as the storage representation for rare explicit values.

**Tech Stack:** C++20, Boost.Multiprecision, CMake/CTest, Linux isolated-process benchmarks.

**Spec:** `docs/superpowers/specs/2026-10-21-domain-formula-separation-design.md`

## Global Constraints

- CPU and RAM only; no GPU or new external dependency.
- Preserve deterministic, budgeted, traceable evaluation and existing C Formula ABI.
- `ResultCache` remains a bounded accelerator, never a source of truth.
- A domain coordinate is not implicitly a formula address or value.
- Direct formula evaluation must work without KEVIN objects.
- Old KEVIN snapshots must never be silently reinterpreted as new override snapshots.
- This workspace is not a Git repository; record verification in the final status instead of making commits.

## Review Focus

- Projection absent: formula evaluation through a domain must reject with `InvalidArgument`, never infer the old path mapping. Covered in Task 2.
- Same grid address in distinct domains/projections: results must remain isolated. Covered in Task 2.
- Override identity: a value attached to one formula/technique binding must not override another. Covered in Task 3.
- Threshold boundary: exactly the configured sparse percentage remains formula-with-overrides; the next address selects a materialized representation. Covered in Task 5.
- Legacy snapshot input: the new loader must reject the old KEVIN magic rather than accepting a mismatched schema. Covered in Task 6.

---

## File Structure

| File | Responsibility |
|---|---|
| `include/bruce/lhme/kevin_domain.hpp` | Pure 8×8 domain identity, coordinates and domain limits. |
| `include/bruce/lhme/domain_projection.hpp` | Versioned explicit bridge from grid to formula address. |
| `src/kevin/domain_projection.cpp` | Canonical `DepthPathProjection` implementation. |
| `include/bruce/lhme/formula_evaluator.hpp` | Small formula-evaluation interface consumed by domain filters. |
| `include/bruce/lhme/formula_override_store.hpp` | Formula-bound sparse override API and value provenance. |
| `src/formula_override_store.cpp` | Override lookup, engine delegation and synchronization. |
| `include/bruce/lhme/formula_override_snapshot.hpp` | New override snapshot API. |
| `src/formula_override_snapshot.cpp` | New versioned snapshot encoding/decoding. |
| `include/bruce/lhme/kevin_filter.hpp` | Domain filter API migrated to explicit projection/evaluator context. |
| `src/kevin/filter.cpp` | Filter implementation using the new context. |
| `include/bruce/lhme/storage_policy.hpp` | Value-representation policy with `FormulaWithOverrides`. |
| `demos/lhme/benchmarks/*` | Separate formula/override measurements from optional domain overhead. |
| `tests/` | Unit, integration, snapshot and benchmark contracts. |

### Task 1: Pure KEVIN domain and projection contracts

**Files:**
- Create: `include/bruce/lhme/kevin_domain.hpp`
- Create: `include/bruce/lhme/domain_projection.hpp`
- Create: `src/kevin/domain_projection.cpp`
- Modify: `CMakeLists.txt`
- Test: `tests/kevin/kevin_domain_tests.cpp`

**Interfaces:**
- Produces `GridDomainSpec { GridIdentity identity; std::uint8_t max_depth; Budget default_budget; }`.
- Produces `ProjectionIdentity { std::string id; std::uint32_t version; }`.
- Produces `class DomainProjection` with `identity()` and `project(const GridAddress&) -> std::optional<FormulaAddress>`.
- Produces `class DepthPathProjection`, whose explicit projection is the existing depth-plus-path canonical mapping.

- [ ] **Step 1: Write failing domain/projection tests**

```cpp
TEST(depth_path_projection_maps_the_same_old_path_only_when_requested) {
  const auto address = *GridAddress::from_steps({{.x = 3, .y = 5}});
  DepthPathProjection projection;
  CHECK(projection.project(address)->index == 1 + 43);
}

TEST(two_domain_specs_can_share_a_grid_coordinate) {
  // Same GridAddress, distinct GridIdentity; neither carries a value binding.
}
```

- [ ] **Step 2: Run the new test target to verify it fails**

Run: `cmake --build build --target kevin_domain_tests -j2`

Expected: FAIL because the domain/projection headers and symbols do not exist.

- [ ] **Step 3: Implement the domain and projection contracts**

Move reusable `GridIdentity`, `GridStep` and `GridAddress` definitions into `kevin_domain.hpp`; do not give `GridAddress` a formula-address accessor. Implement `DepthPathProjection::project` with the old canonical mapping and reject paths beyond the supported depth.

- [ ] **Step 4: Preserve source compatibility for the old grid header**

Make `kevin_grid.hpp` include the domain header and retain old names only as deprecated compatibility aliases where possible. Do not keep an implicit conversion from `GridAddress` to `FormulaAddress`.

- [ ] **Step 5: Run the focused test target**

Run: `ctest --test-dir build -R '^(kevin_domain_tests|kevin_grid_tests)$' --output-on-failure`

Expected: PASS.

### Task 2: Explicit domain evaluation context

**Files:**
- Create: `include/bruce/lhme/formula_evaluator.hpp`
- Modify: `include/bruce/lhme/kevin_filter.hpp`
- Modify: `src/kevin/filter.cpp`
- Modify: `tests/kevin/kevin_filter_tests.cpp`
- Modify: `demos/lhme/kevin_grid_filter_demo.cpp`

**Interfaces:**
- Consumes Task 1 `GridDomainSpec` and `DomainProjection`.
- Produces `DomainEvaluationContext { const GridDomainSpec& domain; const DomainProjection* projection; FormulaEvaluator& evaluator; }`.
- `FormulaEvaluator::evaluate(FormulaAddress, EvaluationMode, Budget, std::optional<BigInt>) -> EvaluationResult` is the minimal adapter consumed by filters.

- [ ] **Step 1: Write failing tests for explicit projection behavior**

```cpp
TEST(domain_filter_rejects_a_missing_projection) {
  // Context has projection == nullptr; result.status is InvalidArgument.
}

TEST(domain_filter_uses_the_registered_projection_for_metric_and_modular_queries) {
  // A fake FormulaEvaluator records the FormulaAddress passed by DepthPathProjection.
}

TEST(distinct_projections_map_one_grid_coordinate_to_distinct_formula_addresses) {
  // Two explicit projections receive the same GridAddress and produce different addresses.
}
```

- [ ] **Step 2: Run the focused filter target to verify it fails**

Run: `cmake --build build --target kevin_filter_tests -j2`

Expected: FAIL because the context and evaluator boundary do not exist.

- [ ] **Step 3: Implement `FormulaEvaluator` and migrate filter calls**

Replace direct `KevinMemoryStore::get` dependencies in modular, metric and exact tile functions with `DomainEvaluationContext`. Missing projection or projection failure returns a branch-local `InvalidArgument`; a filter miss remains local.

- [ ] **Step 4: Update the demo to build an explicit `DepthPathProjection` and an Engine-backed evaluator**

The demo must report the same candidate count as before while showing the projection is explicit.

- [ ] **Step 5: Run focused tests and demo**

Run: `ctest --test-dir build -R '^(kevin_filter_tests|lhme_kevin_grid_filter_demo)$' --output-on-failure`

Expected: PASS.

### Task 3: Formula-bound sparse override store

**Files:**
- Create: `include/bruce/lhme/formula_override_store.hpp`
- Create: `src/formula_override_store.cpp`
- Modify: `CMakeLists.txt`
- Test: `tests/formula_override_store_tests.cpp`

**Interfaces:**
- Produces `TechniqueBinding { TechniqueIdentity identity; std::string canonical_parameters; Budget default_budget; }`.
- Produces `FormulaBinding = std::variant<FormulaSpec, TechniqueBinding>`.
- Produces `FormulaOverrideRequest { FormulaBinding binding; FormulaAddress address; EvaluationMode mode; Budget budget; std::optional<BigInt> modulus; }`.
- Produces `FormulaValueSource { Override, Computed }`, `FormulaValue`, and `FormulaOverrideStore(Engine&)`.
- Produces `FormulaOverrideRecord` and `FormulaOverrideSnapshot`.
- Store methods: `set_override(const FormulaBinding&, FormulaAddress, BigInt)`, `remove_override(...)`, `evaluate(const FormulaOverrideRequest&)`, `snapshot()`, `replace_snapshot(const FormulaOverrideSnapshot&)`, `inspect()`.

- [ ] **Step 1: Write failing direct-formula and technique-binding tests**

```cpp
TEST(formula_override_store_evaluates_a_registered_formula_without_kevin) {
  // Exact override wins; a second address delegates to Engine.
}

TEST(formula_override_store_never_crosses_formula_binding_identities) {
  // Same FormulaAddress, different FormulaBinding; only the matching binding sees override.
}
```

- [ ] **Step 2: Run the new store tests to verify they fail**

Run: `cmake --build build --target formula_override_store_tests -j2`

Expected: FAIL because the public store types do not exist.

- [ ] **Step 3: Implement exact binding keys and lazy engine delegation**

Canonicalize a `FormulaSpec` through `canonical_formula_identity`; canonicalize a `TechniqueBinding` from identity version and canonical parameters. Use an internally synchronized ordered map. Derive Symbolic, Metric, Modular and Exact override results with the existing budget rules, otherwise delegate to `Engine::evaluate` or `Engine::evaluate_technique`.

- [ ] **Step 4: Add concurrency and budget regression tests**

Test parallel exact reads, invalid modulus, caller budget below an override bit length and `Engine::cancel()` delegation.

- [ ] **Step 5: Run the focused store tests**

Run: `ctest --test-dir build -R '^formula_override_store_tests$' --output-on-failure`

Expected: PASS.

### Task 4: Keep the old KEVIN store as a deprecated compatibility boundary

**Files:**
- Modify: `include/bruce/lhme/kevin_memory_store.hpp`
- Modify: `src/kevin/memory_store.cpp`
- Modify: `tests/kevin/kevin_memory_store_tests.cpp`
- Modify: `docs/kevin-memory-store.md`

**Interfaces:**
- Consumes Task 3 `FormulaOverrideStore` semantics.
- Keeps existing `KevinMemoryStore` calls buildable for this release.
- Marks the type and snapshot API deprecated in favor of `GridDomainSpec`, `DomainProjection` and `FormulaOverrideStore`.

- [ ] **Step 1: Add deprecation annotations and migration guidance**

Do not alter legacy runtime semantics. Add compiler-visible deprecation only if it does not turn project warnings into failures; otherwise preserve source API and place the deprecation in API documentation. Describe the replacement route precisely.

- [ ] **Step 2: Run legacy and new store tests together**

Run: `ctest --test-dir build -R '^(kevin_memory_store_tests|formula_override_store_tests)$' --output-on-failure`

Expected: PASS.

### Task 5: Correct storage-policy vocabulary and contracts

**Files:**
- Modify: `include/bruce/lhme/storage_policy.hpp`
- Modify: `src/storage_policy.cpp`
- Modify: `tests/storage_policy_tests.cpp`
- Modify: `docs/kevin-memory-store.md`

**Interfaces:**
- Renames strategy value to `FormulaWithOverrides`.
- Keeps `SparseOverrides = FormulaWithOverrides` as a temporary source-compatible enum alias.
- `choose_storage_strategy(StorageDecisionInput, const StoragePolicy&) -> StorageDecision` keeps exact basis-point comparison.

- [ ] **Step 1: Write a failing policy-boundary test**

```cpp
TEST(storage_policy_names_sparse_as_formula_with_overrides) {
  const auto decision = choose_storage_strategy({1'000'000, 50'000,
      AccessPattern::Random}, StoragePolicy{});
  CHECK(decision.strategy == StorageStrategy::FormulaWithOverrides);
}
```

- [ ] **Step 2: Run the policy test to verify it fails**

Run: `cmake --build build --target storage_policy_tests -j2`

Expected: FAIL because `FormulaWithOverrides` is not defined.

- [ ] **Step 3: Implement the renamed strategy and compatibility alias**

Keep the default at 500 basis points, reject invalid input as today and avoid any KEVIN terminology in strategy decisions.

- [ ] **Step 4: Run the policy target**

Run: `ctest --test-dir build -R '^storage_policy_tests$' --output-on-failure`

Expected: PASS.

### Task 6: Versioned formula-override snapshots

**Files:**
- Create: `include/bruce/lhme/formula_override_snapshot.hpp`
- Create: `src/formula_override_snapshot.cpp`
- Modify: `CMakeLists.txt`
- Test: `tests/formula_override_snapshot_tests.cpp`

**Interfaces:**
- Consumes Task 3 `FormulaOverrideStore` and `FormulaBinding`.
- Produces `save_formula_override_snapshot(const FormulaOverrideStore&, const std::filesystem::path&)` and `load_formula_override_snapshot(FormulaOverrideStore&, const std::filesystem::path&)`.
- New on-disk magic: `BRUCE-FORMULA-OVERRIDE-1`.

- [ ] **Step 1: Write failing snapshot round-trip and legacy-rejection tests**

```cpp
TEST(formula_override_snapshot_round_trips_binding_and_value) { /* ... */ }
TEST(formula_override_snapshot_rejects_bruce_kevin_legacy_magic) { /* ... */ }
```

- [ ] **Step 2: Run the new snapshot target to verify it fails**

Run: `cmake --build build --target formula_override_snapshot_tests -j2`

Expected: FAIL because the new snapshot API does not exist.

- [ ] **Step 3: Implement framed, checksummed serialization**

Use a distinct magic and the existing fail-closed parsing conventions. Persist canonical bindings, FormulaAddresses and decimal BigInts. Never call the legacy KEVIN loader from the new loader.

- [ ] **Step 4: Run focused snapshot tests**

Run: `ctest --test-dir build -R '^(formula_override_snapshot_tests|kevin_snapshot_tests)$' --output-on-failure`

Expected: PASS.

### Task 7: Rebaseline demos, benchmarks and public documentation

**Files:**
- Modify: `demos/lhme/benchmarks/hybrid_storage/hybrid_storage_benchmark.cpp`
- Modify: `demos/lhme/benchmarks/table_memory/kevin_sparse_table.hpp`
- Modify: `demos/lhme/benchmarks/table_memory/table_memory_benchmark.cpp`
- Modify: `docs/architecture.md`
- Modify: `docs/kevin-memory-store.md`
- Modify: `docs/kernel-sdk.md` only if a public example needs the new binding terminology

**Interfaces:**
- Consumes Tasks 1–6.
- Benchmark labels are `FormulaOnly`, `FormulaWithOverrides`, `DenseTable` and `HashTable`.
- A separate KEVIN benchmark reports only projection/filter cost; it never presents KEVIN as a value store.

- [ ] **Step 1: Write failing benchmark assertions**

Add checks that direct FormulaRecipe + overrides builds without a `GridAddress`, and that the optional domain benchmark reports a projection/filter result separately.

- [ ] **Step 2: Run the affected benchmarks to verify the assertions fail**

Run: `ctest --test-dir build -R '^(lhme_hybrid_storage_benchmark|lhme_table_memory_benchmark|lhme_kevin_cpu_benchmark)$' --output-on-failure`

Expected: FAIL because current benchmarks label the KEVIN store as the value representation.

- [ ] **Step 3: Rework benchmark fixtures and output labels**

Replace `KevinSparseTable` with a formula-bound override fixture. Keep the isolated RAM methodology, but report formula, override and domain overhead as separate measurements. Retain the 0 %, 0.1 %, 1 %, 10 % and optional 100 % exception tiers.

- [ ] **Step 4: Update architecture documentation**

State that KEVIN is a domain/filter layer and FormulaRecipe is the value generator. Remove statements implying that a GridSpec is required to evaluate a formula override.

- [ ] **Step 5: Run all benchmark tests**

Run: `ctest --test-dir build -R '^(lhme_hybrid_storage_benchmark|lhme_table_memory_benchmark|lhme_kevin_cpu_benchmark)$' --output-on-failure`

Expected: PASS.

### Task 8: Package and full-regression verification

**Files:**
- Modify: `CMakeLists.txt`
- Modify: `tests/kernel_sdk_contract_tests.cpp` only if needed to include new public headers
- Modify: `examples/external_consumer/CMakeLists.txt` and `examples/external_consumer/main.cpp` to compile the public FormulaOverrideStore path

**Interfaces:**
- Consumes Tasks 1–7.
- The installed package exports all new public headers and links an external consumer using only `find_package`.

- [ ] **Step 1: Add a failing external-consumer use of `FormulaOverrideStore`**

Extend the example to create a registered FormulaSpec, set one override and query a second formula address without any KEVIN object.

- [ ] **Step 2: Run package consumer verification to verify it fails**

Run: `ctest --test-dir build -R '^lhme_external_consumer$' --output-on-failure`

Expected: FAIL until new sources/headers are included by the installed package.

- [ ] **Step 3: Wire all sources, tests and installation rules into CMake**

Keep the library target name and C++20 requirement unchanged. Do not add dependencies.

- [ ] **Step 4: Build and run the full suite**

Run: `cmake --build build -j2 && ctest --test-dir build --output-on-failure`

Expected: PASS with all existing and new tests, benchmarks and package-consumer checks.

