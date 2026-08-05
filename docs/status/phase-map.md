# Phase Map

**Snapshot date:** 2026-08-05  
**Snapshot base:** `main` at `1f77e2076a79216e015a3ddc83b1d1ed89c121c8`

The phases describe the natural evolution of the current C++ repository. They do not redefine the project or force work into the closed-source DMC Rengine product roadmap.

## Phase 0 — Public C++ foundation

**Status:** complete / maintained

Implemented:

- public repository, MIT license, governance, security, support, and clean-room policy;
- C++20/CMake core library and CLI;
- Windows/Ubuntu CI;
- presets, warning policy, formatting, and test invariants;
- Constitution, SDD specifications, ADRs, architecture, history, brand, and Canon documents.

Exit gate: satisfied. The foundation must remain green as later systems evolve.

## Phase 1 — Evidence and resource contracts

**Status:** complete at foundation level / expanding by subsystem

Implemented:

- artifact SHA-256 identity;
- confidence model;
- Evidence records, locations, registry, tags, and supersession;
- versioned Evidence Packets;
- deterministic export and strict untrusted import;
- `ResourceId`, `ResourceRef`, `ResourcePayload`;
- diagnostics, source registry, classifier, graph, router, and typed bundles;
- public packets for selected executable, Item, and save findings.

Remaining work belongs to subsystem-specific evidence promotion, not the base contract.

Exit gate: satisfied for versioned schemas and stable public APIs.

## Phase 2 — Read-only platform and EXE inspection

**Status:** substantially implemented

Implemented:

- bounded binary reader;
- generic PE32/PE32+ parser;
- section and header models;
- checked file offset/RVA/VA conversion;
- hash-gated target metadata;
- Evidence Address Resolver;
- executable workspace manifests;
- EXE reopen lineage by SHA-256;
- synthetic malformed PE fixtures and CLI inspection.

Still open:

- broad recovered function/type database;
- full decompilation coverage;
- behavior-tested recovered subsystems.

Exit gate for safe user-supplied executable inspection: satisfied.

## Phase 3 — Container and format foundation

**Status:** active

Implemented:

- generic container document/entry/result contracts;
- parser registry consuming byte spans only;
- stable child and slot identity;
- empty-slot preservation;
- fallback names and diagnostics;
- GDSpaces child exposure and graph edges;
- child magic reclassification;
- synthetic legal container fixtures.

Still open:

- production evidence-bounded PAC/PNST parser subset;
- NBZ/AFS source contracts and nested exposure;
- `.index` metadata linking;
- real sanitized corpus reports;
- production writer/repack support.

Exit gate: one production read-only archive path must expose nested resources through GDSpaces without tool-local loading.

## Phase 4 — Stage identity and StageBundle

**Status:** active / model and workspace implemented

Implemented:

- confirmed 110 × 4 stage-table descriptor;
- stage identity and four canonical resource roles;
- `st001` resource plan;
- path normalization and deterministic resource matching;
- typed `StageBundleAssembler`;
- `DMC3StageWorkspaceBuilder`;
- shared Stage Workspace and deterministic manifests;
- partial-failure and ambiguity diagnostics.

Still open:

- resolve a complete `st001` bundle from legally supplied game files through production container sources;
- record deterministic game-backed local reports;
- prove all consumers reuse one canonical resource identity.

Exit gate: Stage Ops consumes one complete game-backed typed bundle without resolving files independently.

## Phase 5 — Binary Inspector core

**Status:** substantially implemented at domain level

Implemented:

- structural regions and typed fields;
- parent-child structures;
- ownership claims;
- annotations and evidence links;
- owner/field/annotation selection lookup;
- coverage, unknown gaps, structural conflicts, and ownership conflicts;
- deterministic manifest export;
- format adapters including HITS.

Still open:

- complete desktop hex UI;
- interactive diff and entropy views;
- broader format schemas and EXE navigation UI.

Exit gate for shared resource/evidence/ownership domain: satisfied. UI completion remains separate.

## Phase 6 — Working copy, guarded patching, and source integration

