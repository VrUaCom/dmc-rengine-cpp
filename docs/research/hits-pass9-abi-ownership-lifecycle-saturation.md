# HITS Raw Reverse Pass 9 — ABI Ownership and Lifecycle Saturation

Date: 2026-08-14  
Canonical executable SHA-256: `e454272ed0fb0247fcbcf300e5d55d7a3e96d50b89b9ffaff81bb978dcbdd082`  
Status: **STATIC ABI/OWNERSHIP SATURATED — RUNTIME VALIDATION OPEN**

Canonical detailed authority: Google Drive document `DMC Rengine — HITS Raw Reverse Pass 9 — ABI Ownership and Lifecycle Saturation — 2026-08-14`.

## Source 2 is structurally HITS-runtime-compatible

Phase 3 establishes the StageCollisionWorld source array at `+0x08/+0x10/+0x18`. Source 2 is externally/global-bound, but the same `0x14005EBC0` selector accepts indices `0..2`, loads `world->sources[source]`, validates the selected object's `raw` field through the HitsRuntime ABI, and only then changes `selectedSource`. The selected-source query family indexes the same source array.

Evidence-safe refinement:

- source 2 is **EXE CONFIRMED structurally compatible** with the HitsRuntime interface/layout used by the selector/query family;
- its backing resource, owner/lifetime, gameplay role and live selection path remain **RESEARCH REQUIRED**;
- Phase 4 still stands: no direct static call selecting source 2 was found.

## Stage-local source ownership/lifecycle order

Stage-owner construction at `0x140245A50` creates embedded objects:

- `owner+0x680` — first HitsRuntime;
- `owner+0x6C8` — second HitsRuntime;
- `owner+0x710` — StageCollisionWorld.

`0x14005EBA0` binds the world to the stable embedded runtime addresses plus external source 2 before `0x140245DE0` populates HITS bytes.

`0x140245DE0` then resolves main-PAC member 3/member 6, preserves raw provenance at `owner+0x730/+0x738`, and calls `0x1402D3060` to initialize the already-existing embedded runtime objects. `0x140245D40` later tears both down with `0x1402D29C0` and clears the raw member pointers.

Implementation consequence: future hot reload/rebuild integration must reinitialize the correct embedded HitsRuntime and preserve StageCollisionWorld bindings; it must not replace source pointers with transient PAC member bytes.

## Resource-lifecycle handoff boundary narrowed

Wave 2's general `363 x 0x48` resource lifecycle is a separate layer. Phase 2 proves `0x140245DE0` receives an already usable loaded main stage-PAC descriptor. Therefore the remaining lifecycle gap is specifically the instruction/callback path from the general ready-resource representation to the stage-owner consumer that invokes/schedules `0x140245DE0`.

From `0x140245DE0` onward, member3/member6 -> embedded HitsRuntime initialization is already EXE CONFIRMED.

Do not place HITS in the confirmed MOD/EFM/SCM/SHW typed-fixup list without direct evidence.

## 189-descriptor cross-layer cardinality check

Wave 2 independently establishes 189 observed Stage resource descriptors (110 + 79). HITS Phase 4 independently records 189 unique `scr\\stXXX.pac` paths in the executable HITS inventory.

This exact cardinality match is a strong consistency check for descriptor-driven HITS corpus enumeration, but does **not** by itself prove row ordering or 189 independent gameplay stages.

## Correction delta direction

Pass 7C instruction evidence is authoritative:

```text
postQuery = [object+0x130] after query
SUBPS postQuery, originalSnapshot
```

Therefore:

```text
delta = postQuery - original
```

The older `original - queryModified` statement is **SUPERSEDED/REJECTED**.

## Static saturation closure

A second wide sweep re-read the canonical Phase 2/3/4 and Pass 7/7B/7C evidence against current Drive, GitHub and File Library search.

No additional preserved instruction body/write-window was materialized for:

- `0x14005E7A0`;
- `0x14005B460`;
- `0x14005FEC0`;
- `0x1400601E0`.

Confirmed at closure:

- exact stage-local source0/source1 object ownership and initialization/teardown order;
- StageCollisionWorld source array and selected-source ABI;
- source2 structural HitsRuntime compatibility, without confirmed live selection;
- preserved direct selector paths select source 1 and restore source 0;
- generic query family resolves `sources[selectedSource]`;
- dynamic six-channel category formula and category-to-HITS reject-mask bridge;
- specialized query register contracts for EE40/60790/EBE0/F070;
- explicit 16-byte in/out correction behavior for EBE0/F070;
- canonical correction delta `postQuery - original`;
- static HITS and dynamic collision remain distinct subsystems merged above static traversal.

The remaining P0 questions require instruction bytes or runtime deltas not present in the preserved summary/xref corpus. Repeating searches over the same summaries is not accepted as new reverse evidence.

## P0 handed to Pass 10

- `0x14005E7A0`: complete args/write-set/no-hit initialization/metric/static-dynamic arbitration/tie-break;
- `0x14005B460`: internal body/category-list entry ABI/ownership/candidate production/return semantics;
- `0x14005FEC0`: exact source-1 output ABI;
- `0x1400601E0`: exact in/out field layout/fourth component/accumulation semantics.

## P1 remains open

- source-2 backing resource/type/lifetime and indirect/inlined selection;
- ready-resource -> `0x140245DE0` handoff;
- all-189-descriptor HITS member presence/hash/corpus sweep.

## Pass 10 handoff

Pass 10 changes method. It owns evidence reacquisition and, if necessary, hash-gated observation-only runtime instrumentation. Missing original-game result fields must not be invented. DMC Rengine `ContactResult` remains a product/editor abstraction, not Capcom ABI.

## Evidence boundary

No proprietary EXE/PAC/HITS bytes are committed. Missing instruction windows are not reconstructed by guesswork.
