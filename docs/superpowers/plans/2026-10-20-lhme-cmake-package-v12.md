# LHME CMake Package V1.2 Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Export LHME as an installable CMake package and prove a separate C++
consumer can define and run its own TechniqueKernel.

**Architecture:** Give the project version `0.1.0`, export
`bruce_lhme_core` as `bruce_lhme::core`, and generate relocatable CMake config
and version files. A CTest script stages the current build into a binary-tree
prefix, configures a fixture using only `find_package`, builds it and runs its
external kernel executable.

**Tech Stack:** CMake 3.25, C++20, CTest, CMakePackageConfigHelpers, existing
LHME public headers and static library.

**Spec:** `docs/superpowers/specs/2026-10-20-lhme-cmake-package-v12-design.md`

## Global Constraints

- Package version is exactly `0.1.0`; consumers require `bruce_lhme 0.1`
  through config-mode `find_package`.
- Export target name is exactly `bruce_lhme::core`.
- The installed interface contains only installed include paths and public
  dependency requirements; no source or build directories leak into it.
- The consumer fixture uses only public BRUCE headers and the exported target.
- Its local kernel is `example.double` version `1`, canonical parameters
  `factor=1:2`, and must confirm `2 * 21 = 42` through Engine registration.
- The CTest round-trip works below the build tree with no network access or
  global package installation.
- The workspace is not a Git repository; record Red/Green evidence in an SDD
  ledger instead of creating commits.

## Review Focus

- An exported static target must retain its public Threads dependency; Task 1
  checks an external link rather than only package-file existence.
- A package config must be relocatable; Task 2 stages the install in the build
  tree rather than using hard-coded source paths.
- The fixture must not accidentally gain source-tree include paths; Task 2
  uses only `find_package` and public include directives.
- The external kernel must exercise registration and runtime behavior, not
  merely compile; Task 2 runs it and requires result `42`.
- Repeated CTest executions must not inherit stale staged files; Task 2 clears
  only its specific build-tree stage and consumer directories.

## Planned File Structure

```text
CMakeLists.txt                                  version, install interfaces, export and CTest entry
cmake/bruce_lhmeConfig.cmake.in                 relocatable consumer config with dependency lookup
examples/external_consumer/CMakeLists.txt       independent find_package fixture
examples/external_consumer/main.cpp             host-defined ExternalDoubleKernel and runtime assertion
tests/package_layout_test.cmake                 staged-install package-file assertion
tests/external_consumer_test.cmake              stage/install/configure/build/run driver
.superpowers/sdd/.../progress.md                Red/Green and final verification ledger
```

### Task 1: Exported CMake Package Contract

**Files:**
- Modify: `CMakeLists.txt`
- Create: `cmake/bruce_lhmeConfig.cmake.in`
- Create: `tests/package_layout_test.cmake`

**Interfaces:**
- Produces project version `0.1.0`.
- Produces installed imported target `bruce_lhme::core` through
  `bruce_lhmeTargets.cmake`.
- Produces package files under
  `${CMAKE_INSTALL_LIBDIR}/cmake/bruce_lhme/`.

- [ ] **Step 1: Write the failing package-layout check**

  Add CTest target `lhme_package_layout` running
  `tests/package_layout_test.cmake`. The script stages the current build into
  a named binary-tree directory, checks for the expected three package files,
  and fails with a diagnostic when any are absent. Do not create the config
  template or export rules yet.

- [ ] **Step 2: Verify RED**

  Run: `cmake -S . -B build && cmake --build build && ctest --test-dir build -R lhme_package_layout --output-on-failure`

  Expected: FAIL because the staged install lacks
  `bruce_lhmeConfig.cmake`, `bruce_lhmeConfigVersion.cmake` and exported
  `bruce_lhme::core` target metadata.

- [ ] **Step 3: Implement versioned, relocatable package export**

  Set `project(bruce_lhme VERSION 0.1.0 ...)`. Use BUILD/INSTALL interface
  include directories on `bruce_lhme_core`, set its export name to `core`, and
  install it through named export set `bruce_lhmeTargets` with namespace
  `bruce_lhme::`. Use `CMakePackageConfigHelpers` to configure the package
  config and write a SameMajorVersion-compatible version file. The config
  template must use `find_dependency(Threads)` before including the targets
  file.

