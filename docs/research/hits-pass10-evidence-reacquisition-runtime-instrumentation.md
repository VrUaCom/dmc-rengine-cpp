# HITS Raw Reverse Pass 10 — Evidence Reacquisition and Runtime Instrumentation

Date: 2026-08-14  
Canonical executable SHA-256: `e454272ed0fb0247fcbcf300e5d55d7a3e96d50b89b9ffaff81bb978dcbdd082`  
Status: **ACTIVE — TOP-LEVEL P0 ABI CLOSED; DEEP SEMANTICS / VALIDATION CONTINUE**

Canonical detailed working authority: Google Drive document `DMC Rengine — HITS Raw Reverse Pass 10 — Evidence Reacquisition and Runtime Instrumentation — 2026-08-14`.

Companion research:
- `docs/research/reverse-pass-implementation-review-loop.md`
- `docs/research/hits-pass10-slice4-canonical-combined-query-abi.md`
- `docs/research/hits-pass10-slice5-dynamic-world-update-topology.md`
- `docs/research/hits-pass10-slice6-source1-query-abi.md`
- `docs/research/hits-pass10-slice7-primitive-shape-mapping.md`

## Why Pass 10 exists

Pass 9 reached static ABI/ownership saturation for the evidence then materialized. Pass 10 changed method: reacquire canonical instruction windows from preserved project artifacts with explicit provenance, review complete callers/callees and write/read contracts, and use hash-gated runtime traces only where static evidence still cannot close behavior.

## Mandatory loop

1. acquire direct evidence;
2. review assumptions;
3. deepen through callers/callees/writers/readers/ownership/state/corpus;
4. define the exact promotion boundary;
5. implement only the promoted subset;
6. review implementation against evidence;
7. debug/test without weakening evidence gates;
8. consolidate into the Pass-10 authority;
9. perform a second independent review/debug cycle;
10. synchronize code, GitHub, machine evidence and Google Drive authorities.

No material correction may remain only in chat.

## Canonical-byte provenance

The ChatGPT Project/Library corpus contains `dmc3_phase17_reng_probe.exe` and its Phase-17 verifier receipt. Phase 17 records that the probe was cloned from canonical `dmc3.exe` SHA `e454...d082` and preserves the original PE raw section range `0x400..0x60E600` byte-identically. Every Pass-10 Slice-4/5/6/7 function body promoted below maps inside that preserved range.

`dmc3_phase18_red_orb_x2_hook.exe` independently matches the promoted body windows and serves as a second consistency check. It is not used to replace canonical target identity.

No proprietary instruction bytes are committed. We preserve VAs, ranges, body hashes, ABIs, field offsets, topology and reconstructed behavior.

# Top-level P0 status

The original top-level Pass-10 target set `E7A0 / B460 / FEC0 / 601E0` is now **ABI-closed or correctly reclassified**. Future work must not reopen the old descriptions unless contradictory direct evidence appears.

## CLOSED — `0x14005E7A0` combined query wrapper

Canonical body:
- `0x14005E7A0..0x14005E880`;
- 224 bytes;
- SHA-256 `3716472a87c7edd9ea27b800e165de7fee8254c8b928c3a41e431b0f350b8a6f`;
- 51 direct callers.

Exact six-argument ABI:
1. `RCX` — HITS/runtime wrapper/context;
2. `RDX` — reference/start 16-byte point;
3. `R8` — requested/working 16-byte point;
4. `R9` — corrected/output 16-byte point;
5. stack arg 5 — optional common `0x38` hit-metadata output;
6. stack arg 6 — raw reject mask.

Ordered passes:
1. static HITS `0x14005E880`;
2. dynamic category `0x0E` through `0x14005BCF0`;
3. dynamic category `0x11` through `0x14005BCF0`.

Both dynamic passes receive the same static-or-input baseline. `BCF0` copies that baseline into an internal local and does not write it back through `R8`.

Caller-visible output precedence is therefore:

`static HITS < category 0x0E < category 0x11`

where a later successful pass overwrites an earlier successful output. If all passes fail, `E7A0` copies arg3 to arg4 and returns false.

**REJECTED old model:** there is no wrapper-level candidate metric comparison, equality branch or tie-break in `E7A0`.

### Common `0x38` metadata bridge

