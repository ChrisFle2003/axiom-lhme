# AXIOM LHME

AXIOM LHME is a C++20 library for deterministic, lazy and memory-conscious
formula evaluation. It is designed for cases where a large logical value space
is better represented by a compact rule than by materializing every value in
RAM.

Source repository: [ChrisFle2003/axiom-lhme](https://github.com/ChrisFle2003/axiom-lhme).

## The core idea

```text
FormulaRecipe + FormulaAddress
  -> FormulaOverrideStore (only real deviations)
  -> Engine + bounded ResultCache

KEVIN GridDomain + GridAddress
  -> explicit DomainProjection
  -> FormulaEvaluator
```

`FormulaRecipe` is the source of a value. `KEVIN` is a separate hierarchical
8x8 domain, dimension and filtering coordinate. It never silently chooses a
formula: a `DomainProjection` explicitly maps a grid address to a formula
address. This preserves deterministic reconstruction while keeping stored data
sparse.

## v0.1 scope

- CPU and RAM execution only
- exact `BigInt`, modular, metric and symbolic formula evaluation
- bounded result cache and sparse formula overrides
- deterministic traces, resource budgets and safe cancellation
- KEVIN domain projections and candidate-local filtering
- installable CMake package and extension-kernel SDK

GPU backends, NanoCell runtime, semantic routing, AnswerPoints and Experience
Records are future concepts, not shipped v0.1 features.

## Requirements

- CMake 3.25 or newer
- A C++20 compiler
- C and C++ standard library threading support

## Build and test

```bash
cmake --preset debug
cmake --build --preset debug
ctest --preset debug

cmake --preset release
cmake --build --preset release
ctest --preset release
```

### Optional: GCC + Ninja

Wenn Ninja installiert ist, bleiben GCC und G++ die Compiler; Ninja ersetzt
nur den Build-Generator. Die Profile verwenden getrennte Build-Ordner:

```bash
cmake --preset gcc-ninja-debug
cmake --build --preset gcc-ninja-debug --parallel 2
ctest --preset gcc-ninja-debug

cmake --preset gcc-ninja-release
cmake --build --preset gcc-ninja-release --parallel 2
ctest --preset gcc-ninja-release
```

Additional diagnostic profiles are `address`, `undefined` and `thread`.
They instrument the runtime and are intended for development, not release
binaries.

## Use as a CMake package

Install AXIOM LHME, then consume it from another project:

```cmake
find_package(axiom_lhme 0.1 CONFIG REQUIRED)

add_executable(my_program main.cpp)
target_link_libraries(my_program PRIVATE axiom_lhme::core)
```

The checked external example lives in
[`examples/external_consumer`](examples/external_consumer).

## Documentation

- [Implemented architecture](docs/architecture.md)
- [Benchmark review — 2026-10-21](docs/benchmark-review-2026-10-21.md)
  - [Fair comparison: separate Tracks A, B and C](docs/benchmark-review-2026-10-21.md#fair-comparison-track-a)
- [v0.1 release contract](docs/release-v0.1.md)
- [Concept collection](concepts/README.md)
- [Kernel SDK](docs/kernel-sdk.md)
- [GGUF Inspector V1](docs/gguf-inspector.md)

## License

AXIOM LHME is released under the [MIT License](LICENSE).
