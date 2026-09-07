# ADR-0003 — Modular Format Architecture

**Status:** Accepted  
**Date:** 2026-09-07

## Context

DMC Rengine reverse-engineers and implements many independent DMC3-HD formats. Putting parsing, runtime semantics, rendering knowledge, evidence notes and future writer logic into a single per-format source file creates coupling, makes evidence boundaries hard to review, and turns parallel reverse work into branch and merge debt.

This rule applies to every currently known and future format, including model, animation, texture, effect, stage, shadow and container families.

## Decision

Every researched format MUST use a modular architecture.

A format implementation is split by responsibility rather than accumulated into a monolithic `format.cpp`.

Recommended structure:

```text
include/dmc_rengine/formats/<fmt>/
    abi.hpp              # serialized layout constants / raw contracts
    ir.hpp               # typed read model / semantic-neutral structures
    parser.hpp           # recognition and read-only parsing API

src/formats/<fmt>/
    parser.cpp

include/dmc_rengine/analysis/<fmt>/
    ...                  # runtime semantics, bindings, transforms, validation

src/analysis/<fmt>/
    ...

include/dmc_rengine/writers/<fmt>/
    ...                  # only after writer/authoring authority exists
```

A compatibility facade such as `formats/<fmt>.hpp` MAY aggregate public pieces, but MUST NOT become the place where all parser, analysis, runtime and writer logic is implemented.

### Separation rules

1. **Raw ABI is separate from semantics.** Serialized offsets, widths, magics and strides belong in ABI/layout contracts. Semantic interpretation belongs in typed IR or analysis modules only when evidence authorizes it.
2. **Parser is read-only and fail-closed.** Recognition, bounds validation and byte materialization stay independent from runtime interpretation and UI behavior.
3. **Runtime semantics are analysis modules.** Hierarchy propagation, skinning, texture/material binding, animation linkage, resolver behavior and similar recovered runtime behavior do not belong in the raw parser.
4. **Evidence is explicit.** Research notes and machine-readable reverse receipts remain separate from product code and identify the exact authority for each promoted claim.
5. **Unknown bytes are preserved.** A field may be `PRESERVED_UNDECODED`; lack of semantics is not permission to zero, reinterpret or discard it.
6. **Writer code is a separate authority boundary.** A reader or EXE-confirmed semantic does not automatically authorize mutation, canonical serialization or original-game acceptance.
7. **Shared-family code requires independent proof.** Similar offsets across MOD/EFM/SCM or any other family do not by themselves justify one shared semantic. Shared modules are created only after cross-format behavior is independently confirmed.
8. **No UI dependency in binary/format/analysis modules.** Native Reader, ModViz, inspectors and other frontends consume canonical modules; they do not own duplicate parsing or reverse logic.
9. **Tests follow module boundaries.** ABI/layout, parser, analysis and writer behavior must be independently testable.
10. **Integration slices remain bounded.** Reverse promotions should be small semantic PRs, rebased to current `main`, validated by exact-head Ubuntu/Windows CI, then merged promptly to avoid long-lived format branches.

## Shared primitives

Cross-format primitives belong in explicit shared modules only when the evidence supports that scope, for example:

- `formats/model_family/*` for independently proven model-family ABI/runtime contracts;
- container/common binary utilities for genuinely shared container mechanics;
- generic math helpers that contain no format-specific semantics.

A shared primitive must not erase a format-specific evidence boundary.

## Consequences

### Positive

- less merge and branch debt;
- independent reverse and review of parser, runtime and writer layers;
- easier reuse by Native Reader, GDSpaces and future tooling;
- unknown fields and evidence status stay visible;
- format-specific semantics cannot silently leak into neighboring formats.

### Cost

- more small files and explicit interfaces;
- some initially duplicated code is tolerated until shared behavior is actually proven;
- promotion from research to writer/authoring requires more deliberate gates.

These costs are accepted because evidence integrity and long-term maintainability are higher priorities than minimizing file count.

## Review triggers

Review this ADR only if the repository adopts a replacement module/component system that preserves the same evidence and authority boundaries. Do not weaken the modular split merely to reduce file count.
