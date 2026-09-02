# DMC Rengine Roadmap

**Snapshot:** 2026-09-02  
**Reviewed base:** `main@9483663959e5452f9a224c1535445bb5a3b33520`

The project roadmap is dependency-driven. The active resource-runtime program is no longer described as “finish L1, then the rest”; L1/L2/L3 have separate ownership and completion gates with a shared vertical acceptance target.

## Current GDSpaces status

```text
L1 — Resource Materialization
     INCOMPLETE / NOT 100%
     advanced product authoring capability
     original byte/result reverse + real acceptance open

L2 — Resource Resolution
     ADVANCED / INCOMPLETE
     successful-mount topology correction + trusted real selection evidence open

L3 — Original Runtime / Lifecycle
     INCOMPLETE
     R1 state-writer census bounded-closed
     R2 field/backing ownership ACTIVE
```

See [GDSpaces master roadmap](gdspaces/master-roadmap.md).

## Current critical work

### 1. L3 R2 static ownership research

Now that R1 is closed for the canonical image, the active static lifecycle frontier is:

```text
LoadedResource +0x08/+0x10/+0x18/+0x20/+0x28
 -> family/group producer/writer/owner census
 -> initialization/finalization/release ordering
 -> SCM +0x28 reconciliation
 -> R3/R4 support where ownership requires it
```

### 2. L2 successful-mount topology correction

Port the recovered distinction:

```text
filename discovery / registration attempt
!=
successful linked mount
```

into current-main product code. Resolver traversal must be based on explicit successful registrations, including sparse success.

### 3. L1 original-materialization + real acceptance

Keep product exactness stricter than original unsafe behavior while completing the reverse/evidence scope required by the claimed compatibility boundary.

Then execute:

```text
direct-retail selected member
 -> representation classification
 -> supported real edit/rebuild
 -> next-volume resolver/reopen/rematerialization
 -> original DMC3 consumer-visible effect
 -> rollback
 -> final L1 audit
```

### 4. Trusted live evidence infrastructure

Respawn current-main lifecycle validation from the useful #218 design and implement trusted process-bound publisher/origin binding. Editable JSON fields cannot self-promote.

The same trust discipline applies to original selected-provider evidence in L2.

### 5. First same-lineage vertical receipt

High-value integrated target:

```text
[L2] original selected provider/member identity
 -> [L1] exact materialized bytes/provenance
 -> authored rebuild/rematerialization
 -> [L3] original ready/use lifecycle
 -> deterministic consumer-visible effect
 -> rollback
```

This closes a declared representative acceptance scope without pretending every layer's full breadth is finished.

## Maintained foundations

- C++20/CMake and Windows + Ubuntu CI;
- exact artifact/executable identity and hash-gated reverse authority;
- ResourceId/ResourceRef/ByteProvenance/SourceRegistry;
- canonical NBZ/PAC/PNST product materialization/authoring paths;
- current DMC3 naming/type evidence separation;
- WorkingCopy and guarded publication;
- Binary Inspector byte/structure/evidence authority;
- Reverse Core / Recovered Game Source Tree boundaries;
- EXE Editor linkage to exact recovered-source/binary evidence.

## Layer-specific roadmaps

- [L1 Resource Materialization](gdspaces/l1-roadmap.md)
- [L2 Resource Resolution](gdspaces/l2-roadmap.md)
- [L3 Original Runtime/Lifecycle](gdspaces/l3-roadmap.md)
- [Layer classification](gdspaces/decompilation-layer-classification.md)

## Stage Ops and Stage Semantic Graph

Stage Ops remains a downstream product domain. It may expand in parallel when it consumes canonical GDSpaces identities/materialized resources, but it must not create a second resource resolver/materializer/lifecycle authority.

```text
GDSpaces L2/L1/L3 authority
 -> Stage Ops assembly/orchestration
 -> Stage Semantic Graph
 -> ModViz
```

## EXE Editor / recovered source

Progressive reconstruction remains evidence-driven:

- exact function/data identity;
- source-equivalent bounded C++ units;
- ABI/field ownership/lifetime reconciliation;
- isolated compilation;
- controlled original-vs-reconstruction behavioral receipts;
- progressive replacement/recompilation milestones.

Readable pseudocode or compile success is not equivalence.

## Long-term milestones

1. Current three-layer boundary/status reconciliation promoted.
2. L3 R2 field/backing ownership closed at the declared canonical scope.
3. L2 successful-mount topology corrected in product code.
4. Trusted original-process selection/lifecycle observation infrastructure operational.
5. First same-lineage L2→L1→L3 real vertical acceptance receipt.
6. Layer-specific final audits for declared scopes.
7. Stage Ops game-backed assembly over canonical resource authority.
8. Stable semantic/editor verticals.
9. First bounded recovered-subsystem behavioral equivalence receipt.
10. Progressive recompilation and working rebuilt-executable milestones.

No milestone is promoted because of a percentage or synthetic success alone. See [current status](status/current.md), [blockers](status/blockers.md) and [machine-readable status](status/canonical-status.json).