- static `E880` accepted hit copies a complete 56-byte raw HITS record;
- dynamic `BCF0` writes compatible partial metadata;
- dynamic identity/key is copied `object+0xD8 -> metadata+0x00`;
- a three-float caller-visible vector is written at metadata `+0x28/+0x2C/+0x30`;
- caller `0x1402C65B2` reads those floats after successful `E7A0`.

The exact semantic name of that vector remains **UNRESOLVED**.

Detailed authority: `hits-pass10-slice4-canonical-combined-query-abi.md`.

## RECLASSIFIED AND ABI-CLOSED — dynamic world update `B7B0/B460/B6F0/B8E0`

`0x14005B460` is **not** an `E7A0` candidate producer. It belongs to a separate dynamic-world object-pair update pipeline:

`0x14005B7B0 dispatcher -> 0x14005B460 pair resolver -> 0x14005B6F0 compatibility`, followed by `0x14005B8E0` post/update handling per source object.

Canonical bodies:
- `B460..B6E6`, 646 bytes, SHA `cba77cf4bc20dedfbd590991012fa471e6b5ae02b9cec57c66fce69333a67972`;
- `B6F0..B7A8`, 184 bytes, SHA `c6eaaea738b317c9d92b3e06ff4ba47ec2d39610cb16a19dc21a96651024ae34`;
- `B7B0..B8DD`, 301 bytes, SHA `ea1025e96121a3b117f53412d5d3e6439d81e97a23928f1417793e37ba6efbf3`;
- `B8E0..BA64`, 388 bytes, SHA `1d149357ffa3fe5024c0a6264e356f476c14c6366d89b38bb50a1a3f18e2f477`.

The older truncated `B8E0..B93F` range is not a complete function and must not be used as its canonical body hash.

### `B460` ABI

1. `RCX` — manager/context;
2. `RDX` — source object;
3. `R8` — candidate-list head;
4. `R9D` — source category;
5. stack arg 5 — target category.

Confirmed pair logic includes:
- candidate list linked by `+0x328`;
- candidate base type `+0x00 == 2` requirement;
- self rejection;
- shared-owner rejection using owner pointer `+0xD0`;
- category-pair compatibility through `B6F0`;
- accepted contact displacement/update helpers;
- reciprocal category bits written at `+0x10`.

### `B6F0` category compatibility

Category `0x0E` is the special bridge:
- neither side `0x0E` -> allow;
- both sides `0x0E` -> allow;
- exactly one side `0x0E` -> map the other category to raw flag mask and test opposite object flags `+0xDA/+0xDB`.

Mask bridge:
- `0x02 -> 0x0040`;
- `0x05 -> 0x0002`;
- `0x08 -> 0x0010`;
- `0x0B -> 0x0020`.

### `B7B0` activation/category bindings

- `0x00001000`, manager `+0x10` -> category `0x02`;
- `0x00002000`, `+0x28` -> `0x05`;
- `0x00004000`, `+0x40` -> `0x08`;
- `0x00008000`, `+0x58` -> `0x0B`;
- `0x00010000`, `+0x70` -> `0x0E`;
- `0x00020000`, `+0x88` -> `0x11`.

### `B8E0` post/update bridge

`B8E0` maps source categories `0x02/0x05/0x08/0x0B` to reject masks `0x0040/0x0002/0x0010/0x0020`, calls specialized HITS queries including `60790/F070/EE40/EBE0`, may apply correction/update helpers, and successful handling ORs `0x00020000` into object state/category bits at `+0x10`.

Implementation is kept in separate profile evidence `hits_dynamic_update_evidence.hpp`; this separation is architectural, not cosmetic.

Detailed authority: `hits-pass10-slice5-dynamic-world-update-topology.md`.

## CLOSED — `0x14005FEC0` source-1 segment correction ABI

Canonical body:
- `0x14005FEC0..0x1400601D3`;
- 787 bytes;
- SHA `cfaca9752adf3a581969875e99c6c1b9ffaa7f83d19d22fddbe8dec2d5cab09e`.

Exact three-argument ABI:
1. `RCX` — currently selected HITS runtime;
2. `RDX` — mutable 16-byte point, input/output;
3. `R8` — read-only 16-byte reference/target point.

Exact temporary-source1 route:

`0x1400568F0 select source1 -> FEC0 -> 0x140056936 restore source0`.

Confirmed behavior:
- computes `R8 - RDX`;
- XYZ length controls degenerate-segment rejection;
- direction normalizer explicitly zeroes fourth component;
- broadphase/record traversal runs against selected source runtime;
- raw HITS bit `0x00080000` is directly rejected;
- accepted correction updates internal point;
- non-degenerate path writes final 16-byte point through `RDX`.

