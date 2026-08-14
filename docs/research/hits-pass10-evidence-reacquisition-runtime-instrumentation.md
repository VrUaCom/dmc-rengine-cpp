# HITS Raw Reverse Pass 10 — Evidence Reacquisition and Runtime Instrumentation

Date: 2026-08-14  
Canonical executable SHA-256: `e454272ed0fb0247fcbcf300e5d55d7a3e96d50b89b9ffaff81bb978dcbdd082`  
Status: **ACTIVE — REVERSE / IMPLEMENT / REVIEW / DEBUG LOOP**

Canonical detailed working authority: Google Drive document `DMC Rengine — HITS Raw Reverse Pass 10 — Evidence Reacquisition and Runtime Instrumentation — 2026-08-14`.

Companion research:
- `docs/research/reverse-pass-implementation-review-loop.md`
- `docs/research/hits-pass10-slice4-canonical-combined-query-abi.md`

## Why Pass 10 exists

Pass 9 reached static ABI/ownership saturation for the evidence then materialized. Pass 10 changes method: reacquire canonical instruction windows from preserved project artifacts with explicit provenance, and use hash-gated runtime traces only where static evidence still cannot close behavior.

## Mandatory loop

1. acquire direct evidence;
2. review assumptions;
3. deepen through callers/callees/writers/readers/ownership/state/corpus;
4. define a promotion boundary;
5. implement only the promoted subset;
6. review implementation against evidence;
7. debug/test without weakening gates;
8. consolidate into Pass-10 authority;
9. perform a second independent review/debug cycle;
10. synchronize code, GitHub, machine evidence and Google Drive authorities.

No material correction may remain only in chat.

## Current P0 state

### CLOSED at wrapper ABI level — `0x14005E7A0`

Canonical bytes have now been reacquired and reviewed. `E7A0` is **not** a metric/tie-break result comparator.

Confirmed:
- exact body `0x14005E7A0..0x14005E880`, 224 bytes;
- body SHA-256 `3716472a87c7edd9ea27b800e165de7fee8254c8b928c3a41e431b0f350b8a6f`;
- six-argument ABI;
- 51 direct callers;
- static pass `0x14005E880` followed by dynamic category passes `0x14005BCF0` for `0x0E`, then `0x11`;
- overall boolean success in `AL`;
- total miss copies working point to output;
- common optional `0x38` hit-metadata output;
- both dynamic passes receive the same static-or-input baseline;
- caller-visible precedence is ordered last-successful-writer: `static < 0x0E < 0x11`.

The earlier Pass-10 wording that asked for an `E7A0` comparison metric, equality branch or tie-break is **REJECTED / SUPERSEDED**.

Detailed receipt: `docs/research/hits-pass10-slice4-canonical-combined-query-abi.md`.

### RECLASSIFIED — `0x14005B460`

Canonical reacquisition shows that `B460` is not the missing `E7A0` candidate producer. It belongs to a separate dynamic-world update path.

Current recovered topology:
- `0x14005B7B0` — dynamic update dispatcher;
- `0x14005B460` — object-pair resolution/update loop;
- `0x14005B6F0` — category-pair compatibility filter;
- `0x14005B8E0` — per-object post/update query/constraint step.

The prior P0 wording “B460 category-list candidate production/result contract” is superseded. This topology is being promoted as the next review slice and must remain separate from query-path `E7A0 -> E880 + BCF0`.

### OPEN — `0x14005FEC0`

Need exact source-1 segment-query input/output ABI, write-set and result propagation. Raw reject bit `0x00080000` is already independently confirmed.

### OPEN — `0x1400601E0`

Need exact in/out layout, fourth-component semantics, triangle iteration and accumulation/convergence behavior.

## Canonical byte provenance

The ChatGPT project Library contains `dmc3_phase17_reng_probe.exe` plus the Phase-17 verifier report. Phase 17 records that the probe was cloned from canonical `dmc3.exe` SHA `e454...d082` and that original raw PE section bytes over file range `0x400..0x60E600` remained byte-identical. Slice-4 functions and the newly reacquired dynamic-update functions map inside that preserved range.

A second derivative, `dmc3_phase18_red_orb_x2_hook.exe`, independently matches the promoted function windows. It is used as a consistency check, not as the source of target identity.

No proprietary instruction bytes are committed. Evidence stores VAs, ranges, hashes, field offsets and reconstructed behavior.

## Runtime instrumentation fallback

The generic observation contract remains in:
- `include/dmc_rengine/evidence/runtime_trace.hpp`
- `tests/runtime_trace_tests.cpp`

It models instrumentation metadata, not original DMC3 structures:
- exact executable SHA;
- function VA;
- expected hook bytes;
- capture sequence/phase;
- optional caller/source/mask/category/result metadata;
- raw memory snapshots.

`capture_window_bytes` is tooling-only and is not evidence of an original structure size.

