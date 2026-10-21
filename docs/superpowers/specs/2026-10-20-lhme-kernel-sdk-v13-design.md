# LHME Kernel SDK V1.3 Design

## Purpose

Make BRUCE's public `TechniqueKernel` contract easy to adopt in an unrelated
C++ project. A developer must be able to copy a small, deterministic kernel
example, register it with `Engine`, and verify its behavior without reading
BRUCE internals.

## Scope

- Add a public, header-only `kernel_sdk.hpp` with small authoring helpers.
- Provide a separate CMake consumer fixture that defines `AffineKernel`.
- Add real contract tests for the public kernel lifecycle.
- Add a staged-install CTest round trip for the SDK consumer.
- Add a concise Kernel SDK guide.

## Non-goals

- No runtime plug-in loading, shared libraries, package-manager publishing,
  GPU execution, or dynamic code loading.
- No change to the existing `Engine`, `TechniqueKernel`, calculation kernels,
  or C API contract.
- No mandatory SDK base class: user kernels remain ordinary
  `TechniqueKernel` implementations.

## Public SDK Contract

`include/bruce/lhme/kernel_sdk.hpp` is installed as part of the normal public
header tree. It offers only deterministic helper functions that let a kernel
author create standard results:

- `confirmed_exact_result(value, trace)`;
- `invalid_technique_request_result(mode, reason)`;
- `cancelled_technique_result(mode, reason)`;
- `cancellation_requested(request)`.

`confirmed_exact_result` accepts a `BigInt` value and a non-null trace. The
invalid helper creates one Branch trace event from `reason`; the cancelled
helper creates one Run trace event from `reason`. All three set the supplied
or requested `EvaluationMode` explicitly. This makes their result semantics
testable without adding a base class or changing `Engine`.

The helpers do not register a kernel, execute a formula, alter an `Engine`, or
hide resource limits. They only make the already-public result and cancellation
contract explicit and consistent. A user can always implement the contract
without these helpers.

The expected authoring sequence is:

```text
implement TechniqueKernel
→ validate identity, parameters, mode and address
→ check Engine-provided cancellation signal
→ calculate deterministically inside caller budget rules
→ return Confirmed / InvalidArgument / Cancelled with a trace
→ register through Engine
```

## Example Consumer

`examples/kernel_sdk_consumer/` is a standalone CMake project. It consumes
only:

```cmake
find_package(bruce_lhme 0.1 CONFIG REQUIRED)
target_link_libraries(my_kernel PRIVATE bruce_lhme::core)
```

It defines `AffineKernel` with identity `example.affine`, version `1`, and
canonical parameters `factor=1:2;offset=1:3`. In Exact mode, for a
nonnegative integer address, it returns:

\[
2 \times index + 3
\]

It validates identity, exact parameter spelling, nonnegative address, and
Exact mode. It observes the Engine-provided cancellation signal, returns a
base trace, registers with `Engine`, and demonstrates `index=21 → 45`.

The example is external consumer code; it never becomes a source file in
`bruce_lhme_core`.

## Contract Tests

`tests/kernel_sdk_contract_tests.cpp` tests the public helper behavior and a
small local `TechniqueKernel` implementation through `Engine`. It must prove:

- a confirmed Exact result contains its BigInt and a trace;
- invalid requests preserve their requested mode and include a Branch trace;
- cancelled requests preserve their requested mode and include a Run trace;
- `cancellation_requested` is false for a missing or false signal and true
  for a set signal;
- an independently implemented kernel is registered by `Engine`, receives the
  Engine signal, and confirms `2 × 21 + 3 = 45`.

`tests/kernel_sdk_consumer_test.cmake` stages the current build below its own
binary-tree directory, configures the independent fixture using only
`CMAKE_PREFIX_PATH`, builds it and runs it. It removes only its own stage and
consumer-build directories. Any failed child command produces a CMake
`FATAL_ERROR` with captured stdout and stderr.

## Documentation

`docs/kernel-sdk.md` gives a short copy-and-adapt workflow:

1. Install or stage the BRUCE CMake package.
2. Link `bruce_lhme::core` using config-mode `find_package`.
3. Implement `identity()` and `evaluate()`.
4. Use canonical parameters and validate every input.
5. Observe cancellation before computation.
6. Return a traced result and register the kernel with `Engine`.
7. Run the external consumer and contract tests.

It explicitly distinguishes V1.3's compile-time integration from a dynamic
plug-in system.

## Acceptance Criteria

1. `kernel_sdk.hpp` is a usable installed public header with no new compiled
   library source.
2. A local Contract test validates successful, invalid, and cancelled kernel
   results using real `Engine` behavior.
3. A staged-install external consumer builds without source-tree includes and
   verifies the deterministic result `45`.
4. The complete existing project suite and installation remain green.
