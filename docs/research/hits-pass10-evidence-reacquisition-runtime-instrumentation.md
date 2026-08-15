# HITS Raw Reverse Pass 10 — Evidence Reacquisition and Runtime Instrumentation

Date: 2026-08-14 / updated 2026-08-15  
Canonical executable SHA-256: `e454272ed0fb0247fcbcf300e5d55d7a3e96d50b89b9ffaff81bb978dcbdd082`  
Status: **ACTIVE — TOP-LEVEL P0 ABI CLOSED; DEEP SEMANTICS / SOURCE-EQUIVALENT RECONSTRUCTION CONTINUE**

Canonical detailed working authority: Google Drive document `DMC Rengine — HITS Raw Reverse Pass 10 — Evidence Reacquisition and Runtime Instrumentation — 2026-08-14`.

Companion research:
- `docs/research/reverse-pass-implementation-review-loop.md`
- `docs/research/hits-pass10-slice4-canonical-combined-query-abi.md`
- `docs/research/hits-pass10-slice5-dynamic-world-update-topology.md`
- `docs/research/hits-pass10-slice6-source1-query-abi.md`
- `docs/research/hits-pass10-slice7-primitive-shape-mapping.md`
- `docs/research/hits-pass10-slice8-common-contact-normal.md`
- `docs/research/hits-pass10-slice9-primitive-descriptor-ownership.md`

## Method and evidence boundary

Pass 10 follows the canonical reverse → review → deepen → promotion gate → implement → implementation review → debug/test → consolidate → second review/debug → authority sync loop.

Canonical instruction windows are reacquired from project artifacts with explicit provenance. The preserved Phase-17 probe was cloned from canonical `dmc3.exe` SHA `e454...d082`, and the Phase-17 verifier preserved original PE raw section bytes over file range `0x400..0x60E600` byte-identically. Phase-18 derivative bytes are used only as an independent consistency check.

No proprietary instruction bytes are committed. Public evidence stores VAs, ranges, hashes, ABI/layout facts, topology and reconstructed behavior.

# Top-level P0 closure

The original Pass-10 target set `E7A0 / B460 / FEC0 / 601E0` is closed or correctly reclassified. Do not reopen the old descriptions unless contradictory direct evidence appears.

## `0x14005E7A0` — combined point query

Canonical body `0x14005E7A0..0x14005E880`, 224 bytes, SHA `3716472a87c7edd9ea27b800e165de7fee8254c8b928c3a41e431b0f350b8a6f`, 51 direct callers.

Six-argument ABI:
1. runtime wrapper/context;
2. reference/start 16-byte point;
3. requested/working 16-byte point;
4. corrected/output 16-byte point;
5. optional common `0x38` hit metadata;
6. raw reject mask.

Ordered passes:
1. static HITS `0x14005E880`;
2. dynamic `0x14005BCF0` category `0x0E`;
3. dynamic `0x14005BCF0` category `0x11`.

Both dynamic passes start from the same static-or-input baseline. Caller-visible output precedence is last-successful-writer: `static < 0x0E < 0x11`. Total miss copies arg3 to arg4 and returns false.

**REJECTED:** wrapper-level metric/equality/tie-break model.

## Dynamic world update — separate pipeline

`B460` is not an `E7A0` candidate producer. Separate dynamic-world path:

`B7B0 dispatcher -> B460 pair resolver -> B6F0 compatibility`, followed by `B8E0` post/update processing.

This evidence remains isolated in `hits_dynamic_update_evidence.hpp` so query and world-update contracts cannot collapse into one abstraction.

## `0x14005FEC0` — source/current-runtime segment correction

Canonical body `FEC0..601D3`, 787 bytes, SHA `cfaca9752adf3a581969875e99c6c1b9ffaa7f83d19d22fddbe8dec2d5cab09e`.

ABI: selected runtime, mutable 16-byte point in/out, reference/target point. Direct raw HITS bit `0x00080000` reject confirmed. Direction normalization zeroes component 4.

`AL` is **not** hit/no-hit: false is the degenerate near-zero XYZ-segment case; a non-degenerate processed segment can return true without a record correction.

## `0x1400601E0` — displacement accumulator

Canonical body `601E0..6078E`, 1454 bytes, SHA `7fb7fb7ce9447a5e200ba1471774eafe7e5f21877da71902f047563bd9f7597a`.

ABI: selected/current runtime, mutable 16-byte point in/out, reference/anchor point. Two-stage record correction plus vector displacement accumulation. Return true means material final XYZ displacement from the original point.

Component 4 is excluded from XYZ norm/dot gates but may be transported/scaled/accumulated. Its gameplay-specific semantic name remains unresolved.

