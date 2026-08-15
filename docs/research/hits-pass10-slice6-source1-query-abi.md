# HITS Pass 10 — Slice 6 — Source-1 Query ABI (`FEC0` / `601E0`)

Date: 2026-08-14  
Canonical executable SHA-256: `e454272ed0fb0247fcbcf300e5d55d7a3e96d50b89b9ffaff81bb978dcbdd082`  
Status: **EXE CONFIRMED / IMPLEMENTED — CI VALIDATION**

## Purpose

Close the two source-1 P0 query functions reached through the exact temporary source-selection routes:

- `0x1400568F0 -> select source1 -> 0x14005FEC0 -> 0x140056936 restore source0`;
- `0x140056832 -> select source1 -> 0x1400601E0 -> 0x14005686E restore source0`.

Canonical bytes come from the same Phase-17 verified preserved raw range used by Pass-10 Slices 4 and 5.

## Canonical body receipts

| Role | VA range | Size | SHA-256 |
|---|---:|---:|---|
| source1 segment correction | `0x14005FEC0..0x1400601D3` | `787` | `cfaca9752adf3a581969875e99c6c1b9ffaa7f83d19d22fddbe8dec2d5cab09e` |
| source1 displacement accumulator | `0x1400601E0..0x14006078E` | `1454` | `7fb7fb7ce9447a5e200ba1471774eafe7e5f21877da71902f047563bd9f7597a` |

## `0x14005FEC0` exact ABI

Observed arguments:

1. `RCX` — currently selected HITS runtime wrapper;
2. `RDX` — mutable 16-byte point, input and output;
3. `R8` — read-only 16-byte reference/target point.

The sole preserved caller selects source 1/member 6 immediately before the call and restores source 0/member 3 immediately afterward.

### Algorithmic contract

`FEC0` computes `R8 - RDX`, measures XYZ magnitude, and rejects only a degenerate/near-zero segment before entering broadphase.

Its direction-normalization helper uses XYZ magnitude and explicitly writes `0.0f` to the fourth component of the normalized direction.

For a non-degenerate segment:

- broadphase cells are resolved from the source-1 HITS runtime;
- duplicate raw-record visits are suppressed;
- raw record bit `0x00080000` is rejected directly (`record byte +0x02`, bit `0x08`);
- accepted geometric corrections update an internal working point;
- the final 16-byte working point is written back through `RDX`.

### Return-value correction

`AL` is **not a hit/no-hit boolean**.

- degenerate XYZ segment -> false;
- non-degenerate segment -> true, including the path where no record produces a correction.

Therefore callers must not interpret `FEC0` false/true as collision absence/presence.

## `0x1400601E0` exact ABI

Observed arguments:

1. `RCX` — currently selected HITS runtime wrapper;
2. `RDX` — mutable 16-byte point, input/output and final accumulated point;
3. `R8` — read-only 16-byte reference/anchor point.

Five direct callers are preserved. One exact caller temporarily selects source1 before `601E0`; the remaining callsites use the currently selected runtime according to their surrounding control flow.

## `601E0` two-stage correction model

### Stage A — initial raw-record correction

- preserve original `*RDX` for final movement comparison;
- compute `*RDX - *R8`;
- broadphase candidate cells from the current/reference pair;
- deduplicate raw HITS record visits;
- evaluate raw records through the source runtime;
- accepted corrections replace the internal working point;
- write the Stage-A working point back to `*RDX`.

### Stage B — record-vector displacement accumulation

After Stage A, recompute direction from corrected `*RDX` toward/relative to `*R8`, collect a second candidate set and process raw records again.

Observed operations include:

- XYZ dot gating against record vector `+0x28/+0x2C/+0x30`;
- contact/scalar test through `0x1402CD340`;
- scale a four-component record vector through `0x14032EE50`;
- add the resulting 16-byte displacement to `*RDX`;
- repeat across accepted records.

This is an accumulator, not a one-shot boolean query.

## Fourth-component operational semantics

The gameplay meaning/name of the fourth float remains **UNRESOLVED**, but its algorithmic treatment is now confirmed.

- XYZ magnitude helpers (`0x14032EE80`, `0x14032E5F0`) ignore component `+0x0C`.
- XYZ dot helper (`0x14032E5C0`) also ignores component `+0x0C`.
- the `601E0` normalization path `0x140330390` derives normalization magnitude from XYZ but scales all four components, so the fourth scalar is carried through the normalized vector.
- scale helper `0x14032EE50` scales all four components.
- `601E0` adds all four scaled components into the mutable 16-byte point.

Therefore the fourth component is **not part of spatial length/dot gating**, but it is transported and accumulated through correction math. No gameplay name is assigned without stronger cross-producer evidence.

`FEC0` differs: its normalized direction helper `0x1403301D0` explicitly zeroes component `+0x0C`.

## `601E0` return contract

At function exit, helper `0x14032E5F0` computes XYZ distance between the preserved original `RDX` point and the final accumulated `RDX` point. `AL` is true only when that XYZ displacement is greater than epsilon.

So `601E0` returns **whether it materially moved/corrected the point in XYZ**, not whether candidates existed and not a raw-record hit count.

## Architecture impact

The former P0 wording is now superseded:

- `FEC0`: exact source-1 output ABI is closed; return semantic is non-degenerate-segment processing, not hit boolean.
- `601E0`: exact 3-argument in/out ABI, two-stage accumulation and fourth-component operational role are closed; only gameplay naming remains open.

## Implementation

Profile-specific evidence module:

- `include/dmc_rengine/profiles/dmc3/hits_source1_query_evidence.hpp`
- `tests/hits_source1_query_evidence_tests.cpp`
- CTest stem `hits_source1_query_evidence`

The module is deliberately separate from generic HITS and from dynamic-world update evidence.

Commits:

- `2cd8c302ead5718f291309968c0c06bd961fbfeb` — source1 evidence module;
- `9875b61cc6d07fe9a745f6c116b6a5f3117eff73` — regression coverage;
- `6e667afe50b9aa9d774062d213a9fe475a933495` — CTest registration.

## Remaining boundary

After this slice, the primary open HITS work is no longer `E7A0/B460/FEC0/601E0` top-level ABI discovery. Remaining work moves downward/outward to:

- reconcile coarse `hits_query_evidence` ABI-kind labels with these newly closed specialized contracts;
- semantic naming of unresolved primitive/subtype values only where evidence allows;
- semantic naming of the common metadata/record fourth/vector fields only after cross-producer proof;
- deeper geometry helper reconstruction required for source-equivalent reimplementation;
- controlled runtime validation of reconstructed behavior rather than further static top-level ABI guessing.
