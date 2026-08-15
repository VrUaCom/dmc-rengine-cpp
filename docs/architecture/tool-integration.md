# Cross-Tool Integration Architecture

## Purpose

DMC Rengine tools cooperate over shared resource, stage, mutation and evidence authorities. They are not independent applications joined by file paths.

The integration architecture prevents:

- duplicate resolvers;
- divergent resource identities;
- editor-owned copies of container structure;
- hidden mutable byte buffers;
- untraceable EXE patches;
- competing Stage Ops / ModViz / Semantic Graph scene assemblies;
- evidence claims attached to the wrong executable build;
- product-side scene state being confused with recovered vanilla runtime state.

Architecture ownership for stage/scene work is fixed by `ADR-0003-stageops-scene-authority.md`.

## Canonical resource flow

```text
Source mounting / container expansion
        |
        v
GDSpaces ResourceRef + ResourcePayload + ByteProvenance
        |
        v
ProjectWorkspace
        |
        +--> ResourceWorkspaceSession
        |      +--> FormatIntegrationDescriptor
        |      +--> parser diagnostics
        |      +--> Binary Inspector Document
        |      +--> ExecutableResourceContext
        |      +--> Evidence links
        |      +--> StageResourceContext
        |      +--> WorkingCopy
        |      +--> append-only WorkspaceEventJournal
        |
        +--> GDSpaces ResourceGraph
        +--> Spider Hub ProjectGraph
        +--> Resource / Stage / EXE / Graph manifests
```

No tool may skip this flow and open or mutate a source independently.

## Canonical stage/scene flow

```text
EXE Stage descriptor / selector authority
        |
        v
GDSpaces resolution/materialization/expansion
        |
        v
profile Stage Ops adapter
        |
        v
StageAssemblyWorkspace  <---->  ProjectWorkspace resource sessions
        |
        v
Stage Ops domain assembly + operations
        |
        +--> StageWorkspaceView (UI projection)
        +--> Stage Semantic Graph (semantic/evidence projection)
        +--> ModViz (3D editor projection + edit commands)
        +--> Binary Inspector / EXE Editor / Reverse Core links
        +--> validation / rebuild / export requests
```

The stage aggregate is owned by Stage Ops. Consumers do not rediscover scene membership from lower-level project sessions.

## ResourceWorkspaceSession

A `ResourceWorkspaceSession` is the canonical working context for one `ResourceId`.

It binds:

- immutable source payload;
- format maturity and write policy;
- parser execution and diagnostics;
- shared structural interpretation;
- evidence links;
- optional stage membership;
- optional executable analysis;
- optional revisioned WorkingCopy;
- append-only events.

The source payload remains immutable for the life of the session.

## ProjectWorkspace

A `ProjectWorkspace` coordinates resource sessions across tools.

It owns:

- Tool Registry;
- Format Integration Registry;
- Artifact Registry;
- Evidence Registry;
- Evidence Packet Registry;
- GDSpaces ResourceGraph;
- Spider Hub ProjectGraph;
- all active resource sessions;
- per-resource WorkingCopy/event mutation substrate.

`ProjectWorkspace` is **not** the stage/scene aggregate. Stage Ops builds the stage-scoped operational model over these resource sessions.

Every byte mutation remains applied through ProjectWorkspace so resource graphs, events and manifests stay synchronized.

## StageAssemblyWorkspace

`StageAssemblyWorkspace` is the canonical product-side aggregate for one selected stage/resource-set.

It preserves:

- technical resource-set/catalog identity;
- exact source catalog coordinates;
- optional numeric selector-facing Stage identity;
- optional semantic gameplay identity;
- every required root, including unresolved roots;
- unique canonical resources;
- ByteProvenance for materialized resources;
- direct/nested/shared membership relationships;
- product materialization gate;
- separate recovered-game readiness gate;
- diagnostics.

Later Stage Ops domain layers attach camera, collision, geometry, StageSet/script, door, event, enemy, effect, audio and other scene meaning without changing the underlying ResourceIds.

## Tool roles

### GDSpaces — The Archive

Owns source mounting, canonical resource identity, lookup, bytes, ByteProvenance, container expansion, classification, ResourceGraph and source/write-policy boundaries.

It does not own the complete stage scene.

### Spider Hub — The Nexus

Consumes ProjectGraph and higher-level Stage/Semantic relationships. It displays verified relationships but does not resolve or mutate resources independently.

### Binary Inspector — The Reliquary

Consumes canonical resource identity and supplied bytes. It owns structural regions, fields, ownership maps, unknown gaps, annotations and deterministic binary manifests.

It is not a scene assembler.

### EXE Editor — The Scriptorium

Consumes PE resources through GDSpaces. It owns executable analysis, recovered source views, RVA/VA evidence, target identification and guarded patch plans.

Recovered original-game implementation belongs to the Recovered Game Source Tree rather than being owned by the editor UI.

### Evidence Registry

Owns claims, confidence, artifact identity, Evidence Packets, corrections and provenance.

### Stage Ops — The Theatre

Stage Ops is the **stage/scene assembly and operational orchestration authority**.