# Slice 7 — parsed shape → primitive descriptor → runtime representation

Direct data flow proves three separate enum/representation layers:

| Source token | parser enum | primitive descriptor | primary runtime type |
|---|---:|---:|---:|
| `sphere` | 0 | 2 | 2 |
| `box` | 1 | 3 | 3 |
| `cylinder` | 2 | 6 | 6 |
| `capsule` | 3 | 4 | 4 |

Runtime type 4 has dual origin: parsed capsule/descriptor4, or a moving sphere promoted to a swept representation when displacement exceeds `2 × radius`.

Runtime type 5 is structurally a three-vertex face representation with vertices at runtime `+0x130/+0x140/+0x150` and a normalized direction at `+0x160`. No source-text token/name for descriptor type 5 is promoted.

Validated code head `cccb5fc45c0c4cb1746cd5630d474533fadb6f76`; GitHub Actions run `31815597161` passed Ubuntu + Windows.

# Slice 8 — common collision/contact surface normal

The previous label for common metadata `+0x28/+0x2C/+0x30` as an unresolved vector is **SUPERSEDED**.

Canonical semantic promotion:

**metadata `+0x28/+0x2C/+0x30` = common collision/contact surface normal float3.**

Independent proof chains:
- static `E880` normalizes query direction and computes XYZ dot product against raw HITS record `+0x28..+0x30` as a facing gate;
- dynamic type 2 emits normalized `contact point - sphere center`;
- dynamic type 5 emits its normalized face vector from runtime `+0x160`;
- dynamic type 6 emits axial `±Y` cap normals or normalized radial side normals;
- downstream consumer around `0x1402C65B2` uses the same metadata vector in `v' = v - 2 * dot(v,n) * n`, the vector-reflection formula.

Relevant canonical helper bodies:
- static XYZ dot3 `0x14032E5C0..0x14032E5E5`, SHA `d9dba88696b4f17f15617eb2dd1b2e39713da0d0225f4e2130e0efc0931c0779`;
- XYZ direction normalize `0x140330390..0x14033044A`, SHA `568543ebcb3a7ea60b09e65c54da9f9364417d3679f5e9101a80722472f1a9d4`;
- direction-only 3×3 transform `0x14032DC70..0x14032DCF6`, SHA `46a253c0b7824d5ac26104947597961ff4d1399cabbd069b732ad43bab29fe1d`;
- reflection dot3 `0x140030D30..0x140030DB1`, SHA `7a1a584ecdd42d4c0af7c9f5bfdb02a47d32facd4d0c75efd777e11cac5cdae8`;
- XYZ scale/preserve-W `0x1400311E0..0x1400311FE`, SHA `c4a58e73cfb2af640f32fad70956b98f58038c4d28b23c62ea74a6f239300e40`;
- reflection consumer `0x1402C64F0..0x1402C6805`, SHA `86b77b7431411f7b2ed1c6f7b1109aebd36c305e2e8684c6d6897f915b76e7c4`.

Exact global outward/inward orientation and coordinate-system handedness are **not** promoted.

Implementation:
- `include/dmc_rengine/profiles/dmc3/hits_contact_normal_evidence.hpp`
- `tests/hits_contact_normal_evidence_tests.cpp`
- CTest `hits_contact_normal_evidence`

Validated exact code head `63a2782bb18649f0b7a19e0110671897721f16da`; GitHub Actions run `31852876030` passed Ubuntu + Windows.

Detailed authority: `hits-pass10-slice8-common-contact-normal.md`.

# Slice 9 — primitive descriptor ownership/runtime bridge

Primitive descriptor ownership is closed separately from transform ownership.

## Manager tables

`0x14005C260` writes:
- arg2 / `RDX` -> manager `+0x108` entry-table pointer;
- arg3 / `R8` -> manager `+0x110` primitive-descriptor-table pointer;
- arg4 / `R9D` -> manager `+0x04` entry count.

Entry layout:
- stride `0x04`;
- raw flag byte `+0x00`;
- transform selector `+0x01`;
- primitive descriptor index `u16 +0x02`.

Primitive descriptor resolution:

`descriptor = manager+0x110 + u16(entry+0x02) * 0x50`

so the primitive descriptor stride is directly confirmed as **`0x50`**.

Canonical ownership bodies:
- manager source initializer `C260..C318`, SHA `9f405b59574c4575813b9c15aa146aad62815ff01258e4fca8fa1e34338f93e7`;
- indirect-transform builder `C630..C731`, SHA `97dbb8f5e6cace93530a30c936796a35fca80235467a3a0885f71c8593990d1a`;
- inline-transform builder `C740..C83D`, SHA `0778fa7ecce7855712b1d0bd5cb8ef5b32998e1d4629ee0d5d35e951318e06b6`;
- runtime object initializer `C8D0..C99D`, SHA `f779db92f9fee9d1492ef7208eb9950784d782e51542dd65d551ecdf6b950bfe`.

