# PNST Read-Only Structural Parser

**Date:** 2026-08-18  
**Status:** IMPLEMENTED BOUNDED STRUCTURAL SLICE / SYNTHETIC REGRESSION; REAL-PNST EXECUTION RECEIPT SEPARATE

## Purpose

Promote PNST structural reading on the same evidence-backed relative-slot envelope already used by the clean PAC parser, without treating PAC and PNST as semantically identical resources.

## Shared structural envelope

Current evidence supports both families using:

```text
+0x00  4-byte family magic
+0x04  u32 LE declared slot count
+0x08  u32 LE container-base-relative slot offsets
```

- `0` preserves an empty physical slot;
- populated offsets must lie after the complete header/table and inside the supplied byte span;
- declared sparse topology is preserved exactly;
- populated extent is the next greater distinct populated offset or container end;
- duplicate populated offsets remain separate physical slot identities sharing one bounded span;
- no universal 16-byte alignment reject rule is applied.

The extent rule and `1 << 20` slot limit are product policies, not recovered original DMC3 size/capacity ABI claims.

## Architecture

`RelativeSlotContainerSpec` / `parse_relative_slot_container()` own only the shared binary envelope.

Format facades remain separate:

- `PacParser` requires exact `PAC\0` magic and returns format `PAC`;
- `PnstParser` requires exact `PNST` magic and returns format `PNST`.

This prevents a second independent PAC/PNST structural implementation from drifting while keeping family identity explicit.

Synthetic slot names (`slot_NNNN.bin` / `slot_NNNN.empty`) are presentation identities required by the shared `ContainerEntry` contract. They do not claim original filenames or semantic roles.

## Sparse regression

The PNST test deliberately uses an evidence-shaped 11-slot topology with only physical slots 0 and 10 populated. Slots 1..9 remain explicit empty slots and are never compacted to dense ordinals.

The test also covers:

- exact PNST magic;
- bounds failures into the offset table / at container end;
- shared product slot-count limit;
- a valid populated offset with residue 8 modulo 16.

Classifier regression proves PNST magic overrides a misleading `.pac` extension.

## Boundaries

This slice does not promote:

- `.index` labels as runtime slot authority;
- ordinal non-empty entries as physical slot identity;
- PNST writer/repack behavior;
- recursive container-tree policy;
- original typed post-load recursion semantics;
- whole-resource-runtime equivalence.

A real hash-bound PNST parser-execution receipt remains a separate validation tier even though Pass-17 corpus evidence already establishes real PNST artifacts and the shared structural envelope.
