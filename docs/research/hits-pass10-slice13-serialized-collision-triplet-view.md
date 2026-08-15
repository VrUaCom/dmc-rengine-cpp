# HITS Pass 10 — Slice 13: Serialized Collision Triplet View

Date: 2026-08-15  
Canonical DMC3 target SHA-256: `e454272ed0fb0247fcbcf300e5d55d7a3e96d50b89b9ffaff81bb978dcbdd082`  
Status: **EXE + DATA-SIDE CONFIRMED / IMPLEMENTED / CI VALIDATION ACTIVE**

## Purpose

Slices 9 and 12 recovered how dynamic collision runtime resolves three serialized tables:

1. transform table;
2. 4-byte entry table;
3. primitive descriptor table.

Slice 13 turns that recovered ABI into a read-only C++ view suitable for later Binary Inspector / Stage-CFG / PAC tooling.

The implementation is deliberately profile-scoped under `profiles/dmc3`. The format is proven reusable inside DMC3 PAC resources, but portability to other games/DMC versions is not yet established.

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
- currently observed runtime constructor type range: `0..6`, but the parser preserves unknown raw type values rather than rejecting them.

Reference validation:
- `transform_selector < transform_count`;
- `descriptor_index < primitive_descriptor_count`.

## EXE-side proof

Slice 9 established:

`descriptor = descriptor_table + u16(entry+0x02) * 0x50`

and C740 uses the entry transform selector independently from the primitive descriptor index. Runtime C8D0/CC530 then preserve the descriptor pointer at runtime `+0x118` and dispatch descriptor byte `+0x00`.

Slice 12 established container-specific Stage-CFG slot mappings, but slot numbers are **not** part of this parser.

## Independent data-side validation — Phase-15 `em000.pac`

Preserved Phase-15 PAC metadata contains an exact serialized triplet:

### slot 38 — transform table candidate
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

All four selectors are valid against the 96-record transform table and all descriptor indices are valid against the descriptor count below.

### slot 40 — primitive descriptor table
- size: `1840` bytes;
- `1840 / 0x50 = 23` exact descriptors;
- SHA-1 `d8948c18e4eb0432e42078962cf7835e6fa5a8d2`;
- preserved first descriptor type byte = `2`.

This is an independent data-side match for the EXE-recovered `0x40 / 0x04 / 0x50` ABI.

## Negative control — Phase-15 `id100.pac`

The same PAC slot numbers are **not** globally semantic:

- slot 38 size `12288`;
- slot 39 size `448` and starts with a MOD-like payload;
- slot 40 size `4096`;
- `4096 % 0x50 = 16`, so slot 40 cannot be parsed as an exact primitive-descriptor table.

Therefore:

**PAC slot number alone must never select the collision-triplet parser.**

A container/profile path or structural evidence must first identify the three relevant table spans.

This is why Slice-12 Stage-CFG slot mapping remains profile evidence, while Slice-13 only parses already-resolved table spans.

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
- whole-triplet reference validation.

No PAC slot number and no DMC3 EXE VA is hardcoded in the parser.

Regression:
- `tests/hits_collision_triplet_tests.cpp`;
- CTest `hits_collision_triplet`.

The regression uses synthetic bytes only. Its dimensions mirror the preserved `em000.pac` counts but it does not copy proprietary game records.

## Architecture boundary

This is a **serialized dynamic collision table view**, not the static HITS triangle parser and not a guessed monolithic `CollisionResult`.

Do not merge these three layers:
- static HITS triangles/cells;
- serialized dynamic primitive triplet;
- runtime collision objects produced by C8D0/CC530.

The view is the bridge from PAC slot data into the runtime primitive reconstruction already recovered in Slices 7–10.

## Next step after validation

Once CI is green:
1. expose a profile-aware Stage-CFG/PAC adapter that resolves the Slice-12 slot generation and feeds the three slot spans into this view;
2. add Binary Inspector regions/fields for entries and primitive descriptor type/index relationships;
3. when actual Stage-CFG slot 40/23 bytes become available, perform a real type distribution census, especially descriptor type `5`;
4. keep unknown primitive type bytes visible rather than coercing them to known enums.