## Slice 1 receipt — generic trace contract

Implementation added the generic trace contract and CTest regression.

Correction: the first synthetic regression paired canonical-looking HITS VAs with synthetic bytes. It was replaced by fully synthetic SHA/VAs/bytes so test fixtures cannot masquerade as canonical byte evidence.

## Slice 2 receipt — DMC3 query/dynamic evidence

Promoted into `profiles/dmc3`:
- nine HITS query-family VAs;
- confirmed mutable 16-byte paths for `EBE0/F070`;
- `AL`-observed success paths for `EE40/60790` without claiming non-mutation;
- dynamic categories `0x02/0x05/0x08/0x0B/0x0E/0x11`;
- activation flags and `0x18`-spaced manager offsets;
- dispatcher-level static-HITS reject-mask bridge `0x0040/0x0002/0x0010/0x0020/0/0`;
- three wrapper source slots.

Corrections:
1. DMC3 VAs moved out of generic HITS core into `profiles/dmc3`.
2. `static_hits_reject_mask` narrowed to `dispatcher_static_hits_reject_mask`.
3. all build-specific lookups use canonical SHA gating.

## Slice 3 receipt — runtime topology

Promoted:
- four direct `0x14005EBC0` source-selector callsites;
- two temporary source-1 paths:
  - `0x140056832 -> 0x1400601E0 -> 0x14005686E`;
  - `0x1400568F0 -> 0x14005FEC0 -> 0x140056936`;
- query-family caller census `1/51/1/1/1/1/1/5/1`;
- runtime helper VA/caller census;
- repeated static xref saturation receipt.

Correction: `namespace detail` was initially treated as if it provided access control. It does not. Backing tables are now private static members of `detail::EvidenceStore`, while public access remains exact-SHA-gated. Fix commit: `4845120fbe9a3362d4040a0e37e32387411411d7`.

## Slice 4 receipt — canonical combined-query ABI

Canonical function bodies:
- `E7A0..E880`, 224 bytes, SHA `3716472a87c7edd9ea27b800e165de7fee8254c8b928c3a41e431b0f350b8a6f`;
- `E880..EB95`, 789 bytes, SHA `b3fdeac674795752492e1eca9e7a9d21837552aa6cf90f0a02a14e3546136e8a`;
- `BCF0..C0D6`, 998 bytes, SHA `e7e1c1a56425e0a3c5ef5a5a4fff9105d5e073effeae6fcb268829ec3b451d02`.

`E7A0` ABI:
1. runtime wrapper/context;
2. reference/start 16-byte point;
3. working/requested 16-byte point;
4. corrected/output 16-byte point;
5. optional common `0x38` hit metadata;
6. raw reject mask.

Common metadata bridge:
- static accepted hit copies a complete `0x38` raw HITS record;
- dynamic accepted hit writes compatible metadata, including identity source `object+0xD8 -> metadata+0x00` and a three-float caller-visible vector at `+0x28/+0x2C/+0x30`;
- exact semantic name of this vector remains unresolved.

`BCF0` direct layout evidence:
- list head = `RCX[category]`;
- next `+0x328`;
- primitive/object type `+0x120`, handled values `2..6`;
- raw 16-bit filter flags `+0xDA/+0xDB`;
- identity/key `+0xD8`.

Second-review correction: the first Slice-4 descriptor incorrectly claimed cross-category progressive working-point mutation. `BCF0` uses an internal local copy of its `R8` baseline. Both `0x0E` and `0x11` therefore start from the same baseline, while later successful passes overwrite caller-visible output. Corrected by commits `123ce6331a34d994e3f0121804568efc5ce5c5a4` and `1f0360661624acf41dfdfe776fe8229c5f2edb0b`.

## Safety gates

- exact canonical SHA required for build-specific evidence;
- expected bytes required before any real hook;
- no hooks on unknown/custom executables;
- no semantic field names without direct write/read/runtime evidence;
- DMC3 addresses remain profile-specific;
- no proprietary game bytes committed;
- tests are not weakened merely to become green.

## Next execution order

1. finish/promote the dynamic-world update slice `B7B0 -> B460 -> B6F0 -> B8E0` with exact body hashes and ABI separation from query-path `BCF0`;
2. reverse `0x14005FEC0` exact source-1 ABI;
3. reverse `0x1400601E0` exact in/out/fourth-component/accumulation semantics;
4. only then descend further into geometry helpers where source-equivalent reconstruction requires it.

## Explicit non-goals

- no guessed monolithic original `CollisionResult`;
- no invented gameplay names for dynamic primitive values `2..6`;
- no invented name for metadata vector `+0x28..+0x30`;
- no conflation of query path (`E7A0/E880/BCF0`) with dynamic-world update path (`B7B0/B460/B6F0/B8E0`);
- no GDSpaces ownership of recovered collision runtime.
