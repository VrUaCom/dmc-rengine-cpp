# DMC Rengine Naming System

This document defines public-facing product, community, lore, and subsystem names for DMC Rengine C++.

## Master Brand

### DMC Rengine

**Expanded name:** Devil May Cry 3: Special Edition – HD Collection Decompilation & Recompilation Engine.

**Positioning:** An open-source C++20 research, decompilation, editing, and recompilation framework for Devil May Cry 3 HD.

**Primary line:** Reverse the engine. Rebuild the possibilities.

**Technical line:** Evidence-backed reverse engineering from resource identity to recompilable systems.

**Lore line:** Descend to the bytes. Return with the source.

**Community authorship line:** Built by the Sect of Neuroslop and the Monks of Binary Code.

## Community and Contributor Names

### Sect of Neuroslop — DMC Rengine Community

The **Sect of Neuroslop** (`Секта Нейрошлаку`) is the public community identity of DMC Rengine.

It includes users, researchers, modders, programmers, artists, testers, writers, supporters, and contributors gathered around the project.

It is not an AI-only wing, a permission system, a verification tier, or a second hidden organization.

### Monks of Binary Code — Creators and Recognized Core Contributors

The **Monks of Binary Code** (`Монахи Бінарного Коду`) are the creators and evidence-backed core contributors of DMC Rengine.

The title is derived from accepted contribution records and does not itself grant repository, release, signing, moderation, or administration rights.

### Order of the Inverted Triangle — Core Team Alias

The **Order of the Inverted Triangle** is a lore-facing alias for the core DMC Rengine Team.

It is not the whole community. The wider community is the Sect of Neuroslop.

### Monastery of Binary Code — Core Development Hub

The **Monastery of Binary Code** is the lore-facing name for the DMC Rengine Core Development Hub.

It is a presentation alias for the core team's development workspace, not a separate Team or authority domain.

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

These platform aliases are Chambers of the Monastery. They are not Teams, Orders, ranks, or independent sources of truth.

## Knowledge and Process Names

### Reverse Canon

The versioned body of accepted, corrected, rejected, and research-required project knowledge.

### Evidence Registry

The index connecting claims to hashes, offsets, bytes, traces, tests, recovered symbols, runtime observations, and supersession records.

### The Red Thread

The provenance path from Artifact → Hash → Evidence → Finding → Code → Test → Build.

### The Black Ledger

A public/read-model view over corrected and rejected findings, failed experiments, and superseded assumptions.

### The Codex of Corrections

A curated public explanation of major technical corrections.

### Witness Seal

A presentation badge that displays an exact underlying Evidence status and artifact scope. It never creates a new confirmation level.

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

The overall reverse-engineering and source-recovery journey:

Surface → Behavior → Resources → Structures → Ownership → Functions → Source → The Return.

### The Sealed Archives Campaign

Production read-only container and archive research.

### The Theatre Reconstruction

Stage identity, StageBundle, stage behavior, and scene reconstruction work.

### The Geometry Beneath

SCM, MOD, HITS, collision, mesh, and geometric-format research.

### The Memory of the Game

Save, persistence, state, and memory-card architecture.

### The Scriptorium Campaign

EXE analysis, recovered types, ABI, ownership, functions, and source units.

### The Red Orb Chronicle

HUD, Menu Editor, atlas, digit-mesh, and runtime counter work.

### The Return of Source

Source modification, integration, custom builds, behavioral comparison, and recompilation milestones.

Campaign names are labels over existing Projects, roadmaps, or publication series. They do not create new authority or workflow state.

## Ritual Names

These are optional presentation aliases for existing workflows:

- **The Descent** — opening a bounded research question;
- **The Witnessing** — capturing artifacts, bytes, traces, and observations;
- **The Binding** — linking findings to Evidence, code, graph, ownership, and tests;
- **The Trial** — CI, malformed-input, differential, runtime, or release validation;
- **The Promotion** — moving a finding into reviewed implementation and Canon;
- **The Reconciliation** — synchronizing implementation, research, docs, issues, and receipts;
- **The Closure** — recording exit criteria and residual gaps;
- **The Return** — behavior-tested recovered source entering a controlled build.

Professional workflow names remain canonical in APIs and stored records.

## Naming Rules

1. Product names describe responsibilities, not file extensions.
2. Container names never become top-level architecture names.
3. The Sect names the community; it is not an AI-only subgroup.
4. Monk of Binary Code is evidence-backed contributor recognition, not a permission role.
5. The Order of the Inverted Triangle names the core Team, not the entire community.
6. Tool aliases are Chambers, not additional Orders or authorities.
7. Marketing aliases may appear beside technical names, never instead of them in API contracts.
8. One subsystem has one canonical professional name.
9. Experimental names must be labeled experimental.
10. Legal, security, moderation, recovery, payment, and destructive-operation language remains direct and professional.
11. Old PAC Editor/PAC Manager terminology is retained only in historical migration documents.
12. Public lore must remain self-contained and must not imply technical claims beyond the available evidence.
