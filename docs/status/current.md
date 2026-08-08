# Current Project Status

**Snapshot date:** 2026-08-08  
**Repository generation:** evidence-backed C++ platform  
**Version:** 0.2.0  
**Reviewed `main` baseline before this documentation branch:** `6eb6a07975753e2bbe9414893a76e13c946fa78e`  
**Overall phase:** Evidence-Gated Reconstruction / Architecture Consolidation

## Executive result

DMC Rengine is a functioning C++20 reverse-engineering platform with cross-platform tests, artifact identity, Evidence Packets, GDSpaces contracts, Binary Inspector domain analysis, PE/EXE inspection, stage/resource integration, guarded modification workflows, source/build provenance, HITS reconstruction work, and promoted DMC3 PC-save ABI findings.

The project is not yet a complete DMC3 decompilation or a behaviorally equivalent rebuilt executable. The current constraint is no longer simply "add more parsers/editors". The critical problem is converting accumulated reverse knowledge into a controlled, provenance-preserving reconstruction system.

The accepted architectural response is **Reverse Core**: a reusable, game-agnostic reverse lifecycle built on existing evidence, SDD, MCP/Kanban coordination, and validation concepts. DMC Rengine is its first domain workspace and remains a separate project.

## Validation baseline

The latest reviewed implementation on `main` is commit `6eb6a07975753e2bbe9414893a76e13c946fa78e`, following Binary Inspector Wave 1. The preceding Wave 1 merge recorded 68/68 tests on Ubuntu and Windows. HITS Raw Reverse Pass 5/6 was previously validated with 67 tests per platform before the later test-count increase.

All future work must preserve:

- Windows and Ubuntu build/test support;
- GDSpaces as the only game-resource authority;
- evidence status separate from heuristic confidence;
- working-copy/guarded-output safety;
- exact artifact identity for executable claims;
- no representation of recovered source as original Capcom source.

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

Production PAC/PNST/NBZ/AFS expansion remains incomplete. The next canonical identity milestone is [GDSpaces Resource Identity v1](../gdspaces/resource-identity-v1.md): a logical resource must retain stable identity across filesystem, nested container, extracted working-copy, and EXE-backed semantic representations.

`.index` may contribute evidence-backed metadata/linkage but must not become runtime truth or a second asset authority.

### Near-term gate

Resolve one legal local `st001` resource set through production GDSpaces sources and prove Stage Ops, ModViz, Binary Inspector, and executable evidence all reuse the same canonical resource identities.

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

After that, Binary Inspector should bridge its ranges/fields/owners/annotations into Reverse Core durable objects. It must not become a competing reverse database.

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

The canonical chain is now documented in [EXE Reconstruction Pipeline](../exe/reconstruction-pipeline.md):

```text
bytes -> range/function/data -> CFG/xrefs -> types/ABI -> evidence
      -> reconstruction -> C++ source unit -> isolated build
      -> behavioral comparison -> ValidationReceipt
```

Readable decompiler output is not a completion criterion.

### Next gate

Select one bounded subsystem and complete the first full `binary -> recovered C++ -> build -> behavioral validation` loop before scaling mass decompilation.

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

Parallel agents must use claims/ownership for canonical reconstruction work to avoid function/range/source races. Claims coordinate work; they do not constitute evidence.

See [Reverse Core](../reverse-core/README.md).

## Stage Ops

### Implemented foundation

- confirmed DMC3 110 x 4 stage-table descriptor;
- stage resource matching and `st001` plan;
- StageBundle/Stage Workspace construction;
- shared Stage Ops/ModViz views;
- HITS, DCA, LIG2, and Stage TXT modules/tests.

HITS is already a substantial evidence-driven vertical slice: corrected header-driven format model, runtime-derived spatial behavior, safe edits, deterministic SAT writer, round trips, and spatial comparison tooling. Capcom offline-builder equivalence remains research-required pending real corpus and controlled runtime validation.

### Next

Stage Ops should evolve toward [Stage Semantic Graph v1](../stage/stage-semantic-graph.md):

`Stage -> Room -> Geometry -> Collision -> Lighting -> Camera -> Door/Transition -> Event -> Effect -> Audio -> Runtime reference`.

Unknown resources remain first-class nodes with diagnostics/evidence rather than being discarded.

## ModViz

**Status:** architecture/shared-view contracts implemented; complete desktop UI pending.

ModViz retains two top-level modes:

1. Scene/Model Editor;
2. Menu Editor.

The first Menu Editor vertical slice is the [Red Orb HUD counter](../modviz/menu-editor.md), exercising resource identity, hierarchy/mesh/UV editing, runtime-value preview, linked EXE formatting/limit evidence, guarded patch requests where required, and explicit validation/export.

ModViz must never directly resolve/rewrite PAC/NBZ/AFS topology or patch the EXE independently.

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
2. `st001` is not game-backed end-to-end through GDSpaces;
3. no single canonical binary/range/function/type/reconstruction contract is implemented across EXE Editor/Binary Inspector/agents yet;
4. no complete recovered-subsystem compile + behavioral validation loop exists;
5. Reverse Core TaskClaim/ownership and recovered-source tree contracts are not implemented;
6. real HITS/stage corpus and controlled runtime validation remain incomplete;
7. Wide Pass 33 and much recovered-source research remain unpromoted;
8. no working behaviorally equivalent rebuilt DMC3 executable exists.

A separate infrastructure concern exists around MCP desktop/bootstrap logging and signed release-key availability. It does not block DMC3 binary research itself, but it can block reliable distribution/reinstallation of the broader local coordination platform.

See [Blockers](blockers.md).

## Architecture risks

The highest current risks are:

- creating Reverse Core as a second DMC Rengine instead of a reusable game-agnostic layer;
- duplicate parsers/types/identity stores across editors;
- recovered C++ drifting away from binary truth;
- AI/agent races on functions, ranges, types, or source units;
- premature confidence promotion from generated agreement rather than evidence;
- scope dispersion before one complete reconstruction loop is proven;
- GitHub/research/status drift;
- UI concerns redefining domain identity or write policy.

See [Risk Register](risks.md).

## Current critical path

```text
GDSpaces production Resource Identity/read path
  -> legal local game-backed st001 StageBundle
  -> Reverse Core v0.1 schema + claims + source tree
  -> Binary Inspector / EXE Editor Reverse Core bridges
  -> first isolated behavior-tested recovered subsystem
  -> broader stage semantic/runtime validation
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

The next high-value proof is not "500 more decompiled functions". It is one real DMC3 subsystem whose exact bytes, evidence, recovered types/functions, C++ source, build, behavioral comparison, and validation receipt are all connected and reviewable end-to-end.
