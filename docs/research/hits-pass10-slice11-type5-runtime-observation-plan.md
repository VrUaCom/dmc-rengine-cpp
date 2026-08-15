# HITS Pass 10 — Slice 11: Primitive Type-5 Runtime Observation Plan

Date: 2026-08-15  
Canonical target SHA-256: `e454272ed0fb0247fcbcf300e5d55d7a3e96d50b89b9ffaff81bb978dcbdd082`  
Status: **OBSERVATION PLAN — CORRECTED BY STAGE-CFG PAC SLOT PROVENANCE**

## Why runtime observation is justified

Static reverse has closed the ownership chain:

`stage CFG collision entry -> u16 descriptor index -> primitive descriptor table (0x50 stride) -> C8D0 arg3 -> runtime+0x118 -> constructor dispatch`.

Static writer census also established:
- post-C260 direct primitive-type writes exist for types `1/2/3/4/6`;
- nine register-derived candidate type writes reduce to values `0/2/3`, not `5`;
- no direct type-5 writer was found in the reviewed object-embedded population paths;
- 14 fully bounded static C260 sources / 18 actually referenced entries contain only type `2`;
- parser converter `0x1402481E0` produces only descriptor types `2/3/6/4` from `sphere/box/cylinder/capsule`.

Therefore type `5` is now a targeted Stage-CFG/resource/runtime-origin question rather than a global immediate search.

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
- current stage id when available;
- manager pointer;
- entry pointer;
- `entry+0x00` raw flags;
- `entry+0x01` transform selector;
- `u16(entry+0x02)` descriptor index;
- descriptor pointer;
- `descriptor+0x00` primitive type;
- runtime object pointer;
- transform pointer;
- SHA-256 of the exact `0x50` descriptor snapshot;
- Stage-CFG PAC slot provenance when the descriptor lies in the known slot-backed paths.

When `descriptor[0] == 5`, additionally retain in the private evidence packet:
- the `0x50` descriptor snapshot needed for reverse comparison;
- parsed float lanes at descriptor `+0x10/+0x20/+0x30/+0x40`;
- owning manager/caller context;
- current stage id;
- `room\\stXXXcfg.pac` identity;
- PAC slot generation and descriptor-table slot number when resolved.

Raw proprietary descriptor bytes must **not** be committed to the public repository. Public promotion stores only hashes, offsets, slot identities, decoded structural conclusions and controlled census counts.

## Optional secondary observation — `0x1402CC530`

Use only if needed to prove descriptor stability between C8D0 initialization and constructor dispatch.

At CC530, runtime `+0x118` is the primitive descriptor pointer and descriptor byte `+0x00` selects constructor types `0..6`.

A secondary hash comparison can prove whether the `0x50` descriptor was mutated between initialization and construction. It is not required for the first type-5 census.

## CORRECTED resource provenance — Stage CFG PAC slots

The earlier Slice-11 wording described `+0xA4/+0xA8` and `+0x60/+0x64` as schema-specific fields inside an abstract resource blob. That interpretation is **REJECTED / SUPERSEDED**.

Canonical static reverse now proves:
- resource selector `0x1401AE310` with `resource_kind=1` resolves the current stage resource `room\\stXXXcfg.pac`;
- `0x1401AF000` uses that selector and stores the resolved resource handle at object `+0x650`;
- the resource data consumed by the collision paths is the PAC container itself;
- PAC `+0x04` is slot count and the u32 slot-offset table begins at `+0x08`.

Therefore the previously observed offsets are **PAC offset-table entries**:

### Modern observed Stage-CFG collision layout
- `+0xA0` = PAC slot 38 — related source/runtime block;
- `+0xA4` = PAC slot 39 — C260 entry table;
- `+0xA8` = PAC slot 40 — primitive descriptor table.

The code gates these reads with minimum PAC slot counts 39 / 40 / 41 respectively.

### Legacy observed CEm008 Stage-CFG collision layout
- `+0x5C` = PAC slot 21 — related source/runtime block;
- `+0x60` = PAC slot 22 — C260 entry table;
- `+0x64` = PAC slot 23 — primitive descriptor table.

The code gates these reads with minimum PAC slot counts 22 / 23 / 24 respectively.

These are two observed Stage-CFG PAC slot layouts, not universal inner-blob header offsets.

## Type-5 data-side target

The strongest data-side target is now concrete:
- modern layout: `room\\stXXXcfg.pac` **slot 40**;
- legacy layout: `room\\stXXXcfg.pac` **slot 23**.

The descriptor table is then indexed through the already confirmed `u16(entry+0x02)` with `0x50` stride.

If raw Stage-CFG PAC slot data is unavailable, C8D0 remains the canonical observation point because it sees the resolved descriptor after slot selection and entry-index resolution.

## Promotion gate for type 5

Do not assign a source-text name to descriptor/runtime type `5` until one of the following is proven:
1. a Stage-CFG PAC slot/format token or resource structure directly identifies the referenced type-5 record; or
2. a writer/loader/runtime trace proves the semantic source object that produces the three-vertex-face descriptor.

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
- raw proprietary PAC/descriptor snapshots remain private evidence;
- public code/docs store only non-proprietary metadata, hashes, slot identities, layouts and promoted semantics.
