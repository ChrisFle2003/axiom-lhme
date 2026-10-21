# KEVIN Snapshot Store V1.5 Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Persist and restore KEVIN GridSpecs plus sparse BigInt overrides with a versioned, fail-closed binary snapshot.

**Architecture:** A snapshot codec owns byte encoding, bounds checks and checksum validation. `KevinMemoryStore` exposes an immutable logical snapshot and atomically replaces its logical state only after a complete valid import. Engine cache and formula results never enter the file.

**Tech Stack:** C++20, standard filesystem/streams, Boost.Multiprecision, CMake, existing KEVIN tests.

**Spec:** `docs/superpowers/specs/2026-10-21-kevin-snapshot-store-v15-design.md`

## Global Constraints

- Format version is `1`, magic is `BRUCE-KEVIN`, byte order is little-endian.
- Persist only GridSpecs and exact overrides; never Engine cache or regular computed values.
- Validate the full payload before mutating a Store; invalid input returns an error and preserves its prior state.
- Save through a temporary sibling then rename; a failed save preserves an existing target.
- No network, encryption, database, GPU, incremental snapshot, or new third-party dependency.
- Keep tests in `tests/kevin/`; workspace has no Git, so use a V1.5 SDD ledger instead of commits.

## Review Focus

- A malicious length/count must fail before allocating or reading beyond bounded input.
- A valid checksum must not make duplicate Grid identities or override keys valid.
- BigInt negative zero and noncanonical decimal input must be rejected.
- A missing Engine technique must reject the import without mutating the target store.
- A corrupted snapshot must leave a populated Store byte-for-byte logically unchanged.

---

### Task 1: Store snapshot view and validated replacement

**Files:**
- Modify: `include/bruce/lhme/kevin_memory_store.hpp`
- Modify: `src/kevin/memory_store.cpp`
- Modify: `tests/kevin/kevin_memory_store_tests.cpp`

**Interfaces:** Produces `KevinSnapshot`, `SnapshotGridRecord`, `SnapshotOverrideRecord`, `KevinMemoryStore::snapshot()` and `replace_snapshot(const KevinSnapshot&) -> SnapshotStatus` for Task 2.

- [ ] Write failing tests proving a store round-trips its GridSpecs and overrides through an in-memory snapshot, and that replacement rejects an unregistered technique while retaining the old value.
- [ ] Run `cmake --build build --target kevin_memory_store_tests && ctest --test-dir build -R '^kevin_memory_store_tests$' --output-on-failure`; expect missing snapshot API failure.
- [ ] Implement snapshot export in canonical map order and replacement through a temporary validated map; reject duplicate identities/override keys and unavailable Engine kernels.
- [ ] Re-run the focused target; expect PASS.

### Task 2: Versioned binary codec and atomic file operations

**Files:**
- Create: `include/bruce/lhme/kevin_snapshot.hpp`
- Create: `src/kevin/snapshot.cpp`
- Create: `tests/kevin/kevin_snapshot_tests.cpp`
- Modify: `CMakeLists.txt`

**Interfaces:** Consumes Task 1 `KevinSnapshot`; produces `SnapshotStatus`, `save_snapshot(const KevinMemoryStore&, const std::filesystem::path&)`, and `load_snapshot(KevinMemoryStore&, const std::filesystem::path&)`.

- [ ] Write failing tests for a full disk roundtrip with two grids and negative override `-42`, plus bad magic, unsupported version, truncation, wrong payload length, and checksum mismatch.
- [ ] Run the focused test and observe failure because `kevin_snapshot.hpp` is absent.
- [ ] Implement bounded little-endian scalar/string helpers, FNV-1a 64-bit payload checksum, canonical signed-decimal BigInt codec, strict parsing, temporary sibling write, and rename only after close.
- [ ] Implement load as read → validate header/payload/records → `replace_snapshot`; never call replacement for invalid bytes.
- [ ] Register codec and separate KEVIN snapshot test target in CMake; re-run it and expect PASS.

### Task 3: Failure preservation, demo, and documentation

**Files:**
- Modify: `tests/kevin/kevin_snapshot_tests.cpp`
- Create: `demos/lhme/kevin_snapshot_demo.cpp`
- Create: `docs/kevin-snapshots.md`
- Modify: `docs/architecture.md`
- Modify: `CMakeLists.txt`

**Interfaces:** Consumes public V1.5 API; produces observable snapshot behavior and user documentation.

- [ ] Add a failing test that loads corrupted bytes into an already populated Store and proves its existing override remains unchanged; add a save test that preserves an existing target when serialization fails through an invalid logical snapshot fixture.
- [ ] Implement only the behavior needed to make those tests pass, then run focused tests.
- [ ] Add demo that saves a GridSpec plus override, restores into a fresh Engine/Store with the same kernels, and reports restored source/value without materializing a grid.
- [ ] Document format scope, compatibility key, error behavior, and V1.5 exclusions; list Snapshot Store under implemented architecture.
- [ ] Run `cmake --build build && ./build/lhme_kevin_snapshot_demo && ctest --test-dir build --output-on-failure && cmake --install build --prefix build/install`; expect all tests and staged install to pass.

## Plan Self-Review

- Task 1 covers logical state and non-mutating import; Task 2 covers binary integrity and atomic writing; Task 3 covers failure preservation, public evidence, and documentation.
- All five review-focus cases have explicit tests in Tasks 1–3.
- Types are introduced before their consumers and no task adds excluded systems.
