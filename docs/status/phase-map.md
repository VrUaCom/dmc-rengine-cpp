# Phase Map

## Phase 0 — Public foundation

**Status:** active / substantially implemented

- repository, license, governance;
- C++20/CMake;
- CI;
- CLI;
- clean-room rules;
- architecture and brand Canon.

Exit: Windows/Linux builds pass and public repository policy is complete.

## Phase 1 — Evidence and resource contracts

**Status:** active / initial implementation complete

- confidence model;
- evidence records and registry;
- resource ID/ref/payload;
- diagnostics;
- source registry;
- graph;
- router;
- stage bundle.

Exit: serializable evidence packets, artifact hashing, stable API tests, and versioned schemas.

## Phase 2 — Read-only platform inspection

**Status:** planned next

- PE32+ reader;
- section model;
- offset/RVA/VA conversion;
- hash-gated known-target metadata;
- synthetic PE fixtures;
- sanitized reports.

Exit: CLI can inspect a user-supplied executable without modifying it.

## Phase 3 — Container and format foundation

**Status:** planned

- parser interfaces;
- PAC/PNST synthetic fixtures;
- NBZ/AFS source contracts;
- nested resource exposure;
- `.index` metadata linking;
- DDS child resources.

Exit: read-only vertical source/container/resource graph.

## Phase 4 — Stage identity and StageBundle

**Status:** model implemented, assembly planned

- EXE-backed stage table metadata;
- stage ID normalization;
- four-column stage resource mapping;
- typed categories;
- partial-failure diagnostics;
- local `st001` vertical test.

Exit: Stage Ops can consume one complete typed bundle without resolving files.

## Phase 5 — Binary Inspector core

**Status:** planned migration

- regions and fields;
- structure tree;
- ownership graph;
- unknown/conflict coverage;
- annotations;
- diff and entropy;
- EXE/resource bridge.

Exit: same resource identity and evidence chain visible from GDSpaces through Binary Inspector.

## Phase 6 — Working copy and Patch Engine

**Status:** planned

- immutable source payload;
- editable working copy;
- operations and validation;
- guarded patch plans;
- conflicts/dependencies;
- manifests, backups, rollback;
- export policy.

Exit: no direct original-data writes and repeatable patch artifacts.

## Phase 7 — Stage Ops and format editor migration

**Status:** planned

- TXT, DCA, LIG2, HITS$, CAM;
- model/collision/effect links;
- Stage Ops workspace;
- evidence-aware editors.

Exit: selected historical editors work through GDSpaces and StageBundle.

## Phase 8 — ModViz

**Status:** product architecture confirmed, implementation planned

- Scene/Model Editor;
- Menu Editor;
- hierarchy and properties;
- viewport/rendering layer;
- HUD/digit/icon workflows;
- source/evidence panel.

Exit: first end-to-end visual edit through working copy and guarded export.

## Phase 9 — EXE source recovery

**Status:** long-term active research

- function/data database;
- recovered types and units;
- ABI and ownership validation;
- compile isolation;
- behavioral tests;
- incremental replacement strategy.

Exit: first recovered subsystem compiles and passes behavioral comparison.

## Phase 10 — Recompilation frontier

**Status:** long-term

- linkable recovered modules;
- runtime rebinding;
- replacement boundaries;
- deterministic builds;
- working rebuilt executable milestones.

Exit: defined separately through evidence and release criteria; no completion claim exists today.
