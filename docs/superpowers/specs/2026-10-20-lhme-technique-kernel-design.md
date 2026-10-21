# LHME TechniqueKernel V1 Design

## Purpose

Enable other C++ programs to contribute deterministic BRUCE computation
techniques without modifying BRUCE's execution controls. A technique provides
domain calculation; BRUCE remains responsible for registration, validation,
budget enforcement, cancellation, cache admission and trace publication.

V1 is a compile-time C++ extension interface. Runtime-loaded shared libraries
and generic foreign-language technique definitions are intentionally out of
scope.

## Goals

- Register multiple named, versioned computation techniques in one `Engine`.
- Address a technique using stable ID, version, canonical parameters and an
  address.
- Support `Symbolic`, `Metric`, `Modular` and `Exact` evaluation modes.
- Preserve deterministic results, fail-closed validation and existing Engine
  controls.
- Port the existing power-offset formula to the new interface as the V1
  reference kernel while keeping the current FormulaSpec and C API compatible.

## Non-goals

- Execute arbitrary source code, scripts or unregistered callbacks.
- Load `.so`, `.dll` or network plugins at runtime.
- Change existing `FormulaSpec`, `EvaluationRequest`, streaming APIs or C API
  behavior in this increment.
- Persist kernel objects or cache contents across processes.

## Core Model

```text
TechniqueRequest
  technique_id + version + canonical parameters + address + mode + budget
       ↓
Engine TechniqueRegistry
       ↓ fail closed when no exact registered identity exists
TechniqueKernel
       ↓ domain computation only
Engine
  budget / SafeStop / cache / trace / result publication
```

### Technique identity

`TechniqueIdentity` contains:

- `id`: nonempty ASCII identifier, such as `bruce.power_offset`.
- `version`: positive unsigned integer.

The registry rejects an empty ID, version zero, a null kernel, and a duplicate
identity. Registration is explicit; a request never instantiates a kernel by
name or executes arbitrary code.

### Parameters

`TechniqueParameters` is canonical UTF-8 text supplied by the host program.
The kernel validates and interprets it. V1 treats it as opaque at the Engine
boundary, except that it must be nonempty and bounded in size. A technique
must use one canonical representation for semantically equal parameters.

This makes parameter bytes safe to use in a deterministic cache key. For the
reference kernel, parameters encode positive decimal `base` and nonnegative
decimal `scale` in a fixed, length-delimited representation.

## C++ Interfaces

```cpp
struct TechniqueIdentity {
  std::string id;
  std::uint32_t version;
};

struct TechniqueRequest {
  TechniqueIdentity identity;
  std::string parameters;
  FormulaAddress address;
  EvaluationMode mode;
  Budget budget;
  std::shared_ptr<const std::atomic_bool> cancellation_signal;
};

class TechniqueKernel {
 public:
  virtual ~TechniqueKernel() = default;
  virtual TechniqueIdentity identity() const = 0;
  virtual EvaluationResult evaluate(const TechniqueRequest& request) const = 0;
};
```

The Engine owns registered kernels using `std::shared_ptr<const TechniqueKernel>`.
The registry maps exact `(id, version)` identities to kernels. The public
Engine receives:

```cpp
bool register_technique(std::shared_ptr<const TechniqueKernel> kernel);
EvaluationResult evaluate_technique(const TechniqueRequest& request);
```

`register_technique` returns `false` without changing registry state on every
invalid registration.

## Engine Controls

The Engine validates registry membership before a kernel runs. It overwrites
the effective request's `cancellation_signal` with its shared signal and
handles cache lookup and admission around the kernel call. Kernels must poll
that signal at their natural safe boundaries. The Engine checks every
confirmed fresh result and every cache hit against the caller budget before
publication. A kernel result receives an Engine trace event identifying
registry rejection, cache hit/miss or cancellation. If cancellation arrives
while a kernel runs, the Engine clears result payloads and publishes a
`Cancelled` result with a Run-scope trace event.

The V1 cache key is canonical and length-delimited over:

```text
technique ID + version + parameter bytes + address + mode
```

Only `Exact` and `Metric` outputs are cacheable, matching existing Engine
behavior. A cached exact value must still be rejected when the caller's budget
does not permit publishing it. Cache byte limits account for stored result and
trace data as well as the canonical key and its parameter bytes; they are a
bounded accounting budget, not a claim of allocator-exact physical RAM.

Kernels must validate technique-specific parameters and addresses before
computing. An invalid request returns `Status::InvalidArgument`; an unsupported
mode returns `Status::UnsupportedMode`. Exceptions never cross Engine API
boundaries: they become `Status::InternalError` with a trace reason.

## PowerOffset Reference Kernel

`PowerOffsetKernel` has identity `bruce.power_offset`, version `1`. It parses
canonical base/scale parameters, converts them to the existing `FormulaSpec`,
and delegates actual calculation to the existing formula evaluator. This
preserves the established symbolic, metric and exact semantics while proving
that the extension path uses real BRUCE math.

`TechniqueRequest` V1 deliberately has no modulus field. Therefore the
reference kernel returns `Status::UnsupportedMode` for `Modular`; a future
version may define a canonical modulus contract instead of silently choosing
one.

The existing FormulaSpec-based Engine methods and C API remain unchanged.
They are compatibility APIs, not aliases for the new registry. A later version
may expose generic technique requests through C once the C++ contract is
stable.

## Test Contract

Tests must prove:

- a valid reference kernel registers and evaluates deterministically;
- an unknown ID, version mismatch, null kernel, invalid identity and duplicate
  registration fail closed;
- invalid parameter text reaches no calculation;
- cache identity separates parameter and version changes;
- a cached exact result still respects a tighter caller budget;
- a fresh technique result still respects the caller budget;
- a throwing identity cannot escape registration;
- cancellation that arrives during a kernel run emits a Run trace;
- Engine cancellation is visible to a technique request;
- FormulaSpec and C API compatibility tests remain green.

## Acceptance Criteria

1. A host program can register and call `PowerOffsetKernel` through
   `TechniqueRequest` without using `FormulaSpec` directly.
2. An unregistered identity cannot produce a confirmed result.
3. Changing ID, version, parameters, address or mode cannot reuse another
   technique's cached result.
4. Existing tests and ABI-facing C header smoke test remain compatible.
5. No runtime dynamic-library loading or arbitrary code execution is added.
