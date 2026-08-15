# HITS Pass 10 — Slice 13: Serialized Collision Triplet View

Date: 2026-08-15  
Canonical DMC3 target SHA-256: `e454272ed0fb0247fcbcf300e5d55d7a3e96d50b89b9ffaff81bb978dcbdd082`  
Status: **EXE + DATA-SIDE CONFIRMED / IMPLEMENTED / VALIDATED**

## Purpose

Slice 9 recovered the serialized dynamic-collision entry/descriptor ABI and the runtime bridge into C8D0/CC530. Independent Phase-15 PAC evidence then exposed a complete serialized table set in `em000.pac` with exact `0x40 / 0x04 / 0x50` record strides.

Slice 13 turns that proven three-span ABI into a read-only C++ view suitable for Binary Inspector/PAC tooling when a transform table, entry table and primitive descriptor table have each been independently identified.

The implementation is deliberately profile-scoped under `profiles/dmc3`. Portability to other games/DMC versions is not yet established.

## Serialized ABI

### Transform table
- record stride: `0x40` bytes.

### Entry table
- record stride: `0x04` bytes;
- `+0x00`: raw flags `u8`;
- `+0x01`: transform selector `u8`;
- `+0x02`: primitive descriptor index `u16 little-endian`.

### Primitive descriptor table
- record stride: `0x50` bytes;
- `+0x00`: primitive type byte;
- observed runtime constructor types are `0..6`, but the parser preserves unknown raw type values instead of rejecting them.

Reference validation:
- `transform_selector < transform_count`;
- `descriptor_index < primitive_descriptor_count`.

## EXE-side proof

Slice 9 established:

`descriptor = descriptor_table + u16(entry+0x02) * 0x50`

C740 independently uses `entry+0x01` as an index into a transform source with `0x40` stride, while C8D0/CC530 preserve the selected primitive descriptor at runtime `+0x118` and dispatch descriptor byte `+0x00`.

This proves the runtime relationships and strides independently of any PAC slot number.

## Independent data-side validation — Phase-15 `em000.pac`

Preserved Phase-15 PAC metadata contains a complete matching table set:

### slot 38 — `0x40` table
- size: `6144` bytes;
- `6144 / 0x40 = 96` exact records;
- SHA-1 `088d32720e5c3c970d96012b03c4e1c3c301ffe7`.

### slot 39 — entry table
- size: `96` bytes;
- `96 / 4 = 24` exact records;
- SHA-1 `28585e6b75468bb4874c1d66c2592ea993351804`.

The preserved first four entry records decode as:

| entry | flags | transform selector | descriptor index |
|---:|---:|---:|---:|
| 0 | `0x06` | 0 | 0 |
| 1 | `0x02` | 3 | 1 |
| 2 | `0x01` | 9 | 2 |
| 3 | `0x04` | 17 | 3 |

All four selectors are in range for the 96-record `0x40` table and all descriptor indices are in range for the descriptor count below.

### slot 40 — primitive descriptor table
- size: `1840` bytes;
- `1840 / 0x50 = 23` exact descriptors;
- SHA-1 `d8948c18e4eb0432e42078962cf7835e6fa5a8d2`;
- preserved first descriptor type byte = `2`.

This independently validates the EXE-recovered `0x40 / 0x04 / 0x50` serialized ABI.

## Negative control — Phase-15 `id100.pac`

The same PAC slot numbers are **not** globally semantic:
- slot 38 size `12288`;
- slot 39 size `448` and begins with a MOD-like payload;
- slot 40 size `4096`;
- `4096 % 0x50 = 16`.

Therefore `id100.pac` slots 38/39/40 must not be accepted as this collision table set merely because the slot numbers match `em000.pac`.

**Canonical rule: PAC slot number alone never selects the parser.** A container/profile path or independent structural evidence must first identify the input spans.

## Stage-CFG boundary correction

Slice 12 proves that reviewed Stage-CFG paths use `room\\stXXXcfg.pac` slot 39 as the C260 entry table and slot 40 as the primitive descriptor table (legacy observed slots 22/23).

However, a deeper representative-path review shows Stage-CFG slot 38 is consumed by `0x1400594B0`, whose payload starts with its own `u16` relative-offset structure. It is **not proven to be the `0x40` transform table** consumed by C740.

Therefore:
- Stage-CFG slot 39/40 -> entry/descriptor pair remains confirmed;
- Stage-CFG slot 38 remains a related collision/source block, not a promoted transform table;
- do **not** wire Stage-CFG slots 38/39/40 directly into this three-span view until the transform-table provenance is independently closed;
- `em000.pac` remains the current direct data-side three-table validation sample.

This correction prevents a correct parser from being attached to an unproven Stage-CFG transform source.

## Implementation

`include/dmc_rengine/profiles/dmc3/hits_collision_triplet.hpp`

`View::open(transform_table, entry_table, primitive_descriptor_table)` rejects non-multiple table sizes and exposes:
- `transform_count()`;
- `entry_count()`;
- `primitive_descriptor_count()`;
- decoded `entry(index)`;
- raw `transform_record(index)` span;
- raw `primitive_descriptor(index)` span;
- `primitive_type(index)`;
- per-entry reference validation;
- whole-table reference validation.

No PAC slot number and no executable VA is hardcoded in the parser.

Regression:
- `tests/hits_collision_triplet_tests.cpp`;
- CTest `hits_collision_triplet`.

Synthetic fixtures mirror only the preserved Phase-15 dimensions/decoded indices; no proprietary records are committed. The regression also uses an `id100.pac`-sized 4096-byte synthetic descriptor span as a negative stride check.

## Validation receipt

Exact code head `cc64f6ffdffc96a5c92596f37f33ba4a3769a37f` passed GitHub Actions run `31855809613` on Ubuntu and Windows, including `hits_collision_triplet`.

Later documentation/evidence corrections do not alter the validated C++ implementation.

## Architecture boundary

Keep separate:
- static HITS triangles/cells;
- serialized dynamic primitive tables;
- runtime collision objects produced by C8D0/CC530.

The view is a data-layer bridge only after its three spans have been identified by profile/container evidence.

## Next step

1. expose Binary Inspector fields for entry flags, transform selector, descriptor index and primitive type on independently identified table spans;
2. close Stage-CFG transform-source provenance before making a Stage-CFG three-table adapter;
3. when actual Stage-CFG slot40/23 bytes become available, perform real descriptor-type census, especially type `5`;
4. keep unknown primitive type bytes visible rather than coercing them to known enums.
