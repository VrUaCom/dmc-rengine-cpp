# Phase Map

**Snapshot date:** 2026-08-27  
**Reconciled canonical main:** through merged PR #242 (`f886f27e62ec9a05b6829df7fd074981a06a4b49`)  
**Pending branch truth:** #226, #238, #240, #241

The project is tracked by subsystem/layer gates rather than one linear phase number.

## Foundation — maintained

C++20/CMake, Windows+Ubuntu CI, evidence/artifact identity, GDSpaces identity/source/provenance, Binary Inspector, EXE evidence, WorkingCopy and guarded output infrastructure are maintained foundations.

## GDSpaces L1 — ACTIVE PRIMARY / INTERNAL PRODUCT PATH CLOSED

Canonical execution plan: [L1 roadmap](../gdspaces/l1-roadmap.md).

Remaining sequence:

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

Internal NBZ/PAC/PNST materialization and authoring are not the primary blocker. The open work is real same-lineage evidence.

Merged #233 proves protected `dmc3.exe` and the 960,358,951-byte `DMC3-0.nbz` are locatable. The connected materialization channel cannot ingest the full NBZ because of the observed 268,435,456-byte ceiling. This is transport/access scope, not archive absence or an L1 parser failure.

Pocket GDS PR #2 and pending DMC Rengine PR #238 provide an out-of-band exact-member evidence bridge. It can narrow L1-C/D but does not replace protected selected-provider authority, real edit/rebuild closure, #209 consumption or rollback.

### L1/L3 materialization completion seam — merged #230/#242

Normal `0x1401B8DC0` receives one u32 registry-relative context and no raw transport status, error flag, byte count, FileSlot/ReadRequest handle or child/outstanding-work metadata. Lower transport/materialization success or failure must therefore already be terminal before normal state2 publication, or the completion must be suppressed/removed.

FIFO insertion order alone is not a proven dependency barrier. The open seam is **materialization completion ordering / dependency bridge**, not a generic fan-in counter.

Exact-byte priority:

```text
0x1402EF4D0 queued materialization job
 -> relevant 0x1402EF790 persistence/re-poll/retirement case
 -> reacquire historical 0x1400333E0 / 0x140033390 hypotheses
 -> 0x1400335A0 transport terminal writes
 -> identify incomplete/failure suppression of normal 0x1401B8DC0
 -> 0x1402EF460 higher-scheduler clear/rollback
 -> only then .lst recursive child failure ordering
```

This supporting reverse does not reopen closed L1 product materialization/authoring and does not make L3 complete.

## GDSpaces L2 — ADVANCED SUPPORTING / REAL EVIDENCE + TOPOLOGY CORRECTION OPEN

Merged slices:

- #215: type-0 physical-provider final-open static reverse and product model;
- #219: legacy runtime-window/mapping tooling;
- #221: selected-provider content-candidate normalizer/validator/artifact binder;
- #235: process-instance-bound R2B v2 with PID + creation FILETIME + module identity + seven canonical anchors.

Open L2 gates:

```text
exact retail 0x0E collision census
 -> issue #237 / pending PR #241 discovery-vs-successful-mount correction
 -> real protected-process seven-anchor R2B v2 packet
 -> trusted zero-loss R3 selected-provider capture
 -> exact actually-successful archive/member binding
 -> final L2 audit
```

Merged #235 confirms filename discovery and successful archive mounting are distinct. First missing filename bounds discovery only; an existing archive can fail registration while discovery continues, so successful mounts can be sparse. Successful archive registrations prepend, preserving higher successful volume -> lower successful volume -> physical precedence.

## Original runtime / lifecycle L3 — ADVANCED STATIC SPINE / NOT COMPLETE

Current vertical validation boundary:

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

Pending #240 proposes exact canonical `LoadedResource +0x04` writer census as `STATIC BOUNDED-CLOSED / APPROVED / CONTRADICTION-GATED`. Until merge this is branch truth. Even after promotion, R2-R5 and V1-V7 remain open.

No generic fan-in/outstanding-work field should be invented absent direct evidence.

## Validation V/LV — ACTIVE CROSS-CUTTING

Validation does not own resolution/materialization/lifecycle. It binds evidence across those owners and decides promotion.

The L1 completion verdict requires one reconciled lineage rather than independent PASS packets matched by filenames. V/LV is not L4.

## Resource Control Plane / grey boundary — DRAFT ORCHESTRATION, NOT L4

Draft #226 models request/dependency planning, pending/ready coordination, claims and transition control above the three execution layers. It may not move resource byte ownership, selection authority or lifecycle truth out of L1/L2/L3.

## Stage Ops / Stage Semantic Graph / ModViz — DOWNSTREAM

Stage Ops consumes GDSpaces outputs and owns downstream assembly/orchestration. ModViz consumes Stage Ops state. None may create a private resolver/materializer. Major game-backed Stage Ops expansion must not displace L1 closure.

## EXE Editor / Recovered Game Source Tree — PARALLEL EVIDENCE TRACK

Recovered source must remain linked to exact binary ranges, ABI/ownership/lifetime evidence and validation receipts. Progressive recompilation remains downstream of bounded behavioral comparison.

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

This is dependency priority, not a ban on parallel evidence work.

No documentation update, synthetic CI result or pending PR creates an L1/L2/L3 COMPLETE claim by itself.