## Runtime ownership

`0x14005C8D0` stores:
- entry/source pointer -> runtime object `+0x110`;
- primitive descriptor pointer -> runtime object **`+0x118`**;
- transform pointer -> runtime object **`+0x20`**.

`0x1402CC530` uses `runtime+0x20` for transform/matrix construction and separately reads `runtime+0x118`, then descriptor byte `+0x00`, to dispatch primitive types `0..6`.

**CORRECTED / REJECTED transient model:** `runtime+0x20` is not the primitive descriptor pointer.

Current ownership graph:

`entry -> u16 descriptor index -> manager descriptor table (0x50 stride) -> runtime +0x118 -> CC530 type dispatch`

Transform graph remains independent:

`transform source -> runtime +0x20 -> runtime matrix +0x30`.

## Bounded descriptor census

Only C260 callsites with statically resolved entry table, descriptor table and exact R9 entry count were accepted:
- 14 fully bounded static callsites;
- 18 actually referenced entries;
- observed primitive type set `{2}`;
- no type 5 in this bounded static subset.

This is negative evidence, not proof of absence. Unbounded `0x50` scans are rejected because they cross neighboring unrelated data. Type-5 upstream work must follow actual referenced indices into runtime/object-populated descriptor sources.

Implementation:
- `include/dmc_rengine/profiles/dmc3/hits_primitive_descriptor_ownership_evidence.hpp`
- `tests/hits_primitive_descriptor_ownership_evidence_tests.cpp`
- CTest `hits_primitive_descriptor_ownership_evidence`

Validated exact code head `b919e89084d5583a940fc1fe8ecc8f90cb1968fd`; GitHub Actions run `31853246146` passed Ubuntu + Windows.

Detailed authority: `hits-pass10-slice9-primitive-descriptor-ownership.md`.

# Profile implementation architecture

Current DMC3-specific responsibilities are deliberately split:
- `hits_query_evidence.hpp` — query family, combined query, source selection/topology;
- `hits_dynamic_update_evidence.hpp` — dynamic-world pair/update pipeline;
- `hits_source1_query_evidence.hpp` — exact `FEC0/601E0` contracts;
- `hits_primitive_shape_evidence.hpp` — parser/descriptor/runtime primitive mapping;
- `hits_contact_normal_evidence.hpp` — common surface-normal semantic contract;
- `hits_primitive_descriptor_ownership_evidence.hpp` — descriptor-table and runtime ownership bridge;
- `runtime_trace.hpp` — generic observation-only trace contract.

All build-specific lookup APIs remain exact-canonical-SHA gated. Packed SHA `81c7...c7d6` receives no canonical VA/body/layout evidence.

# Major canonical corrections

1. DMC3 HITS VAs belong under `profiles/dmc3`, not generic HITS core.
2. Dispatcher reject-mask mapping is dispatcher-scoped, not a global category property.
3. `namespace detail` is not access control; backing tables use private `EvidenceStore` members.
4. `E7A0` metric/equality/tie-break model rejected.
5. BCF0 categories `0x0E/0x11` share one static-or-input baseline rather than progressive chaining.
6. `B460` candidate-producer role rejected; dynamic-world update role confirmed.
7. `FEC0 AL == hit` rejected.
8. `601E0 AL` narrowed to material XYZ displacement.
9. metadata `+0x28..+0x30` promoted from unresolved to collision/contact surface normal.
10. runtime object `+0x20` primitive-descriptor interpretation rejected; descriptor pointer is `+0x118`, transform pointer is `+0x20`.

# Current next reverse boundary

Do **not** restart top-level ABI discovery. Next work is:
1. trace population/writers of manager `+0x110` primitive descriptor tables and locate actually referenced descriptor type 5 producers;
2. resolve type 5 source/origin vocabulary only if direct evidence exists;
3. continue source-equivalent reconstruction of primitive-specific geometry/contact helpers;
4. close remaining component-4 semantics where cross-producer/consumer proof allows;
5. run controlled canonical runtime traces against reconstructed C++ behavior.

## Explicit non-goals

- no guessed monolithic original `CollisionResult`;
- no invented source-text name for primitive type 5;
- no unbounded descriptor-table scans treated as semantic evidence;
- no conflation of transform selectors with primitive types;
- no reopening superseded `E7A0/B460/FEC0/601E0` claims without contradictory direct evidence;
- no GDSpaces ownership of recovered collision runtime;
- no proprietary game bytes committed.
