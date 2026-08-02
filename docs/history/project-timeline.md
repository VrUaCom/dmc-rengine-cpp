# DMC Rengine Project Timeline

This timeline records the known evolution of DMC Rengine before and during the clean C++ repository. Dates describe project milestones and research snapshots, not release guarantees.

## Before the C++ repository

### 2026-06-18 — Resource-runtime investigation begins

The project began by investigating how the Devil May Cry 3 HD executable discovers and consumes game resources.

Early work established that the resource system cannot be modeled as a single PAC editor. The game uses layered sources and containers, including NBZ, AFS, PAC, and PNST, with logical identities that are not always equivalent to physical filenames.

### 2026-06 — DMC Rengine identity is established

The project name became **DMC Rengine**: Devil May Cry 3: Special Edition – HD Collection Decompilation & Recompilation Engine.

The goal expanded from format editing to:

- executable decompilation and source recovery;
- typed resource discovery and editing;
- stage reconstruction;
- safe mod packaging;
- future recompilation of working engine modules;
- evidence-backed reconstruction rather than undocumented byte patching.

### 2026-06 — GDSpaces becomes the central resource architecture

A canonical decision replaced container-centric tooling with **GDSpaces**:

- one virtual resource space;
- one source registry;
- one resource identity model;
- container expansion below the tool layer;
- tools as clients of `GDSpacesAPI`/`GDOpenRouter`;
- stage grouping through typed bundles rather than raw path passing.

This decision deprecated PAC Editor/PAC Manager as top-level architecture. PAC remains a supported internal container format.

### 2026-06 to 2026-07 — Binary Inspector matures

Binary Inspector evolved from a hex-oriented utility into an evidence and structure environment.

Recorded capabilities included:

- Structure Tree;
- Ownership Map and Ownership Tab;
- coverage, unknown regions, and conflicts;
- selection ownership;
- analysis cache;
- canonical container parsing;
- diagnostics;
- `.index` attachment;
- slot-preserving export experiments;
- manifest output and duplicate-offset warnings.

Future work remained Field Inspector, diff, entropy, RVA/VA resolution, annotations, templates, unknown analysis, EXE bridge, and patch safety.

### 2026-06 to 2026-07 — EXE reverse-engineering phases

The project accumulated confirmed and high-confidence findings about the HD executable, including PE identity, stage resource tables, TXT helpers, StageSet token classification, door/box parsing candidates, resource I/O, container runtime, and texture architecture.

Phases 12–16 produced increasingly structured analysis artifacts and recovered C/C++ seed units. These units were research seeds, not yet a complete recompilable executable.

### 2026-07 — Stage Ops format editors

Stage-focused tools were developed or specified for:

- DCA;
- LIG2;
- HITS$;
- TXT stage scripts;
- CAM;
- SCM/model overlays;
- stage grouping and visualization.

The architecture later clarified that Stage Ops must receive `GDStageBundle`, not resolve raw sources itself.

### 2026-07 — Item Editor reaches a practical baseline

Item Editor became one of the most functional subsystems, with standalone and hub integration, schema migration, validation, item library, guarded executable changes, inventory-limit work, mod packaging, backup, manifests, and user tests against several item IDs.

Its success also exposed architectural risk: editor-specific runtime and file logic can outrun the shared GDSpaces and Patch Engine contracts.

### 2026-07 — ModViz becomes a dual-mode product

ModViz was defined with two top-level modes:

1. Scene/Model Editor;
2. Menu Editor for HUD/menu/UI resources.

The Menu Editor's first target was the red-orb counter resource family. Runtime limits and formatting belong to EXE evidence and guarded patch planning, while visual model/layout work belongs to ModViz.

### 2026-07-26 — MCP, memory, and SDD integration

MCP Space DMC Rengine, MemPalace, Obsidian, a knowledge graph, and GitHub Spec Kit were integrated into a single project workflow.

Canonical process roles:

- Obsidian: human-readable chronicle;
- graph: relationships;
- MemPalace: long-term context;
- SDD: controlled specifications and acceptance criteria;
- event/change journal: append-only workflow history.

The first managed specification focused on item Y rotation.

## Clean C++ generation

### 2026-08-02 — Public C++ repository created

Repository: `VrUaCom/dmc-rengine-cpp`.

The new repository intentionally did not copy the legacy implementation wholesale. It began with:

- C++20;
- CMake;
- cross-platform CI;
- a minimal CLI;
- a stable `ResourceId` seed;
- architecture and roadmap documentation;
- proprietary-content exclusions.

### 2026-08-02 — Public brand canon established

The public C++ era introduced a structured lore layer:

- The Order of the Inverted Triangle;
- Monks of Reverse;
- The Sect of Neuroslop;
- The Long Descent;
- The Heresy of the Second Resolver;
- subsystem marketing aliases.

The lore is explicitly fictional and must not replace technical naming or governance.

### 2026-08-02 — Governance and clean-room policy added

The repository gained:

- MIT license;
- contribution guide;
- code of conduct;
- security policy;
- governance and maintainer policy;
- support policy;
- clean-room/content rules;
- issue and pull-request templates.

### 2026-08-02 — First functional C++ architecture slice

The repository moved beyond a placeholder identity type and added:

- confidence model;
- evidence locations and records;
- `EvidenceRegistry`;
- GDSpaces diagnostics;
- `ResourceRef` and `ResourcePayload`;
- abstract source interface;
- safe read-only `LocalDirectorySource`;
- `SourceRegistry`;
- `ResourceGraph`;
- `OpenRouter`;
- typed `StageBundle`;
- CLI doctor/scan/route commands;
- synthetic integration tests.

## Current direction

The next stages are:

1. stabilize the foundation through CI and tests;
2. add artifact hashing and serializable evidence packets;
3. implement read-only PE inspection;
4. build synthetic container fixtures and parser interfaces;
5. create EXE-backed stage identity data without shipping game content;
6. implement working-copy and guarded patch contracts;
7. begin UI shell and Spider Hub only after the core contracts are stable.