It consumes existing GDSpaces, ProjectWorkspace, parser and recovered-runtime authorities and owns:

- `StageAssemblyWorkspace`;
- stage-scoped domain assembly;
- partial/complete scene state;
- dependency invalidation;
- coordination of multi-resource edits;
- reload/rebuild/validation operations;
- stable scene state published to consumers.

Stage Ops never becomes a second resource resolver and never claims vanilla runtime behavior that is not recovered.

### Stage Semantic Graph — The Map

Stage Semantic Graph is a deterministic semantic/evidence representation of Stage Ops state.

It indexes relationships, confidence, evidence, unknowns and recovered-runtime links. It does not load, resolve, mutate or assemble the stage.

### ModViz — The Observatory / Scene Editor

ModViz is a 3D scene and asset editor over Stage Ops state.

It consumes StageAssemblyWorkspace/domain projections plus Semantic Graph relationships. It does **not** independently discover stage membership from ProjectWorkspace.

ModViz edits are submitted back through Stage Ops and the shared ProjectWorkspace WorkingCopy/event path.

### Item Editor — The Forge

Consumes ITM resources and runtime evidence through shared contracts. It may create WorkingCopy changes and guarded EXE patch requests, but not bypass GDSpaces or Patch Engine policy.

### Build & Test Lab — The Trial Chamber

Consumes validation requests, manifests, patch plans, test fixtures and reproducibility metadata.

## Recovered Game Source Tree boundary

Original DMC3 functions/types for resource loading, typed post-load, factory dispatch, caches, ownership, stage transition and teardown live under `recovered-game`.

Stage Ops may consume their evidence-backed contracts/results but may not reimplement approximations merely to make product assembly look complete.

The two readiness gates remain explicit:

```text
product materialization complete
        !=
original game ready equivalent
```

Only recovered-game evidence may close the second gate.

## Tool Capability Registry

A format may route to several tools simultaneously:

- one primary editing/operations tool;
- companion inspection tools;
- Evidence Registry;
- Build & Test validation.

Example for HITS:

```text
Stage membership/operations: Stage Ops
3D editing: ModViz
Binary structure: Binary Inspector
Resource authority: GDSpaces
Evidence: Evidence Registry
Validation: Build & Test Lab
```

A route is a capability relationship, not permission to open the source independently.

## Format Integration Registry

Every known format declares:

- maturity;
- parser ID;
- Binary Inspector adapter availability;
- stage category;
- evidence claims;
- write policy;
- limitations.

Write policies:

- `read-only`;
- `working-copy-only`;
- `guarded-export`.

PAC, PNST, AFS and NBZ remain read-only until exact evidence-backed write implementations exist.

## Event-driven mutations

WorkingCopy mutations are not exposed as generic mutable buffers.

The resource workspace records:

- parser completion;
- parser diagnostics;
- Binary Document attachment;
- executable context attachment;
- evidence linking;
- stage attachment;
- WorkingCopy creation;
- edit application;
- undo/reset;
- validation requests;
- manifest exports.

Stage Ops adds stage-scoped orchestration around those events: a resource edit may mark dependent Stage Ops domain state stale and trigger graph/view refresh, but the underlying byte edit still flows through ProjectWorkspace.

## Evidence boundaries

A generic PE resource does not inherit DMC3 evidence because its format is PE.

Executable evidence attaches through:

```text
resource SHA-256
  -> ArtifactRegistry match
  -> EvidenceRecord locations referencing that artifact
  -> ResourceWorkspaceSession
  -> Stage Ops / Semantic Graph references by stable identity
```

This prevents findings from one executable build being treated as facts about another build.

## Stage integration migration

The legacy `DMC3StageWorkspaceBuilder` path creates ProjectWorkspace sessions and a StageBundle from already-read payloads. The newer runtime-materialization path produces `StageRuntimeLoadReport` and exact recursive expansion state.

These are not allowed to remain two competing scene architectures.

Migration direction:

1. GDSpaces/runtime resolver produces exact resource requests and materialized payloads;
2. DMC3 profile adapter creates the generic StageAssemblyWorkspace without another lookup;
3. Stage Ops binds/coordinates the aggregate with ProjectWorkspace resource sessions;
4. parsers/evidence/domain assemblers enrich Stage Ops state;
5. StageWorkspaceView, Semantic Graph and ModViz become projections/consumers of that single state;
6. legacy project-only stage views are retired after compatibility migration.

## Forbidden patterns

- editor opens local path directly;
- format parser mounts a source;
- Binary Inspector resolves a container;
- Stage Ops implements a second GDSpaces resolver;
- Stage Semantic Graph searches for missing stage resources;
- ModViz discovers/assembles scene membership independently from Stage Ops;
- ModViz stores a parallel resource graph as scene authority;
- Stage Ops copies original-game factory/cache/lifetime functions out of `recovered-game`;
- StageBundle is called the complete/original runtime scene;
- product materialization is called game-ready equivalence without evidence;
- Item Editor writes original bytes;
- EXE Editor attaches DMC3 evidence to an unrecognized binary;
- UI treats display name as resource identity;
- WorkingCopy mutation without an event;
- new PAC Editor/PAC Manager as a top-level architecture.
