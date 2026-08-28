# DMC Rengine Documentation

## Start here

- [File format reference — what every format is for, and on what evidence](formats/README.md)
- [Current project status](status/current.md)
- [GDSpaces Layer 1 roadmap](gdspaces/l1-roadmap.md)
- [Project roadmap](roadmap.md)
- [Architecture](architecture.md)
- [GDSpaces contract](gdspaces-contract.md)
- [DMC3 HD format and resource-purpose catalog](formats/dmc3-hd-format-catalog.md)
- [DMC3 HD machine-readable format-purpose registry](formats/dmc3-hd-format-purpose-registry.json)
- [DMC3 HD format-purpose closure pass — 2026-08-27](research/dmc3-format-purpose-closure-pass-2026-08-27.md)
- [Format documentation index](formats/README.md)
- [Status system](status/README.md)
- [Constitution](../.specify/memory/constitution.md)
- [Contributing](../CONTRIBUTING.md)

## Current execution authority

The primary execution program is **GDSpaces L1 — Resource Materialization**. Its completion authority is gate-based:

```text
publication integrity
 -> artifact-stable retail acquisition
 -> direct-retail provenance
 -> representation classification
 -> real bounded edit/rebuild
 -> next-volume reopen/rematerialization
 -> original-game consumption
 -> final L1 acceptance audit
```

Do not derive L1 completion from percentages, synthetic A-to-Z tests, resolver progress, or structural parsing alone.

## Architecture and contracts

- [Canonical decisions](history/canonical-decisions.md)
- [Deprecated architecture](history/deprecated-architecture.md)
- [Architecture Decision Records](architecture-decisions/README.md)
- [Tool integration](architecture/tool-integration.md)
- [DMC3 reverse-data authority](architecture/dmc3-reverse-data-authority.md)
- [Reverse-engineering rules](reverse-engineering-rules.md)
- [Clean-room policy](legal/clean-room-policy.md)

### GDSpaces

- [GDSpaces contract](gdspaces-contract.md)
- [Canonical L1 roadmap](gdspaces/l1-roadmap.md)
- [Layer classification](gdspaces/decompilation-layer-classification.md)
- [Reverse progress scale](gdspaces/reverse-progress-scale.md) — planning aid only; roadmap gates control completion
- [Resource classification](gdspaces/classification.md)
- [Working copy](gdspaces/working-copy.md)
- [Container tree reconciliation](gdspaces/container-tree-reconciliation.md)
- [DMC3 runtime resource resolver](gdspaces/dmc3-runtime-resource-resolver.md)
- [DMC3 resource lookup policy](gdspaces/dmc3-resource-lookup-policy.md)
- [DMC3 loose-container `.lst` materialization](gdspaces/dmc3-loose-container-list.md)
- [DMC3 layout-preserving relative-slot writer](gdspaces/dmc3-layout-preserving-relative-slot-writer.md)
- [DMC3 runtime-synth relative-slot writer](gdspaces/dmc3-runtime-synth-relative-slot-writer.md)
- [DMC3 nested relative-slot reintegration](gdspaces/dmc3-nested-relative-slot-reintegration.md)
- [DMC3 NBZ STORE overlay writer](gdspaces/dmc3-nbz-store-overlay-writer.md)
- [DMC3 NBZ retail serialization preservation](gdspaces/dmc3-nbz-retail-serialization.md)

### Formats and resource semantics

- [Format documentation index](formats/README.md)
- [Canonical DMC3 HD format and resource-purpose catalog](formats/dmc3-hd-format-catalog.md)
- [Machine-readable DMC3 HD format-purpose registry](formats/dmc3-hd-format-purpose-registry.json)
- [Direct format-purpose closure pass — 2026-08-27](research/dmc3-format-purpose-closure-pass-2026-08-27.md)
- [HITS collision resource](formats/hits.md)
- [PAC structural parser](formats/pac-readonly-parser.md)
- [PNST structural parser](formats/pnst-readonly-parser.md)
- [Residual DMC3 format census — 2026-08-26](gdspaces/l3-residual-format-pass-2026-08-26.md)

