# LHME Fair Algorithm Benchmarks Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Add reproducible, fair CPU/RAM benchmark tracks for FormulaRecipe storage, ordinary materialized containers, a BSGS toy problem, and a non-security precomputed-chain toy.

**Architecture:** A shared internal benchmark contract creates deterministic workloads, computes timing summaries and verifies checksums. Track A uses adapters that expose the same modular FormulaRecipe scalar through FormulaRecipe, sparse overrides, dense, sorted and hash representations. Tracks B and C have their own toy-problem headers and executables, use generated public data only, and print an explicit non-comparability marker.

**Tech Stack:** C++20, existing BRUCE LHME Engine and FormulaOverrideStore, CMake/CTest, `std::chrono`, Linux `fork`/`getrusage` where peak-RAM isolation is available.

**Spec:** `docs/superpowers/specs/2026-10-21-lhme-fair-algorithm-benchmark-design.md`

## Global Constraints

- Benchmark code remains under `demos/lhme/benchmarks/` and is not part of the installed public library API.
- Track A uses the common scalar `h(i) mod M`, where `h(i) = 100 * (2^i - 1)` and `M = 2^61 - 1 = 2,305,843,009,213,693,951`.
- Track A contains `FormulaRecipe`, `FormulaRecipe + Overrides`, `DenseVector`, `SortedPairs`, and `HashTable`; it uses identical address sequences and validates results before recording a measurement.
- Domains are 1,024; 65,536; and 1,000,000 addresses. Override densities are 0 %, 0.1 %, 1 %, 10 % and 100 %.
- Timed cases have one excluded warm-up followed by exactly eleven samples; reports show median, minimum and maximum, never a total winner.
- A FormulaRecipe cold-read case uses `EngineConfig{.cache_max_entries = 0, .cache_max_bytes = 0}`. A warm-cache case, if added, is separately labelled and never substituted for cold results.
- BSGS uses only the public toy group `(p = 1019, generator = 2)` and known exponent `777`; no password hashes, secrets or real credential material are accepted or generated.
- The precomputed-chain experiment uses only a fixed non-cryptographic `uint16_t` transition over a public 65,536-token domain and is labelled `PrecomputedChainToy`, never `RainbowTable`.
- Isolated process peak RAM is reported on Linux; unsupported platforms print `unavailable`.
- No cross-track ranking, total score, universal-winner label or security claim is permitted.

## Review Focus

- Cache isolation: Track A cold FormulaRecipe results must not carry entries from the warm-up or previous strategies; Task 2 tests zero cache residency before and after a cold workload.
- Full override crossover: 100 % override layouts must return the same scalars as materialized strategies and must be reported as `100-percent`, never as sparse; Task 2 tests it.
- Invalid benchmark inputs: zero domain, a zero modulus, out-of-range query addresses and unsupported override counts must fail deterministically before timing; Task 1 tests it.
- Non-generator/out-of-subgroup BSGS targets: BSGS must return no answer rather than a false exponent; Task 4 tests it with a target outside the generated subgroup.
- Chain collision and cycle accounting: the precomputed-chain toy must report only verified resolutions and must not count a shared endpoint as proof of coverage; Task 5 tests it.

## File Structure

| Path | Responsibility |
|---|---|
| `demos/lhme/benchmarks/fair/benchmark_contract.hpp` | shared workload validation, deterministic address streams, checksum and eleven-sample timing summary |
| `demos/lhme/benchmarks/fair/fair_storage.hpp` | Track A FormulaRecipe and materialized strategy adapters |
| `demos/lhme/benchmarks/fair/fair_isolated_memory.hpp` | Linux child-process peak-memory measurement protocol for a named Track A case |
| `demos/lhme/benchmarks/fair/fair_storage_benchmark.cpp` | Track A report and command-line mode selection |
| `demos/lhme/benchmarks/fair/bsgs_toy.hpp` | bounded public BSGS and linear-scan reference implementations |
| `demos/lhme/benchmarks/fair/bsgs_toy_benchmark.cpp` | Track B report |
| `demos/lhme/benchmarks/fair/precomputed_chain_toy.hpp` | generated non-cryptographic chain-table build, coverage and verified lookup |
| `demos/lhme/benchmarks/fair/precomputed_chain_toy_benchmark.cpp` | Track C report |
| `tests/fair_benchmark_contract_tests.cpp` | deterministic workload, validation, scalar-equality and timing-summary contracts |
| `tests/bsgs_toy_tests.cpp` | Track B correctness and failure contracts |
| `tests/precomputed_chain_toy_tests.cpp` | Track C coverage, collision and resolution contracts |
| `CMakeLists.txt` | build and CTest registration for the three executables and three unit-test targets |
| `docs/benchmark-review-2026-10-21.md` | first full-run data, method, raw parameters and limitations |

