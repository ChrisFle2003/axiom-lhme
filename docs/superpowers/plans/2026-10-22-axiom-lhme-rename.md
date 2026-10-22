# Axiom LHME Rename Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Rename the public project identity from BRUCE LHME to Axiom LHME and
publish the resulting repository as `ChrisFle2003/axiom-lhme`.

**Architecture:** The migration changes every public identity boundary in one
breaking release line: CMake package, C++ namespace and header root, C API,
CLI and documentation. The algorithms, data models, test semantics and version
`0.1.0` are preserved; no compatibility aliases are introduced.

**Tech Stack:** CMake 3.25+, C++20, C11 C API, CTest, GitHub Actions, GitHub.

**Spec:**
`docs/superpowers/specs/2026-10-22-axiom-lhme-rename-design.md`

## Global Constraints

- Rename `BRUCE LHME` to `Axiom LHME` and `bruce_lhme` to `axiom_lhme`.
- Replace `bruce::lhme` with `axiom::lhme` and `include/bruce/lhme/` with
  `include/axiom/lhme/`.
- Replace the C API prefix `lhme_` with `axiom_lhme_`.
- Replace `bruce_gguf_inspect` with `axiom_gguf_inspect` while preserving its
  command-line arguments, output formats and exit codes.
- Do not provide old-name aliases or compatibility headers.
- Do not change version `0.1.0`, algorithms, test semantics or `model/`.
- Keep CMake 3.25+, C++20 and C11 requirements unchanged.

## Review Focus

- A clean external project must find `axiom_lhme` and link
  `axiom_lhme::core`, not a stale target or package name.
- A C translation unit must compile against only the new `axiom_lhme_*` API.
- Every installed public header must be reachable as `axiom/lhme/<header>.hpp`
  and contain no `bruce::lhme` declaration.
- The renamed CLI must retain its required options and documented exit codes.
- GitHub rename, remote update and push must occur only after local tests pass;
  the local `model/` directory must stay untracked and unchanged.

## File Structure

| Area | Files | Responsibility |
| --- | --- | --- |
| Build/package | `CMakeLists.txt`, `CMakePresets.json`, `cmake/*Config*.in` | Project, library, executable, package and install identities |
| Public API | `include/bruce/lhme/` → `include/axiom/lhme/` | C++ headers and C API declarations |
| Implementation | `src/`, `tools/`, `demos/` | Namespace-qualified definitions and CLI executable name |
| Verification | `tests/`, `examples/` | Compile-time C API and external package-consumer contracts |
| Repository text | `README.md`, `docs/`, `concepts/`, `.github/` | User-facing name, commands, links and templates |
| Hosting | Git remote configuration | GitHub repository name and new origin URL |

### Task 1: Prepare an isolated, verified baseline

**Files:**
- Modify: no repository files unless an ignored worktree directory must be
  added under the worktree skill
- Verify: `CMakePresets.json`, existing configured build directories

**Interfaces:**
- Consumes: the current `main` checkout and its two committed design documents.
- Produces: an isolated checkout or an explicitly recorded in-place fallback,
  with a passing Debug baseline.

- [ ] **Step 1: Detect whether the checkout is already a linked worktree and
  verify that it is not a submodule.**

Run the detection commands from `superpowers:using-git-worktrees` before
creating any checkout.

- [ ] **Step 2: Create a managed isolated worktree after obtaining any required
  consent, or record the sandbox fallback.**

Use a branch beginning `codex/axiom-lhme-rename`; do not copy or modify
`model/`.

- [ ] **Step 3: Configure, build and run the Debug preset baseline.**

Run: `cmake --preset debug && cmake --build --preset debug && ctest --preset debug`

Expected: configuration, build and every registered CTest test pass before
identity changes.

### Task 2: Rename CMake project, package and installed C++ API root

**Files:**
- Modify: `CMakeLists.txt`, `cmake/bruce_lhmeConfig.cmake.in`,
  `CMakePresets.json`, `examples/*/CMakeLists.txt`, `tests/*consumer*.cmake`,
  `tests/package_layout_test.cmake`
- Move: `cmake/bruce_lhmeConfig.cmake.in` → `cmake/axiom_lhmeConfig.cmake.in`
- Move: `include/bruce/lhme/` → `include/axiom/lhme/`

**Interfaces:**
- Consumes: the CMake project currently named `bruce_lhme` and its exported
  target `bruce_lhme::core`.
