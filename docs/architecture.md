# Architecture

## Mission

DMC Rengine is a C++20 reverse-engineering, decompilation, editing, and future recompilation framework for Devil May Cry 3 HD.

## Core rule

GDSpaces is the single resource authority. No editor may independently mount game sources, resolve paths, classify formats, expand containers, or invent competing resource identities.

## Layers

1. **Sources** — local game folders and read-only mounted archives.
2. **GDSpaces** — source mounting, resolution, classification, container expansion, resource graph, diagnostics, evidence context, and working-copy orchestration.
3. **Domain services** — EXE evidence, patch planning, stage bundling, model and UI representations.
4. **Tools** — Binary Inspector, Stage Ops, ModViz, Item Editor, and future format editors.
5. **Export** — validated patch plans and explicit user-owned output copies.

## Canonical tool boundaries

- **EXE Editor** owns PE analysis, RVA/VA mapping, decompilation evidence, recovered C++ units, and guarded patch requests.
- **Binary Inspector** receives bytes and structural regions; it is not a source resolver.
- **Stage Ops** receives `GDStageBundle` objects, never loose paths as its primary contract.
- **ModViz** owns scene/model and menu/HUD editing views, not container access.
- **Item Editor** owns item semantics and editing, not game-source discovery.

## Container policy

PAC, PNST, NBZ, and AFS are implementation details inside GDSpaces. Legacy PAC Editor/PAC Manager logic is not part of the top-level architecture.

## Safety policy

All edits operate on working copies. Writes require validation, source hashes, expected bytes, conflict checks, output manifests, rollback information, and explicit export targets.
