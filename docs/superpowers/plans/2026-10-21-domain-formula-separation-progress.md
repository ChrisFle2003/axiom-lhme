# SDD ledger — plan: docs/superpowers/plans/2026-10-21-domain-formula-separation.md

## Pre-flight ruling

The workspace is not a Git repository. The plan's normal isolated task
workspace and commit checkpoints are therefore unavailable; this ledger is
kept beside the approved plan and every task records its red/green evidence
here instead.

## Dependency handoff

- Task 1 produces pure `GridDomainSpec`, `GridAddress`, and explicit
  `DomainProjection` contracts.
- Task 2 consumes those contracts for domain filters and needs no implicit
  grid-to-formula conversion.
- Task 3 adds formula-bound sparse overrides, which Task 4 adapts at the
  legacy KEVIN boundary.
- Tasks 5–7 rename policy/snapshots and separate demonstrations and
  benchmarks.
- Task 8 verifies the public package and complete regression suite.

## Task status

- [x] Task 1 — pure KEVIN domain and projection contracts
  - RED: `kevin_domain_tests` failed because the new public headers were absent.
  - GREEN: `ctest --test-dir build -R '^(kevin_domain_tests|kevin_grid_tests)$' --output-on-failure` passed (2/2).
- [x] Task 2 — explicit domain evaluation context
  - RED: `kevin_filter_tests` failed because `FormulaEvaluator` did not exist.
  - GREEN: `ctest --test-dir build -R '^(kevin_filter_tests|lhme_kevin_grid_filter_demo)$' --output-on-failure` passed (2/2).
- [x] Task 3 — formula-bound sparse override store
  - RED: `formula_override_store_tests` failed because the public store header was absent.
  - GREEN: `ctest --test-dir build -R '^formula_override_store_tests$' --output-on-failure` passed (5/5).
- [x] Task 4 — legacy KEVIN store compatibility boundary
  - GREEN: `ctest --test-dir build -R '^(kevin_memory_store_tests|formula_override_store_tests)$' --output-on-failure` passed (2/2).
- [x] Task 5 — storage-policy vocabulary
  - RED: `storage_policy_tests` failed because `FormulaWithOverrides` was absent.
  - GREEN: `ctest --test-dir build -R '^storage_policy_tests$' --output-on-failure` passed (5/5).
- [x] Task 6 — versioned formula-override snapshots
  - RED: `formula_override_snapshot_tests` failed because the new snapshot header was absent.
  - GREEN: `ctest --test-dir build -R '^(formula_override_snapshot_tests|kevin_snapshot_tests)$' --output-on-failure` passed (2/2).
- [x] Task 7 — demos, benchmarks, and documentation
  - RED: `table_storage_tests` failed because `FormulaOverrideTable` was absent.
  - GREEN: `ctest --test-dir build -R '^(lhme_table_memory_benchmark|lhme_hybrid_storage_benchmark|lhme_kevin_cpu_benchmark)$' --output-on-failure` passed (3/3).
- [x] Task 8 — public package and full regression verification
  - GREEN: `cmake --build build -j2` completed successfully.
  - GREEN: `ctest --test-dir build --output-on-failure` passed all 35 tests, including package layout and external-consumer installation checks.
