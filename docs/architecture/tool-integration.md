# Cross-Tool Integration Architecture

## Purpose

DMC Rengine tools are cooperating consumers/projections over shared resource, evidence, recovered-runtime and Stage Ops authorities. They are not independent applications joined by file paths or duplicated scene models.

This architecture prevents:

- second resource resolvers;
- divergent `ResourceId` identities;
- tool-owned copies of original-game runtime logic;
- multiple stage/scene assembly models;
- stale WorkingCopy/parser/Binary Document lineage;
- untraceable EXE edits;
- Stage Ops / Semantic Graph / ModViz disagreement;
- executable evidence being attached to the wrong build;
- branch-scoped implementation being mistaken for whole-project completion.

See `docs/status/completion-and-evidence-policy.md`.

## Canonical authority flow

```text
canonical executable / runtime evidence
        │
        ├── Reverse Core identities / claims / reconstructions / receipts
        └── Recovered Game Source Tree original-runtime reconstruction

Mounted sources / executable Stage-resource authority
        │
        ▼
GDSpaces
  ResourceId / ResourceRef / ResourcePayload
  resolution / provenance / container expansion
        │
        ▼
ProjectWorkspace / resource sessions
        │
        ├── Binary Inspector Document / evidence links
        ├── WorkingCopy / parser byte lineage
        └── Stage materialization / StageRuntimeLoadReport
                        │
                        ▼
                     Stage Ops
               StageAssemblyWorkspace
                 / operations state
                  ┌─────┴─────┐
                  ▼           ▼
          Semantic Graph     ModViz
          derived projection editor projection
```

No consumer may skip the authority chain and independently open/resolve a game source.

## ResourceWorkspaceSession

A `ResourceWorkspaceSession` is a product working context for one canonical source resource identity.

It binds, as available:

- immutable source payload;
- active WorkingCopy and revision;
- format integration policy;
- parser result and byte-source lineage;
- Binary Inspector Document;
- evidence links;
- Stage membership/context;
- executable context;
- tool routes/capabilities;
- append-only workspace events.

### Mutable-byte lineage correction

`ResourceId` source-span identity is immutable. A WorkingCopy may legally change byte size.

Therefore an attached parser/Binary Document for dirty data must match the **active WorkingCopy revision and active byte size**, not be rejected merely because that size differs from the immutable source payload.

This rule is required for size-changing Stage TXT and other future edits.

## ProjectWorkspace

A `ProjectWorkspace` coordinates product resource sessions and shared registries/graphs. It is coordination infrastructure, not original DMC3 runtime ownership.

It may own or coordinate:

- Tool/Format Integration registries;
- Artifact/Evidence registries;
- Evidence Packet imports;
- GDSpaces ResourceGraph;
- product ProjectGraph;
- resource sessions and deterministic manifests/events.

Original executable functions/types/factories/cache/lifetime remain Recovered Game Source Tree concerns even when ProjectWorkspace links to their evidence identities.

## Tool roles

### GDSpaces

Owns product resource identity, source lookup/resolution, bytes, provenance, classification, container expansion and WorkingCopy resource boundaries.

It does not own reconstructed original DMC3 resource lifecycle/factory/cache or collision/gameplay runtime.

### Reverse Core

Owns generic reverse identities/evidence/hypotheses/experiments/task claims/reconstructions/ValidationReceipts and parallel-agent coordination.

### Recovered Game Source Tree

Owns evidence-backed reconstruction of original DMC3 runtime code. Recovered code may be compiled/tested without being declared original-source or behaviorally equivalent.

### Spider Hub

Displays/navigates shared graph relationships, tasks, evidence and tools. It does not become resource or reconstruction authority.

### Binary Inspector

Consumes supplied exact byte lineage plus stable resource/artifact identity. Owns structural/ownership/evidence inspection views and deterministic analysis representations.

### EXE Editor

Consumes executable bytes/address evidence, Reverse Core reconstruction identities and recovered-source units. It is an editing frontend, not a separate reconstruction truth store.

