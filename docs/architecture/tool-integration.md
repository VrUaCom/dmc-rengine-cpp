# Cross-Tool Integration Architecture

## Purpose

DMC Rengine tools are cooperating views over one canonical resource and evidence model. They are not independent applications joined by file paths.

The integration architecture exists to prevent:

- duplicate resolvers;
- divergent resource identities;
- editor-owned copies of container structure;
- hidden mutable byte buffers;
- untraceable EXE patches;
- Stage Ops and ModViz disagreeing about the same stage;
- evidence claims being attached to the wrong executable build.

## Canonical flow

```text
Source mounting / container expansion
        |
        v
GDSpaces ResourceRef + ResourcePayload
        |
        v
ProjectWorkspace
        |
        +--> ResourceWorkspaceSession
        |      +--> FormatIntegrationDescriptor
        |      +--> Tool routes and capabilities
        |      +--> parser diagnostics
        |      +--> Binary Inspector Document
        |      +--> ExecutableResourceContext
        |      +--> Evidence record links
        |      +--> StageResourceContext
        |      +--> WorkingCopy
        |      +--> append-only WorkspaceEventJournal
        |
        +--> GDSpaces ResourceGraph
        +--> Spider Hub ProjectGraph
        +--> Resource / Stage / EXE / Graph manifests
```

No tool may skip this flow and open a source independently.

## ResourceWorkspaceSession

A `ResourceWorkspaceSession` is the canonical working context for one `ResourceId`.

It binds:

- immutable source payload;
- format maturity and write policy;
- primary and companion tool routes;
- parser execution and diagnostics;
- shared structural interpretation;
- evidence links;
- optional stage membership;
- optional executable analysis;
- optional revisioned WorkingCopy;
- append-only events.

The source payload remains immutable for the life of the session.

## ProjectWorkspace

A `ProjectWorkspace` coordinates many resource sessions.

It owns:

- Tool Registry;
- Format Integration Registry;
- Artifact Registry;
- Evidence Registry;
- Evidence Packet Registry;
- GDSpaces ResourceGraph;
- Spider Hub ProjectGraph;
- all active resource sessions.

Every mutation is applied through the ProjectWorkspace so both graphs and manifests remain synchronized.

## Tool roles

### GDSpaces — The Archive

Owns source mounting, canonical resource identity, bytes, container expansion, classification, ResourceGraph, WorkingCopy creation, and write policy boundaries.

### Spider Hub — The Nexus

Consumes the ProjectGraph. It displays verified relationships but does not resolve or mutate resources independently.

### Binary Inspector — The Reliquary

Consumes resource identity and supplied bytes. It owns structural regions, fields, ownership, unknown gaps, annotations, and deterministic manifests.

### EXE Editor — The Scriptorium

Consumes PE resources through GDSpaces. It owns executable analysis, recovered source, RVA/VA evidence, target identification, and guarded patch plans.

### Evidence Registry

Owns claims, confidence, artifact identity, Evidence Packets, corrections, and provenance.

### Stage Ops — The Theatre

Consumes shared StageBundle and Stage Workspace state. It coordinates stage-oriented editors but never resolves stage files itself.

### ModViz — The Observatory

Consumes the same Stage Workspace state as Stage Ops. Direct ModViz routes are required only for visual categories such as models, textures, cameras, lighting, effects, positions, and collision.

### Item Editor — The Forge

Consumes ITM resources and runtime evidence through shared contracts. It may create WorkingCopy changes and guarded EXE patch requests, but not bypass GDSpaces or Patch Engine policy.

### Build & Test Lab — The Trial Chamber

Consumes validation requests, manifests, patch plans, test fixtures, and reproducibility metadata.

## Tool Capability Registry

A format may route to several tools simultaneously:

- one primary tool;
- companion inspection tools;
- Evidence Registry;
- Build & Test validation.

Example for HITS:

```text
Primary: Stage Ops
Companions: GDSpaces, Binary Inspector, ModViz Scene, Spider Hub
Evidence: Evidence Registry
Validation: Build & Test Lab
```

A route is a capability relationship, not a license to open the source independently.

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

PAC, PNST, AFS, and NBZ remain read-only until exact evidence-backed implementations exist.

## Event-driven mutations

WorkingCopy mutations are not exposed as a generic mutable buffer.

The workspace records:

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

Every event identifies:

- canonical resource;
- producer;
- optional consumer;
- revision;
- subject;
- human-readable message.

This preserves the conveyor principle used by MCP, Obsidian, MemPalace, SDD, and future UI modules: tools exchange events instead of silently overwriting each other.

## Evidence boundaries

A generic PE resource does not inherit DMC3 evidence because its format is PE.

Executable evidence attaches through:

```text
resource SHA-256
  -> ArtifactRegistry match
  -> EvidenceRecord locations referencing that artifact
  -> ResourceWorkspaceSession
```

This prevents findings from one executable build being treated as facts about another build.

## Stage integration

The DMC3 StageWorkspaceBuilder accepts already-read GDSpaces payloads. It does not search directories or containers.

It performs:

1. session creation;
2. Phase 12 parent-path matching;
3. StageBundle assembly;
4. resource analysis where parsers exist;
5. Evidence Packet import;
6. evidence linking;
7. Stage context attachment;
8. ProjectGraph synchronization.

Stage Ops and ModViz consume the resulting Stage Workspace Manifest.

## Forbidden patterns

- editor opens local path directly;
- format parser mounts a source;
- Binary Inspector resolves a container;
- Stage Ops searches for PAC files;
- ModViz stores a parallel resource graph;
- Item Editor writes original bytes;
- EXE Editor attaches DMC3 evidence to an unrecognized binary;
- UI treats display name as resource identity;
- WorkingCopy mutation without an event;
- new PAC Editor/PAC Manager as a top-level architecture.
