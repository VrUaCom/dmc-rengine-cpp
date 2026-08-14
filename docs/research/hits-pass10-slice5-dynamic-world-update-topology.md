# HITS Pass 10 — Slice 5 — Dynamic World Update Topology

Date: 2026-08-14  
Canonical executable SHA-256: `e454272ed0fb0247fcbcf300e5d55d7a3e96d50b89b9ffaff81bb978dcbdd082`  
Status: **EXE CONFIRMED / IMPLEMENTED — CI VALIDATION**

## Purpose

Separate the dynamic-world collision/update pipeline from the combined query path closed in Slice 4.

The previous P0 wording incorrectly treated `0x14005B460` as a candidate producer/results helper for `0x14005E7A0`. Canonical instruction reacquisition rejects that model.

The two paths are now explicitly distinct:

- combined query: `E7A0 -> E880 + BCF0`;
- dynamic world update: `B7B0 -> B460 -> B6F0`, followed per source object by `B8E0` post/update processing.

## Canonical body receipts

All bodies come from the same Phase-17 canonical-preserving raw range used by Slice 4.

| Role | VA range | Size | SHA-256 |
|---|---:|---:|---|
| pair resolver/update | `0x14005B460..0x14005B6E6` | `646` | `cba77cf4bc20dedfbd590991012fa471e6b5ae02b9cec57c66fce69333a67972` |
| category compatibility | `0x14005B6F0..0x14005B7A8` | `184` | `c6eaaea738b317c9d92b3e06ff4ba47ec2d39610cb16a19dc21a96651024ae34` |
| update dispatcher | `0x14005B7B0..0x14005B8DD` | `301` | `ea1025e96121a3b117f53412d5d3e6439d81e97a23928f1417793e37ba6efbf3` |
| post/update step | `0x14005B8E0..0x14005BA64` | `388` | `1d149357ffa3fe5024c0a6264e356f476c14c6366d89b38bb50a1a3f18e2f477` |

The earlier truncated `B8E0..B93F` range must not be used as a complete-function hash. Canonical control flow continues through `0x14005BA63 RET`.

## `0x14005B7B0` — update dispatcher

Observed arguments:

- `RCX` — dynamic manager/category-table view;
- `EDX` — source category index.

The dispatcher resolves source list head as `RCX[sourceCategory]`, iterates objects through `+0x328`, requires object base type `2`, and for each enabled target-category activation bit dispatches `0x14005B460`.

Exact target bindings:

| Activation bit | manager list offset | target category |
|---:|---:|---:|
| `0x00001000` | `+0x10` | `0x02` |
| `0x00002000` | `+0x28` | `0x05` |
| `0x00004000` | `+0x40` | `0x08` |
| `0x00008000` | `+0x58` | `0x0B` |
| `0x00010000` | `+0x70` | `0x0E` |
| `0x00020000` | `+0x88` | `0x11` |

After pair dispatch for the current source object, `B7B0` calls `0x14005B8E0(manager, sourceCategory, object)`.

## `0x14005B460` — object-pair resolver/update

Windows x64 ABI:

1. `RCX` — dynamic manager/context;
2. `RDX` — source object;
3. `R8` — candidate-list head for the selected target category;
4. `R9D` — source category;
5. stack arg 5 — target category.

The function returns immediately when the candidate-list head is null, then iterates candidates through `candidate+0x328`.

Direct pair filters include:

- candidate base type at `+0x00` must equal `2`;
- candidate must not be the source object;
- source and candidate owner pointers at `+0xD0` must differ;
- candidate activation/state bits must permit the source-owner category relation;
- category-pair compatibility helper `0x14005B6F0` must accept the pair.

Accepted interactions route through the dynamic contact/overlap helpers already present in the executable. Depending object state, the path uses `0x14005CD00` or `0x14005A980`; accepted displacement/correction is applied through `0x1402CC090`, with `0x14005C890` used by the observed movement/update path.

After an accepted pair, reciprocal category bits are ORed into both objects at `+0x10`, using each opposite owner/category index as `0x1000 << index`.

This is update-state mutation, not an `E7A0` query-result buffer.

## `0x14005B6F0` — category-pair compatibility

This helper is special only when source category or target category is `0x0E`.

- if neither category is `0x0E`, return true;
- if both are `0x0E`, return true;
- if exactly one side is `0x0E`, map the other category to a raw-mask family and test the opposite object’s 16-bit flags at `+0xDA/+0xDB`.

Exact bridge:

| Other category | mask |
|---:|---:|
| `0x02` | `0x0040` |
| `0x05` | `0x0002` |
| `0x08` | `0x0010` |
| `0x0B` | `0x0020` |
| other | `0x0000` |

Acceptance condition for a mapped pair is `(rawFlags & mask) == 0`.

This independently connects dynamic category `0x0E` pair compatibility to the same raw HITS mask families already seen in dispatcher/query evidence.

## `0x14005B8E0` — per-object post/update query step

Observed ABI from `B7B0`:

- `RCX` — manager;
- `EDX` — source category;
- `R8` — current object.

The source category maps to the same reject-mask family:

- `0x02 -> 0x0040`;
- `0x05 -> 0x0002`;
- `0x08 -> 0x0010`;
- `0x0B -> 0x0020`;
- other categories -> zero in this routine.

For object base type `2`, the routine branches on object subtype/state and invokes already-known specialized HITS queries including `0x140060790`, `0x14005F070`, `0x14005EE40` and `0x14005EBE0`. Successful correction paths may apply deltas through `0x1402CC090` and notify/update through `0x14005C890`.

Successful contact/query handling ORs bit `0x00020000` into object state/category bits at `+0x10`.

This makes `B8E0` a post/update query/constraint bridge, not the dispatcher itself and not the `E7A0` combined query wrapper.

## Architecture correction

Rejected old model:

`B460 -> unknown E7A0 candidate producer/result contract`

Canonical model:

`B7B0 dispatcher -> B460 object-pair update -> B6F0 pair compatibility`, then `B8E0` post/update handling for each source object.

This pipeline mutates dynamic object state, pair bits and positions. It must remain architecturally separate from `E7A0` query output/metadata ABI.

## Implementation

New profile-specific evidence module:

- `include/dmc_rengine/profiles/dmc3/hits_dynamic_update_evidence.hpp`
- `tests/hits_dynamic_update_evidence_tests.cpp`
- CTest stem `hits_dynamic_update_evidence`

The evidence module is intentionally separate from `hits_query_evidence.hpp` so code consumers cannot accidentally treat world-update ABI as query-result ABI.

All lookup APIs require the canonical executable SHA and return no evidence for packed SHA `81c7e61983564113b5105e931d9f185accc14e44ae147d27f720c2d50935c7d6`.

Implementation commits:

- `7e3f532e84c7a3e24f2282a0dee0e6123b9c93bb` — profile evidence module;
- `a565d196f94055c9bfb3003e44ed1b442d8b8272` — regression;
- `3a9049b655f5c4c0d35a5b9308b72a150afbda45` — CTest registration.

## Remaining boundary

Still unresolved after Slice 5:

- gameplay semantics of dynamic object subtype/type values beyond directly observed numeric contracts;
- deeper semantics of contact helpers `A980/CD00/CC090/C890` unless needed for source-equivalent reconstruction;
- `0x14005FEC0` exact source-1 query ABI;
- `0x1400601E0` exact in/out/fourth-component/accumulation semantics.
