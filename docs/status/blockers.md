# Current Blockers

**Snapshot:** 2026-09-02  
**Reviewed base:** `main@9483663959e5452f9a224c1535445bb5a3b33520`

Canonical execution order is defined by the [master roadmap](../gdspaces/master-roadmap.md) and the per-layer roadmaps.

## L1 blockers — Resource Materialization

**Layer status:** INCOMPLETE / NOT 100%.

### B-L1-01 — original byte/result reverse breadth

**Status:** OPEN / EVIDENCE-DRIVEN

Current product authoring capability is advanced, but original materialization semantics are not exhaustively closed. Relevant remaining scope includes the failure/width/queue-result semantics exposed by newer reverse work, recursive `.lst` cycle/depth/allocation/free/error behavior where required, and a final original-L1 contradiction sweep.

Do not treat one upstream boolean as proof of exact completion of all child/materialization work.

### B-L1-02 — direct-retail selected-member provenance

**Status:** REAL RETAIL RECEIPT REQUIRED

Start from a real game request and preserve the actual resolver-selected provider/volume/member identity, exact archive/member metadata and materialized byte provenance. Do not predeclare a `GData*.afs/...` path as authority.

### B-L1-03 — exact retail representation classification

**Status:** REAL RECEIPT REQUIRED

Classify the exact selected bytes before selecting a writer. Historical/transformed corpus identity does not substitute for direct-retail representation authority.

### B-L1-04 — real edit/rebuild/rematerialization

**Status:** PRODUCT CAPABILITY PRESENT / REAL RECEIPT OPEN

Require one supported bounded edit, bottom-up rebuild where nested, next-volume publication, exact resolver winner and exact rematerialized authored bytes.

### B-L1-05 — original DMC3 consumption + rollback

**Status:** OPEN / FINAL GAME-BACKED ACCEPTANCE

Tracking: #209. A crash-free launch is insufficient; the result must be consumer-visible, attributable to the authored resource and rolled back without retail mutation.

### B-L1-06 — final Layer-1 audit

**Status:** OPEN

Requires the activated original reverse scope, real lineage, representation, rebuild/rematerialization, game consumption, rollback and exact-head validation to agree.

## L2 blockers — Resource Resolution

**Layer status:** ADVANCED / INCOMPLETE.

### B-L2-01 — discovery vs successful mount topology product correction

**Status:** STRONGER RAW EVIDENCE / CURRENT-MAIN CODE GAP

Current `VolumeBootstrapPlan` still conflates discovered pre-gap archives with registered archives. Stronger #246 evidence proves registration attempt != successful linked mount and allows sparse successful topology.

Required product correction:

```text
discovery/attempt plan
 -> explicit registration outcomes
 -> successful mount topology
 -> resolver traversal only over successful mounts
```

### B-L2-02 — real-retail `0x0E` collision census

**Status:** EXTERNAL ARTIFACT EVIDENCE REQUIRED

Need an exact cryptographically bound retail central-directory/member-name surface before claiming retail normalized-key collision freedom.

### B-L2-03 — real protected-process runtime mapping receipt

**Status:** TOOLING MERGED / REAL RECEIPT REQUIRED

#219 tooling exists. A real bounded packet for the protected process is still required before canonical analysis RVAs may be used as live observation anchors.

### B-L2-04 — trusted original-process selected identity

**Status:** CANDIDATE/BINDER TOOLING MERGED / TRUSTED ORIGIN MISSING

Merged #221 cannot make self-authored JSON original-process evidence. Promotion requires same-process mapped observation, zero-loss trace, observer artifact binding, exact mounted archive binding and trusted publisher/origin binding.

Provider/backend failure after a normalized lookup hit must remain distinct from a clean miss.

### B-L2-05 — final Layer-2 audit

**Status:** OPEN

Requires topology correction, retail collision evidence, real mapping, trusted selected identity, exact-head validation and contradiction-free docs/code.

## L3 blockers — Original Runtime / Lifecycle

**Layer status:** R1 CLOSED / R2 ACTIVE / L3 INCOMPLETE.

### B-L3-01 — R2 field/backing ownership

**Status:** ACTIVE P0 STATIC REVERSE

Complete family/group ownership of:

```text
record +0x08
record +0x10 where applicable
record +0x18
record +0x20
record +0x28
stable adjacent fields
```

Need producer/writer/owner/borrow/release ordering, fixed-family vs group-5 distinctions and SCM `mesh +0x28` reconciliation.

### B-L3-02 — R3 typed/factory/dependency breadth

**Status:** PARTIAL

The merged runtime type systems are now explicitly split and MOD/EFM/SCM/SHW semantics are stronger, but external factories/dependencies/failure behavior and family-specific readiness remain incomplete.

### B-L3-03 — R4 shared-owner breadth

**Status:** PARTIAL

Loader-node claims are bounded, but there is no universal `LoadedResource.refCount` proof. Fixed groups, dynamic group 5 and specialized managers require separate ownership reconciliation.

### B-L3-04 — current-main lifecycle validator

**Status:** NOT IN MAIN

Old #218 provides a useful fail-closed design but must be semantically respawned from current main rather than mechanically merged.

### B-L3-05 — trusted process-bound lifecycle publisher/binder

**Status:** NOT IMPLEMENTED

Editable trace fields cannot self-declare trusted origin. Need a process-instance-bound publisher/origin binder with zero-loss/overflow semantics and exact L1/L2 identity binding.

### B-L3-06 — original-process lifecycle receipts

**Status:** OPEN

Order:

```text
V1 initial load
 -> V5 in-flight cancellation
 -> V2 transition
 -> V3 restart/reload
 -> V4 full reset/menu
 -> V6 shutdown
 -> V7 family/build breadth
```

### B-L3-07 — final Layer-3 audit

**Status:** OPEN

R1 is not a blocker anymore unless contradicted. Final L3 acceptance still requires R2–R5 as declared, trusted receipts and a contradiction-free audit.

## Cross-layer blocker — same-lineage vertical proof

No single trusted same-resource chain currently proves:

```text
[L2] original selected identity
 -> [L1] exact materialized bytes/provenance
 -> authored rebuild/rematerialization
 -> [L3] original ready/use lifecycle
 -> deterministic consumer-visible effect
 -> rollback
```

This is the highest-value integrated acceptance artifact once the required retail/protected-process access is available.

## Frozen non-blockers unless new evidence activates them

- binary AFS backend from `.afs/` namespace strings;
- original PACK runtime authority from historical product parsers;
- Capcom offline writer equivalence as a requirement for DMC Rengine product authoring;
- Stage Ops/ModViz success as a substitute for L1/L2/L3 evidence;
- reopening L3-R1 broad state-writer discovery without exact contradictory LoadedResource provenance.