- Produces: CMake project/package `axiom_lhme`, target `axiom_lhme::core`,
  static target `axiom_lhme_core`, and installed headers under `include/axiom`.

- [ ] **Step 1: Update the package-layout CMake test so it expects
  `axiom_lhmeConfig.cmake`, `axiom_lhmeTargets.cmake` and
  `axiom_lhme::core`.**

The unmodified build must now fail this contract because it exports only the
old package identity.

- [ ] **Step 2: Rename the CMake project, all target names, export namespace,
  package directory and config-template filename.**

Preserve the library sources, compile features, `Threads::Threads`, install
destinations and package version compatibility. Change only identifiers that
encode the old project name.

- [ ] **Step 3: Move the public header tree to `include/axiom/lhme/` and update
  CMake's install rule from `include/bruce` to `include/axiom`.**

- [ ] **Step 4: Update both external consumer CMake projects and their CMake
  contract tests to use `find_package(axiom_lhme 0.1 CONFIG REQUIRED)` and
  `axiom_lhme::core`.**

- [ ] **Step 5: Configure and run the package-layout plus external-consumer
  tests.**

Run: `ctest --preset debug -R 'lhme_package_layout|lhme_external_consumer|lhme_kernel_sdk_consumer' --output-on-failure`

Expected: all three tests pass with the new package identity.

### Task 3: Rename the C++ namespace and all internal/public includes

**Files:**
- Modify: every `.hpp`, `.cpp`, `.c` and CMake source reference under `src/`,
  `tests/`, `demos/`, `examples/` and `tools/`
- Verify: all files under `include/axiom/lhme/`

**Interfaces:**
- Consumes: header tree and CMake target generated by Task 2.
- Produces: `axiom::lhme` as the sole C++ namespace and
  `#include "axiom/lhme/..."` as the sole public include convention.

- [ ] **Step 1: Update one existing compile test (`tests/core_formula_tests.cpp`)
  to include `axiom/lhme/formula.hpp` and call `axiom::lhme` symbols.**

The test must fail before the corresponding header and namespace migration.

- [ ] **Step 2: Replace the namespace declarations, qualified references and
  public-header includes throughout the product, tests, demos, tools and
  examples.**

Do not rename stable domain vocabulary such as `LHME`, `KEVIN`,
`FormulaRecipe` or `FormulaOverrideStore`.

- [ ] **Step 3: Compile and run focused core, engine, KEVIN and Kernel SDK
  contract tests.**

Run: `ctest --preset debug -R 'core_formula_tests|engine_tests|kevin_.*_tests|kernel_sdk_contract_tests' --output-on-failure`

Expected: every selected test passes under `axiom::lhme`.

### Task 4: Replace the C API identity

**Files:**
- Move: `include/axiom/lhme/c_api.h` → `include/axiom/lhme/axiom_lhme_c_api.h`
- Modify: `src/c_api.cpp`, `tests/c_api_header_smoke.c`, `tests/c_api_tests.cpp`,
  all C API references

**Interfaces:**
- Consumes: C API declarations previously named `lhme_*`.
- Produces: opaque handles, enums, structs and functions named
  `axiom_lhme_*`, including `axiom_lhme_engine_create`,
  `axiom_lhme_formula_register`, `axiom_lhme_evaluate` and
  `axiom_lhme_cancel`.

- [ ] **Step 1: Change `tests/c_api_header_smoke.c` to include the new C header
  and declare/use the new `axiom_lhme_*` types and functions.**

Expected before implementation: compile failure because the new header and
symbols do not exist.

- [ ] **Step 2: Rename the C header and every public C declaration and
definition to the `axiom_lhme_` prefix.**

Preserve `extern "C"`, ownership functions, enum values' semantic ordering,
and existing null/error behavior.

- [ ] **Step 3: Update the C++ C-API test to use the new prefix and compile the
  C smoke target.**

Run: `ctest --preset debug -R 'c_api_tests|c_api_header_smoke' --output-on-failure`

Expected: both C API contracts pass without any old-name include.

### Task 5: Rename the GGUF CLI and consumer-facing executable references

**Files:**
- Modify: `CMakeLists.txt`, `tools/gguf_inspect/*`,
  `tests/gguf_report_tests.cpp`, `docs/gguf-inspector.md`, `README.md`

**Interfaces:**
- Consumes: `run_gguf_inspect(...)` and its existing parser behavior.
- Produces: executable `axiom_gguf_inspect`; the function and CLI arguments
  remain otherwise unchanged.

