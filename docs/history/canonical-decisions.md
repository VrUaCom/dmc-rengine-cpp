# Canonical Project Decisions

This document records architecture decisions that are treated as current project law until explicitly replaced through governance and a reviewed specification.

## CD-001 — GDSpaces is the single resource API

**Status:** confirmed, active

All tools consume resources through GDSpaces contracts. GDSpaces owns:

- source mounting;
- game-folder scanning;
- NBZ/AFS/PAC/PNST access and expansion;
- namespaces and logical paths;
- resource identity and display identity;
- classification;
- resource graph;
- diagnostics;
- evidence context;
- OpenRouter;
- typed stage bundles;
- working-copy and future write policy.

Tools own format-specific interpretation, visualization, editing, and validation.

## CD-002 — Containers are internal layers

**Status:** confirmed, active

PAC, PNST, NBZ, and AFS are formats inside the resource architecture. They are not top-level products and do not define editor boundaries.

A stage PAC and a non-stage PAC receive the same generic container treatment. Stage semantics are added through stage identity and `GDStageBundle`.

## CD-003 — No second resolver

**Status:** confirmed, active

Stage Ops, ModViz, Item Editor, Binary Inspector, texture tooling, and future editors may not create independent path/container resolution systems.

Direct `FileSystemHandle`, raw path, or archive ownership in UI state is prohibited as canonical resource identity.

## CD-004 — Identity is layered

**Status:** confirmed, active

The project distinguishes:

- source identity;
- logical path;
- container chain;
- slot/index identity;
- byte offset and size;
- canonical resource ID;
- display name;
- synthetic fallback name;
- EXE-backed semantic identity.

Display names never replace canonical IDs.

## CD-005 — Evidence precedes Canon

**Status:** confirmed, active

Reverse-engineering conclusions must carry confidence and evidence. The accepted states are:

- hypothesis;
- candidate;
- low;
- medium;
- high;
- confirmed;
- corrected;
- rejected.

A correction preserves history rather than silently rewriting it.

## CD-006 — C++ is the permanent source-recovery center

**Status:** confirmed, active

EXE Editor is ultimately a source-recovery and recompilation environment. Decompiled C/C++ is the central durable representation; disassembly, hex, bytes, symbols, ownership, and evidence are supporting views.

Recovered source units are not described as original source and are not considered complete until ABI, ownership, behavior, and tests support them.

## CD-007 — Binary Inspector is an evidence consumer

**Status:** confirmed, active

Binary Inspector receives bytes, regions, resource identity, diagnostics, and evidence from GDSpaces or EXE services. It does not independently resolve sources.

## CD-008 — Stage Ops consumes typed bundles

**Status:** confirmed, active

Stage Ops receives `GDStageBundle`/`StageBundle`, not a list of raw files. A bundle can contain scripts, models, textures, animation, cameras, lighting, events, positions, effects, collision, sounds, and unknown resources.

Partial failures must not erase the rest of a stage bundle.

## CD-009 — ModViz has two product modes

**Status:** confirmed, active

ModViz contains:

1. Scene/Model Editor;
2. Menu Editor.

The Menu Editor handles HUD/menu models, hierarchy, screen-space layout, transforms, UV atlas regions, draw order, visibility, counter layouts, digit/icon slots, and runtime-value preview.

Executable behavior remains linked through EXE evidence and guarded patch plans.

## CD-010 — Writes are explicit and guarded

**Status:** confirmed, planned implementation

No tool silently overwrites original game content. The write path requires:

1. working copy;
2. edit operations;
3. validation;
4. conflict and capacity checks;
5. manifest;
6. explicit export/build;
7. backup/rollback information.

Executable patches additionally require source hash, source bytes, target bytes, address mapping, dependencies, rollback, and runtime tests.

## CD-011 — Public repository remains content-clean

**Status:** confirmed, active

The repository contains original implementation, documentation, evidence metadata, and synthetic fixtures. It does not contain proprietary game files, leaked source, or redistributed extracted assets.

## CD-012 — The clean C++ repository is not a blind port

**Status:** confirmed, active

Legacy functionality is migrated only when its responsibility, evidence, architecture fit, and tests are understood. Old code is not imported merely because it already exists.

## CD-013 — Knowledge systems have distinct responsibilities

**Status:** confirmed, active workflow

- Obsidian: human-readable project chronicle.
- Knowledge graph: machine-readable relations.
- MemPalace: long-term contextual memory.
- SDD: specifications and acceptance criteria.
- Git repository: implementation and public canonical documents.

No system silently overwrites another. Changes move through explicit events/proposals.

## CD-014 — Brand lore never overrides engineering language

**Status:** confirmed, active

The Order, monks, sect, chambers, and mottos are fictional brand language. APIs, schemas, tests, permissions, governance, and security use direct technical names.