The format catalog is the canonical human-readable inventory of currently observed/named DMC3-HD resource families. The JSON registry is its machine-readable companion and currently enumerates 66 evidence-tracked families, including rejected and synthetic-only entries needed to prevent historical claims from resurfacing as game truth. Both separate resource identity, subsystem purpose, binary-schema maturity, original-runtime evidence and current clean-product support. Unknown semantics stay explicitly bounded; extensions and short ASCII hits are never promoted as semantic truth by themselves.

### Stage and downstream tools

- Stage Ops owns product-side stage/scene assembly and orchestration over GDSpaces outputs.
- Stage Semantic Graph is a derived representation/index of Stage Ops state.
- ModViz consumes Stage Ops/Semantic Graph state and must not create a second resolver or scene truth.
- `st001` is a regression/compatibility fixture, not the complete Stage architecture.

See [Stage reconstruction plan](stage/dmc3-stage-resource-plan.md) for historical/current Stage evidence boundaries.

### Evidence, executable and recovered source

- [Public evidence registry](../evidence/README.md)
- [Evidence Packets](evidence/evidence-packets.md)
- [Strict Evidence JSON import](evidence/json-import.md)
- [PE Inspector](exe/pe-inspector.md)
- [Known executable targets](exe/known-targets.md)
- [Guarded patching](patch/guarded-patching.md)

Recovered original-game functions/types/lifetime code belong to the Recovered Game Source Tree. EXE Editor is a frontend over exact binary/reconstruction authority; GDSpaces consumes confirmed behavior without owning original runtime code.

### Binary Inspector

- [Binary document model](binary/document-model.md)
- [Web-to-C++20 cross-port plan](binary/web-crossport.md)

## Current status and planning

- [Current status](status/current.md)
- [Canonical L1 roadmap](gdspaces/l1-roadmap.md)
- [Project roadmap](roadmap.md)
- [Phase map](status/phase-map.md)
- [Blockers](status/blockers.md)
- [Risk register](status/risks.md)
- [Machine-readable status](status/canonical-status.json)
- [Weekly reports](status/weekly/)
- [Specifications](../specs/)

## Research and history

Research baselines and historical receipts remain useful evidence sources, but they do not override current `main`/roadmap authority without explicit promotion.

- [DMC3 Format-Purpose Closure Pass — 2026-08-27](research/dmc3-format-purpose-closure-pass-2026-08-27.md)
- [DMC3 Vanilla Research Baseline](research/dmc3-vanilla-research-baseline.md)
- [DMC3 Vanilla Deep Research Wave 2](research/dmc3-vanilla-deep-research-wave-2.md)
- [Project timeline](history/project-timeline.md)
- [Migrated findings](history/migrated-findings.md)
- [Artifact registry](history/artifact-registry.md)

## Product ownership map

- **GDSpaces — The Archive:** only product resource resolver/materializer/provenance authority.
- **Recovered Game Source Tree:** original DMC3 reconstructed code/ABI/lifecycle authority.
- **Reverse Core:** generic evidence/reconstruction/claim/validation infrastructure.
- **EXE Editor — The Scriptorium:** executable/recovered-source editing and navigation frontend.
- **Binary Inspector — The Reliquary:** byte/structure/evidence inspection.
- **Stage Ops — The Theatre:** stage/scene assembly and operational workspace authority.
- **Stage Semantic Graph:** derived semantic/evidence representation of Stage Ops state.
- **ModViz — The Observatory:** editor/visualization consumer.
- **Build & Test Lab — The Trial Chamber:** reproducibility and behavioral validation.

## Status language

- **canonical / in main:** reviewed promoted repository truth at the stated snapshot;
- **implemented:** code exists in the stated branch/revision;
- **confirmed / high / bounded:** reverse-evidence status, not subsystem completion;
- **validated:** only the named bounded test/receipt scope;
- **open / research required:** unresolved by direct evidence;
- **superseded / historical:** preserved for audit history but not current execution authority;
- **complete:** allowed only after the applicable formal completion gate passes.