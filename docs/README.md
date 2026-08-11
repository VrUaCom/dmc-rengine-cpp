# DMC Rengine Documentation

## Start here

- [Architecture](architecture.md)
- [Roadmap](roadmap.md)
- [Current status](status/current.md)
- [Phase map](status/phase-map.md)
- [Constitution](../.specify/memory/constitution.md)
- [Contributing](../CONTRIBUTING.md)

## Architecture and contracts

- [Canonical decisions](history/canonical-decisions.md)
- [Deprecated architecture](history/deprecated-architecture.md)
- [Architecture Decision Records](architecture-decisions/README.md)
- [Reverse-engineering rules](reverse-engineering-rules.md)
- [Clean-room policy](legal/clean-room-policy.md)

### GDSpaces

- [GDSpaces contract](gdspaces-contract.md)
- [Resource classification](gdspaces/classification.md)
- [Working copy](gdspaces/working-copy.md)
- [StageBundle assembly](gdspaces/stage-bundle-assembly.md)

### Stage reconstruction

- [DMC3 Phase 12 stage resource plan](stage/dmc3-stage-resource-plan.md)

### Formats and containers

- [Generic container foundation](formats/container-foundation.md)
- [Synthetic slot container](formats/synthetic-slot-container.md)

### Evidence, executable, and patching

- [Public evidence registry](../evidence/README.md)
- [Evidence Packets](evidence/evidence-packets.md)
- [Strict Evidence JSON import](evidence/json-import.md)
- [Read-only PE Inspector](exe/pe-inspector.md)
- [Known executable targets](exe/known-targets.md)
- [Guarded patching](patch/guarded-patching.md)

### Binary Inspector

- [Binary document model](binary/document-model.md)
- [Web-to-C++20 cross-port plan](binary/web-crossport.md)

## Research baselines

- [DMC3 Vanilla Research Baseline](research/dmc3-vanilla-research-baseline.md) — pre-roadmap reconciled authority for vanilla runtime/resource architecture, cross-build identity, ST namespace coverage, HD-port translation, HITS corrections, and remaining research frontiers.
- [DMC3 Vanilla Deep Research Wave 2](research/dmc3-vanilla-deep-research-wave-2.md) — direct cross-build evidence for the full 189-descriptor Stage surface, numeric Stage-ID resolver, cross-stage aliases, `.lst` fallback, 363-entry resource lifecycle manager, typed post-load fixups, and HD ADX→OGG / SFD→WMV translation.

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

## Brand and community

- [Brand Canon index](brand/README.md)
- [Lore](brand/lore.md)
- [Naming system](brand/naming-system.md)
- [Brand glossary](brand/glossary.md)
- [Voice and marketing](brand/voice-and-marketing.md)
- [Sect of Neuroslop](brand/sect-of-neuroslop.md)

## Product map

- **GDSpaces — The Archive:** resource sources, identity, graph, routing, diagnostics, classification, working copies, container expansion, and typed bundles.
- **Spider Hub — The Nexus:** visual navigation across tools, evidence, and specifications.
- **EXE Editor — The Scriptorium:** executable inspection, source recovery, known-target evidence, and guarded patch planning.
- **Binary Inspector — The Reliquary:** regions, fields, ownership, range selection, conflicts, unknown coverage, annotations, byte diff, and entropy analysis.
- **Stage Ops — The Theatre:** typed stage workflows over `StageBundle`.
- **ModViz — The Observatory:** Scene/Model Editor and Menu Editor.
- **Item Editor — The Forge:** typed item editing through shared resource/runtime contracts.
- **Build & Test Lab — The Trial Chamber:** reproducibility, validation, regression, and release artifacts.

## Documentation status language

- **implemented:** exists in this repository and is connected to the build/tests;
- **historical:** existed in an earlier/private generation;
- **confirmed:** reverse finding supported by recorded evidence;
- **high/medium/low:** confidence level, not implementation status;
- **planned:** accepted direction without completed code;
- **deprecated/rejected:** must not be used as the new architecture.
