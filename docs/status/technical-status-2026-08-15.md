# DMC Rengine — Technical Status — 2026-08-15

## Scope

This status records the current architecture and active implementation stack. It deliberately distinguishes merged `main` truth from active stacked PR implementation and Drive research evidence.

## Architecture

```text
canonical dmc3.exe
  ├─> Reverse Core evidence / claims / reconstruction identities
  ├─> Recovered Game Source Tree — reconstructed original runtime behavior
  └─> executable Stage descriptor / selector authority
          -> GDSpaces
          -> Stage Ops / StageAssemblyWorkspace
          -> Stage Semantic Graph
          -> ModViz
```

Binary Inspector and EXE Editor are evidence/reconstruction frontends over the same identities. They do not own independent reverse truth.

## GDSpaces

GDSpaces remains the only product-side resource authority.

Current executable-backed Stage/resource model:

- 189 descriptors total;
- Bank A = 110;
- Bank B = 79;
- separate 193-entry selector space;
- separate 10-pointer group-base table;
- four-role descriptor planning;
- provenance across bank/row/global-row;
- recursive materialization/expansion with partial-result preservation.

Independent identity axes:

```text
resource_set_id / catalog_entry_id
numeric_stage_id
semantic_stage_id
```

`st001` is a regression fixture only.

The main unresolved GDSpaces/recovered-runtime boundary is original state-2 -> typed post-load/factory/cache/ownership -> state-3 readiness and teardown. Materialized bytes are not automatically a game-ready original runtime object.

## Recovered Game Source Tree / EXE reconstruction

Recovered original DMC3 runtime behavior belongs in `recovered-game`, not in GDSpaces, Stage Ops, ModViz, Binary Inspector, EXE Editor, or Reverse Core.

Required promotion chain:

```text
artifact/range/function/type identity
  -> evidence
  -> reconstruction
  -> recovered C++
  -> isolated compile
  -> behavioral comparison
  -> ValidationReceipt
  -> promote / correct / reject
```

Recovered C++ is an evidence-backed executable specification, never represented as original Capcom source.

## Reverse Core

Reverse Core remains generic infrastructure around:

`BinaryArtifact`, `AddressRange`, `Function`, `DataObject`, `RecoveredType`, `EvidenceRecord`, `Hypothesis`, `Experiment`, `TaskClaim`, `Reconstruction`, `ValidationReceipt`, `Subsystem`.

TaskClaims coordinate canonical mutation. They do not create evidence or semantic game ownership.

Acceptance gate: one bounded DMC3 subsystem must complete the full binary -> recovered C++ -> build -> behavioral comparison -> ValidationReceipt loop.

## Stage Ops

PR #91 already implements the central Stage Ops authority model on its active stack:

- `StageAssemblyWorkspace`;
- `StageOpsIngress` from `StageRuntimeLoadReport` without second resource resolution;
- `StageOperationsSession`;
- shared WorkingCopy/event coordination;
- retained typed parser results and byte lineage;
- Stage domain workspace/source spans/relations;
- explicit recovered-runtime links and validation;
- Stage scene snapshots;
- deterministic Semantic Graph projection;
- ModViz Stage Ops projection/edit plumbing;
- invalidation and validation-receipt plumbing.

Therefore Phase A is no longer accurately described as “not implemented”. The remaining work is representative game validation, domain breadth, recovered lifecycle linkage, compatibility-path migration, and promotion of the stacked integration spine.

A size-changing Stage TXT WorkingCopy regression was corrected during this reconciliation: immutable `ResourceId` source-span size is no longer required to equal active mutable working bytes when constructing the Binary Inspector view. A regression test now covers this boundary.

## Stage Semantic Graph

The graph is a deterministic representation/index over Stage Ops state. It does not resolve resources, traverse archives, own WorkingCopy bytes, manufacture runtime objects, or keep a second scene model.

It preserves separate structural/product facts, recovered-runtime facts, confirmed semantics, inferred semantics, and unresolved relationships.

## ModViz

ModViz consumes Stage Ops state plus semantic/runtime evidence. It does not independently resolve GDSpaces resources or originate Stage assembly.

The active Menu Editor vertical slice remains the Red Orb HUD counter: resource identity -> HUD hierarchy/digit meshes/UV/layout -> WorkingCopy -> EXE/reverse constraints -> guarded modification -> preview -> validation -> export.

## HITS / collision reverse

Strong implemented/reconstructed assets include the deterministic parser/writer, `0x38` records, spatial-table reconstruction, exact inclusive 13-axis triangle-box SAT, deterministic topology rebuild, and source0/source1 separation.

Current P0 reverse targets:

- `0x14005E7A0` — central candidate arbitration/result ABI;
- `0x14005B460` — dynamic category candidate path;
- `0x14005FEC0` — source-1 output ABI;
- `0x1400601E0` — in/out structure and accumulation.

Collision source2 backing/lifetime remains unknown.

## Canonical artifact correction

The canonical executable SHA-256 `e454272ed0fb0247fcbcf300e5d55d7a3e96d50b89b9ffaff81bb978dcbdd082` is paired with size **6,356,432 bytes**. The stale 3,735,552-byte registry tuple is superseded and the authority manifest/test now encode the corrected size.

## Current blockers

1. close the original resource readiness/lifetime path;
2. close collision arbitration around `0x14005E7A0` and related functions;
3. complete one recovered-game compile + behavioral ValidationReceipt;
4. finish shared Reverse Core TaskClaim/reconstruction infrastructure;
5. validate representative Bank-A, Bank-B, shared/selector/fallback/partial Stage cases;
6. resolve SCM `mesh+0x28` before post-load promotion;
7. recover collision source2 backing/lifetime;
8. promote the composed integration stack with whole-stack Windows/Ubuntu CI;
9. no behaviorally equivalent recompiled DMC3 executable is yet claimed.

## Critical path

```text
raw high-value reverse evidence
  + first recovered compile/behavior proof
  -> resource lifecycle closure
  -> representative StageAssemblyWorkspace validation
  -> Semantic Graph + ModViz over the same Stage Ops authority
  -> Reverse Core validated subsystem receipt
  -> broader reconstruction / controlled recompilation
```
