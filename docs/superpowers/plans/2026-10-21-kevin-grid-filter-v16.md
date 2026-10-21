# KEVIN Grid Filter V1.6 Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:executing-plans task-by-task with TDD.

**Goal:** Filter one 8×8 KEVIN tile through deterministic 64-bit masks before exact evaluation.

**Architecture:** `CandidateMask64` owns pure bit operations. `GridTile64` maps stable `bit=y*8+x` positions to child addresses. A filter runner requests Metric or Modular only and emits surviving addresses for later Exact work.

**Tech Stack:** C++20, existing KEVIN Store/Engine, CMake tests.

**Spec:** `docs/superpowers/specs/2026-10-21-kevin-grid-filter-v16-design.md`

## Global Constraints

- Exactly 64 candidates per 8×8 tile; no row-crossing bit shifts.
- Modular and Metric before Exact; rejected bits never trigger Exact.
- Local masks never cancel unrelated tiles or the Engine.
- Tests remain under `tests/kevin/`; no GPU or callback filters.

## Review Focus

- bit 0 and bit 63 map to `(0,0)` and `(7,7)` respectively;
- AND/OR/XOR/NOT and popcount preserve all 64 positions;
- invalid modulus produces no accepted candidates;
- an empty mask performs zero exact requests;
- same tile/filter returns the same mask twice.

### Task 1: CandidateMask64 and GridTile64

**Files:** Create `include/bruce/lhme/kevin_filter.hpp`, `src/kevin/filter.cpp`, `tests/kevin/kevin_filter_tests.cpp`; modify `CMakeLists.txt`.

- [x] Write tests for bit position mapping, all/none masks, boolean operations, popcount, and row masks.
- [x] Implement the 64-bit candidate mask and deterministic child-address mapping with `bit=y*8+x`.
- [x] Re-run focused tests; PASS.

### Task 2: Lazy mathematical filters

**Files:** Modify Task 1 header/source/tests.

- [x] Test ModularEquals and MetricBitLengthRange against a registered PowerOffset grid; empty masks make zero Exact calls.
- [x] Implement filter evaluation through `KevinMemoryStore::get(... Modular|Metric ...)`, returning mask plus reason counts.
- [x] Run focused KEVIN and full project tests; PASS.

### Task 3: Observable demo and verification

**Files:** Create `demos/lhme/kevin_grid_filter_demo.cpp`; modify `CMakeLists.txt`, `docs/kevin-memory-store.md`.

- [x] Add demo assertions for a 64-cell tile, accepted-mask count and zero Exact calls before explicit survivor evaluation.
- [x] Run demo, full CTest suite and staged install; PASS.

## Plan Self-Review

Tasks isolate bit representation, lazy math integration and observability. Every review-focus condition is assigned to Task 1 or 2; excluded callback/GPU work is absent.
