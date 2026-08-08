# Architecture

**Snapshot date:** 2026-08-08

## Mission

DMC Rengine is a C++20 reverse-engineering, decompilation, editing, and future recompilation framework for Devil May Cry 3 HD. It is also the first domain workspace intended to validate the reusable Reverse Core architecture.

DMC Rengine remains its own project/workspace. The broader Triangle Forge direction is a platform-level context, not permission to collapse DMC-specific domain services into a monolith.

## Core laws

1. GDSpaces is the single game-resource authority.
2. No claim without evidence.
3. Recovered source preserves provenance to exact binary identity and validation state.
4. No original-file write without an explicit working-copy/export contract.
5. Shared reverse infrastructure must remain game-agnostic; DMC semantics remain in the DMC Rengine workspace.

## Platform layering

```text
Triangle Forge platform context
  -> shared workflow / MCP / SDD / evidence / coordination services
  -> Reverse Core
       -> binary, range, function, data, type, evidence,
          hypothesis, experiment, reconstruction, validation
  -> DMC Rengine workspace
       -> GDSpaces
       -> EXE Editor
       -> Binary Inspector
       -> Stage Ops
       -> ModViz
       -> Item Editor
       -> Build & Test Lab
```

Reverse Core extends the existing workflow. It does not replace GDSpaces, SDD, Kanban, MemPalace, Obsidian, the Project Graph, or editor-specific domain models.

## Resource layers

1. **Sources** — local game folders and read-only mounted archives.
2. **GDSpaces** — source mounting, resolution, classification, container expansion, canonical resource identity, graph, diagnostics, evidence context, and working-copy orchestration.
3. **Domain services** — EXE evidence/reconstruction, patch planning, stage semantics, model/UI representations, item semantics.
4. **Tools** — Binary Inspector, EXE Editor, Stage Ops, ModViz, Item Editor, and future domain editors.
5. **Validation/export** — guarded patch plans, copied outputs, manifests, rollback, tests, and future controlled builds.

## Canonical tool boundaries

### GDSpaces

GDSpaces owns game-resource access. No editor independently mounts game sources, resolves archive paths, classifies formats, expands containers, or creates competing resource identities.

The next identity milestone is [Resource Identity v1](gdspaces/resource-identity-v1.md): stable logical identity across filesystem/container/extracted/EXE-backed representations.

### Reverse Core

Reverse Core owns the reusable reverse lifecycle and stable identities for binary artifacts, address ranges, functions, data objects, recovered types, evidence, hypotheses, experiments, task claims, reconstructions, validation receipts, and subsystem groupings.

It does not know DMC-specific gameplay/resource semantics.

### EXE Editor

EXE Editor owns the DMC executable recovery workflow: PE analysis, RVA/VA mapping, recovered C++ presentation, subsystem navigation, linked evidence, guarded patch requests, and rebuilt-output lineage. It consumes Reverse Core identities rather than inventing a second function/evidence database.

See [EXE Reconstruction Pipeline](exe/reconstruction-pipeline.md).

### Binary Inspector

Binary Inspector receives bytes, resource/artifact identity, regions, fields, ownership, diagnostics, annotations, diff, entropy, and evidence. It contributes structural observations to Reverse Core but does not resolve game sources independently or become a competing project database.

### Stage Ops

Stage Ops receives `GDStageBundle`/`StageBundle` objects and evolves toward the [Stage Semantic Graph](stage/stage-semantic-graph.md), linking stage, room, geometry, collision, lighting, cameras, transitions, events, effects, audio, and runtime evidence.

### ModViz

ModViz owns visual editing. It has two product modes: Scene/Model Editor and Menu Editor. The first Menu Editor vertical slice is the Red Orb HUD counter. Runtime formatting/limit behavior remains linked through EXE/Reverse Core evidence and guarded patch planning.

See [ModViz Menu Editor](modviz/menu-editor.md).

### Item Editor

Item Editor owns item semantics and typed edit requests, not game-source discovery or direct executable patching.

### Build & Test Lab

The Trial Chamber owns compile/test/runtime comparison, guarded copied-output execution, rollback, regression evidence, and future release/build validation receipts.

## Reverse reconstruction path

```text
Binary artifact
  -> AddressRange
  -> Function / DataObject
  -> CFG / calls / references
  -> RecoveredType / ABI hypotheses
  -> EvidenceRecord / Experiment
  -> Reconstruction
  -> C++ source unit
  -> isolated build
  -> behavioral comparison
  -> ValidationReceipt
  -> Canon promotion / correction / rejection
```

Mass decompilation is not the first gate. The architecture must first prove one isolated real subsystem through the complete loop.

## Parallel-agent coordination

Agents or contributors must claim canonical reconstruction work before mutating the same function/range/type/subsystem. Claims are coordination metadata, not evidence. Independent duplicate analysis is allowed only when intentionally requested and must converge through review rather than file/function races.

## Container policy

PAC, PNST, NBZ, and AFS are implementation details inside GDSpaces. Legacy PAC Editor/PAC Manager logic is not part of the top-level architecture.

## Safety policy

All edits operate on working copies. Writes require validation, source hashes, expected bytes, conflict checks, output manifests, rollback information, and explicit export targets. Recovered-source promotion additionally requires exact artifact provenance, ABI/lifetime review appropriate to the subsystem, compilation, and behavioral evidence.
