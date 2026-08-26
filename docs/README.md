# DMC Rengine Documentation

## Start here

- [Current project status](status/current.md)
- [GDSpaces Layer 1 roadmap](gdspaces/l1-roadmap.md)
- [GDSpaces L1/L2/L3 master roadmap](gdspaces/master-roadmap.md)
- [Project roadmap](roadmap.md)
- [Architecture](architecture.md)
- [GDSpaces contract](gdspaces-contract.md)
- [DMC3 HD format and resource-purpose catalog](formats/dmc3-hd-format-catalog.md)
- [Format documentation index](formats/README.md)
- [Status system](status/README.md)
- [Constitution](../.specify/memory/constitution.md)
- [Contributing](../CONTRIBUTING.md)

## Current execution authority — 2026-08-27

The primary execution program is **GDSpaces L1 — final real same-lineage Resource Materialization acceptance**.

Canonical current authority is reconciled through merged PR #242, with merged #233/#235 supporting access/L2 evidence. Open #226/#238/#240/#241 remain branch truth until merged.

L1 completion remains gate-based:

```text
exact real selected/member lineage
 -> exact materialized bytes + ByteProvenance
 -> representation classification
 -> real bounded edit/rebuild
 -> next-volume publication + canonical reopen/rematerialization
 -> deterministic original-game consumer-visible effect
 -> rollback
 -> final cross-stack/V audit
```

Do not derive L1 completion from percentages, synthetic A-to-Z tests, resolver progress, structural parsing, a Pocket export or crash-free game launch alone.

### Current cross-layer corrections

- **#233 access:** protected `dmc3.exe` and executable-relative `data/dmc3/dmc3-0.nbz` are locatable; the NBZ is observed at 960,358,951 bytes while the connected raw materialization ceiling is 268,435,456 bytes. This is transport/access scope, not archive absence.
- **#235 L2:** numbered filename discovery and actual successful mount topology are separate. The successful set may be sparse; successful mounts prepend, preserving higher successful volume -> lower successful volume -> physical precedence. Issue #237 / pending PR #241 owns the product correction.
- **#230/#242 L1/L3:** normal `0x1401B8DC0` receives one u32 registry-relative context and cannot inspect raw transport success/error. Lower materialization must be terminal before normal state2 dispatch, or completion must be suppressed/removed. FIFO alone is not a proven barrier; no generic fan-in counter is claimed.
- **pending #240 L3:** exact canonical `LoadedResource +0x04` R1 writer census may become static bounded-closed if merged, but R2-R5/V1-V7 remain open and L3 is not complete.
- **draft #226 RCP:** Resource Control Plane is orthogonal orchestration, not L4; V/LV is cross-cutting validation/live observation, also not L4.

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
- [Canonical L1/L2/L3 master roadmap](gdspaces/master-roadmap.md)
- [Materialization completion dependency Pass 2](gdspaces/materialization-completion-dependency-pass2-2026-08-26.md)
- [L2 EXE reconciliation](gdspaces/l2-exe-reconciliation-2026-08-26.md)
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
- [HITS collision resource](formats/hits.md)
- [PAC structural parser](formats/pac-readonly-parser.md)
- [PNST structural parser](formats/pnst-readonly-parser.md)
- [Residual DMC3 format census — 2026-08-26](gdspaces/l3-residual-format-pass-2026-08-26.md)

The format catalog is the canonical human-readable inventory of currently observed/named DMC3-HD resource families. Unknown families stay explicitly `RESEARCH_REQUIRED`; extensions and short ASCII hits are never promoted as semantic truth by themselves.

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
- [Master L1/L2/L3 roadmap](gdspaces/master-roadmap.md)
- [Project roadmap](roadmap.md)
- [Phase map](status/phase-map.md)
- [Blockers](status/blockers.md)
- [Risk register](status/risks.md)
- [Machine-readable status](status/canonical-status.json)
- [Status system rules](status/README.md)
- [Weekly reports](status/weekly/)
- [Specifications](../specs/)

## Research and history

Research baselines and historical receipts remain useful evidence sources, but they do not override current `main`/roadmap authority without explicit promotion.

- [DMC3 Vanilla Research Baseline](research/dmc3-vanilla-research-baseline.md)
- [DMC3 Vanilla Deep Research Wave 2](research/dmc3-vanilla-deep-research-wave-2.md)
- [Project timeline](history/project-timeline.md)
- [Migrated findings](history/migrated-findings.md)
- [Artifact registry](history/artifact-registry.md)

Historical pass/audit documents are not silently rewritten to look current. New current truth is recorded by explicit reconciliation/addendum and synchronized current-status surfaces.

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
- **RCP / V / LV:** orthogonal orchestration/validation surfaces; not numbered execution layers.

## Status language

- **canonical / in main:** reviewed promoted repository truth at the stated snapshot;
- **implemented:** code exists in the stated branch/revision;
- **confirmed / high / bounded:** reverse-evidence status, not subsystem completion;
- **validated:** only the named bounded test/receipt scope;
- **open / research required:** unresolved by direct evidence;
- **pending / branch truth:** present in an unmerged PR, not current `main` authority;
- **superseded / historical:** preserved for audit history but not current execution authority;
- **complete:** allowed only after the applicable formal completion gate passes.