- [ ] **Step 1: Update the CLI report/command test expectation to invoke or
  name `axiom_gguf_inspect`.**

Expected before target renaming: the target/reference lookup fails.

- [ ] **Step 2: Rename the CMake executable target from `bruce_gguf_inspect`
  to `axiom_gguf_inspect` without changing `run_gguf_inspect` or its options.**

- [ ] **Step 3: Build the new executable and run the GGUF-focused tests.**

Run: `cmake --build --preset debug --target axiom_gguf_inspect && ctest --preset debug -R 'gguf_.*_tests' --output-on-failure`

Expected: the executable builds and all selected tests pass.

### Task 6: Update human-facing repository material and eliminate stale public names

**Files:**
- Modify: `README.md`, `docs/**/*.md`, `concepts/**/*.md`,
  `.github/ISSUE_TEMPLATE/bug_report.yml`, `.github/pull_request_template.md`,
  documentation plans/specs that instruct current integration
- Verify: repository-wide tracked text search

**Interfaces:**
- Consumes: the new CMake, namespace, C API and CLI names from Tasks 2–5.
- Produces: Axiom LHME documentation and contribution templates with valid
  commands, package names, URLs and header paths.

- [ ] **Step 1: Update the README's package example to use `find_package(axiom_lhme 0.1 CONFIG REQUIRED)` and `axiom_lhme::core`.**

Expected before document migration: repository text search reports stale public
integration instructions.

- [ ] **Step 2: Replace current-product occurrences of BRUCE/bruce in
documentation, concepts, GitHub templates and generated command examples with
Axiom/axiom.**

Preserve only explicit historical references where the old identity is required
to explain a historical commit or migration; none should be presented as the
current API.

- [ ] **Step 3: Update known repository URL references to
`https://github.com/ChrisFle2003/axiom-lhme`.**

- [ ] **Step 4: Run a tracked-file scan for stale identity tokens and manually
review any intentional historical exceptions.**

Run: `git grep -n -i -E 'bruce(_lhme)?|bruce::lhme|bruce_gguf_inspect|lhme_'`

Expected: no current public API or documentation result remains; any deliberate
historical exception is documented in the commit message.

### Task 7: Perform full verification, rename GitHub, update origin and publish

**Files:**
- Modify: GitHub repository settings and local `origin` remote
- Verify: Git history, GitHub Actions-triggering push, remote URL

**Interfaces:**
- Consumes: a clean, fully passing local Axiom LHME migration.
- Produces: GitHub repository `ChrisFle2003/axiom-lhme` and local `origin`
  configured to its new URL.

- [ ] **Step 1: Run the complete Debug and Release preset suites.**

Run: `cmake --preset debug && cmake --build --preset debug && ctest --preset debug && cmake --preset release && cmake --build --preset release && ctest --preset release`

Expected: both configurations build and every registered CTest test passes.

- [ ] **Step 2: Review the final diff and ensure only planned tracked files are
  staged; explicitly leave `model/` untouched and untracked.**

- [ ] **Step 3: Commit the complete migration with message
  `feat!: rename BRUCE LHME to Axiom LHME`.**

- [ ] **Step 4: Rename the GitHub repository to `axiom-lhme` using authorized
  GitHub access.**

Expected: the repository owner remains `ChrisFle2003` and the repository URL
resolves to `https://github.com/ChrisFle2003/axiom-lhme`.

- [ ] **Step 5: Change `origin` to the new URL and push the commits.**

Run: `git remote set-url origin https://github.com/ChrisFle2003/axiom-lhme.git` followed by `git push origin main`.

Expected: local `main` and `origin/main` agree, and GitHub Actions can run on
the renamed repository.

## Plan Self-Review

- **Spec coverage:** Tasks 2–5 implement every public-name mapping; Task 6
  updates repository text; Task 7 covers validation and GitHub publication.
- **Type consistency:** The only new public C++ namespace is `axiom::lhme`; the
  only C prefix is `axiom_lhme_`; the only package/target identifiers are
  `axiom_lhme` and `axiom_lhme::core`.
- **Review focus coverage:** Package, C header, C++ include, CLI and GitHub/
  untracked-file risks are covered in Tasks 2, 4, 3, 5 and 7 respectively.
- **Proportion:** The plan gives exact identities and checks without prescribing
  mechanical edit bodies; all functional behavior stays unchanged.
