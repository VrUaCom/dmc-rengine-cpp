# HITS Pass 10 — Slice 8: Common Collision / Contact Surface Normal

Date: 2026-08-15  
Canonical target SHA-256: `e454272ed0fb0247fcbcf300e5d55d7a3e96d50b89b9ffaff81bb978dcbdd082`  
Status: **EXE CONFIRMED / IMPLEMENTED / CI VALIDATION ACTIVE**

## Promotion

The three floats at common hit metadata offsets `+0x28/+0x2C/+0x30` are promoted from `UNRESOLVED vector` to **common collision/contact surface normal (float3)**.

This promotion is supported independently by static HITS production/consumption, multiple dynamic primitive producers, and a downstream reflection consumer. Exact normal orientation/handedness is **not** promoted.

## Static HITS evidence

`0x14005E880` normalizes the query direction and reads raw HITS record floats at `record+0x28/+0x2C/+0x30`.

It passes the normalized query direction and the record vector to `0x14032E5C0`.

`0x14032E5C0..0x14032E5E5` is an exact XYZ dot product. The result is used as a facing gate before the triangle/contact test.

Canonical helper body:
- range `0x14032E5C0..0x14032E5E5`;
- 37 bytes;
- SHA-256 `d9dba88696b4f17f15617eb2dd1b2e39713da0d0225f4e2130e0efc0931c0779`.

Direction normalization helper:
- `0x140330390..0x14033044A`;
- 186 bytes;
- SHA-256 `568543ebcb3a7ea60b09e65c54da9f9364417d3679f5e9101a80722472f1a9d4`.

Therefore raw static HITS `+0x28..+0x30` is a surface-facing direction vector, consistent with a surface normal.

## Dynamic producer evidence

Dynamic query `0x14005BCF0` writes the same metadata offsets from independent geometry representations.

### Runtime type 2

For a successful sphere contact it computes `contact_point - sphere_center`, normalizes the vector, and writes XYZ to metadata `+0x28/+0x2C/+0x30`.

### Runtime type 5

The accepted three-vertex-face path copies runtime object `+0x160/+0x164/+0x168` to metadata `+0x28/+0x2C/+0x30`.

The type-5 constructor transforms descriptor `+0x40` through `0x14032DC70`, which uses the upper `3x3` transform without translation, then normalizes the result before storing it at runtime object `+0x160`.

Direction-transform helper:
- `0x14032DC70..0x14032DCF6`;
- 134 bytes;
- SHA-256 `46a253c0b7824d5ac26104947597961ff4d1399cabbd069b732ad43bab29fe1d`.

This closes the earlier type-5 `auxiliary normalized vector` as a face/surface normal at the operational geometry level. A source-text name for descriptor type 5 remains unresolved.

### Runtime type 6

The runtime type-6 contact path emits either:
- axial `(0,+1,0)` / `(0,-1,0)` for cap contacts; or
- a normalized radial vector with `Y=0` for side contacts.

These are explicit geometric surface normals.

## Reflection-consumer proof

Caller `0x1402C65B2` invokes combined query `0x14005E7A0`. On success, its enclosing function reads metadata `+0x28/+0x2C/+0x30` as vector `n`.

It then computes the equivalent of:

`v' = v - 2 * dot(v, n) * n`

which is the standard vector-reflection formula across a plane/surface normal.

Relevant canonical bodies:
- dot3 helper `0x140030D30..0x140030DB1`, 129 bytes, SHA `7a1a584ecdd42d4c0af7c9f5bfdb02a47d32facd4d0c75efd777e11cac5cdae8`;
- XYZ scalar application preserving W `0x1400311E0..0x1400311FE`, 30 bytes, SHA `c4a58e73cfb2af640f32fad70956b98f58038c4d28b23c62ea74a6f239300e40`;
- reflection consumer `0x1402C64F0..0x1402C6805`, 789 bytes, SHA `86b77b7431411f7b2ed1c6f7b1109aebd36c305e2e8684c6d6897f915b76e7c4`.

The reflection consumer is the decisive cross-consumer proof that the common vector is a surface normal rather than an arbitrary auxiliary vector.

## Canonical semantic contract

Confirmed:
- metadata offset `+0x28`;
- three `float32` components at `+0x28/+0x2C/+0x30`;
- same semantic role across static HITS and dynamic collision metadata;
- static path uses it for facing/dot gating;
- dynamic type 2 emits normalized radial contact normal;
- dynamic type 5 emits pre-normalized face normal;
- dynamic type 6 emits axial/radial surface normals;
- downstream consumer uses it in the vector-reflection formula.

Not confirmed:
- global orientation convention (outward/inward under all producers);
- coordinate-system handedness implied by the normal;
- a gameplay-specific name beyond collision/contact surface normal;
- source-text vocabulary for runtime/descriptor type 5.

## Implementation

Profile-specific implementation:
- `include/dmc_rengine/profiles/dmc3/hits_contact_normal_evidence.hpp`
- `tests/hits_contact_normal_evidence_tests.cpp`
- CTest `hits_contact_normal_evidence`

All lookup APIs are exact-canonical-SHA gated. The packed SHA `81c7...c7d6` receives no canonical VA/body or semantic descriptors.

## Correction

**SUPERSEDED:** the prior description of metadata `+0x28/+0x2C/+0x30` as an unresolved caller-visible vector.

**CURRENT:** common collision/contact surface normal float3, with orientation/handedness deliberately unresolved.
