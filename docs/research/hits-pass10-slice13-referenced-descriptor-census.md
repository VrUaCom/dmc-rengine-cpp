# HITS Pass 10 — Slice 13 — Referenced Stage-CFG primitive descriptor census

**Date:** 2026-08-15  
**Status:** IMPLEMENTED / SYNTHETICALLY TESTED; REAL CORPUS EVIDENCE PENDING

## Purpose

Slice 12 closed the Stage-CFG PAC provenance boundary for dynamic collision tables:

- modern observed layout: entry table slot 39, primitive descriptor table slot 40;
- legacy observed layout: entry table slot 22, primitive descriptor table slot 23;
- entry stride: `0x04`;
- primitive descriptor stride: `0x50`;
- entry `+0x02` contains the `u16` primitive descriptor index.

The next investigation must not return to global immediate scans or unbounded `0x50` table walks. Slice 13 therefore adds a deterministic **referenced-only census** over the existing SHA-gated Stage-CFG collision-table view.

## Product contract

`hits_stage_cfg_collision_tables::View::referenced_descriptor_census()`:

1. walks only bounded Stage-CFG entry records;
2. validates each referenced descriptor index against the exact selected descriptor-table size;
3. counts references per descriptor index;
4. emits only descriptor indices that are actually referenced by entries;
5. preserves primitive type as the raw descriptor byte at `+0x00`;
6. reports invalid entry indices separately instead of silently skipping them;
7. emits descriptors in deterministic ascending descriptor-index order.

The census exposes:

- total entry count;
- valid reference count;
- invalid entry indices;
- referenced descriptor index;
- raw primitive type;
- reference count per referenced descriptor;
- helper counts for a requested raw primitive type.

## Type-5 boundary

A census result with raw primitive type `5` means only:

> an entry in the bounded Stage-CFG entry table references a `0x50` primitive descriptor whose byte `+0x00` is `5`.

It does **not** by itself prove:

- an authored/source token for type 5;
- gameplay naming;
- producer identity;
- complete type-5 geometry semantics;
- runtime use in a particular room/frame;
- original-game behavioral equivalence.

The already-promoted structural statement remains: descriptor/runtime type 5 is a three-vertex face representation. Authored/source vocabulary is unresolved.

## Why this is the correct next step

The prior bounded static census observed only type 2 in its 14 fully resolved C260 callsites, but that was explicitly negative evidence rather than proof of type-5 absence. Slice 12 gave a concrete resource-backed target. Slice 13 now provides the safe scanner primitive needed to answer the next evidence question from representative user-supplied Stage-CFG resources:

```text
Stage-CFG PAC
  -> exact slot pair by evidenced generation
  -> bounded entry table
  -> referenced descriptor indices only
  -> raw descriptor type census
  -> type-5 referenced indices, if any
  -> targeted raw descriptor / upstream producer investigation
```

## Validation

Synthetic regression covers:

- modern slot 39/40 selection;
- legacy slot 22/23 selection;
- repeated references to the same descriptor;
- deterministic aggregation;
- raw type-5 detection without semantic promotion;
- invalid descriptor-reference reporting;
- exact canonical executable SHA gate;
- packed-build rejection;
- malformed descriptor-table size rejection.

Synthetic tests validate the implementation contract only. They are **not** evidence that any real Stage-CFG resource contains a referenced type-5 descriptor.

## Next evidence gate

Run this census over representative legally supplied Stage-CFG PAC resources selected from the full executable-derived Stage catalog, preserving at minimum:

- resource-set/catalog identity;
- numeric Stage identity when applicable;
- source bank/row/global row provenance;
- Stage-CFG resource identity/hash;
- detected slot generation;
- entry count;
- descriptor count;
- invalid reference count;
- referenced descriptor index/type/reference-count tuples.

If a referenced type-5 descriptor is found, the next reverse step becomes targeted producer/authored-source tracing for that exact descriptor/resource identity. If none is found, the result remains corpus-scoped negative evidence and must not be promoted to global absence.
