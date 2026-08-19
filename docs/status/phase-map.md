# Phase Map

**Snapshot date:** 2026-08-08  
**Reviewed `main` baseline before this documentation branch:** `6eb6a07975753e2bbe9414893a76e13c946fa78e`

The phases describe the evolution of the reviewed C++ repository and the accepted architecture direction. They do not turn planned work into implemented work.

## Phase 0 — Public C++ foundation

**Status:** complete / maintained

C++20/CMake, Windows/Ubuntu CI, governance, security, clean-room rules, warning policy, tests, Constitution, SDD, and public documentation are established.

## Phase 1 — Evidence and resource contracts

**Status:** foundation complete / expanding

Implemented artifact identity, confidence states, Evidence Packets, strict import/export, resource IDs/references/payloads, graph/routing/classification, typed bundles, and working-copy foundations.

Current expansion target: GDSpaces Resource Identity v1 across production container-backed representations.

## Phase 2 — Read-only platform and EXE inspection

**Status:** substantially implemented

Implemented bounded binary reading, PE32/PE32+ parsing, offset/RVA/VA conversion, canonical executable identity, Evidence Address Resolver, executable workspace/reopen lineage, and selected promoted runtime evidence.

Open: broad function/type recovery and behavior-tested recovered subsystems.

## Phase 3 — Production container/read path

**Status:** active

Generic read-only container contracts and synthetic legal fixtures are implemented.

Open:

- evidence-bounded production PAC/PNST subset;
- NBZ/AFS exposure through GDSpaces;
- nested real-resource classification;
- `.index` metadata linking without treating it as runtime truth;
- deterministic local integration reports.

**Exit gate:** one production read-only archive path exposes nested resources through GDSpaces without tool-local loading.

## Phase 4 — Stage identity and game-backed StageBundle

**Status:** active

Implemented 110 x 4 stage-table descriptor, `st001` role plan, matching, typed StageBundle assembly, Stage Workspace, and shared Stage Ops/ModViz views.

Open: complete legal local `st001` assembly through production sources and proof of canonical identity reuse by every consumer.

**Exit gate:** Stage Ops consumes one complete/diagnostic game-backed typed bundle without resolving files independently.

## Phase 5 — Binary Inspector domain

**Status:** Wave 1 implemented / Wave 2 active

Implemented regions, typed fields, ownership, annotations, selection/range context, coverage/unknown/conflicts, deterministic manifests, byte diff, and entropy analysis.

Wave 2:

- artifact-keyed Analysis Cache;
- generic diagnostics;
- unknown-region analysis;
- versioned templates;
- deterministic analysis-result export.

Following Wave 2: Reverse Core bridge and EXE address/guarded-patch bridge.

## Phase 6 — Working copy, guarded patching, and source/build lineage

**Status:** substantially implemented / production export pending

Implemented revisioned working copies, expected-byte guards, guarded patch plans, copied-output execution, verified rollback, source modification packages, integration project state, Custom Build identity, and source-to-output mappings.

Open: production output-file/export contracts, deterministic composite source builds, signed public release pipeline, and a working rebuilt executable.

## Phase 7 — Stage Ops semantic integration

**Status:** active

Implemented format/domain work includes HITS, DCA, LIG2, Stage TXT, stage workspace building, and shared Stage Ops/ModViz state.

Next direction is Stage Semantic Graph v1:

`Stage -> Room -> Geometry -> Collision -> Lighting -> Camera -> Door/Transition -> Event -> Effect -> Audio -> Runtime reference`.

HITS original-builder equivalence remains research-required pending real corpus and controlled runtime validation.

## Phase 8 — ModViz

**Status:** architecture/shared-view foundation implemented; complete UI pending

Two canonical modes:

1. Scene/Model Editor;
2. Menu Editor.

First Menu Editor gate: Red Orb HUD counter vertical slice through GDSpaces identity, working-copy edit, linked EXE evidence/guarded runtime request, preview, validation, and export.

## Phase 9 — EXE source recovery

**Status:** active long-term research with selected product promotions

Promoted foundations include executable identity/address evidence, Item runtime boundaries, HITS runtime-derived specifications, PC-save Pass 31/32 ABI, and source/build provenance contracts.

Research outside reviewed product source still includes Wide Pass 33, most recovered-source skeleton units, and broader runtime/resource/texture findings.

**Exit gate:** one recovered subsystem compiles as reviewed C++, has explicit ABI/lifetime evidence, and passes behavioral comparison with the canonical executable or a controlled equivalent boundary.

## Phase 9A — Reverse Core v0.1

**Status:** canonical architecture accepted / implementation pending

Reverse Core introduces reusable game-agnostic identities and workflow for:

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

It must reuse existing SDD, MCP/Kanban, evidence, Obsidian, MemPalace, and GitHub roles rather than create a competing workflow stack.

Required implementation gates:

1. schema v0.1;
2. TaskClaim ownership protocol;
3. recovered-source tree/export contract;
4. Binary Inspector bridge;
5. EXE Editor bridge;
6. deterministic provenance manifests.

**Exit gate:** one real DMC3 subsystem can complete the reverse lifecycle without DMC-specific concepts leaking into the reusable layer.

## Phase 9B — First compilable reconstruction island

**Status:** planned critical milestone

```text
canonical bytes
  -> stable reverse identities
  -> evidence-backed functions/data/types
  -> reviewed C++ unit
  -> isolated build
  -> behavioral comparison
  -> ValidationReceipt
  -> Canon promotion/correction/rejection
```

This milestone precedes mass decompilation.

## Phase 10 — Recompilation frontier

**Status:** long-term / architecture prepared, executable milestone not achieved

Required:

- multiple behavior-tested recovered modules;
- replacement/rebinding boundaries;
- deterministic composite builds;
- source-to-output address provenance;
- runtime regression receipts;
- first working rebuilt executable only when evidence supports the claim.

## Current critical sequence

```text
production GDSpaces read path + Resource Identity v1
  -> legal local game-backed st001 StageBundle
  -> Reverse Core v0.1 schema/claims/source tree
  -> Binary Inspector + EXE Editor Reverse Core bridges
  -> first behavior-tested recovered subsystem
  -> broader stage/runtime semantic validation
  -> controlled recompilation milestones
```

Parallel work continues on Binary Inspector Wave 2, HITS real-corpus/runtime validation, narrow Wide Pass 33 promotion, and the ModViz Red Orb vertical slice when shared identity/evidence contracts are ready.
