# LHME Fair Algorithm Benchmark Design

**Date:** 2026-10-21
**Status:** Proposed
**Scope:** CPU/RAM benchmark suite for BRUCE LHME and comparable in-memory strategies

## 1. Intent

The benchmark suite must compare BRUCE LHME's FormulaRecipe approach with
ordinary in-memory representations without presenting any algorithm as a
universal winner. Every report must make three things explicit:

1. the problem each strategy is intended to solve;
2. the data semantics and operation being measured; and
3. the cases in which a direct comparison is not valid.

BRUCE's primary value proposition is deterministic reconstruction of a value
from a compact formula plus sparse exceptions. Dense vectors, sorted pairs and
hash tables instead materialize values. Baby-step giant-step (BSGS) and
precomputed chains are time-memory trade-offs for different mathematical or
lookup problems. They therefore belong in separate tracks, not in a single
leaderboard.

## 2. Non-goals and safety boundary

- The suite does not implement password recovery, password hashes, real user
  credentials, rainbow-table attack workflows or cryptographic target search.
- A precomputed-chain experiment uses a fixed, non-cryptographic synthetic
  state transition over a small public toy domain. It is named
  `PrecomputedChainToy`, not `RainbowTable`.
- BSGS uses a deliberately small public modular toy group with a known test
  exponent. It is an educational time-memory measurement, not a practical
  discrete-log solver.
- The suite does not claim cross-machine performance, security strength,
  asymptotic superiority or a universally optimal storage policy.
- GPU, networking, persistent production storage and semantic-routing work are
  out of scope.

## 3. Benchmark tracks

### Track A — common virtual-value storage

Track A compares representations of the *same observable scalar values* for a
fixed virtual address domain. It is the only track that may place BRUCE beside
ordinary value containers.

| Strategy | Intended role | Stored state |
|---|---|---|
| `FormulaRecipe` | deterministic reconstruction | formula binding and bounded cache only |
| `FormulaRecipe + Overrides` | mostly regular values with exceptions | formula binding plus actual exceptions |
| `DenseVector` | sequential, fully materialized values | one scalar per address |
| `SortedPairs` | ordered sparse materialization | address/value pair per stored value |
| `HashTable` | random-access, fully materialized values | hash-keyed address/value entries |

The common value is `h(i) mod M`, evaluated through the same FormulaRecipe
where `h(i) = 100 * (2^i - 1)` and `M` is one fixed non-zero 64-bit modulus.
Materialized strategies store that resulting `UInt64` scalar. FormulaRecipe
evaluates the same modular result on demand. This makes lookup values exactly
comparable while retaining FormulaRecipe's normal deterministic evaluation
path.

Sparse override scenarios use the same deterministic address set and the same
replacement scalar for all strategies that support exceptions. The benchmark
must verify every sampled lookup against an independent reference evaluator
before timing is recorded.

The first domain sizes are 1,024; 65,536; and 1,000,000 addresses. Override
densities are 0 %, 0.1 %, 1 %, 10 % and 100 %. The 100 % row is a deliberate
crossover control, not evidence that FormulaRecipe should be used for fully
exceptional data.

### Track B — BSGS toy time-memory trade-off

Track B measures a small, public modular equation with a known exponent. It
reports BSGS beside a deterministic linear scan only for that equation. Both
are checked against the known answer.

The report contains group size, baby-step count, solved instances, elapsed
time, isolated process peak RAM and correctness. It does **not** compare BSGS
to FormulaRecipe, DenseVector or HashTable, because the algorithms solve
different problems.

### Track C — synthetic precomputed chains

Track C measures a toy state-transition chain over a fixed public domain. The
chain function is non-cryptographic and operates only on generated numerical
tokens. It measures chain count, chain length, endpoint-table memory, coverage,
lookup work, false-chain candidates and successful toy-token resolution.

This explains a generic precomputation time-memory trade-off without claiming
to represent password-hash recovery or a production security use case. It is
reported separately from Track A and Track B.

## 4. Measurement contract

Every measurement records:

- benchmark track and strategy classification;
- data domain size and override density or toy-problem parameters;
- deterministic workload seed and exact number of read operations;
- successful correctness checks and a result checksum;
- build time, median lookup time, minimum and maximum lookup time;
- isolated process peak RAM on Linux when available; and
- container accounting as supplementary data, never as a replacement for peak
  RAM.

Each timed case executes one unmeasured warm-up plus eleven measured runs.
The median is the primary timing value; minimum and maximum expose run-to-run
spread. A measurement that fails a correctness check is reported as failed and
must not contribute timing or RAM numbers.

All Track A strategies receive exactly the same pre-generated address sequence:

- sequential reads;
- deterministic pseudo-random reads; and
- a repeat-heavy locality workload.

The seed, domain, modulus, query count, cache policy and override layout are
printed in every report. FormulaRecipe's bounded cache is either disabled for
every cold-read case or measured as its own explicitly labelled warm-cache
case; it is never silently enabled only for BRUCE.

## 5. Fairness rules

1. No total score, winner label, bar-chart ordering by a single metric or
   cross-track ranking is allowed.
2. Every strategy is named with its intended access pattern and materialization
   policy before its numeric results.
3. Same-semantic lookups are required in Track A. A symbolic BigInt capability
   demo may be reported separately, but cannot be used to claim a direct
   `UInt64` table win.
4. Process peak RAM is the decision-oriented memory number. Analytic container
   bytes are retained only to explain allocator and metadata overhead.
5. BSGS and `PrecomputedChainToy` remain separate reports with an explicit
   `not comparable to Track A` marker.
6. Any platform that lacks isolated peak-RAM support prints `unavailable`, not
   a fabricated replacement value.

## 6. Architecture and public outputs

The benchmark code stays in `demos/lhme/benchmarks/` and is not part of the
public BRUCE LHME library API. It uses small internal benchmark adapters:

```text
BenchmarkCase
  -> strategy adapter / toy experiment
  -> deterministic workload
  -> isolated measurement when applicable
  -> correctness verifier
  -> structured report
```

The planned executables are:

```text
lhme_fair_storage_benchmark
lhme_bsgs_toy_benchmark
lhme_precomputed_chain_toy_benchmark
```

The reports are human-readable text today and must keep stable labels suitable
for CTest assertions. A later machine-readable output format may be added only
as an additional format, not by weakening the text contract.

## 7. Testing

Tests and benchmark assertions cover:

- identical Track A lookup values across every strategy and every workload;
- deterministic query sequences, override layouts and checksums;
- all required domain sizes and override densities;
- warm-up exclusion and exactly eleven recorded timing samples;
- median/minimum/maximum ordering;
- isolated-memory availability handling;
- BSGS and linear scan returning the known toy exponent; and
- precomputed-chain toy coverage and resolution claims matching the generated
  chain data.

The existing benchmark suite remains intact. The new suite is added to CTest
only when its default workload completes in an ordinary developer cycle; larger
parameter sweeps remain explicit command-line runs.

## 8. Acceptance criteria

- Track A reports all five storage strategies over the specified workloads and
  verifies equal observed values before measurements.
- Reports contain median, minimum, maximum, build time and isolated peak RAM
  (or explicit unavailability) for every successful case.
- Track B and Track C run solely on generated, non-sensitive toy data and
  visibly state that they cannot rank against Track A.
- No result page contains a global winner or a claim that any strategy is best
  outside its stated workload.
- A benchmark review updates the public documentation with method, raw
  parameters, results and limitations after the first full run.
