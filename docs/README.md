# DMC Rengine Documentation

## Start here

1. [Completion & Evidence Policy](status/completion-and-evidence-policy.md)
2. [Current status](status/current.md)
3. [Architecture](architecture.md)
4. [Roadmap](roadmap.md)
5. [Reverse-engineering rules](reverse-engineering-rules.md)
6. [Constitution](../.specify/memory/constitution.md)
7. [Contributing](../CONTRIBUTING.md)

## Mandatory interpretation rule

DMC Rengine contains many real implemented/tested/EXE-confirmed/validated slices, but **no major end-to-end subsystem is currently COMPLETE or proven equivalent to the original DMC3 runtime**.

Historical documents remain evidence snapshots. Later `CORRECTED`, `REJECTED`, `BOUNDED CLOSED`, `VALIDATED`, or supersession records override conflicting older wording without deleting history.

An active PR is branch-scoped truth, not merged `main` truth. Green synthetic CI is not original-game behavioral proof.

## Architecture and ownership

- [Architecture](architecture.md)
- [Canonical decisions](history/canonical-decisions.md)
- [Deprecated architecture](history/deprecated-architecture.md)
- [Architecture Decision Records](architecture-decisions/README.md)
- [Reverse-engineering rules](reverse-engineering-rules.md)
- [Clean-room policy](legal/clean-room-policy.md)

Canonical ownership:

- **GDSpaces** — product resource identity/resolution/materialization/provenance;
- **Recovered Game Source Tree** — reconstructed original DMC3 runtime behavior;
- **Reverse Core** — generic reverse/evidence/reconstruction/claim infrastructure;
- **Stage Ops** — product-side stage/scene assembly and operational state;
- **Stage Semantic Graph** — derived representation/index over Stage Ops state;
- **ModViz** — scene/asset/HUD editing consumer;
- **Binary Inspector** — bytes/structure/ownership/evidence inspection;
- **EXE Editor** — executable/reconstruction editing frontend over shared authorities.

No tool creates a second resource resolver or competing stage/reconstruction truth.

## GDSpaces

- [GDSpaces contract](gdspaces-contract.md)
- [Resource classification](gdspaces/classification.md)
- [Working copy](gdspaces/working-copy.md)
- [StageBundle assembly](gdspaces/stage-bundle-assembly.md)

A product `StageBundle` is not automatically an original DMC3 state-3/game-ready runtime object.

## Stage reconstruction

Current Stage authority is catalog/selector driven, not `st001`-driven:

- 110 Bank-A observed descriptors;
- 79 Bank-B observed descriptors;
- 189 observed descriptors total;
- 193 selector entries;
- 10 group-base pointers;
- independent resource-set/catalog, numeric Stage and separately evidenced semantic Stage identities.

`st001` is regression/compatibility data only.

Relevant current coordination: issue #4, #55, #90 and active Stage/GDSpaces/Stage Ops stacks.

Historical/compatibility documents such as [DMC3 Phase 12 stage resource plan](stage/dmc3-stage-resource-plan.md) must be interpreted through the current completion/Stage-identity policy.

## Formats and containers

- [Generic container foundation](formats/container-foundation.md)
- [Synthetic slot container](formats/synthetic-slot-container.md)

Format parsing or a deterministic writer is a bounded capability; it does not by itself prove original runtime/offline-builder equivalence.

## Evidence, Reverse Core, executable and patching

- [Public evidence registry](../evidence/README.md)
- [Evidence Packets](evidence/evidence-packets.md)
- [Strict Evidence JSON import](evidence/json-import.md)
- [Read-only PE Inspector](exe/pe-inspector.md)
- [Known executable targets](exe/known-targets.md)
- [Guarded patching](patch/guarded-patching.md)

Recovered C++ is evidence-backed reconstruction, not automatically original Capcom source. Agent consensus is not evidence.

## Binary Inspector

- [Binary document model](binary/document-model.md)
- [Web-to-C++20 cross-port plan](binary/web-crossport.md)

## Research baselines

Research baselines are chronological evidence records, not whole-project completion claims.

- [DMC3 Vanilla Research Baseline](research/dmc3-vanilla-research-baseline.md)
- [DMC3 Vanilla Deep Research Wave 2](research/dmc3-vanilla-deep-research-wave-2.md)

Later active research may supersede older target lists. For HITS specifically, current Pass-10 work in PR #85 supersedes the earlier Pass-8/Pass-9 statement that the entire E7A0/B460/FEC0/601E0 upper wrapper set is still open; those bounded closures do not make collision complete.

## History and Canon migration

- [Project timeline](history/project-timeline.md)
- [Migrated findings](history/migrated-findings.md)
- [Artifact registry](history/artifact-registry.md)

## Status and planning

- [Completion & Evidence Policy](status/completion-and-evidence-policy.md)
- [Current status](status/current.md)
- [Phase map](status/phase-map.md)
- [Blockers](status/blockers.md)
- [Risk register](status/risks.md)
- [Machine-readable status](status/canonical-status.json)
- [Weekly reports](status/weekly/)
- [Specifications](../specs/)

## Documentation status language

Use the vocabulary defined in the completion policy. In particular:

- `IMPLEMENTED` — code exists for the bounded scope;
- `TESTED` — deterministic tests pass for that scope;
- `EXE CONFIRMED` — directly supported by canonical executable evidence;
- `BOUNDED CLOSED` — the specified reverse target is closed at that boundary;
- `VALIDATED` — a reproducible validation receipt exists for the stated bounded behavior;
- `NOT PROVEN` / `RESEARCH REQUIRED` — stronger semantics/equivalence remain withheld;
- `CORRECTED` / `REJECTED` — older claims are superseded/disproved;
- `COMPLETE` — reserved for the full applicable completion gate.

Never infer `COMPLETE` from `IMPLEMENTED`, `TESTED`, green CI, or a readable recovered C++ unit.
