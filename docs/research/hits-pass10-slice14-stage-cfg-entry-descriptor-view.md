# HITS Pass 10 — Slice 14: Stage-CFG Entry / Primitive-Descriptor View

Date: 2026-08-15  
Canonical target SHA-256: `e454272ed0fb0247fcbcf300e5d55d7a3e96d50b89b9ffaff81bb978dcbdd082`  
Status: **IMPLEMENTED / REVIEWED / DEBUGGED / VALIDATED**

## Purpose

Slice 12 proves Stage-CFG PAC collision table slots:
- modern observed layout: entry table slot 39, primitive descriptor table slot 40;
- legacy observed CEm008 layout: entry table slot 22, primitive descriptor table slot 23.

Slice 13 proves the serialized entry and primitive-descriptor ABI, while second review proves Stage-CFG slot38 is not established as the `0x40` transform table.

Slice 14 therefore exposes only the **confirmed Stage-CFG entry→descriptor relationship**. It intentionally does not validate transform-selector bounds.

## Architecture

Input:
- exact canonical executable SHA;
- raw PAC bytes;
- existing `formats::ContainerDocument` produced by the canonical PAC/PNST parser;
- selected Stage-CFG slot generation.

No second PAC parser is introduced.

The adapter resolves the appropriate `ContainerEntry` records and opens bounded spans over:
- entry table;
- primitive descriptor table.

## Modern generation

`SlotGeneration::modern_cem_stage_cfg`
- entry table slot 39;
- primitive descriptor table slot 40.

## Legacy observed CEm008 generation

`SlotGeneration::legacy_cem008_stage_cfg`
- entry table slot 22;
- primitive descriptor table slot 23.

## Decoded entry ABI

Each 4-byte entry exposes:
- `flags` — `u8 +0x00`;
- `transform_selector` — `u8 +0x01`;
- `descriptor_index` — `u16 LE +0x02`.

Descriptor table:
- exact record stride `0x50`;
- primitive type byte at descriptor `+0x00`.

The view validates only:

`descriptor_index < primitive_descriptor_count`

because descriptor ownership is closed by Slice 9.

## Transform-selector boundary

`transform_selector` is exposed exactly as raw evidence, but:

`transform_selector_bounds_available() == false`

This is deliberate. Representative Stage-CFG review shows slot38 feeds `0x1400594B0` and is not proven to be the `0x40` transform table used by C740.

Therefore the adapter must not mark a Stage-CFG transform selector valid/invalid until its transform-source provenance is closed separately.

## Safety gates

`View::open(...)` requires:
- exact canonical executable SHA gate;
- a valid existing `ContainerDocument`;
- `container_size` bounded by supplied PAC bytes;
- required slots present and valid according to the real container API;
- entry and descriptor ranges bounded by supplied bytes;
- entry slot size divisible by `0x04`;
- descriptor slot size divisible by `0x50`.

Synthetic tests cover:
- modern 39/40 layout;
- legacy 22/23 layout;
- canonical SHA acceptance / packed SHA rejection;
- descriptor type extraction including type `5`;
- invalid descriptor reference detection;
- malformed descriptor-table stride rejection;
- preservation of an intentionally large raw transform selector without inventing transform bounds.

## Review/debug cycle

The first Slice-14 code head `5444030d88fe4ea55b8e71a7e5e804a281a3edcd` failed Ubuntu build in Actions run `31856145056`.

Root cause was **integration drift against the actual existing container model**, not a collision-ABI contradiction:
- the adapter treated `ContainerEntry::valid` as a boolean field instead of the real `valid(container_size)` method;
- the synthetic fixture used obsolete/imagined `ContainerDocument` members (`source_name/header`) rather than the actual `format/schema_version/declared_slot_count` contract.

Fix:
- use `ContainerEntry::valid(container.container_size)`;
- construct synthetic `ContainerDocument` through its real public fields;
- mark populated synthetic slots with `logical_name` and `populated=true` so `ContainerDocument::valid()` exercises the real invariant set;
- retain explicit byte-range checks and only convert `uint64_t` offsets/sizes to `size_t` after bounds have been proven.

No test/evidence gate was weakened.

## Implementation

- `include/dmc_rengine/profiles/dmc3/hits_stage_cfg_collision_tables.hpp`
- `tests/hits_stage_cfg_collision_tables_tests.cpp`
- CTest `hits_stage_cfg_collision_tables`.

## Validation receipt

Exact corrected code head `8bcc309664ba68f9e490cb1569c457aee2fa8fe2` passed GitHub Actions run `31856349259` on:
- Ubuntu — build + test success;
- Windows — build + test success.

Later documentation/evidence commits do not alter the validated C++ implementation.

## Tooling consequence

Binary Inspector / Stage-CFG tooling can now safely display:
- Stage-CFG collision entry rows;
- flags;
- raw transform selector;
- descriptor index;
- referenced descriptor type;
- broken descriptor references.

It must visually label transform-selector resolution as **unresolved provenance** rather than treating slot38 as the transform table.

## Next boundary

Transform provenance is a separate Slice. Current direct evidence shows some object-local C740 paths build/use explicit 0x40 matrix tables, while Stage-CFG slot38 is a different related structure. The next reverse must trace the actual Stage-CFG transform provider before enabling transform-selector bounds in this adapter.
