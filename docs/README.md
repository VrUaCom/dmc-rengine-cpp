# DMC Rengine Documentation

## Start here

- [Current project status](status/current.md)
- [GDSpaces master roadmap](gdspaces/master-roadmap.md)
- [GDSpaces Layer 1 roadmap](gdspaces/l1-roadmap.md)
- [GDSpaces Layer 2 roadmap](gdspaces/l2-roadmap.md)
- [GDSpaces Layer 3 roadmap](gdspaces/l3-roadmap.md)
- [Layer/decompilation classification](gdspaces/decompilation-layer-classification.md)
- [2026-09-02 layer-boundary + L3 research/review](gdspaces/layer-boundaries-l3-research-review-2026-09-02.md)
- [L3 R1 current-main reconciliation](gdspaces/l3-r1-current-main-reconciliation-2026-09-02.md)
- [Project roadmap](roadmap.md)
- [Architecture](architecture.md)
- [GDSpaces contract](gdspaces-contract.md)
- [DMC3 HD format/resource-purpose catalog](formats/dmc3-hd-format-catalog.md)
- [Machine-readable format-purpose registry](formats/dmc3-hd-format-purpose-registry.json)
- [Status system](status/README.md)
- [Constitution](../.specify/memory/constitution.md)
- [Contributing](../CONTRIBUTING.md)

## Current execution authority

GDSpaces is no longer represented as one L1-only closure program. The current canonical execution model is three separate ownership layers with dependency-driven work:

```text
L2 selects identity
 -> L1 materializes/authors exact bytes
 -> L3 owns original runtime lifecycle
```

Current status:

```text
L1  INCOMPLETE / NOT 100%
L2  ADVANCED / INCOMPLETE
L3  INCOMPLETE — R1 bounded-closed, R2 active
```

Product capability, reverse closure and original-process acceptance are tracked separately. Do not derive layer completion from percentages, synthetic A-to-Z tests, resolver success, structural parsing, preview success or crash-free launch.

## GDSpaces architecture and contracts

- [GDSpaces contract](gdspaces-contract.md)
- [Master roadmap](gdspaces/master-roadmap.md)
- [Layer 1 roadmap](gdspaces/l1-roadmap.md)
- [Layer 2 roadmap](gdspaces/l2-roadmap.md)
- [Layer 3 roadmap](gdspaces/l3-roadmap.md)
- [Layer classification](gdspaces/decompilation-layer-classification.md)
- [Reverse progress scale](gdspaces/reverse-progress-scale.md) — planning aid only; mandatory gates control completion
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

### Current cross-layer boundary

- **L2 — Resource Resolution:** request/candidates/normalization/successful provider-volume-member identity.
- **L1 — Resource Materialization:** representation, byte/result semantics, exact bytes/provenance, edit/rebuild/repack/reopen.
- **L3 — Original Runtime/Lifecycle:** scheduler/callback lifetime, LoadedResource states, typed-ready, claims/reuse, cancellation/release/reset/teardown.
- **DOMAIN:** Stage Ops, ModViz and editors consume the above authority.
- **V:** validation/live observation is cross-cutting, not L4.

## Current L3 reverse authority

The current-main R1 reconciliation closes broad `LoadedResource +0x04` state-writer discovery for the canonical analysis executable unless contradicted by exact record provenance.

Active static L3 work is now **R2**: family-complete ownership of `+0x08/+0x18/+0x20/+0x28` and stable adjacent fields. Typed/factory/shared-owner breadth remains partial and trusted lifecycle validation remains open.

Runtime type evidence must remain split across its instruction-backed sites rather than flattened into one universal detector/format enum.

## Formats and resource semantics

- [Format documentation index](formats/README.md)
- [Canonical DMC3 HD format/resource-purpose catalog](formats/dmc3-hd-format-catalog.md)
- [Machine-readable DMC3 HD format-purpose registry](formats/dmc3-hd-format-purpose-registry.json)
- [Runtime type-evidence split — 2026-08-31](research/dmc3-runtime-type-evidence-split-2026-08-31.md)
- [Primary 3D/render family reverse — 2026-08-31](research/dmc3-primary-3d-render-family-reverse-2026-08-31.md)
- [Real MOD/SHW payload binding — 2026-09-01](research/dmc3-real-mod-shw-payload-binding-2026-09-01.md)
- [HITS collision resource](formats/hits.md)
- [PAC structural parser](formats/pac-readonly-parser.md)
- [PNST structural parser](formats/pnst-readonly-parser.md)

