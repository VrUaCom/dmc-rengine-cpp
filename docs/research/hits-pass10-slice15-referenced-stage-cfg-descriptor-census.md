# HITS Pass 10 — Slice 15: Referenced Stage-CFG Primitive Descriptor Census

**Date:** 2026-08-15  
**Status:** IMPLEMENTED / SYNTHETICALLY TESTED; REAL STAGE-CFG CORPUS EVIDENCE PENDING

## Position in Pass 10

This is **Slice 15**.

Do not confuse it with the already existing validated slices:

- Slice 13 — Serialized Collision Triplet View (`0x40 / 0x04 / 0x50`), including independent `em000.pac` data-side validation;
- Slice 14 — Stage-CFG Entry / Primitive-Descriptor View for confirmed slot pairs `39/40` modern and `22/23` legacy observed, intentionally without transform-selector bounds.

Slice 15 extends the Slice-14 view with a deterministic **referenced-only descriptor census**. It does not reopen or rename Slices 13/14.

## Purpose

The current Stage-CFG evidence establishes:

- modern observed entry table slot 39 and primitive descriptor table slot 40;
- legacy observed entry table slot 22 and primitive descriptor table slot 23;
- entry stride `0x04`;
- descriptor index `u16 LE @ entry+0x02`;
- primitive descriptor stride `0x50`;
- raw primitive type byte `@ descriptor+0x00`.

The next type-5 investigation must not return to global immediate scans or unbounded `0x50` walks. Slice 15 provides the bounded scanner primitive needed for representative Stage-CFG corpus work.

## Product contract

`hits_stage_cfg_collision_tables::View::referenced_descriptor_census()`:

1. walks only bounded Stage-CFG entry records already admitted by the Slice-14 view;
2. validates each referenced descriptor index against the exact selected descriptor-table size;
3. counts references per descriptor index;
4. emits only descriptor indices actually referenced by entries;
5. preserves raw primitive type byte at descriptor `+0x00`;
6. reports invalid entry indices separately instead of silently dropping them;
7. emits referenced descriptors deterministically in ascending descriptor-index order.

The census exposes:

- total entry count;
- valid descriptor-reference count;
- invalid entry indices;
- referenced descriptor index;
- raw primitive type;
- reference count per referenced descriptor;
- helper counts for any requested raw primitive type.

## Type-5 evidence boundary

A census result with raw primitive type `5` means only:

> a bounded Stage-CFG entry references a `0x50` primitive descriptor whose byte `+0x00` equals `5`.

It does **not** by itself prove:

- an authored/source token for type 5;
- a gameplay name;
- an upstream producer identity;
- runtime activation in a specific room/frame;
- complete type-5 geometry/contact behavior;
- original-game behavioral equivalence.

The already promoted bounded semantic authority remains: descriptor/runtime type 5 is structurally a three-vertex face representation. Authored/source vocabulary remains unresolved.

## Why this is the correct next scanner

Earlier bounded static C260 census work observed only type 2 in the fully resolved subset, but that was explicitly corpus/scope-limited negative evidence. Slice 13 then established a generic serialized triplet view, and Slice 14 established the Stage-CFG entry→descriptor adapter without inventing transform provenance.

Slice 15 now supports this evidence path:

```text
Stage-CFG PAC
  -> canonical production container parse (outside this adapter)
  -> Slice-14 exact slot-pair view
  -> bounded entry table
  -> actually referenced descriptor indices only
  -> raw descriptor type census
  -> referenced type-5 indices, if any
  -> targeted descriptor/upstream producer investigation
```

No PAC parser, archive resolver or filesystem resolver is introduced by Slice 15.

## Synthetic validation

Regression covers:

- modern slot 39/40 selection;
- legacy slot 22/23 selection;
- repeated references to the same descriptor;
- deterministic aggregation;
- synthetic raw type-5 observation without semantic promotion;
- invalid descriptor-reference reporting;
- exact canonical executable SHA gate;
- packed-build rejection;
- malformed descriptor-table-size rejection.

Synthetic tests validate the implementation contract only. They are **not evidence that a real Stage-CFG resource contains a referenced type-5 descriptor**.

## Production PAC / CLI boundary

The Pass-10 branch contains generic container contracts and the Slice-14 adapter, but a production PAC parser is not owned by this HITS slice. Do not add a private/direct PAC parser to HITS CLI code merely to run the census.

Real corpus execution must consume a `ContainerDocument` and bytes from the canonical production GDSpaces/container path when that stack is present in the integration branch. This preserves the no-second-resolver/no-second-parser rule.

## Current corpus status

Current File Library and Google Drive search did not expose raw `room/stXXXcfg.pac` binaries suitable for direct corpus execution. Existing sanitized research material confirms the structural/provenance boundaries but does not contain a complete real Stage-CFG referenced-descriptor census.

Therefore no real type-5 presence/absence claim is promoted by Slice 15 yet.

## Next evidence gate

Run the census over representative legally supplied Stage-CFG resources selected from the full executable-derived Stage catalog while preserving at minimum:

- resource-set/catalog identity;
- numeric Stage identity when applicable;
- source bank/source row/global catalog row;
- Stage-CFG canonical resource identity and hash;
- detected slot generation;
- entry count;
- primitive descriptor count;
- invalid descriptor-reference count;
- referenced descriptor index / raw type / reference-count tuples.

If referenced type-5 descriptors are found, continue with targeted upstream authored/population tracing for those exact resource/descriptor identities. If none are found, retain the result as representative-corpus negative evidence only; do not promote global absence.

## Separate open boundary

Stage-CFG transform-source provenance remains a separate reverse target from Slice 14. Slot 38 is a related collision/source block and is not promoted as the `0x40` transform table. Slice 15 does not change that boundary.