- [ ] **Step 4: Verify GREEN and record it**

  Re-run Step 2. The package-layout phase must now find the config, version and
  target files. Record the exact Red/Green evidence in the SDD ledger.

### Task 2: Independent External Consumer Round Trip

**Files:**
- Create: `examples/external_consumer/CMakeLists.txt`
- Create: `examples/external_consumer/main.cpp`
- Create: `tests/external_consumer_test.cmake`
- Modify: `CMakeLists.txt`

**Interfaces:**
- Consumes only `find_package(bruce_lhme 0.1 CONFIG REQUIRED)` and
  `bruce_lhme::core` from Task 1.
- Produces CTest target `lhme_external_consumer`.
- Produces executable result code zero only after Engine confirms value `42`.

- [ ] **Step 1: Write the failing external fixture CMake project**

  Add `examples/external_consumer/CMakeLists.txt` using the required
  `find_package` and target link command, but leave `main.cpp` absent. Add
  CTest target `lhme_external_consumer` running
  `tests/external_consumer_test.cmake`, which stages the current installation
  and configures/builds this fixture below a named binary-tree directory.

- [ ] **Step 2: Verify RED**

  Run: `cmake -S . -B build && cmake --build build && ctest --test-dir build -R lhme_external_consumer --output-on-failure`

  Expected: FAIL because the fixture source does not exist; this proves CTest
  reached an independent `find_package` configure/build rather than relying on
  a source-tree target.

- [ ] **Step 3: Implement the fixture and strict driver**

  Implement `ExternalDoubleKernel` in fixture `main.cpp`. It validates the
  exact identity/parameter/address contract, checks the supplied cancellation
  signal, returns a base trace, and confirms Exact index `21` as BigInt `42`
  after `Engine::register_technique`. The CTest script must use
  `execute_process` for install, configure, build and executable run; each
  nonzero result is a `FATAL_ERROR` containing captured stdout/stderr. It may
  remove only the named test stage and fixture build directories under the
  current binary tree.

- [ ] **Step 4: Verify GREEN and record it**

  Re-run Step 2. The staged consumer must configure, build and exit zero. Add
  the targeted command/result to the SDD ledger.

### Task 3: Compatibility and Installation Verification

**Files:**
- Modify: `.superpowers/sdd/2026-10-20-lhme-cmake-package-v12/progress.md`

**Interfaces:**
- Verifies package files, all existing public headers and CTest targets after a
  fresh install.

- [ ] **Step 1: Run package and legacy compatibility checks**

  Run: `cmake --build build --target c_api_tests c_api_header_smoke technique_tests lhme_external_consumer && ctest --test-dir build -R 'c_api_tests|c_api_header_smoke|technique_tests|lhme_external_consumer' --output-on-failure`

  Expected: all four targets pass.

- [ ] **Step 2: Run full verification**

  Run: `cmake -S . -B build && cmake --build build && ctest --test-dir build --output-on-failure && cmake --install build --prefix build/install`

  Expected: all CTest targets pass and the install contains headers, static
  library and the three CMake package files. Record final evidence in the SDD
  ledger; do not create a commit in this non-Git workspace.

## Plan Self-Review

**Spec coverage:** Task 1 exports the versioned package and public dependency
contract. Task 2 proves independent discovery, compile, link and runtime
registration of a host-defined TechniqueKernel. Task 3 protects existing API
and install behavior.

**Type consistency:** Task 1 exports `bruce_lhme::core`; Task 2 consumes that
exact spelling through `find_package(bruce_lhme 0.1 CONFIG REQUIRED)`. The
fixture's kernel follows the existing public `TechniqueKernel` API without new
BRUCE interfaces.

**Review focus coverage:** Task 2 validates Threads linkage, relocatable staged
installation, absence of source-relative consumption, runtime `42`, and stale
test-directory cleanup. Task 3 rechecks the C API and public headers.

**Scope:** V1.2 adds local CMake packaging and one external consumer fixture;
it excludes network publishing, shared libraries, C API changes and runtime
plugins.
