# LHME CMake Package V1.2 Design

## Purpose

Turn the installed LHME static library into a normal CMake package that an
unrelated C++ project can discover, link and use without copying BRUCE source
paths or manually locating libraries. A separate consumer fixture must prove
the installed package can host a user-defined `TechniqueKernel`.

## Scope

- Publish CMake package version `0.1.0` for the `bruce_lhme` project.
- Export the installed static library as imported target `bruce_lhme::core`.
- Install `bruce_lhmeConfig.cmake`, `bruce_lhmeConfigVersion.cmake` and
  `bruce_lhmeTargets.cmake` below the standard library CMake package path.
- Add a standalone consumer fixture with its own CMake project and C++ source.
- Add one CTest driver that stages an install, configures the fixture with
  `CMAKE_PREFIX_PATH`, builds it and runs its executable.

## Non-goals

- No package-manager release, network publishing, shared-library build or ABI
  compatibility promise beyond this local CMake package.
- No change to BRUCE calculation, TechniqueKernel behavior, C API or GPU work.
- No runtime plugin loader. The consumer's kernel is ordinary compile-time C++.

## Installed Package Contract

The project version is `0.1.0`. The installation retains public headers under
`include/bruce/` and the static library under the configured library directory.
It additionally installs:

```text
<libdir>/cmake/bruce_lhme/
├─ bruce_lhmeConfig.cmake
├─ bruce_lhmeConfigVersion.cmake
└─ bruce_lhmeTargets.cmake
```

Consumers use:

```cmake
find_package(bruce_lhme 0.1 CONFIG REQUIRED)
target_link_libraries(my_program PRIVATE bruce_lhme::core)
```

`bruce_lhme::core` exposes the installed include directory and transitively
resolves its required Threads dependency. It must not expose a source-tree or
build-tree include path in the installed target interface.

## Consumer Fixture

The fixture is a separate CMake project under `examples/external_consumer/`.
It has no relative include or link path into the BRUCE source tree; it only
uses `find_package(bruce_lhme 0.1 CONFIG REQUIRED)` and
`bruce_lhme::core`.

Its source defines `ExternalDoubleKernel`, a local
`TechniqueKernel` implementation with identity `example.double`, version `1`
and canonical parameters `factor=1:2`. It accepts a nonnegative address in
Exact mode, returns `2 * index` as BigInt, observes the Engine-provided
cancellation signal, and emits a base trace. The consumer registers that
kernel in `Engine`, evaluates index `21`, and exits successfully only when the
confirmed result is `42`.

The fixture deliberately does not become part of `bruce_lhme_core`; it is
evidence that a host project can define its own code against installed public
interfaces.

## CTest Installation Round Trip

`tests/external_consumer_test.cmake` is a CMake script invoked by one CTest
entry. It uses directories below the current binary tree:

1. remove any previous test-only stage and consumer build directories;
2. run `cmake --install` for the current BRUCE build into the test stage;
3. configure the consumer fixture with
   `-DCMAKE_PREFIX_PATH=<test stage>`;
4. build the consumer fixture;
5. run the produced executable.

Every external command must fail the test on a nonzero exit code and print its
captured output for diagnosis. The test does not write into the source tree or
require an already-installed global package.

## Test Contract

Tests must prove:

- normal project installation includes all three CMake package files;
- the exported target is discoverable as `bruce_lhme::core` at version `0.1.0`;
- the fixture configures without source-relative BRUCE paths;
- the fixture links the installed static library and runs with result `42`;
- the normal complete build, existing tests and installed header layout remain
  compatible.

## Acceptance Criteria

1. A clean external CMake build can use `find_package(bruce_lhme 0.1 CONFIG
   REQUIRED)` against a staged install.
2. That build can implement and register a local TechniqueKernel without
   changing BRUCE source.
3. The external executable confirms its deterministic result at runtime.
4. The round-trip is an ordinary CTest target and runs without network access
   or global installation state.