### Task 1: Shared fair-benchmark contract

**Files:**
- Create: `demos/lhme/benchmarks/fair/benchmark_contract.hpp`
- Create: `tests/fair_benchmark_contract_tests.cpp`
- Modify: `CMakeLists.txt`

**Interfaces:**
- Produces `FairBenchmarkConfig`, `WorkloadKind`, `make_workload(...)`, `TimingSummary`, `summarize_samples(...)` and `validate_config(...)` for Tasks 2–6.
- Uses no later-task symbols.

- [ ] **Step 1: Write failing contract tests**

Add tests named `fair_workloads_are_repeatable_and_in_range`,
`fair_config_rejects_invalid_domain_modulus_and_override_count`, and
`eleven_sample_summary_reports_ordered_minimum_median_and_maximum`.
Use `record_count = 1'024`, `query_count = 257`, seed `0x4252554345ULL`,
modulus `2'305'843'009'213'693'951ULL`, and the sample list
`{11, 5, 9, 3, 7, 1, 13, 15, 17, 19, 21}` so the expected median is `11`.

- [ ] **Step 2: Run the new test target and verify failure**

Run: `cmake --build --preset debug --target fair_benchmark_contract_tests && ctest --test-dir out/build/debug -R '^fair_benchmark_contract_tests$' --output-on-failure`

Expected: build or test registration fails because the contract header and target do not exist.

- [ ] **Step 3: Implement the contract header and CMake target**

Define:

```cpp
enum class WorkloadKind { Sequential, PseudoRandom, RepeatHeavy };
struct FairBenchmarkConfig {
  std::uint64_t record_count;
  std::uint64_t query_count;
  std::uint64_t modulus;
  std::uint64_t override_count;
  std::uint64_t seed;
};
struct TimingSummary {
  std::uint64_t minimum_microseconds;
  std::uint64_t median_microseconds;
  std::uint64_t maximum_microseconds;
};
```

`make_workload` returns exactly `query_count` in-range addresses for every
kind and a deterministic checksum. `validate_config` rejects zero record
count/modulus, out-of-range override count and zero query count. `summarize_samples`
requires exactly eleven samples and sorts a copy to select element five.

- [ ] **Step 4: Run contract tests**

Run: `cmake --build --preset debug --target fair_benchmark_contract_tests && ctest --test-dir out/build/debug -R '^fair_benchmark_contract_tests$' --output-on-failure`

Expected: PASS, 1/1.

- [ ] **Step 5: Commit**

```bash
git add CMakeLists.txt demos/lhme/benchmarks/fair/benchmark_contract.hpp tests/fair_benchmark_contract_tests.cpp
git commit -m "feat: add fair benchmark contract"
```

### Task 2: Same-semantic Track A strategy adapters

**Files:**
- Create: `demos/lhme/benchmarks/fair/fair_storage.hpp`
- Modify: `tests/fair_benchmark_contract_tests.cpp`

**Interfaces:**
- Consumes: `FairBenchmarkConfig`, `WorkloadKind` and `make_workload(...)` from Task 1.
- Produces `StorageStrategyKind`, `FairStorageCase`, `build_storage_case(...)`, `lookup_scalar(...)`, `verify_workload(...)` and `storage_case_label(...)` for Task 3.

- [ ] **Step 1: Write failing equal-value and cache-isolation tests**