Format identity, runtime classifier identity, post-load handler existence, geometry semantics and writer maturity are separate claims.

## Stage and downstream tools

- Stage Ops owns product-side stage/scene assembly and orchestration over GDSpaces outputs.
- Stage Semantic Graph is a derived representation/index of Stage Ops state.
- ModViz consumes Stage Ops/Semantic Graph state and must not create a second resolver/materializer/lifecycle authority.
- `st001` remains a regression/compatibility fixture, not the complete Stage architecture.

See [Stage reconstruction plan](stage/dmc3-stage-resource-plan.md).

## Evidence, executable and recovered source

- [Public evidence registry](../evidence/README.md)
- [Evidence Packets](evidence/evidence-packets.md)
- [Strict Evidence JSON import](evidence/json-import.md)
- [PE Inspector](exe/pe-inspector.md)
- [Known executable targets](exe/known-targets.md)
- [Guarded patching](patch/guarded-patching.md)

Recovered original-game functions/types/lifetime code belong to the Recovered Game Source Tree/profile evidence domain. GDSpaces consumes confirmed contracts without laundering original ownership code into generic product modules.

## Binary Inspector

- [Binary document model](binary/document-model.md)
- [Web-to-C++20 cross-port plan](binary/web-crossport.md)

## Current status and planning

- [Current status](status/current.md)
- [Master roadmap](gdspaces/master-roadmap.md)
- [L1 roadmap](gdspaces/l1-roadmap.md)
- [L2 roadmap](gdspaces/l2-roadmap.md)
- [L3 roadmap](gdspaces/l3-roadmap.md)
- [Project roadmap](roadmap.md)
- [Phase map](status/phase-map.md)
- [Blockers](status/blockers.md)
- [Risk register](status/risks.md)
- [Machine-readable status](status/canonical-status.json)
- [Weekly reports](status/weekly/)
- [Specifications](../specs/)

## Research and history

Historical research/checkpoints remain evidence sources but do not override current roadmap/status authority without explicit reconciliation.

- [DMC3 Format-Purpose Closure Pass — 2026-08-27](research/dmc3-format-purpose-closure-pass-2026-08-27.md)
- [DMC3 Vanilla Research Baseline](research/dmc3-vanilla-research-baseline.md)
- [DMC3 Vanilla Deep Research Wave 2](research/dmc3-vanilla-deep-research-wave-2.md)
- [Project timeline](history/project-timeline.md)
- [Migrated findings](history/migrated-findings.md)
- [Artifact registry](history/artifact-registry.md)

## Product ownership map

- **GDSpaces — The Archive:** product resource selection/materialization/provenance/authoring contracts, separated internally by L2/L1 boundaries.
- **Recovered Game Source Tree / runtime profiles:** original DMC3 reconstructed code/ABI/lifecycle authority, including L3 evidence.
- **Reverse Core:** generic evidence/reconstruction/claim/validation infrastructure.
- **EXE Editor — The Scriptorium:** executable/recovered-source editing/navigation frontend.
- **Binary Inspector — The Reliquary:** byte/structure/evidence inspection.
- **Stage Ops — The Theatre:** stage/scene assembly and operational workspace authority.
- **Stage Semantic Graph:** derived semantic/evidence representation of Stage Ops state.
- **ModViz — The Observatory:** editor/visualization consumer.
- **Build & Test Lab — The Trial Chamber:** reproducibility and behavioral validation.

## Status language

- **canonical / in main:** merged repository truth at the stated snapshot;
- **reconciliation:** current-main semantic port proposed for promotion after review;
- **implemented:** code exists at the stated branch/revision;
- **confirmed / high / bounded:** reverse-evidence status, not subsystem completion;
- **validated:** only the named bounded test/receipt scope;
- **open / research required:** unresolved by direct evidence;
- **superseded / historical:** preserved for audit history but not current execution authority;
- **complete:** allowed only after the applicable mandatory completion gates pass.
