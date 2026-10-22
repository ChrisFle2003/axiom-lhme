# AXIOM LHME v0.1 — Release Contract

## Scope

Version 0.1 is a CPU-and-RAM C++20 library. Its stable public boundary is:

```text
FormulaRecipe / FormulaAddress
  -> FormulaOverrideStore
  -> Engine + bounded ResultCache

GridDomainSpec / GridAddress
  -> explicit DomainProjection
  -> FormulaEvaluator
```

The public source repository is
[ChrisFle2003/axiom-lhme](https://github.com/ChrisFle2003/axiom-lhme).

`FormulaRecipe` is the deterministic source of values. `KEVIN` is a separate
8×8 domain and dimension layer; it neither owns values nor implicitly selects a
formula. A value lookup through KEVIN therefore always needs an explicit
projection.

## Stable v0.1 guarantees

- `PowerOffset` FormulaRecipes with a positive base, non-negative scale,
  non-negative address contract and a non-zero version are accepted only when
  their specification hash is canonical.
- Formula overrides are sparse, synchronized and isolated by their canonical
  formula or technique identity.
- Override snapshots are fail-closed: format, record count, canonical integer
  format, checksum, exact end-of-file and duplicate keys are validated before
  the target store changes.
- `ResultCache` is only an optional bounded accelerator. Reconstructing a
  value from its FormulaRecipe must not depend on the cache.
- A local domain-filter miss stops only that candidate. It never acts as a
  global execution stop.
- Each domain evaluation trace identifies its Domain, Projection and resulting
  FormulaAddress.

`KevinMemoryStore` is preserved as a documented compatibility boundary for
older callers. New integrations should use FormulaRecipe plus
FormulaOverrideStore and, where needed, explicit KEVIN projections.

## Reproducible local builds

The project contains named CMake profiles:

```text
cmake --preset debug
cmake --build --preset debug
ctest --preset debug

cmake --preset release
cmake --build --preset release
ctest --preset release
```

For diagnostics, use `address`, `undefined` or `thread` in place of `debug`.
Sanitizers are intentionally opt-in and are not included in release binaries.
When a sanitizer is enabled, the exported static CMake target propagates the
matching runtime link option to `find_package()` consumers. This keeps the
package and its external consumers linkable under instrumentation.
When an instrumented run is hosted under `ptrace` and LeakSanitizer reports its
known incompatibility, use `ASAN_OPTIONS=detect_leaks=0` only for that local
AddressSanitizer run. CI must retain leak detection.

## Verified locally

- GNU C++ 14.2 on Linux: complete Debug profile, 42/42 tests.
- GNU C++ 14.2 on Linux: complete Release profile, 42/42 tests.
- GNU C++ 14.2 on Linux: complete AddressSanitizer profile, 42/42 tests.
- GNU C++ 14.2 on Linux: complete UndefinedBehaviorSanitizer profile, 42/42
  tests.

The latest complete verification includes the read-only GGUF inspector tests,
its bounded storage planning and its report/CLI surface. The local
AddressSanitizer run used `ASAN_OPTIONS=detect_leaks=0` because this host
monitors processes with `ptrace`, an environment in which LeakSanitizer is not
reliable.

## Still required before a public v0.1 tag

1. Repeat the AddressSanitizer profile with LeakSanitizer enabled on a host
   that permits it.
2. Build and test with an independently installed supported C++20 compiler.
3. Build and test on Windows before claiming Windows support.
4. Choose a release owner and distribution channel. The repository already
   includes the MIT License.

GPU backends, NanoCell runtime, semantic routing, AnswerPoints and Experience
Records are explicitly not v0.1 release requirements.