Add tests named `all_track_a_strategies_return_the_same_modular_scalars`,
`track_a_full_override_layout_is_correct_and_explicit`, and
`formula_recipe_cold_case_keeps_cache_empty`. Use a 1,024-address domain,
sequential/pseudo-random/repeat-heavy workloads, and densities 0 % and 100 %.
For each address, compare every strategy with direct modular FormulaRecipe
evaluation; assert the cold Engine cache has zero resident entries and bytes.

- [ ] **Step 2: Run tests to verify failure**

Run: `cmake --build --preset debug --target fair_benchmark_contract_tests && ctest --test-dir out/build/debug -R '^fair_benchmark_contract_tests$' --output-on-failure`

Expected: FAIL because Track A adapters are absent.

- [ ] **Step 3: Implement strategy adapters**

`build_storage_case(FairBenchmarkConfig, StorageStrategyKind)` builds all
materialized values from the same `EvaluationMode::Modular` FormulaRecipe
reference. The FormulaRecipe strategy owns an `Engine` configured with both
cache limits zero. The override strategy uses `FormulaOverrideStore` and the
deterministic override layout; its replacement scalar is the same value used
when materialized competitors apply the layout. `lookup_scalar` returns
`std::optional<std::uint64_t>` and rejects absent/out-of-range values rather
than silently substituting a result.

- [ ] **Step 4: Run tests to verify pass**

Run: `cmake --build --preset debug --target fair_benchmark_contract_tests && ctest --test-dir out/build/debug -R '^fair_benchmark_contract_tests$' --output-on-failure`

Expected: PASS, 1/1.

- [ ] **Step 5: Commit**

```bash
git add demos/lhme/benchmarks/fair/fair_storage.hpp tests/fair_benchmark_contract_tests.cpp
git commit -m "feat: add fair storage strategy adapters"
```

### Task 3: Track A report and isolated RAM measurement

**Files:**
- Create: `demos/lhme/benchmarks/fair/fair_isolated_memory.hpp`
- Create: `demos/lhme/benchmarks/fair/fair_storage_benchmark.cpp`
- Modify: `CMakeLists.txt`

**Interfaces:**
- Consumes: Task 1 workload/timing interfaces and Task 2 storage adapters.
- Produces executable `lhme_fair_storage_benchmark` with `--quick` and `--full` modes.

- [ ] **Step 1: Write failing executable-contract tests**

Extend `tests/fair_benchmark_contract_tests.cpp` with a report-contract helper
test that asserts Track A labels contain the exact strategy names, workload
kind, `not a global ranking`, sample count `11`, and either `peak RAM bytes`
or `peak RAM unavailable`. Assert 100 % is labelled `100-percent-overrides`.

- [ ] **Step 2: Run test to verify failure**

Run: `cmake --build --preset debug --target fair_benchmark_contract_tests && ctest --test-dir out/build/debug -R '^fair_benchmark_contract_tests$' --output-on-failure`

Expected: FAIL because the Track A report contract is absent.

- [ ] **Step 3: Implement isolated measurement and report executable**

Implement `fair_isolated_memory.hpp` with the existing Linux pipe/fork/wait
pattern from `table_memory_benchmark.cpp`; the child builds exactly one named
storage case, verifies its checksum, then reports its own `getrusage` peak.
On non-Linux, return `{.available = false}`.

`lhme_fair_storage_benchmark --quick` uses 1,024 addresses and all three
workload kinds; `--full` runs all specified domain sizes and densities.
For each case: warm up once without sampling, execute eleven checksum-verified
samples, summarize them, then print build time, min/median/max lookup time,
isolated peak RAM, domain, density, workload seed and non-ranking marker.
Unknown arguments fail with exit code 1.

- [ ] **Step 4: Register and verify the quick executable**

Run: `cmake --build --preset debug --target lhme_fair_storage_benchmark && ctest --test-dir out/build/debug -R '^lhme_fair_storage_benchmark$' --output-on-failure`

Expected: PASS, 1/1. Register the executable as CTest with `--quick`.

- [ ] **Step 5: Commit**

