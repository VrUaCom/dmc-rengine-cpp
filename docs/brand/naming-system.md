# DMC Rengine Naming System

This document defines the public-facing product and subsystem names for the C++ generation of DMC Rengine.

## Master Brand

### DMC Rengine

**Expanded name:** Devil May Cry 3: Special Edition – HD Collection Decompilation & Recompilation Engine.

**Positioning:** An open-source C++20 research, decompilation, editing, and recompilation framework for Devil May Cry 3 HD.

**Primary line:** Reverse the engine. Rebuild the possibilities.

**Technical line:** Evidence-backed reverse engineering from resource identity to recompilable systems.

**Lore line:** Descend to the bytes. Return with the source.

## Core Platform Names

### GDSpaces — The Archive

The single resource access API and virtual resource space.

Responsibilities include source mounting, game-folder scanning, NBZ/AFS/PAC/PNST expansion, path normalization, resource identity, classification, graph construction, diagnostics, evidence context, typed bundles, working copies, and future safe write policy.

### Spider Hub — The Nexus

The visual hub connecting tools, resources, evidence, scenes, executable findings, specifications, and project memory.

### EXE Editor — The Scriptorium

The executable research and source-recovery environment.

It owns PE analysis, disassembly evidence, recovered C/C++, function and data annotations, RVA/VA mapping, guarded patch plans, and the future compile/rebuild pipeline.

### Binary Inspector — The Reliquary

The evidence-oriented binary structure environment.

It owns byte regions, structure trees, ownership maps, field inspection, diffs, unknown regions, annotations, and diagnostics. It does not independently resolve game sources.

### Stage Ops — The Theatre

The stage and scene operations workspace.

It receives typed `GDStageBundle` objects and coordinates stage scripts, models, cameras, lighting, effects, collision, sound, events, positions, and unknown resources.

### ModViz — The Observatory

The visual editing and reconstruction environment.

Top-level modes:

- **Scene/Model Editor** — 3D scene, model, transform, hierarchy, and resource visualization.
- **Menu Editor** — HUD, menu, digit meshes, icon slots, screen-space layout, UV regions, draw order, and runtime-value preview.

### Item Editor — The Forge

The typed item-authoring workspace for ITM resources, item libraries, runtime-linked limits, guarded EXE changes, validation, and mod packaging.

### Build & Test Lab — The Trial Chamber

The controlled environment for builds, patch validation, fixture tests, runtime tests, regression checks, reproducibility, and artifact reports.

## Knowledge and Process Names

### Reverse Canon

The versioned body of accepted project knowledge.

### Evidence Registry

The index connecting claims to hashes, offsets, bytes, traces, tests, recovered symbols, and runtime observations.

### MemPalace — The Long Memory

Long-term AI-assisted project memory.

### Obsidian Vault — The Human Chronicle

Human-readable project notes, journals, architecture pages, and research narratives.

### Knowledge Graph — The Web of Relations

Machine-usable relationships between resources, formats, executable functions, tools, specifications, evidence, and decisions.

### SDD Library — The Book of Specifications

The collection of controlled software design specifications, proposals, acceptance criteria, and compilation packets.

### Event/Change Journal — The Chronicle

The append-only record of decisions, corrections, migrations, tests, and architecture changes.

## Research Campaign Names

### The Long Descent

The overall decompilation and recompilation journey.

### Archive Mapping

The campaign to identify sources, containers, logical paths, resource identities, and dependencies.

### Function Recovery

The campaign to recover executable behavior into evidence-backed source units.

### Stage Reconstruction

The campaign to reconstruct complete stages through EXE-backed identities and `GDStageBundle`.

### Runtime Rebinding

The campaign to connect edited resources and recovered systems to guarded runtime behavior.

### Recompilation Frontier

The phase where isolated recovered systems become compilable, linkable, and behaviorally testable.

## Naming Rules

1. Product names describe responsibilities, not file extensions.
2. Container names never become top-level architecture names.
3. Marketing aliases may appear beside technical names, never instead of them in API contracts.
4. One subsystem has one canonical public name.
5. Experimental names must be labeled experimental.
6. Old PAC Editor/PAC Manager terminology is retained only in historical migration documents.
