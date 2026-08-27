# Phase Map

**Snapshot date:** 2026-08-27  
**Canonical base reviewed:** `main@f886f27e62ec9a05b6829df7fd074981a06a4b49`  
**Boundary/status authority:** `../gdspaces/layer-boundary-status-reconciliation-2026-08-27.md`

The project is tracked by subsystem/layer gates rather than one linear phase number.

## Foundation — maintained

C++20/CMake, Windows+Ubuntu CI, evidence/artifact identity, GDSpaces source/provenance, Binary Inspector, EXE evidence, WorkingCopy, guarded output infrastructure and canonical-EXE acquisition/disassembly support are maintained foundations.

## GDSpaces L1 — INCOMPLETE / NOT 100%

Canonical execution plan: [L1 roadmap](../gdspaces/l1-roadmap.md).

L1 now explicitly includes selected-byte FileSlot/ReadRequest transport, transform/decompression, packed/loose representation materialization, the terminal materialization dependency and normal `state 1 -> 2` publication, plus product edit/rebuild/repack/rematerialization.

### Mandatory static gate — materialization terminal dependency

```text
0x1401B8CA0 materialization dispatch
 -> 0x1402EF4D0 job/submission
 -> lower whole-file/FileSlot transport
 -> UNKNOWN terminal success/error condition
 -> normal 0x1401B8DC0 eligibility/suppression
 -> state 1 -> 2
```

Normal `0x1401B8DC0` receives only a registry-relative context, so success/error eligibility must be resolved before its dispatch or the queued completion must be suppressed/removed. No generic fan-in counter is evidenced.

Current raw-pass priority:

```text
0x1402EF4D0 queued job identity/type
 -> matching 0x1402EF790 persistence/re-poll/retirement case
 -> 0x1400333E0 pending/success/error domain
 -> 0x140033390 terminal cleanup/release ordering
 -> 0x1400335A0 transport completion/status binding
 -> prove failed/incomplete suppression before 0x1401B8DC0
 -> 0x1402EF460 relevant pending-entry clear/rollback
 -> .lst child/recursive failure ordering using confirmed terminal model
```

### Real acceptance still mandatory

```text
direct-retail selected-member provenance
 -> exact representation classification
 -> supported real edit/rebuild
 -> next-volume publication/reopen/rematerialization
 -> original-game consumption
 -> rollback
 -> final L1 audit
```

The product implementation is advanced, but neither implementation maturity nor a future successful Level-E run alone may bypass the static terminal-dependency gate for an L1 `100% / COMPLETE` claim.

## GDSpaces L2 — ADVANCED / INCOMPLETE

L2 owns logical request/candidate/provider/source/volume selection, ambiguity/fallback/failure classification and successful selected ResourceRef/provider/member identity.

Strong/integrated slices include type-0 static provider behavior and protected runtime mapping/selected-identity tooling. Real-retail collision evidence, real protected-process mapping, trusted selected identity and final audit remain open.

Important boundary: once a usable selected provider/member exists, byte transfer/transform is L1.

## Original runtime / lifecycle L3 — ADVANCED / INCOMPLETE

Canonical L3 now begins from completed state2/materialized bytes.

Strong static authority includes typed post-load -> optional callback -> state3 ready, cancellation policy, quiescence, state4 cleanup, distinct release/reset policies, representative typed families, loader-node claim/release and teardown distinctions.

The earlier wording that placed selected-byte FileSlot/AsyncIO transport and normal state1->2 completion wholly in L3 is superseded.

L3 cancellation/replacement policy may invalidate state1/state2 records, but the byte-terminal dependency it suppresses is L1.

Open L3 work is residual static ownership breadth plus protected original-process V1–V7 lifecycle receipts.

## Stage Ops / Stage Semantic Graph / ModViz — DOWNSTREAM DOMAIN

Stage Ops owns stage assembly/orchestration over GDSpaces outputs. Stage Semantic Graph represents domain state. ModViz consumes it.

None may create a private resource resolver/materializer or treat Stage/domain progress as L1/L2/L3 completion.

## EXE Editor / Recovered Game Source Tree — PARALLEL EVIDENCE TRACK

EXE Editor should expose one canonical recovered-source tree linked to exact binary ranges, ABI/ownership/lifetime evidence and validation receipts. Progressive recompilation remains downstream of bounded behavioral comparison.

## Long-term ordering

```text
L1 terminal materialization dependency closure
 -> L2/L1 real selected-resource evidence
 -> L1 real edit/rebuild/rematerialization
 -> L3 consumer-ready/use evidence for same resource
 -> original-game consumption + rollback
 -> final L1 audit
 -> independent final L2/L3 audits
 -> Stage Ops game-backed assembly
 -> semantic/editor verticals
 -> bounded recovered-subsystem equivalence
 -> progressive recompilation
```

This ordering is dependency priority, not a ban on parallel evidence work.

**Current layer labels: L1 NOT COMPLETE, L2 NOT COMPLETE, L3 NOT COMPLETE.**