**Return correction:** `AL` is not hit/no-hit. It is false for a degenerate near-zero XYZ segment; a non-degenerate processed segment returns true even when no raw record changes the point.

## CLOSED — `0x1400601E0` displacement accumulator ABI

Canonical body:
- `0x1400601E0..0x14006078E`;
- 1454 bytes;
- SHA `7fb7fb7ce9447a5e200ba1471774eafe7e5f21877da71902f047563bd9f7597a`;
- five direct callers.

Exact three-argument ABI:
1. `RCX` — selected/current HITS runtime;
2. `RDX` — mutable 16-byte point, input/output;
3. `R8` — read-only reference/anchor point.

One canonical caller explicitly selects source1 before `601E0` and restores source0 afterward.

Two-stage model:
1. initial raw-record correction pass mutates the working point;
2. secondary record-vector displacement accumulation applies accepted scaled 16-byte displacements repeatedly.

### Fourth component — operational semantics closed, gameplay name unresolved

- XYZ norm helpers ignore component `+0x0C`;
- XYZ dot helper ignores component `+0x0C`;
- `601E0` normalization derives magnitude from XYZ but can scale all four components;
- scale/add path carries all four components into the mutable 16-byte point.

Therefore component 4 is **excluded from spatial length/dot gating but transported/scaled/accumulated through correction math**. No gameplay name is assigned yet.

`FEC0` is different: its direction-normalization path explicitly zeroes component 4.

### `601E0` return contract

At exit, XYZ distance between the preserved original point and final `RDX` is measured. `AL` is true only when the final point materially moved beyond epsilon in XYZ.

So `601E0` return means **material XYZ displacement/correction occurred**, not candidate existence and not raw hit count.

Detailed authority: `hits-pass10-slice6-source1-query-abi.md`.

# Slice 7 — parsed shape → descriptor → runtime primitive

The parser/runtime semantic layer is now partially closed with direct data-flow proof. Parser enum, internal descriptor type and runtime primitive type are three different layers and must not be collapsed.

Canonical source mapping:
- `sphere`, parser enum `0` -> descriptor `2` -> primary runtime type `2`;
- `box`, enum `1` -> descriptor `3` -> runtime type `3`;
- `cylinder`, enum `2` -> descriptor `6` -> runtime type `6`;
- `capsule`, enum `3` -> descriptor `4` -> runtime type `4`.

The data-flow bridge is exact: the shape structure written by parser `0x140247AD0` at caller frame `[rbp-0x38]` is the same storage passed through builder `0x140249710` member `+0x98` into converter `0x1402481E0`.

Canonical bodies:
- parser cluster `247AD0..2481E0`, 1808 B, SHA `1bd8a8e369105512166e5f2557d274c86e3801b56631f0d4f7022983adfda6e2`;
- converter `2481E0..2482C5`, 229 B, SHA `ed50d780aa2bd2868b783c4924edcac6c2af9b4269225ce92b7c778535d7c58a`;
- runtime constructor dispatcher `2CC530..2CC5EF`, 191 B, SHA `e8e0065e0aeb76b6839d0506e09623b48091d853bd9ccee3127bd0bec7e19364`.

## Runtime type `4` dual origin

Runtime type `4` is not semantically equivalent to the text token `capsule` alone.

- parsed `capsule` -> descriptor `4` -> runtime type `4`;
- parsed `sphere` -> descriptor `2` normally produces runtime type `2`, but the descriptor-2 constructor may promote a sufficiently moving sphere to runtime type `4` when displacement is greater than `2 × radius`.

Both origins use endpoint/radius representation at runtime offsets `+0x130`, `+0x140`, `+0x150`. Canonical wording is **capsule / swept-sphere representation**.

## Runtime type `5`

Descriptor/runtime type `5` is structurally confirmed as a **three-vertex face representation**:
- vertices at runtime `+0x130/+0x140/+0x150`;
- auxiliary vector at `+0x160`;
- auxiliary vector is normalized;
- the associated geometry path uses a three-element contract.

No direct canonical source-text token/name for type `5` is proven. `three-vertex face` is a structural description, not a claimed game-authored token.

Second review also rejected an unrelated state-machine immediate `=5` as collision-descriptor evidence.