### Stage Ops

Owns product-side stage/scene assembly and operational state through `StageAssemblyWorkspace` and related operations/session/domain structures.

It consumes GDSpaces outputs and recovered-runtime links. It must not resolve game resources again or implement original-game runtime functions locally.

### Stage Semantic Graph

Consumes Stage Ops assembly state and emits a derived semantic/evidence graph. It does not assemble a second scene or traverse archives/resources independently.

### ModViz

Consumes the same Stage Ops assembly/scene state for 3D/asset/HUD editing. It does not maintain a parallel Stage membership/resource graph.

### Item/HUD editors

Consume shared resource/runtime/evidence contracts and create WorkingCopy changes or guarded runtime/EXE requests. They do not directly patch original files or independently resolve sources.

### Build & Test Lab

Consumes validation requests, manifests, comparison inputs, guarded patch plans and reproducibility metadata. It owns validation/receipt workflow, not reverse truth by itself.

## Tool Capability Registry

A resource/format may route to multiple tools. A route means **capability/visibility**, not resource ownership.

For example HITS may be visible in Stage Ops, Binary Inspector and ModViz while original collision ABI remains in recovered-game and resource bytes remain in GDSpaces.

## Format Integration Registry

Format descriptors may declare parser/adapters, product maturity, Stage/domain hints, evidence links, write policy and limitations.

Format maturity does not imply original-runtime equivalence. A deterministic product writer does not imply an original Capcom offline-builder reconstruction unless separately proven.

## Stage integration

The canonical Stage path is catalog/selector driven:

```text
EXE descriptor + selector authority
  -> resource_set/catalog identity
  -> GDSpaces exact resource resolution/materialization
  -> StageRuntimeLoadReport / StageBundle
  -> Stage Ops StageAssemblyWorkspace
  -> Semantic Graph / ModViz
```

Keep resource-set/catalog, numeric Stage and semantic/gameplay identities separate.

`st001` is a regression/compatibility fixture only. It is not the Stage architecture or completion gate.

## Event-driven mutations

WorkingCopy mutation is never an untracked mutable-buffer side channel.

Record enough event/revision context for:

- edit application;
- undo/reset;
- parser reanalysis;
- Binary Document refresh;
- stale derived-state invalidation;
- Stage Ops refresh;
- validation request/receipt;
- guarded export/reintegration.

Consumers must be able to detect when a projection/parser result belongs to an older WorkingCopy revision.

## Evidence boundaries

A generic PE/resource format does not inherit DMC3 evidence just because its bytes look compatible.

Evidence attaches through exact artifact/build/resource identities and location records. Build-specific addresses remain hash-gated.

If a pass did not actually mount the raw artifact, project evidence may be reused but must not be described as a fresh independent byte-level measurement.

## Completion boundary

Cross-tool integration can have implemented/tested flows while DMC3 runtime equivalence remains incomplete.

Do not infer major-subsystem `COMPLETE` from:

- shared workspace existence;
- green CI;
- Stage Ops assembly success;
- compiled recovered targets;
- parser/writer round trips;
- bounded ABI closure.

Whole-subsystem completion requires the applicable evidence/corpus/runtime/lifecycle/ValidationReceipt gate.

## Forbidden patterns

- editor opens a local source path independently;
- parser mounts/discovers sources;
- Binary Inspector resolves archives;
- Stage Ops searches for Stage PAC files;
- Semantic Graph discovers scene membership independently;
- ModViz stores a parallel Stage/resource model;
- original-game recovered functions are placed into GDSpaces/Stage Ops/ModViz;
- EXE Editor stores a disconnected second reconstruction truth;
- display name becomes canonical identity;
- WorkingCopy edit is not revision/event tracked;
- green CI is cited as original-game equivalence;
- `st001` is used as the canonical Stage universe;
- a bounded reverse closure is advertised as complete subsystem reconstruction;
- PAC Editor/PAC Manager is reintroduced as top-level architecture.
