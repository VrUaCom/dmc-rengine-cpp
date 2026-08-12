# Current Project Status

**Snapshot date:** 2026-08-12  
**Repository generation:** evidence-backed C++ platform  
**Version:** 0.2.0  
**Reviewed `main` baseline before this documentation branch:** `6eb6a07975753e2bbe9414893a76e13c946fa78e`  
**Overall phase:** Evidence-Gated Reconstruction / Architecture Consolidation

## Executive result

DMC Rengine is a functioning C++20 reverse-engineering platform with cross-platform tests, artifact identity, Evidence Packets, GDSpaces contracts, Binary Inspector domain analysis, PE/EXE inspection, stage/resource integration, guarded modification workflows, source/build provenance, HITS reconstruction work, and promoted DMC3 PC-save ABI findings.

The project is not yet a complete DMC3 decompilation or a behaviorally equivalent rebuilt executable. The current constraint is no longer simply "add more parsers/editors". The critical problem is converting accumulated reverse knowledge into a controlled, provenance-preserving reconstruction system.

The accepted architectural response is **Reverse Core**: a reusable, game-agnostic reverse lifecycle built on existing evidence, SDD, MCP/Kanban coordination, and validation concepts. DMC Rengine is its first domain workspace and remains a separate project.

A second canonical distinction is now explicit: **Recovered Game Source Tree** represents the reconstructed DMC3 program itself. Recovered functions/data/types are not owned by EXE Editor, Reverse Core, GDSpaces, Binary Inspector, ModViz, Stage Ops, or any other tool. Tool relationships, semantic game-subsystem membership, and temporary TaskClaim coordination are separate concepts.

## Validation baseline

The latest reviewed implementation on `main` is commit `6eb6a07975753e2bbe9414893a76e13c946fa78e`, following Binary Inspector Wave 1. The preceding Wave 1 merge recorded 68/68 tests on Ubuntu and Windows. HITS Raw Reverse Pass 5/6 was previously validated with 67 tests per platform before the later test-count increase.

All future work must preserve:

- Windows and Ubuntu build/test support;
- GDSpaces as the only DMC Rengine game-resource authority;
- recovered game code independent from tool ownership;
- evidence status separate from heuristic confidence;
- working-copy/guarded-output safety;
- exact artifact identity for executable claims;
- no representation of recovered source as original Capcom source.

## Recovered Game Source Tree

**Status:** canonical architecture rule accepted; source recovery remains partial.

The recovered source tree follows the reconstructed architecture of the game rather than editor names. A recovered function may be classified under game resource runtime, stage runtime, renderer, collision, UI/HUD, save, input, audio, gameplay, or an unresolved subsystem while still being referenced by multiple DMC Rengine tools.

The same game function can simultaneously have:

- source/disassembly workflow in EXE Editor;
- byte/structure evidence in Binary Inspector;
- stable reconstruction/evidence identities in Reverse Core;
- behavior consumed by GDSpaces or another product subsystem;
- compile/runtime validation in Build & Test Lab.

This remains one game function. A `TaskClaim` only coordinates who may mutate the canonical reconstruction; it does not grant semantic ownership.

See [Recovered Game Source Tree](../reverse-core/game-source-tree.md).

## GDSpaces

### Implemented foundation

- stable resource/reference/payload contracts;
- read-only local source with containment guards;
- source registry;
- profile-aware classification and post-read correction;
- resource graph and OpenRouter;
- typed StageBundle assembly;
- generic read-only container contracts and synthetic fixtures;
- child identity, empty-slot preservation, diagnostics, and graph edges;
- revisioned working copies and deterministic workspace manifests.

### Current gap

Production PAC/PNST/NBZ/AFS expansion remains incomplete. More importantly, the executable-side DMC3 resource runtime is not yet fully reconstructed.

GDSpaces must be completed from evidence of how the game actually performs:

```text
request
  -> lookup / tables / indexes
  -> source/archive selection
  -> byte acquisition / decompression
  -> nested resource expansion
  -> classification / factory dispatch
  -> dependencies
  -> allocation / construction
  -> cache / ownership / lifetime
  -> reload / transition
  -> release / unload
```

Recovered game loader/cache/factory functions remain in the Recovered Game Source Tree. GDSpaces separately implements the DMC Rengine product API based on confirmed behavior.

The canonical reverse program is [GDSpaces Resource Runtime Reconstruction](../gdspaces/runtime-reconstruction.md), tracked by issue #55.

The identity milestone remains [GDSpaces Resource Identity v1](../gdspaces/resource-identity-v1.md): a logical resource must retain stable identity across filesystem, nested container, extracted working-copy, and EXE-backed semantic representations.

`.index` may contribute evidence-backed metadata/linkage but must not become runtime truth or a second asset authority.

