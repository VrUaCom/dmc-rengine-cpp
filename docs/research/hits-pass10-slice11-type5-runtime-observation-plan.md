# HITS Pass 10 — Slice 11: Primitive Type-5 Runtime Observation Plan

Date: 2026-08-15  
Canonical target SHA-256: `e454272ed0fb0247fcbcf300e5d55d7a3e96d50b89b9ffaff81bb978dcbdd082`  
Status: **OBSERVATION PLAN — NO GAME PATCH / NO SEMANTIC PROMOTION YET**

## Why runtime observation is now justified

Static reverse has closed the ownership chain:

`manager entry -> u16 descriptor index -> manager+0x110 descriptor table (0x50 stride) -> C8D0 arg3 -> runtime+0x118 -> constructor dispatch`.

Static writer census also established:
- post-C260 direct primitive-type writes exist for types `1/2/3/4/6`;
- nine register-derived candidate type writes reduce to values `0/2/3`, not `5`;
- no direct type-5 writer was found in the reviewed object-embedded population paths;
- 14 fully bounded static C260 sources / 18 actually referenced entries contain only type `2`;
- parser converter `0x1402481E0` produces only descriptor types `2/3/6/4` from `sphere/box/cylinder/capsule`.

Therefore type `5` is now a targeted runtime/resource-origin question rather than an unresolved global immediate search.

## Important timing correction

`0x14005C260` is **not** a reliable observation point for primitive-type census.

At least 13 reviewed C260 callsites write primitive descriptor type bytes after C260 returns. C260 may register entry/descriptor table pointers before object-local table population is complete.

So a C260-entry hook could observe stale/uninitialized descriptor bytes and must not be used as the canonical type-5 discriminator.

## Primary observation target — `0x14005C8D0` entry

C8D0 is the preferred observation boundary because descriptor resolution and local table population have already happened.

Windows x64 entry state:
- `RCX` = collision manager/context;
- `RDX` = resolved 4-byte entry pointer;
- `R8` = resolved `0x50` primitive descriptor pointer;
- `R9` = allocated runtime collision object;
- stack arg 5 = resolved transform pointer/source.

This single point preserves manager + entry + descriptor + runtime-object identity without cross-hook correlation.

## Capture contract

For every C8D0 observation, capture privately:
- monotonic sequence id / thread id;
- caller VA;
- manager pointer;
- manager category/id fields already established by the dynamic-runtime evidence;
- entry pointer;
- `entry+0x00` raw flags;
- `entry+0x01` transform selector;
- `u16(entry+0x02)` descriptor index;
- descriptor pointer;
- `descriptor+0x00` primitive type;
- runtime object pointer;
- transform pointer;
- SHA-256 of the exact `0x50` descriptor snapshot.

When `descriptor[0] == 5`, additionally retain in the private evidence packet:
- the `0x50` descriptor snapshot needed for reverse comparison;
- parsed float lanes corresponding to descriptor `+0x10/+0x20/+0x30/+0x40`;
- owning manager/caller context;
- resource/blob identity when the pointer provenance is recoverable.

Raw proprietary descriptor bytes must **not** be committed to the public repository. Public promotion should store only hashes, offsets, decoded structural conclusions and controlled census counts.

## Optional secondary observation — `0x1402CC530`

Use only if needed to prove descriptor stability between C8D0 initialization and constructor dispatch.

At CC530, runtime `+0x118` is the primitive descriptor pointer and descriptor byte `+0x00` selects constructor types `0..6`.

A secondary hash comparison can prove whether the `0x50` descriptor was mutated between initialization and construction. It is not required for the first type-5 census.

## Resource-backed descriptors

Several C260 callers resolve entry/descriptor tables directly from serialized resource blobs rather than local writes.

A repeated modern resolver pattern uses blob-relative u32 offsets at raw blob header fields `+0xA4` (entry table) and `+0xA8` (descriptor table), guarded by a field-count/version check. Another resource class uses analogous earlier fields `+0x60/+0x64`.

These offsets are schema-specific and must **not** become universal descriptor-table constants.

Runtime observation at C8D0 intentionally occurs after schema-specific resolution and therefore works for every resource layout.

## Promotion gate for type 5

Do not assign a source-text name to descriptor/runtime type `5` until one of the following is proven:
1. a resource schema/string/token directly identifies the referenced type-5 record; or
2. a writer/loader data-flow proves the semantic source object that produces the three-vertex-face descriptor.

Already confirmed structural facts remain valid:
- descriptor/runtime type `5` uses three vertices;
- descriptor `+0x40` is transformed as a direction without translation and normalized;
- runtime `+0x160` is the face/surface normal;
- dynamic collision metadata receives that normal at `+0x28/+0x2C/+0x30`.

## Safety

- canonical SHA gate required before observation;
- no patching of unknown/custom executable builds;
- expected hook bytes must be acquired/verified privately before any live hook is installed;
- observation-only first; no runtime mutation;
- raw proprietary descriptor snapshots remain private evidence;
- public code/docs store only non-proprietary metadata, hashes, layouts and promoted semantics.
