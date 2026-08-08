# Canonical Project Decisions

This document records architecture decisions that are treated as current project law until explicitly replaced through governance and a reviewed specification.

## CD-001 — GDSpaces is the single resource API

**Status:** confirmed, active

All tools consume resources through GDSpaces contracts. GDSpaces owns source mounting, game-folder scanning, NBZ/AFS/PAC/PNST access and expansion, logical/resource identity, classification, graph, diagnostics, evidence context, OpenRouter, typed bundles, and working-copy policy.

Tools own format-specific interpretation, visualization, editing, and validation.

## CD-002 — Containers are internal layers

**Status:** confirmed, active

PAC, PNST, NBZ, and AFS are formats inside the resource architecture. They are not top-level products and do not define editor boundaries. Stage semantics are added through stage identity and typed bundles.

Legacy PAC Editor/PAC Manager logic is intentionally excluded from the current product architecture.

## CD-003 — No second resolver

**Status:** confirmed, active

Stage Ops, ModViz, Item Editor, Binary Inspector, EXE Editor, texture tooling, Reverse Core, and future editors may not create independent game path/container resolution systems.

## CD-004 — Identity is layered

**Status:** confirmed, active

The project distinguishes source identity, logical path, container chain, slot/index identity, byte offset/size, canonical resource ID, display identity, synthetic fallback names, artifact hash, and EXE-backed semantic identity.

Display names never replace canonical IDs.

## CD-005 — Evidence precedes Canon

**Status:** confirmed, active

Accepted confidence/correction states are:

`hypothesis`, `candidate`, `low`, `medium`, `high`, `confirmed`, `corrected`, `rejected`.

A correction preserves history rather than silently rewriting it.

## CD-006 — C++ is the permanent source-recovery center

**Status:** confirmed, active

EXE Editor is ultimately a source-recovery and recompilation environment. Decompiled/recovered C++ is the durable representation; disassembly, hex, bytes, symbols, ownership, and evidence are supporting views.

Recovered source is not original Capcom source and is not considered complete from readability alone.

## CD-007 — Binary Inspector is an evidence consumer and structural producer

**Status:** confirmed, active

Binary Inspector receives bytes, identity, diagnostics, and evidence from GDSpaces or executable services. It produces structural ranges, typed fields, ownership, annotations, diff/entropy/diagnostics, and may bridge those observations into Reverse Core. It does not independently resolve game sources or become a second canonical reverse database.

## CD-008 — Stage Ops consumes typed bundles

**Status:** confirmed, active

Stage Ops receives `GDStageBundle`/`StageBundle`, not a list of raw files. Partial failures and unknown resources must not erase the rest of the stage.

## CD-009 — ModViz has two product modes

**Status:** confirmed, active

ModViz contains:

1. Scene/Model Editor;
2. Menu Editor.

The Menu Editor handles HUD/menu models, hierarchy, screen-space layout, transforms, UV atlas regions, draw order, visibility, counter layouts, digit/icon slots, and runtime-value preview. Executable behavior remains linked through EXE evidence and guarded patch requests.

The first canonical Menu Editor vertical slice is the Red Orb HUD counter.

## CD-010 — Writes are explicit and guarded

**Status:** confirmed, active architecture / implementation varies by subsystem

No tool silently overwrites original game content. The write path requires working copy, edit operations, validation, conflict/capacity checks, manifest, explicit export/build, and backup/rollback information.

Executable patches additionally require exact artifact identity, source bytes, target bytes, address mapping, dependencies, rollback, and runtime/behavior evidence appropriate to the claim.

## CD-011 — Public repository remains content-clean

**Status:** confirmed, active

The repository contains original implementation, documentation, evidence metadata, and synthetic fixtures. It does not contain proprietary game files, leaked source, or redistributed extracted assets.

## CD-012 — The clean C++ repository is not a blind port

**Status:** confirmed, active

Legacy functionality is migrated only when responsibility, evidence, architecture fit, and tests are understood. Old code is not imported merely because it already exists.

## CD-013 — Knowledge systems have distinct responsibilities

**Status:** confirmed, active workflow

- Obsidian: human-readable project chronicle;
- knowledge graph: machine-readable relations;
- MemPalace: long-term contextual memory;
- SDD/Spec Kit: specifications and acceptance criteria;
- MCP/Kanban: execution coordination, events, and future claims;
- Git repository: reviewed implementation and public canonical documents.

No system silently overwrites another. Changes move through explicit events/proposals/review.

## CD-014 — Brand lore never overrides engineering language

**Status:** confirmed, active

Lore names and community branding never override APIs, schemas, tests, permissions, evidence states, governance, security, or legal language.

## CD-015 — Triangle Forge is the broader platform context; DMC Rengine remains a separate workspace

**Status:** confirmed, active architectural direction

Triangle Forge may host reusable services and additional workspaces in the future. DMC Rengine remains an independent DMC-focused project/workspace with its own domain models and release truth.

This naming/platform direction does not authorize a monolithic rewrite or automatic renaming of existing internal modules.

## CD-016 — Reverse Core is a reusable game-agnostic subsystem

**Status:** confirmed architecture direction / implementation pending

Reverse Core is built on top of the existing MCP/SDD/evidence/coordination environment rather than replacing it.

Its canonical objects are generic reverse-engineering concepts such as BinaryArtifact, AddressRange, Function, DataObject, RecoveredType, EvidenceRecord, Hypothesis, Experiment, TaskClaim, Reconstruction, ValidationReceipt, and Subsystem.

DMC-specific concepts must remain outside Reverse Core.

## CD-017 — Canonical reconstruction mutation requires ownership coordination

**Status:** confirmed architecture direction / implementation pending

Parallel agents or contributors may independently inspect the same evidence when intentionally requested, but canonical mutation of the same function/range/type/subsystem/source unit requires a `TaskClaim`/ownership protocol.

Claims coordinate work and prevent races. They do not constitute Evidence and do not raise confidence.

## CD-018 — Recovered source must preserve binary provenance and remain exportable as ordinary C++

**Status:** confirmed architecture direction / implementation pending

Every recovered source unit must preserve links to exact binary artifact/ranges, evidence, reconstruction revision, ABI/lifetime assumptions, and validation receipts.

Recovered source must be exportable as a normal C++ project suitable for VS Code/CMake or equivalent tooling; compilation must not depend on the editor UI being the compiler.

## CD-019 — First complete reconstruction loop precedes mass decompilation

**Status:** confirmed milestone rule

The project will not treat function-count growth as the primary decompilation success metric before proving one real subsystem through:

`binary -> stable reverse identities -> evidence-backed recovered C++ -> isolated build -> behavioral comparison -> ValidationReceipt -> Canon update`.

This vertical proof is the gate before scaling broad automated recovery.

## CD-020 — Stage Ops evolves through a shared semantic graph, not isolated format silos

**Status:** confirmed architecture direction / implementation pending

The target relation model is:

`Stage -> Room -> Geometry -> Collision -> Lighting -> Camera -> Door/Transition -> Event -> Effect -> Audio -> Runtime reference`.

All nodes retain GDSpaces identity; inferred edges retain confidence/evidence; unknown resources remain first-class nodes.

## CD-021 — GDSpaces Resource Identity v1 must survive physical representation changes

**Status:** confirmed milestone rule

A logical resource must not receive unrelated canonical identities merely because it is seen through filesystem, NBZ, AFS, PAC, PNST, extracted working-copy, or EXE-backed references. Path and display identity remain separate from canonical `ResourceId`.