**Status:** substantially implemented / production export pending

Implemented:

- immutable source payload and revisioned `WorkingCopy`;
- expected-byte operations, history, reset, and undo;
- guarded patch plans with source hash, range, expected-byte, and overlap validation;
- evidence-gated patch compiler;
- in-memory copied-output execution;
- output SHA-256 and verified rollback;
- deterministic manifests and Project Graph provenance;
- `SourceModificationPackage` and `IntegrationProject`;
- dependency/conflict analysis;
- `CustomBuildIdentity`, `CustomBuildRecord`, source-to-binary mappings, and EXE reopen lineage.

Still open:

- production output-file export contract;
- container repack integration;
- reproducible signed public releases;
- an actual rebuilt DMC3 executable.

Exit gate for safe in-memory guarded modification and provenance: satisfied. Production export gate remains open.

## Phase 7 — Stage Ops and format-system integration

**Status:** active / substantial modules implemented

Implemented C++ modules and tests include:

- canonical HITS parser/runtime/safe-edit/writer/comparison stack;
- DCA;
- LIG2;
- Stage TXT;
- resource analysis;
- Stage Workspace builder and shared Stage Ops/ModViz views.

HITS has advanced beyond a parser into a tested vertical slice, but Capcom offline-builder equivalence remains unproven.

Still open:

- game-backed stage integration across production archive sources;
- complete format editors and desktop Stage Ops workspace;
- CAM and broader effect/model/collision links;
- runtime and restart/reload validation.

Exit gate: selected stage formats operate end-to-end through GDSpaces, shared workspace state, working copies, validation, and guarded export.

## Phase 8 — ModViz

**Status:** architecture and shared-view contracts implemented; complete UI pending

Implemented foundation:

- shared Stage Workspace consumption;
- shared Stage Ops/ModViz tool views;
- tool and capability registry;
- resource/evidence provenance contracts.

Planned product areas:

- Scene/Model Editor;
- Menu/HUD Editor;
- hierarchy and properties;
- viewport/rendering layer;
- HUD digit/icon workflows;
- visual HITS and stage editing;
- source/evidence panels.

Exit gate: first visual edit travels through the existing working-copy, validation, and export contracts without adding a second resolver.

## Phase 9 — EXE source recovery

**Status:** active long-term research with selected product promotions

Research state:

- Drive reverse canon extends through Wide Pass 33;
- supporting whole-system work includes Phases 12–17;
- Recovered Source Skeleton v1.8 exists as research input.

Promoted into reviewed product C++:

- selected executable identity and address evidence;
- Item runtime evidence and patch boundaries;
- HITS runtime-derived specifications;
- DMC3 PC-save Pass 31 and Pass 32 ABI;
- source/build lineage contracts.

Product-promotion-pending:

- Wide Pass 33 payload partition and MissionResultMatrix;
- most recovered source units;
- broader runtime ownership and texture/resource systems.

Exit gate: one recovered subsystem compiles as reviewed C++, has explicit ABI/lifetime evidence, and passes behavioral comparison with the canonical executable.

## Phase 10 — Recompilation frontier

**Status:** long-term / architecture prepared, executable milestone not achieved

Implemented preparation:

- source modification packages;
- integration project state and dependency/conflict graph;
- custom build identity and records;
- compiler/linker/dependency identity;
- source-line/symbol to output-address mappings;
- test, release, attestation, revocation, rollback, and reopen lineage.

Still required:

- linkable behavior-tested recovered modules;
- runtime replacement/rebinding boundaries;
- deterministic composite source builds;
- first working rebuilt executable milestone;
- controlled game validation and release criteria.

Exit gate: defined only through evidence and behavior-tested build receipts. No recompilation-complete claim exists today.

## Current near-term sequence

```text
production read-only container subset
  → legal local game-backed st001 StageBundle
  → real HITS/stage corpus and runtime validation
  → narrow Pass 33 promotion
  → first behavior-tested recovered subsystem
  → controlled recompilation milestone
```

Complete UI and public release automation remain downstream and must consume the established domain, identity, provenance, and safety contracts.