# Cross-Tool Integration Architecture

## Purpose

DMC Rengine tools are cooperating views over shared resource, workspace, evidence and recovered-source authorities. They are not independent applications joined by file paths.

The architecture prevents duplicate resolvers/materializers, divergent resource identities, editor-owned archive trees, hidden mutable buffers, Stage Ops/ModViz scene divergence and evidence attached to the wrong executable artifact.

## Canonical product flow

```text
GDSpaces sources / resolver / materialization / provenance
        |
        v
ResourceId + ResourcePayload + ResourceGraph
        |
        v
ProjectWorkspace / ResourceWorkspaceSession
        |
        +--> canonical parser/typed result
        +--> Binary Inspector document
        +--> Evidence / Reverse Core links
        +--> WorkingCopy + revision/event history
        +--> Stage Ops ingress
                  |
                  v
        StageAssemblyWorkspace / operations state
                  |
                  +--> Stage Semantic Graph
                  +--> ModViz/editor projections
```

No downstream tool may skip GDSpaces and open/resolve game sources independently.

## Reverse/executable flow

```text
exact executable artifact
 -> Evidence / Reverse Core identities
 -> Recovered Game Source Tree
 -> EXE Editor navigation/editing frontend
 -> validated behavior links consumed by product/domain layers
```

Recovered original-game code does not move into GDSpaces or Stage Ops merely because those layers consume its contracts.

## ResourceWorkspaceSession

A session binds one canonical `ResourceId` to:

- immutable source/materialized payload;
- ByteProvenance;
- format capability and canonical parser result;
- diagnostics;
- evidence/reconstruction links;
- optional stage membership;
- optional executable context;
- optional revisioned WorkingCopy;
- append-only events/validation state.

Source bytes remain immutable. Authored bytes receive their own output receipt and do not inherit source provenance.

## Tool roles

### GDSpaces — The Archive

Owns source mounting, request resolution, resource identity, materialization, provenance, container expansion, ResourceGraph, WorkingCopy handoff and bounded authoring/publication contracts.

For Layer 1, generated outputs also obey artifact-stability and atomic/no-replace publication requirements from the [L1 roadmap](../gdspaces/l1-roadmap.md).

### Reverse Core

Owns generic artifact/range/function/data/type/evidence/claim/reconstruction/validation identities.

### Recovered Game Source Tree

Owns reconstructed original DMC3 runtime code, ABI, ownership and lifetime behavior.

### EXE Editor — The Scriptorium

Frontend over executable mappings, recovered-source identities, evidence and guarded patch/rebuild requests. It does not create an independent truth store for recovered game code.

### Binary Inspector — The Reliquary

Consumes supplied bytes and canonical revision lineage. Owns byte regions, fields, ownership, unknown gaps, annotations and deterministic structural views. It is not a resource resolver.

### Stage Ops — The Theatre

Owns product-side stage/scene assembly and operational workspace state over already resolved/materialized GDSpaces resources. It does not search for PAC/NBZ files or create private format loaders.

### Stage Semantic Graph

Derived evidence-aware representation/index of Stage Ops state. It does not load resources or orchestrate the scene.

### ModViz — The Observatory

Consumes Stage Ops/Semantic Graph projections and sends revision-guarded edits through shared WorkingCopy/Stage Ops contracts. It must not own a second scene-membership discovery path.

### Spider Hub — The Nexus

Navigation/index UI over shared project/resource/evidence relationships. It does not become an alternate resolver or evidence authority.

### Build & Test Lab — The Trial Chamber

Owns reproducibility, generated output validation, original-vs-reconstruction/game-backed receipts and rollback/test metadata.

## Format/write capability policy

Format capability is explicit and may vary by representation/domain:

- `read-only`;
- `working-copy-only`;
- bounded `guarded-export` / authoring.

PAC/PNST/NBZ are **not globally read-only anymore**: current code contains bounded writer/reflow/repack/overlay paths at specific evidenced scopes. A writer capability must therefore be declared per supported representation and acceptance boundary, not by a blanket archive-family label.

A binary AFS backend is not currently promoted from `.afs/` logical namespaces. Historical PACK product parsing is not original DMC3 runtime writer/read authority.

## Event and revision rules

Mutations are not anonymous buffer writes. Workspace events preserve resource identity, revision, producer/consumer, operation, validation and stale-derived-state information.

Stage/ModViz views must reject stale revision edits rather than overwriting newer workspace state.

## Evidence boundaries

A format or executable family name does not grant evidence authority. Evidence is attached through exact artifact identity and bounded locations/claims.

For GDS provenance-grade archive acquisition, path equality alone is insufficient: archive identity, selected member and materialized bytes must be bound to one stable artifact observation.

## Stage identity

Preserve separately:

- catalog/resource-set descriptor identity;
- numeric Stage selector identity;
- semantic gameplay Stage/room identity when separately evidenced.

`st001` remains a regression fixture only.

## Forbidden patterns

- editor/tool opens a game source independently;
- parser mounts/resolves a source;
- Binary Inspector reopens an archive;
- Stage Ops searches directories/PAC files;
- ModViz builds a parallel resource or scene graph;
- EXE Editor invents recovered-source authority from UI state;
- original runtime code is moved into GDSpaces for convenience;
- display name is treated as canonical resource identity;
- authored output inherits original source provenance;
- `exists() -> ofstream` is described as atomic/no-clobber publication;
- evidence acquisition writes into the measured retail tree;
- a `GData*.afs/...` member winner is hard-coded instead of recorded from canonical resolver selection;
- binary AFS or original PACK authority is inferred without direct evidence;
- legacy PAC Editor/PAC Manager is revived as top-level architecture.
