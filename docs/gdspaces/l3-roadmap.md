# GDSpaces Layer 3 Roadmap

**Status:** R1 STATIC BOUNDED-CLOSED / R2 ACTIVE / L3 INCOMPLETE  
**Snapshot:** 2026-09-02  
**Reviewed base:** `main@9483663959e5452f9a224c1535445bb5a3b33520`  
**R1 current-main reconciliation:** `l3-r1-current-main-reconciliation-2026-09-02.md`

Layer 3 owns **Original Runtime / Resource Lifecycle** after resource selection/materialization reaches runtime ownership. It does not own logical resolver authority, byte-authoring authority or downstream Stage Ops domain semantics.

## Canonical boundary

```text
L2 exact selected identity
 -> L1 exact materialized byte/result identity
 -> [L3]
    request / scheduler / callback lifetime
    LoadedResource state publication
    typed post-load
    optional ready callback
    state-3 consumer visibility
    claims / reuse / family ownership
    cancellation / replacement
    deferred cleanup
    owner/group/full release and reset
    runtime / CRT / process-lifetime teardown
```

L3 is semantic and lifetime-scoped, not one executable address interval.

## Core static authority

```text
registry base   0x140C99D30
record count    363
record stride   0x48
groups          [4,136,60,28,1,128,6]
```

Canonical state spine:

```text
0x1401B84E0  state 0 -> 1 after bounded acquisition/materialization success
0x1401B8DC0  normal completion state 1 -> 2
0x1401B92D0  typed post-load -> optional callback -> state 2 -> 3
0x1401B8430  cancellation state 1|2 -> 4
0x1401B8F00  deferred state 4 -> 0 cleanup
0x1401B9530  ordinary owner release
0x1401B9560  group reset
0x1401B95E0  full registry reset
```

## Work-package status

### R1 — LoadedResource state-writer/caller census

**STATIC BOUNDED-CLOSED / CONTRADICTION-GATED**

The current-main semantic port of historical #240 closes broad `LoadedResource +0x04` writer discovery for the canonical image. Newer merged type/family research was checked and supplies no contradictory state writer.

Reopen R1 only with concrete exact record provenance.

### R2 — field/backing ownership census

**ACTIVE / P0 STATIC REVERSE**

Primary target fields:

```text
+0x08  family-specific selector/index metadata
+0x10  callback/context path where applicable
+0x18  descriptor/type authority
+0x20  loaded payload
+0x28  owned/backing subobject region
```

R2 must determine, by family/group/caller provenance:

- producer and writer set for each stable field;
- ownership vs borrow/alias semantics;
- initialization order relative to state1;
- mutation/finalization order around state2/state3;
- release/reset ordering and failure conditions;
- fixed-family vs dynamic group-5 differences;
- whether `+0x28` always denotes the same backing concept or is family-specialized;
- cross-links to external factories/dependencies without assigning their whole subsystem to L3.

The SCM `mesh +0x28` contradiction remains an explicit R2/R3 reconciliation target.

### R3 — typed post-load / factory / dependency breadth

**PARTIAL / MATERIAL ADVANCE FROM 31.08–01.09 RESEARCH**

Current type evidence must remain split:

```text
0x1402DB1F0  registry three-byte content probe
0x1401B9FA0  container post-load dispatcher
0x1402FD650  four-byte family-mask classifier
```

Central normal handlers established:

```text
MOD -> 0x1402FE3B0
EFM -> 0x1402F7A90
SCM -> 0x1403051B0
SHW -> 0x1403204C0
```

Important current boundaries:

- MOD/EFM/SCM are related mesh-bearing model-document families but not field-identical;
- SHW is a distinct self-contained shadow-hull mesh resource;
- MRP has byte-backed runtime identity but no normal central handler established;
- MCV family recognition is a separate four-byte classifier result;
- EFW/EFE are dispatcher sentinels without proven normal handlers;
- runtime identity does not imply parser/writer maturity or universal geometry behavior.

Remaining R3 work includes external factory/dependency failure paths, handler breadth, dependency ownership and family-specific ready semantics.

### R4 — shared owner / claims / reuse breadth

**PARTIAL**

Strong bounded loader-node anchors:

```text
0x1401AE220 claim++
0x1401AF6A0 claim--
0x1401AF6F0 zero-claim sweep/release
```

Do not convert this into a universal `LoadedResource.refCount`. R4 must reconcile fixed groups, dynamic group 5, loader nodes and specialized family managers separately.

### R5 — dynamic lifecycle timing / transitions / teardown

**OPEN**

Static transition shape is strong, but dynamic current-slot cancellation/concurrency, transition/reload/reset/shutdown timing and cross-build/profile behavior require original-process evidence.

## L1/L3 terminal-completion seam

Semantic ownership remains:

```text
[L1]
byte execution / exact byte-result semantics
 -> terminal materializer result

[L3]
scheduler/request lifetime
 -> normal completion callback
 -> state 1 -> 2
```

Stronger #258/#269 evidence reports for admitted type-2 work:

```text
status 2 -> pending / remains current
status 4 -> retry / remains current
status 3 -> retire / FIFO advances
```

Only after retirement may a later admitted type-3 normal callback become current. This strengthens the static seam but does not close dynamic cancellation/concurrency. Product byte exactness also remains stricter than any original short-success behavior.

## Lifecycle validation implementation

Open #218 contains a useful fail-closed validator design but is old-base and absent from current main. It should be semantically respawned rather than mechanically merged.

Required current-main implementation slices:

1. `Dmc3LoadedResourceContract` static/profile contract;
2. current-main `L3LifecycleTrace` schema/validator;
3. strict L1/L2 identity binding by exact resource/provenance identity;
4. trusted process-bound publisher/origin binder;
5. zero-loss/event-overflow diagnostics;
6. rollback binding;
7. aggregate breadth validation.

Editable trace metadata must never self-declare promotion eligibility.

## Dynamic acceptance order

Capture in this order:

```text
V1 initial load
 -> V5 in-flight cancellation
 -> V2 room/stage transition
 -> V3 restart/reload
 -> V4 return-to-menu/full reset
 -> V6 shutdown
 -> V7 family/build breadth
 -> final contradiction-free L3 audit
```

V1 establishes the normal load/ready spine. V5 is intentionally early because cancellation/concurrency remains the weakest important boundary after the normal terminal-completion seam was narrowed.

## Current execution order

1. R1 semantic port/current-main closure — **done on this reconciliation branch**;
2. synchronize canonical layer/status docs — active in the same branch;
3. begin R2 family-complete field/backing ownership census;
4. use R3/R4 only where needed to resolve R2 ownership ambiguity;
5. respawn #218 validator on current main;
6. implement trusted process-bound lifecycle publisher/binder;
7. capture V1 and V5;
8. continue R3/R4 breadth and V2/V3/V4/V6/V7;
9. run final L3 audit.

## Completion rule

L3 is complete only when static reverse and trusted original-process receipts jointly prove the declared lifecycle scope across representative families/builds. Synthetic traces, manually edited JSON, Stage Ops success, preview success or crash-free launch are not promotion evidence by themselves.
