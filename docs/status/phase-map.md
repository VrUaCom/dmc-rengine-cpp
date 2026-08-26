# Phase Map

**Snapshot date:** 2026-08-27  
**Reconciled canonical main:** through merged PR #242 (`f886f27e62ec9a05b6829df7fd074981a06a4b49`)  
**Pending branch truth:** #226, #238, #240, #241

The project is tracked by subsystem/layer gates rather than one linear phase number.

## Foundation — maintained

C++20/CMake, Windows+Ubuntu CI, evidence/artifact identity, GDSpaces identity/source/provenance, Binary Inspector, EXE evidence, WorkingCopy and guarded output infrastructure are maintained foundations.

## GDSpaces L1 — ACTIVE PRIMARY / INTERNAL PRODUCT PATH CLOSED

Canonical execution plan: [L1 roadmap](../gdspaces/l1-roadmap.md).

Current remaining sequence:

```text
exact real selected/member lineage
 -> exact member materialization receipt
 -> representation classification
 -> real bounded edit
 -> bottom-up PAC/PNST rebuild
 -> next-volume NBZ publication/reopen/rematerialization
 -> original-game consumption + rollback
 -> final V:L1 acceptance audit
```

Internal NBZ/PAC/PNST materialization and authoring are not the primary blocker now. The open work is real same-lineage evidence.

Connected artifact discovery can locate protected `dmc3.exe` and the 960,358,951-byte `DMC3-0.nbz`, but the connected raw-transfer/materialization channel cannot ingest the full NBZ because of the observed 268,435,456-byte ceiling. This is transport/access scope, not archive absence or an L1 parser failure.

Pocket GDS PR #2 / pending DMC Rengine PR #238 provide a real-device exact-member evidence bridge over canonical mobile GDSpaces. It can narrow the L1-C member-byte sub-gate and L1-D representation classification when the archive is already local on-device. It does not replace protected selected-provider authority, real edit/rebuild closure, #209 original-game consumption or rollback.

### L1/L3 materialization completion seam — merged #230/#242

Normal `0x1401B8DC0` receives one u32 registry-relative context and no raw transport status, error flag, byte count, FileSlot/ReadRequest handle or child/outstanding-work metadata. Therefore lower transport/materialization success or failure must already be terminal before normal state2 completion publication, or the queued completion must be suppressed/removed.

FIFO insertion order alone is not a proven dependency barrier. The remaining reverse seam is **materialization completion ordering / dependency bridge**, not a generic fan-in counter.

Focused exact-byte order:

```text
0x1402EF4D0 queued materialization job
 -> relevant 0x1402EF790 persistence/re-poll/retirement case
 -> reacquire historical 0x1400333E0 / 0x140033390 hypotheses
 -> 0x1400335A0 transport terminal writes
 -> identify suppression/blocking of normal 0x1401B8DC0 on incomplete/failure
 -> 0x1402EF460 higher-scheduler clear/rollback
 -> only then .lst recursive child failure ordering
```

This is supporting cross-layer reverse. It does not reopen already-closed L1 product materialization/authoring work and does not make L3 complete.

## GDSpaces L2 — ADVANCED SUPPORTING / REAL EVIDENCE + TOPOLOGY CORRECTION OPEN

Closed/integrated slices include:

- #215: type-0 physical-provider final-open static reverse and bounded product model;
- #219: legacy bounded runtime-window/mapping tooling;
- #221: selected-provider content-candidate normalizer/validator/artifact binder;
- #235: process-instance-bound R2B v2 tooling using PID + creation FILETIME + module identity and seven canonical resolver/bootstrap anchors.

Open L2 gates:

```text
exact retail 0x0E collision census
 -> discovery-vs-successful-mount product correction (issue #237 / PR #241)
 -> real protected-process seven-anchor R2B v2 packet
 -> trusted zero-loss R3 selected-provider capture
 -> exact actually-successful archive/member binding
 -> final L2 audit
```

Merged #235 confirms filename discovery and successful archive mounting are distinct. First-missing filename bounds discovery, not independently the successful-mounted set. An existing archive can fail registration while later filenames are still discovered, so the successful mounted set can be sparse. Successful archive registrations prepend, preserving higher successful volume -> lower successful volume -> physical precedence among successful mounts.

Issue #237 tracks the correction and PR #241 is the active product implementation. #241 remains branch truth until merged.

L2 remains supporting while L1 acceptance is open, but any L2 source-identity/topology assumption used in the final L1 lineage must already be evidenced at the scope required by that run.

## Original runtime / lifecycle L3 — ADVANCED STATIC SPINE / NOT COMPLETE

Recovered static authority includes FileSlot/AsyncIO materialization, ZIP read/inflate, LoadedResource state progression, typed post-load and major claim/reset/release structure.

For the first full vertical proof the current validation boundary is:

```text
exact L2 selected identity
 -> exact L1 materialized byte identity
 -> L3 acquisition/state1
 -> lower transport/materialization terminal condition
 -> normal completion/state2
 -> typed-ready/state3
 -> deterministic consumer effect
 -> rollback
```

Pending PR #240 proposes the exact canonical `LoadedResource +0x04` writer census as `STATIC BOUNDED-CLOSED / APPROVED / CONTRADICTION-GATED`. Until merge this is branch truth only. Even after promotion, R2-R5 and V1-V7 remain open and Layer 3 remains incomplete.

No generic fan-in/outstanding-work field should be invented absent direct evidence.

## Validation V/LV — ACTIVE CROSS-CUTTING

Validation does not own resource resolution/materialization/lifecycle. It binds evidence across those owners and decides promotion.

The L1 completion verdict requires one reconciled lineage rather than independent PASS packets matched by filenames.

Draft V/LV work remains validation architecture, not a fourth execution layer.

## Resource Control Plane / grey boundary — DRAFT ORCHESTRATION, NOT L4

Draft PR #226 models request planning/emission, dependency planning/emission, pending/ready coordination, claims, cancellation/quiescence and replacement above the three execution layers. It is an orthogonal control plane and may not move resource byte ownership, selection authority or lifecycle truth out of L1/L2/L3.

## Stage Ops / Stage Semantic Graph / ModViz — DOWNSTREAM

Stage Ops owns product-side stage assembly/orchestration over GDSpaces outputs. Stage Semantic Graph represents that state. ModViz consumes it. None may create a private resource resolver or archive materializer.

Major game-backed Stage Ops expansion should not displace L1 closure.

## EXE Editor / Recovered Game Source Tree — PARALLEL EVIDENCE TRACK

EXE Editor should expose one canonical recovered-source tree linked to exact binary ranges, ABI/ownership/lifetime evidence and validation receipts. Progressive recompilation remains downstream of bounded behavioral comparison.

## Long-term ordering

```text
L1 real-retail/original-game acceptance
 -> narrow L2 real-evidence closure
 -> representative L3 dynamic lifecycle validation
 -> Stage Ops game-backed assembly
 -> semantic/editor verticals
 -> bounded recovered-subsystem equivalence
 -> progressive recompilation
 -> working rebuilt executable milestones
```

This ordering is dependency priority, not a ban on parallel evidence work.

No documentation update, synthetic CI result or pending PR creates an L1/L2/L3 COMPLETE claim by itself.
