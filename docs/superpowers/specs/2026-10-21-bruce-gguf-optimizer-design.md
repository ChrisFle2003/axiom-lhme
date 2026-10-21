# BRUCE GGUF Optimizer — Design Specification

## Status

Proposed architecture, approved for specification on 2026-10-21. This document
does not authorize implementation until its review and a separate implementation
plan are approved.

## Purpose

Add a deterministic BRUCE layer that inspects existing GGUF model files and
produces a reproducible CPU/RAM storage plan. The project must reuse the GGUF
container rather than invent a replacement model format or an inference engine.

The optimizer answers questions such as:

- Which tensors occupy the file and their data types, shapes, offsets and sizes?
- What RAM is required by a selected loading policy and budget?
- Which tensor chunks should be resident, mapped on demand or deferred?
- Which blocks are candidates for an exact, separately verified FormulaRecipe
  representation in a future extension?

It never changes the mathematical meaning of a model merely to improve storage
or ordering.

## Scope

### Version 1: inspection and planning only

Version 1 reads a local GGUF file without modifying it. It validates its
container structure, builds a tensor inventory and emits a deterministic
storage-plan report plus a trace.

Its core responsibilities are:

1. Parse the GGUF header, key-value metadata and tensor descriptors.
2. Validate offsets, sizes, alignment and addressable ranges before planning.
3. Calculate file and RAM estimates per tensor and per configurable chunk.
4. Produce a budget-aware loading plan: resident, memory-mapped/on-demand or
   deferred.
5. Identify only *candidates* for future exact pattern analysis. No candidate
   may alter stored tensor data in Version 1.
6. Record every decision in a deterministic trace.

### Explicit non-goals

Version 1 does not:

- run inference, implement an LLM or replace llama.cpp/ggml;
- quantize, dequantize, retrain or change model parameters;
- reorder scalar tensor values or dimensions;
- write a changed GGUF file;
- use BSGS, chain tables, password material or security-related lookup logic;
- require an embedding model, a small LLM or natural-language control;
- claim that arbitrary trained weights are formula-compressible.

## Design principles

### Preserve model semantics

A tensor element's coordinate is part of its meaning. For example,
`weight[output][input]` cannot be moved merely because another value is smaller
or closer to it numerically. BRUCE may sort *inventory records* and *chunk
descriptors* for deterministic planning, but it must not reorder weight values.

### FormulaRecipe is opt-in and exact

`FormulaRecipe` is a representation option only when it reconstructs every
covered value exactly and any exceptions are explicitly recorded. A heuristic,
statistical similarity or approximate fit is not sufficient. Version 1 reports
potentially repetitive blocks but stores no FormulaRecipe-backed tensor.

### GGUF remains the compatibility boundary

GGUF remains the source file and the interoperability format. A BRUCE report or
future sidecar must never make an unchanged GGUF unreadable by a standard GGUF
consumer. The GGUF specification and the matching ggml/llama.cpp implementation
are the format authority.

### Determinism and fail-closed validation

Given identical bytes and identical configuration, the inventory, plan and trace
must be byte-for-byte stable. Malformed headers, unsupported versions, overflow,
misaligned tensor offsets and out-of-file ranges fail before a plan is emitted.

## Architecture

```text
unchanged GGUF file
        |
        v
GGUF Reader + Validator
        |
        v
Tensor Inventory (metadata only)
        |
        +--> Chunk Estimator
        |         |
        |         v
        +--> BRUCE Storage Planner ---> Plan + Trace + Report
        |
        +--> Exact Pattern Candidate Scanner (report-only)
```

### GGUF Reader and Validator

The reader decodes only standard GGUF structure needed for safe inventory:
magic, supported version, metadata count, tensor count, tensor name, rank,
dimensions, type and data offset. It resolves each descriptor into an absolute
file range only after overflow and alignment checks.

The reader does not materialize tensor payloads during ordinary inspection.

### Tensor Inventory

Each immutable inventory record contains:

```text
name
shape
element type
logical element count
file offset and stored byte range
alignment status
estimated decoded RAM, when known
```

Records have a stable presentation order: data offset first, then tensor name
as a tie-breaker. This order is an index convention, not a change to model data.

### Storage Planner

The planner consumes the inventory and an explicit configuration:

```text
memory budget
chunk-size limit
loading policy: all-resident | budgeted | metadata-only
optional ordered tensor-use profile
```

Without a supplied use profile, the planner must not pretend to know inference
execution order. It uses deterministic offset order and marks that choice as a
fallback in its trace. With a user-supplied profile, it may prioritize listed
tensors while keeping all unlisted tensors available on demand.

The output assigns each tensor or chunk one of:

```text
resident
memory-mapped / on-demand
deferred
rejected (invalid or over budget under the selected strict policy)
```

### Exact Pattern Candidate Scanner

This optional report-only pass operates on explicitly selected tensor chunks.
It calculates inexpensive facts such as constant runs, repeated blocks, sparse
exceptions and simple exact arithmetic progressions. It must distinguish:

```text
candidate found       = evidence worth later exact verification
exactly verified      = every value reproduced with a declared formula
not representable     = retain ordinary GGUF payload
```

No approximate compression, neural prediction or silent value replacement is
allowed.

### Trace and report

The report states original file bytes, per-tensor stored bytes, plan assignment,
budget, assumptions and validation outcome. The trace includes the input file
hash, parser/version decision, policy inputs and the reason for every assignment.

## Interfaces

The future command-line interface is intentionally structural rather than
natural-language based:

```text
bruce_gguf_inspect MODEL.gguf --memory-budget BYTES --policy budgeted
```

It produces a human-readable report and a stable machine-readable plan suitable
for tests and later consumers. Natural language, embeddings and model-assisted
decisions are outside Version 1.

## Safety and error handling

- Refuse a file before payload access if any count, dimension product, offset or
  range would overflow its destination type.
- Refuse unsupported GGUF versions rather than guessing their layout.
- Treat metadata and tensor names as data, never as instructions or paths.
- Enforce caller-provided file-size and work-budget limits.
- Keep parser errors distinct from a valid file that simply exceeds the selected
  RAM budget.
- Do not open network connections or download model files.

## Testing strategy

Tests use small, generated public fixtures only:

1. A valid minimal GGUF fixture yields a stable inventory and plan.
2. Equal inputs and configuration yield equal report and trace bytes.
3. Invalid magic, unsupported version, truncated descriptors, overflow and
   misaligned/out-of-range offsets fail closed.
4. Budgeted planning assigns predictable resident/on-demand/deferred states.
5. Pattern scanning can report a candidate but cannot classify it as exact
   without complete verification.
6. A round-trip compatibility test proves inspection leaves the input GGUF bytes
   unchanged.

## Acceptance criteria

Version 1 is complete when it can inspect a supported local GGUF file under a
configured resource budget, generate a deterministic plan and trace, reject
malformed files safely, and prove through tests that it does not modify the
source file or tensor value ordering.

## Future phases

1. Optional GGUF-compatible lossless rewriter, only after independent
   compatibility and byte-level validation.
2. Optional BRUCE sidecar for *exactly verified* FormulaRecipe plus Override
   metadata, without changing the original GGUF payload.
3. Optional runtime integration that can consume a sidecar. It remains separate
   from standard GGUF compatibility and must be explicitly enabled.