### Near-term gate

Drive stage-related resource validation from the complete executable-derived [Stage Catalog](../stage/stage-catalog.md), not from one fixed stage.

```text
canonical EXE stage-resource table
  -> StageCatalog[110 rows]
  -> select representative catalog entries/variants
  -> recovered game resource-runtime call path
  -> production GDSpaces source/container path
  -> canonical resource identities
  -> StageBundle per selected entry
  -> Stage Semantic Graph
  -> load/transition/cache/lifetime observations
  -> ValidationReceipts
```

`st001` may remain one regression fixture but is not the architecture or completion gate.

## Binary Inspector

### Implemented

- regions and typed fields;
- ownership and annotations;
- selected-offset/range context;
- coverage, unknown gaps, structural/ownership conflicts;
- deterministic manifests;
- format adapters including HITS;
- offset-aligned byte diff;
- Shannon entropy analysis with zero-ratio and unique-byte metrics.

Wave 1 is implemented in native C++20. Entropy remains a heuristic visualization, not an Evidence state.

### Next

Wave 2 remains the active domain milestone:

- persistent/artifact-keyed Analysis Cache;
- generic duplicate/order/overlap/alignment/stride diagnostics;
- unknown-region feature analysis;
- reusable versioned binary templates;
- deterministic analysis-result JSON.

After that, Binary Inspector should bridge its ranges/fields/owners/annotations into Reverse Core durable objects. It must not become a competing reverse database or owner of recovered game functions.

## EXE Editor and decompilation

### Implemented foundation

- bounded PE32/PE32+ inspection;
- checked file offset/RVA/VA conversion;
- canonical executable target identity and SHA-256 matching;
- Evidence Address Resolver;
- executable workspace/reopen lineage;
- selected promoted runtime evidence;
- guarded patch contracts;
- source-to-output mapping and Custom Build identity models.

### Current state

The repository contains strong executable research footholds, but there is no complete production pipeline from arbitrary DMC3 functions to reviewed, behaviorally validated recovered C++.

The canonical chain is documented in [EXE Reconstruction Pipeline](../exe/reconstruction-pipeline.md):

```text
bytes -> range/function/data -> CFG/xrefs -> game-subsystem membership
      -> types/ABI -> evidence -> reconstruction
      -> C++ source unit in Recovered Game Source Tree
      -> isolated build -> behavioral comparison -> ValidationReceipt
```

Readable decompiler output is not a completion criterion. EXE Editor is a workflow/view over recovered game code, not its semantic owner.

### Next gate

Select one bounded game subsystem and complete the first full `binary -> recovered C++ -> build -> behavioral validation` loop before scaling mass decompilation.

## Reverse Core

**Status:** architecture accepted; shared implementation pending.

Reverse Core formalizes:

- BinaryArtifact;
- AddressRange;
- Function;
- DataObject;
- RecoveredType;
- EvidenceRecord;
- Hypothesis;
- Experiment;
- TaskClaim;
- Reconstruction;
- ValidationReceipt;
- Subsystem.

It reuses SDD/Spec Kit, Kanban/MCP coordination, evidence workflows, Obsidian human-readable research, and long-term memory systems rather than creating a second project-management stack.

Reverse Core stores/coordinates reverse identities and reconstruction metadata. The recovered game source itself remains the Recovered Game Source Tree.

Parallel agents must use claims/ownership for canonical reconstruction mutation to avoid function/range/source races. Claims coordinate work; they do not constitute evidence or semantic ownership of game code.

See [Reverse Core](../reverse-core/README.md).

## Stage Ops

### Implemented foundation

- confirmed DMC3 110 x 4 stage-table descriptor;
- executable-row/resource-role modeling;
- StageBundle/Stage Workspace construction;
- shared Stage Ops/ModViz views;
- HITS, DCA, LIG2, and Stage TXT modules/tests;
- historical `st001` fixture support used only as one regression example.

The canonical target is now the complete [DMC3 Stage Catalog](../stage/stage-catalog.md): all 110 executable rows, their four observed resource references, shared/duplicate relations, variants/special cases where evidence supports them, and deterministic arbitrary-entry selection.

HITS is already a substantial evidence-driven vertical slice: corrected header-driven format model, runtime-derived spatial behavior, safe edits, deterministic SAT writer, round trips, and spatial comparison tooling. Capcom offline-builder equivalence remains research-required pending real corpus and controlled runtime validation.

### Next

Stage Ops should evolve toward [Stage Semantic Graph v1](../stage/stage-semantic-graph.md):

`StageCatalog -> StageCatalogEntry -> Stage -> Room -> Geometry -> Collision -> Lighting -> Camera -> Door/Transition -> Event -> Effect -> Audio -> Runtime reference`.