```bash
git add CMakeLists.txt demos/lhme/benchmarks/fair/fair_isolated_memory.hpp demos/lhme/benchmarks/fair/fair_storage_benchmark.cpp tests/fair_benchmark_contract_tests.cpp
git commit -m "feat: add fair storage benchmark"
```

### Task 4: Track B bounded BSGS toy

**Files:**
- Create: `demos/lhme/benchmarks/fair/bsgs_toy.hpp`
- Create: `demos/lhme/benchmarks/fair/bsgs_toy_benchmark.cpp`
- Create: `tests/bsgs_toy_tests.cpp`
- Modify: `CMakeLists.txt`

**Interfaces:**
- Consumes: `TimingSummary` from Task 1.
- Produces `linear_discrete_log_toy(...)`, `baby_step_giant_step_toy(...)` and executable `lhme_bsgs_toy_benchmark`.

- [ ] **Step 1: Write failing BSGS tests**

Create tests `bsgs_toy_recovers_known_public_exponent` and
`bsgs_toy_rejects_target_outside_generated_subgroup`. The first uses
`p = 1019`, `generator = 2`, `exponent = 777` and checks both BSGS and linear
scan return `777`. The second passes a target not generated by the supplied
base/order contract and expects `std::nullopt`.

- [ ] **Step 2: Run test to verify failure**

Run: `cmake --build --preset debug --target bsgs_toy_tests && ctest --test-dir out/build/debug -R '^bsgs_toy_tests$' --output-on-failure`

Expected: build or test registration fails because the BSGS toy does not exist.

- [ ] **Step 3: Implement bounded BSGS and report**

Implement overflow-safe modular multiplication/exponentiation for values below
1,019, then a `ceil(sqrt(order))` baby-step table and the giant-step lookup.
Verify a candidate exponent before returning it. The executable runs eleven
samples for both BSGS and linear scan, verifies the known exponent, reports
their independent time/RAM data, and prints `not comparable to Track A`.

- [ ] **Step 4: Register and run tests**

Run: `cmake --build --preset debug --target bsgs_toy_tests lhme_bsgs_toy_benchmark && ctest --test-dir out/build/debug -R '^(bsgs_toy_tests|lhme_bsgs_toy_benchmark)$' --output-on-failure`

Expected: PASS, 2/2.

- [ ] **Step 5: Commit**

```bash
git add CMakeLists.txt demos/lhme/benchmarks/fair/bsgs_toy.hpp demos/lhme/benchmarks/fair/bsgs_toy_benchmark.cpp tests/bsgs_toy_tests.cpp
git commit -m "feat: add bounded bsgs toy benchmark"
```

### Task 5: Track C synthetic precomputed chains

**Files:**
- Create: `demos/lhme/benchmarks/fair/precomputed_chain_toy.hpp`
- Create: `demos/lhme/benchmarks/fair/precomputed_chain_toy_benchmark.cpp`
- Create: `tests/precomputed_chain_toy_tests.cpp`
- Modify: `CMakeLists.txt`

**Interfaces:**
- Consumes: `TimingSummary` from Task 1.
- Produces `PrecomputedChainToy`, `build_precomputed_chain_toy(...)`, `resolve_toy_token(...)` and executable `lhme_precomputed_chain_toy_benchmark`.

- [ ] **Step 1: Write failing chain-table tests**

Create tests `precomputed_chain_toy_resolves_known_generated_token` and
`precomputed_chain_toy_reports_collisions_without_false_resolution`. Use the
public 65,536-token domain, 128 deterministic starts and chain length 64.
Require resolution to replay the chain and match the original token; a shared
endpoint alone must not produce a success.

- [ ] **Step 2: Run test to verify failure**

Run: `cmake --build --preset debug --target precomputed_chain_toy_tests && ctest --test-dir out/build/debug -R '^precomputed_chain_toy_tests$' --output-on-failure`

Expected: build or test registration fails because the toy chain code is absent.

- [ ] **Step 3: Implement synthetic chain table and report**

