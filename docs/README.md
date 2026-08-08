# DMC Rengine Documentation

## Start here

- [Architecture](architecture.md)
- [Roadmap](roadmap.md)
- [Current status](status/current.md)
- [Phase map](status/phase-map.md)
- [Reverse Core](reverse-core/README.md)
- [Constitution](../.specify/memory/constitution.md)
- [Contributing](../CONTRIBUTING.md)

## Architecture and contracts

- [Canonical decisions](history/canonical-decisions.md)
- [Deprecated architecture](history/deprecated-architecture.md)
- [Architecture Decision Records](architecture-decisions/README.md)
- [Reverse-engineering rules](reverse-engineering-rules.md)
- [Clean-room policy](legal/clean-room-policy.md)

### Reverse Core and source recovery

- [Reverse Core v0.1](reverse-core/README.md)
- [EXE reconstruction pipeline](exe/reconstruction-pipeline.md)
- [Read-only PE Inspector](exe/pe-inspector.md)
- [Known executable targets](exe/known-targets.md)

### GDSpaces

- [GDSpaces contract](gdspaces-contract.md)
- [Resource Identity v1](gdspaces/resource-identity-v1.md)
- [Resource classification](gdspaces/classification.md)
- [Working copy](gdspaces/working-copy.md)
- [StageBundle assembly](gdspaces/stage-bundle-assembly.md)

### Stage reconstruction

- [DMC3 Phase 12 stage resource plan](stage/dmc3-stage-resource-plan.md)
- [Stage Semantic Graph v1](stage/stage-semantic-graph.md)

### ModViz

- [Menu Editor and Red Orb vertical slice](modviz/menu-editor.md)

### Formats and containers

- [Generic container foundation](formats/container-foundation.md)
- [Synthetic slot container](formats/synthetic-slot-container.md)

### Evidence and patching

- [Public evidence registry](../evidence/README.md)
- [Evidence Packets](evidence/evidence-packets.md)
- [Strict Evidence JSON import](evidence/json-import.md)
- [Guarded patching](patch/guarded-patching.md)

### Binary Inspector

- [Binary document model](binary/document-model.md)
- [Web-to-C++20 cross-port plan](binary/web-crossport.md)

## History and Canon migration

- [Project timeline](history/project-timeline.md)
- [Migrated findings](history/migrated-findings.md)
- [Artifact registry](history/artifact-registry.md)

## Status and planning

- [Current status](status/current.md)
- [Phase map](status/phase-map.md)
- [Blockers](status/blockers.md)
- [Risk register](status/risks.md)
- [Machine-readable status](status/canonical-status.json)
- [Weekly reports](status/weekly/)
- [Specifications](../specs/)

## Product map

- **GDSpaces — The Archive:** only game-resource access authority; sources, identity, graph, routing, diagnostics, classification, working copies, container expansion, and typed bundles.
- **Reverse Core:** reusable, game-agnostic binary/function/type/evidence/reconstruction/validation lifecycle used first by DMC Rengine.
- **Spider Hub — The Nexus:** visual navigation across tools, evidence, specifications, tasks, and project provenance.
- **EXE Editor — The Scriptorium:** executable inspection, recovered-source work, address mappings, Reverse Core integration, and guarded patch planning.
- **Binary Inspector — The Reliquary:** regions, fields, ownership, range selection, conflicts, unknown coverage, annotations, byte diff, entropy, and future Reverse Core bridge.
- **Stage Ops — The Theatre:** typed stage workflows over `StageBundle` and the Stage Semantic Graph.
- **ModViz — The Observatory:** Scene/Model Editor and Menu Editor over shared GDSpaces/workspace state.
- **Item Editor — The Forge:** typed item editing through shared resource/runtime contracts.
- **Build & Test Lab — The Trial Chamber:** reproducibility, validation, regression, rollback, and release artifacts.

PAC, PNST, NBZ, and AFS are internal resource/container layers. Legacy PAC Editor/PAC Manager logic is intentionally excluded from the product architecture.

## Documentation status language

- **implemented:** exists in this repository and is connected to the build/tests;
- **historical:** existed in an earlier/private generation;
- **confirmed:** reverse finding supported by recorded evidence;
- **high/medium/low:** confidence level, not implementation status;
- **planned:** accepted direction without completed code;
- **research required:** stronger compatibility/behavior claims are intentionally blocked pending evidence;
- **deprecated/rejected:** must not be used as the new architecture.