Unknown resources remain first-class nodes with diagnostics/evidence rather than being discarded. Shared resources can connect multiple catalog entries without being duplicated into invented per-stage identities.

Stage-related recovered executable functions remain game code and are linked into the graph/evidence model; they do not become Stage Ops implementation merely because Stage Ops consumes their semantics.

## ModViz

**Status:** architecture/shared-view contracts implemented; complete desktop UI pending.

ModViz retains two top-level modes:

1. Scene/Model Editor;
2. Menu Editor.

The first Menu Editor vertical slice is the [Red Orb HUD counter](../modviz/menu-editor.md), exercising resource identity, hierarchy/mesh/UV editing, runtime-value preview, linked EXE formatting/limit evidence, guarded patch requests where required, and explicit validation/export.

ModViz must never directly resolve/rewrite PAC/NBZ/AFS topology, patch the EXE independently, or become the owner of recovered UI/HUD game functions.

## Item and guarded modification

The Item layer already demonstrates the intended separation of concerns:

- item semantics create typed runtime requests;
- evidence gates patch-plan compilation;
- patch plans carry source hash, expected bytes, ranges, and overlap checks;
- execution occurs on copied bytes;
- rollback is verified;
- manifests record provenance and non-modification of originals.

This remains a reference pattern for other editor/runtime bridges.

## DMC3 PC-save promotion

Pass 31/32 record-envelope/checksum ABI is reviewed in C++ with compile-time layouts, diagnostics, Evidence Packets, and regression tests. Wide Pass 33 remains research-ready/product-promotion-pending and should be promoted narrowly, not bulk-imported with unrelated recovered-source snapshots.

## Current blockers

P0/P1 blockers are:

1. production read-only container source expansion is incomplete;
2. the DMC3 resource runtime is not fully reconstructed from request through cache/lifetime/unload;
3. the complete 110-row Stage Catalog is not yet game-backed end-to-end through GDSpaces across representative entry/variant types;
4. no single canonical binary/range/function/type/reconstruction contract is implemented across EXE Editor/Binary Inspector/agents yet;
5. no complete recovered-game-subsystem compile + behavioral validation loop exists;
6. Reverse Core TaskClaim and Recovered Game Source Tree contracts are not implemented as shared code/export machinery;
7. real HITS/stage corpus and controlled runtime validation remain incomplete;
8. Wide Pass 33 and much recovered-source research remain unpromoted;
9. no working behaviorally equivalent rebuilt DMC3 executable exists.

A separate infrastructure concern exists around MCP desktop/bootstrap logging and signed release-key availability. It does not block DMC3 binary research itself, but it can block reliable distribution/reinstallation of the broader local coordination platform.

See [Blockers](blockers.md).

## Architecture risks

The highest current risks are:

- confusing recovered game code with DMC Rengine tool implementation;
- creating Reverse Core as a second DMC Rengine instead of a reusable game-agnostic metadata/lifecycle layer;
- duplicate parsers/types/identity stores across editors;
- recovered C++ drifting away from binary truth;
- AI/agent races on functions, ranges, types, or source units;
- premature confidence promotion from generated agreement rather than evidence;
- forcing uncertain game functions into tool-shaped subsystem folders;
- treating a convenient fixture such as `st001` or a filename pattern such as `stNNN` as the canonical stage architecture;
- scope dispersion before one complete reconstruction loop is proven;
- GitHub/research/status drift;
- UI concerns redefining domain identity or write policy.

See [Risk Register](risks.md).

## Current critical path

```text
full GDSpaces resource-runtime reverse + production read path + Resource Identity v1
  -> complete executable-derived StageCatalog[110] + representative game-backed entry/variant validation
  -> arbitrary StageCatalogEntry -> StageBundle pipeline
  -> Reverse Core v0.1 schema + TaskClaims + Recovered Game Source Tree contract
  -> Binary Inspector / EXE Editor Reverse Core bridges
  -> first isolated behavior-tested recovered game subsystem
  -> broader Stage Semantic Graph/runtime validation
  -> controlled source-integration/recompilation milestones
```

Parallel work:

```text
Binary Inspector Wave 2
Wide Pass 33 narrow promotion
HITS real-corpus/runtime validation
ModViz Red Orb vertical slice after shared identity/evidence gates are ready
```

## Milestone statement

The next high-value proof is not "500 more decompiled functions" and not "one `st001` loads". It is a generic evidence-backed system in which the complete Stage Catalog is recoverable from the executable, representative different catalog entries travel through the same GDSpaces/StageBundle pipeline, and recovered game subsystems remain connected to exact bytes, evidence, source, builds, behavioral comparison, and ValidationReceipts.
