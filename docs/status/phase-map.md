# Phase Map

**Snapshot date:** 2026-08-27  
**Canonical base reviewed:** `main@f886f27e62ec9a05b6829df7fd074981a06a4b49`  
**Boundary/status authority:** `../gdspaces/layer-boundary-status-reconciliation-2026-08-27.md`  
**Focused L1 gap authority:** PR #244

The project is tracked by subsystem/layer gates, not one linear phase number.

## Foundation — maintained

C++20/CMake, Windows+Ubuntu CI, evidence/artifact identity, GDSpaces source/provenance, Binary Inspector, EXE evidence, WorkingCopy, guarded output and canonical-EXE acquisition/disassembly support remain maintained foundations.

## GDSpaces L1 — INCOMPLETE / NOT 100%

L1 product implementation is advanced, but original byte-materialization reverse is **not exhaustive**.

Current static order:

```text
size/zero/error: 0x14002F9F0 -> 0x140048E20
 -> rounded-transfer/final-chunk/EOF/short-read: 0x140033390..0x1400335A0 + backend clamp
 -> capacity/alignment/overflow: 0x1401B7B90
 -> backing initialization / .lst padding
 -> representation/planning: 0x1401B79E0 + 0x1401B7FD0
 -> .lst writer/failure equivalence: 0x1401B85C0
 -> exact byte-producing ingress/context behind 0x1402EF4D0
 -> partial STORE/InflateRead terminal composition
 -> L1 terminal-result / L3 completion seam
```

The original L1 byte-materialization cut ends at the materializer result through `0x1401B8CA0`. Normal LoadedResource `state1 -> state2` remains L3 lifecycle publication.

Real acceptance then remains:

```text
direct-retail selected-member provenance
 -> exact representation classification
 -> supported real edit/rebuild/rematerialization
 -> original DMC3 consumption
 -> rollback
 -> final L1 audit
```

## GDSpaces L2 — ADVANCED / INCOMPLETE

L2 owns logical request/candidate/provider/source/volume/member selection and pre-usable-selection failure semantics.

Open gates:

- retail `0x0E` collision census;
- real protected-process mapping;
- trusted selected-provider identity;
- final L2 audit.

Once usable identity exists, exact byte production is L1.

## Original runtime / lifecycle L3 — ADVANCED / INCOMPLETE

L3 owns:

- request/queue/callback ownership and scheduler lifetime;
- normal LoadedResource `state1 -> state2` publication;
- typed post-load / state2 -> state3 ready;
- cancellation/replacement;
- quiescence and state4 cleanup;
- claims/cache/factory ownership;
- release/reset/shutdown and lifetime distinctions.

The cross-layer completion seam is:

```text
[L1] terminal materializer result
 -> eligibility/suppression dependency
 -> [L3] scheduler/callback -> state1 -> state2
```

Open L3 work remains scheduler/lifecycle breadth plus V1–V7 original-process receipts.

## Stage Ops / Stage Semantic Graph / ModViz — DOWNSTREAM DOMAIN

Stage Ops owns stage assembly/orchestration over GDSpaces outputs. Stage/domain progress does not count as L1/L2/L3 completion and must not create a private resolver/materializer/lifecycle authority.

## EXE Editor / Recovered Game Source Tree — PARALLEL EVIDENCE TRACK

Recovered-source work stays linked to exact binary ranges, ABI/ownership/lifetime evidence and validation receipts. Progressive recompilation is downstream of bounded behavioral comparison.

## Current long-term ordering

```text
L1 byte-exactness reverse closure
 -> L1/L3 terminal-completion seam reconciliation
 -> L2/L1 real selected-resource evidence
 -> L1 real edit/rebuild/rematerialization
 -> L3/V same-resource typed-ready/use evidence
 -> original-game consumption + rollback
 -> final L1 audit
 -> independent final L2/L3 audits
 -> Stage Ops game-backed assembly
 -> semantic/editor verticals
 -> bounded recovered-subsystem equivalence
 -> progressive recompilation
```

This is dependency priority, not a ban on parallel evidence work.

**Current labels: L1 NOT COMPLETE, L2 NOT COMPLETE, L3 NOT COMPLETE.**