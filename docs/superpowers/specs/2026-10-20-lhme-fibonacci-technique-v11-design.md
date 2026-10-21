# LHME Fibonacci Technique V1.1 Design

## Purpose

Prove that LHME is a reusable deterministic C++ computation library, not a
single-formula engine. V1.1 adds a second, independent technique,
`FibonacciKernel`, and lets every technique receive an optional modulus through
the public request contract. A small consumer demo uses only installed-style
public headers and the static library interface.

The intended result is a lazy CPU/RAM implementation: the engine stores a
compact request, parameters and bounded cache entry; it only materializes a
Fibonacci integer when an Exact request has sufficient budget.

## Scope

- Add `FibonacciKernel` with identity `bruce.fibonacci`, version `1`.
- Extend `TechniqueRequest` with `std::optional<BigInt> modulus`.
- Enable `PowerOffsetKernel` Modular mode by forwarding that modulus to the
  existing formula evaluator.
- Add a standalone C++ technique demo target built solely against public
  headers and `bruce_lhme_core`.
- Preserve FormulaSpec, EvaluationRequest, C API, streaming and existing
  cache-mode behavior.

## Non-goals

- No GPU implementation, runtime-loaded plugins, foreign-language ABI or C
  API expansion.
- No persistence of Fibonacci values or unbounded memoization tables.
- No range scheduler support for arbitrary techniques in this increment.
- No claim that the Metric output is an exact Fibonacci bit length for every
  representable address.

## Public Contract

`TechniqueRequest` gains the following field after `budget`:

```cpp
std::optional<BigInt> modulus;
```

It is meaningful only for `EvaluationMode::Modular`. The Engine passes it to
the selected kernel unchanged. A technique that does not implement Modular
returns `Status::UnsupportedMode`; a technique that implements it rejects a
missing, zero or negative modulus with `Status::InvalidArgument`.

Modular results remain non-cacheable in V1.1, matching the existing Engine
policy that only Exact and Metric results are cached. Therefore a modulus does
not become cache-key material in this version.

## FibonacciKernel

### Identity and parameters

```text
identity:   bruce.fibonacci
version:    1
parameters: algorithm=8:fast_dbl
```

The public constant for this parameter text is an immutable character array so
hosts can assign it directly to the owning `std::string` parameter field in a
C++ aggregate request.

The parameter text is the sole accepted canonical representation. It is
nonempty so it meets the existing generic request validation and is present in
the deterministic cache key. Any other parameter text returns
`Status::InvalidArgument` before computation.

The address index is `n` in `F(n)` and must be nonnegative.

### Evaluation behavior

| Mode | Confirmed output | Validation and storage behavior |
|---|---|---|
| Symbolic | `symbolic_expression = "F(n)"` | No integer materialization. |
| Metric | `metric_bit_length = ceil(7*n/10)` | This is a deterministic safe upper bound, not an exact claim; no Fibonacci integer is materialized. |
| Modular | `integer = F(n) mod modulus` | Requires positive modulus; intermediate values are reduced every Fast-Doubling step. |
| Exact | `integer = F(n)` | Uses Fast-Doubling only after a safe budget preflight. |

The metric upper bound is zero for `n = 0` and `ceil(7*n/10)` otherwise. It
uses saturating unsigned arithmetic so an extremely large valid address cannot
wrap to a smaller estimate.

### Algorithm and SafeStop

The kernel uses iterative Fast-Doubling. It derives `(F(k), F(k+1))` from the
bits of `n`, with `O(log n)` loop iterations. Exact mode uses BigInt values;
Modular mode reduces each intermediate operation by the supplied modulus.

The kernel checks the Engine-owned cancellation signal before each doubling
step. On cancellation it returns `Status::Cancelled` without publishing an
integer. The Engine remains responsible for final cancellation publication,
trace composition and cache admission.

Before Exact computation, the kernel uses the metric upper bound to reject
requests exceeding `max_bit_length`. Its temporary-memory estimate is eight
times the rounded-up metric bytes, accounting conservatively for the pair,
products and sums held by Fast-Doubling. It estimates Fast-Doubling work as
`2 * bit_length(n)` and rejects an insufficient `max_operations` budget. All
three estimates use saturating arithmetic. The Engine independently rechecks a
confirmed result before publication.

## PowerOffsetKernel Modular Behavior

When its `TechniqueRequest` has `mode == Modular`, PowerOffsetKernel forwards
the optional modulus to the existing `EvaluationRequest`. A positive modulus
returns the existing deterministic residue; missing, zero or negative modulus
returns `Status::InvalidArgument`. Symbolic, Metric and Exact behavior remain
unchanged.

## Demo

`lhme_technique_demo` is an executable source under `demos/lhme/`. It includes
only public `bruce/lhme/*` headers, links `bruce_lhme_core`, registers
`FibonacciKernel`, and verifies:

- Exact `F(10) = 55`;
- Modular `F(10) mod 17 = 4`;
- a Metric request returns a size metric without an integer;
- a too-small Exact budget returns `BudgetExceeded`.

The demo is registered as a CTest target so the consuming workflow is checked
in ordinary project verification.

## Test Contract

Tests must prove:

- only the canonical Fibonacci parameters and nonnegative addresses are
  accepted;
- Exact Fibonacci reference values `F(0)`, `F(1)`, `F(10)` and `F(100)` are
  correct;
- Metric does not materialize an integer and is at least the exact bit length
  for representative values;
- Exact rejects insufficient bit, operation and temporary-memory budgets before
  a large result is returned;
- Modular requires a positive modulus and returns known residues;
- a pre-set Engine cancellation signal prevents Exact computation;
- PowerOffset gains Modular mode through TechniqueRequest without changing the
  old FormulaSpec/C API behavior;
- the new public-header demo exits successfully; all existing tests remain
  green.

## Acceptance Criteria

1. A C++ host registers `FibonacciKernel` and receives deterministic Exact,
   Symbolic, Metric and Modular results through `Engine::evaluate_technique`.
2. A million-index Metric request produces no giant Fibonacci integer or table.
3. Exact evaluation is fail-closed for invalid input, cancellation and every
   configured resource limit.
4. PowerOffset technique Modular mode returns the established formula residue
   when given a positive modulus.
5. The public library installation and all CTest targets remain compatible.