Use a fixed non-cryptographic `uint16_t` transition with documented constants;
store only generated start/endpoints and replay chains when resolving a token.
Return a result that distinguishes verified resolution, collision candidate and
miss. The executable reports chain count, length, endpoint bytes, observed
coverage, lookup work, collision candidates and verified resolutions; it prints
`synthetic non-security toy` and `not comparable to Track A`.

- [ ] **Step 4: Register and run tests**

Run: `cmake --build --preset debug --target precomputed_chain_toy_tests lhme_precomputed_chain_toy_benchmark && ctest --test-dir out/build/debug -R '^(precomputed_chain_toy_tests|lhme_precomputed_chain_toy_benchmark)$' --output-on-failure`

Expected: PASS, 2/2.

- [ ] **Step 5: Commit**

```bash
git add CMakeLists.txt demos/lhme/benchmarks/fair/precomputed_chain_toy.hpp demos/lhme/benchmarks/fair/precomputed_chain_toy_benchmark.cpp tests/precomputed_chain_toy_tests.cpp
git commit -m "feat: add synthetic chain toy benchmark"
```

### Task 6: Full run and public benchmark review

**Files:**
- Modify: `docs/benchmark-review-2026-10-21.md`
- Modify: `README.md`

**Interfaces:**
- Consumes the three executables from Tasks 3–5.
- Produces the first published fair-comparison review with raw parameters and no global ranking.

- [ ] **Step 1: Write a failing documentation contract test**

Add a small CMake script `tests/fair_benchmark_docs_test.cmake` and register it
as `fair_benchmark_docs_test`. It must require headings for Tracks A, B and C,
the phrase `not comparable to Track A`, the fixed modulus, eleven samples,
the non-security toy boundary and a limitations section in the review.

- [ ] **Step 2: Run test to verify failure**

Run: `cmake --build --preset debug && ctest --test-dir out/build/debug -R '^fair_benchmark_docs_test$' --output-on-failure`

Expected: FAIL because the first fair-comparison run has not been documented.

- [ ] **Step 3: Run the complete release benchmark matrix and update documentation**

Run:

```bash
cmake --build --preset release --parallel 2
ctest --test-dir out/build/release -R '^(fair_benchmark_contract_tests|lhme_fair_storage_benchmark|bsgs_toy_tests|lhme_bsgs_toy_benchmark|precomputed_chain_toy_tests|lhme_precomputed_chain_toy_benchmark)$' --output-on-failure
./out/build/release/lhme_fair_storage_benchmark --full
./out/build/release/lhme_bsgs_toy_benchmark
./out/build/release/lhme_precomputed_chain_toy_benchmark
```

Copy the measured parameters and raw results to the existing review. State
that Track A, B and C are separate; report no total winner. Add a README link
to the fair-comparison section.

- [ ] **Step 4: Run the documentation test and full relevant suite**

Run: `cmake --build --preset release --parallel 2 && ctest --test-dir out/build/release --output-on-failure`

Expected: all tests pass, including the three new benchmark executables and
`fair_benchmark_docs_test`.

- [ ] **Step 5: Commit**

```bash
git add CMakeLists.txt README.md docs/benchmark-review-2026-10-21.md tests/fair_benchmark_docs_test.cmake
git commit -m "docs: publish fair benchmark review"
```

## Plan self-review

- **Spec coverage:** Task 1 implements repeatability, configuration validation
  and eleven-sample statistics. Tasks 2–3 implement equal-value Track A,
  cold-cache isolation, workloads, Linux peak RAM and non-ranking reports.
  Tasks 4–5 isolate BSGS and generated chains from Track A. Task 6 creates the
  required public review and full-run evidence.
- **Interface consistency:** all Track A consumers use Task 1's
  `FairBenchmarkConfig` and Task 2's `StorageStrategyKind`; Tracks B and C
  only consume `TimingSummary`, avoiding an accidental cross-track value API.
- **Review focus coverage:** cache isolation and 100 % overrides are tested in
  Task 2; invalid inputs in Task 1; BSGS false answers in Task 4; chain
  collisions in Task 5.
- **Scope:** the plan excludes production crypto, password material, GPU and
  public API changes. The six tasks each end in a testable deliverable.