Implementation:
- `hits_primitive_shape_evidence.hpp`;
- `hits_primitive_shape_evidence_tests.cpp`;
- CTest `hits_primitive_shape_evidence`.

Detailed authority: `hits-pass10-slice7-primitive-shape-mapping.md`.

# Implementation architecture

Pass-10 profile evidence is deliberately split by runtime responsibility:

- `hits_query_evidence.hpp` — query-family summary, combined query, source switching and query topology;
- `hits_dynamic_update_evidence.hpp` — dynamic world pair/update pipeline;
- `hits_source1_query_evidence.hpp` — exact `FEC0/601E0` specialized source/current-runtime contracts;
- `hits_primitive_shape_evidence.hpp` — parser/descriptor/runtime primitive mapping and shape-constructor evidence;
- `runtime_trace.hpp` — generic observation-only instrumentation contract.

`hits_query_evidence.hpp` has been reconciled so `FEC0/601E0` are no longer labeled `unresolved`; they use specialized ABI kinds backed by Slice-6 detailed evidence.

All build-specific evidence APIs are SHA-gated against the canonical target. Packed SHA `81c7...c7d6` must receive no canonical VA/body ABI descriptors.

# Major Pass-10 corrections preserved as canonical

1. **CORRECTED:** DMC3 VAs belong under `profiles/dmc3`, not generic HITS core.
2. **CORRECTED:** `dispatcher_static_hits_reject_mask` is dispatcher-scoped, not a global category property.
3. **CORRECTED:** `namespace detail` is not access control; raw backing tables moved to private `EvidenceStore` members.
4. **REJECTED:** `E7A0` metric/equality/tie-break arbitration model.
5. **CORRECTED:** `BCF0` does not chain category `0x0E` output as category `0x11` input baseline.
6. **REJECTED:** `B460` as `E7A0` candidate producer; it is dynamic-world pair/update logic.
7. **CORRECTED:** `FEC0 AL` is not a hit boolean.
8. **CORRECTED:** `601E0 AL` reports material XYZ displacement, not generic query success.
9. **CORRECTED:** fourth component is not part of XYZ norm/dot gating, but `601E0` can transport and accumulate it.
10. **REJECTED:** parser shape enum equals runtime primitive type.
11. **REJECTED:** runtime type `4` has only capsule origin; swept moving sphere is a second proven origin.
12. **UNRESOLVED:** runtime/descriptor type `5` has structural three-vertex-face semantics, but its source-text name is not proven.

# Validation state

Exact Slices-4–6 implementation head `db7a557de428d4e379ecd51b9670e97dc8c435f7` passed GitHub Actions run `31813329645` on Ubuntu and Windows.

Exact Slice-7 code head `cccb5fc45c0c4cb1746cd5630d474533fadb6f76` passed GitHub Actions run `31815597161` on Ubuntu and Windows, including the new `hits_primitive_shape_evidence` regression.

Documentation/machine-evidence commits after those heads do not alter the validated C++ code.

# Remaining work after top-level P0 closure and Slice 7

The next work is **not another restart of `E7A0/B460/FEC0/601E0` ABI discovery**. It moves deeper into semantic identity and source-equivalent behavior:

1. identify upstream producer/source vocabulary for descriptor/runtime type `5` if direct evidence exists;
2. resolve source semantics of descriptor/runtime types `0` and `1` without assuming they duplicate parsed sphere/box;
3. close exact semantics of the common metadata vector `+0x28/+0x2C/+0x30` and the related fourth component across static/dynamic producers;
4. reconstruct geometry/contact helpers required for source-equivalent implementation;
5. build controlled canonical runtime traces for edge cases and compare them against reconstructed C++ behavior;
6. implement executable/source-equivalent collision behavior only when the evidence packet is sufficient, preserving profile isolation and exact-build gates.

## Explicit non-goals

- no guessed monolithic original `CollisionResult`;
- no invented text/gameplay name for runtime type `5`;
- no invented semantic name for metadata vector `+0x28..+0x30` or fourth component;
- no conflation of parser enum, descriptor enum and runtime primitive enum;
- no conflation of query path (`E7A0/E880/BCF0`) with dynamic-world update path (`B7B0/B460/B6F0/B8E0`);
- no reopening superseded Pass-10 P0 descriptions without contradictory direct evidence;
- no GDSpaces ownership of recovered collision runtime;
- no proprietary game bytes committed.
