# Architecture

**Snapshot date:** 2026-08-08

## Mission

DMC Rengine is a C++20 reverse-engineering, decompilation, editing, and future recompilation framework for Devil May Cry 3 HD. It is also the first domain workspace intended to validate the reusable Reverse Core architecture.

DMC Rengine remains its own project/workspace. The broader Triangle Forge direction is a platform-level context, not permission to collapse DMC-specific domain services into a monolith.

## Core laws

1. GDSpaces is the single game-resource authority.
2. No claim without evidence.
3. Recovered source preserves provenance to exact binary identity and validation state.
4. Recovered game code is not owned by DMC Rengine tools; tool relationships and game-subsystem membership are separate concepts.
5. No original-file write without an explicit working-copy/export contract.
6. Shared reverse infrastructure must remain game-agnostic; DMC semantics remain in the DMC Rengine workspace.

## Platform layering

```text
Triangle Forge platform context
  -> shared workflow / MCP / SDD / evidence / coordination services
  -> Reverse Core
       -> binary, range, function, data, type, evidence,
          hypothesis, experiment, reconstruction, validation
  -> DMC Rengine workspace
       -> Recovered Game Source Tree
       -> GDSpaces
       -> EXE Editor
       -> Binary Inspector
       -> Stage Ops
       -> ModViz
       -> Item Editor
       -> Build & Test Lab
```

Reverse Core extends the existing workflow. It does not replace the Recovered Game Source Tree, GDSpaces, SDD, Kanban, MemPalace, Obsidian, the Project Graph, or editor-specific domain models.

## Recovered Game Source Tree

The reconstructed DMC3 source tree represents the game itself. Its functions, data, types, classes, tables, and source units are organized according to the recovered architecture of the game, not according to which DMC Rengine tool discovered or displays them.

Three axes must remain independent:

1. **game source identity** — the recovered object in the target executable/source reconstruction;
2. **game subsystem membership** — resource runtime, stage runtime, renderer, collision, UI/HUD, save, AI, input, audio, unknown, etc.;
3. **tool relationship** — EXE Editor, Binary Inspector, GDSpaces, Stage Ops, ModViz, Build & Test Lab, or another consumer/view.

A single game function may appear in multiple tool workflows without being duplicated or reassigned. `TaskClaim` ownership applies only to work coordination on a reconstruction, not semantic ownership of the game function.

See [Recovered Game Source Tree](reverse-core/game-source-tree.md).

## Resource layers

1. **Sources** — local game folders and read-only mounted archives.
2. **GDSpaces** — source mounting, resolution, classification, container expansion, canonical resource identity, graph, diagnostics, evidence context, and working-copy orchestration.
3. **Domain services** — EXE evidence/reconstruction, patch planning, stage semantics, model/UI representations, item semantics.
4. **Tools** — Binary Inspector, EXE Editor, Stage Ops, ModViz, Item Editor, and future domain editors.
5. **Validation/export** — guarded patch plans, copied outputs, manifests, rollback, tests, and future controlled builds.

The recovered game source tree is not one of these tool layers. It is the reconstructed target program that these layers analyze and validate.

## Canonical tool boundaries

### GDSpaces

GDSpaces owns DMC Rengine game-resource access. No editor independently mounts game sources, resolves archive paths, classifies formats, expands containers, or creates competing resource identities.

This does not mean recovered game resource-loader functions belong to GDSpaces. Those functions remain in the Recovered Game Source Tree under evidence-driven game subsystem membership. GDSpaces implements/consumes confirmed resource behavior as product infrastructure.

The next identity milestone is [Resource Identity v1](gdspaces/resource-identity-v1.md): stable logical identity across filesystem/container/extracted/EXE-backed representations.

The resource-runtime reverse program is defined in [GDSpaces Resource Runtime Reconstruction](gdspaces/runtime-reconstruction.md).

### Reverse Core

Reverse Core owns the reusable reverse lifecycle and stable metadata identities for binary artifacts, address ranges, functions, data objects, recovered types, evidence, hypotheses, experiments, task claims, reconstructions, validation receipts, and subsystem groupings.

Reverse Core does not own the recovered game code itself. It records and coordinates reconstruction of that code.

It does not know DMC-specific gameplay/resource semantics.

### EXE Editor

EXE Editor owns the DMC executable recovery workflow UI: PE analysis, RVA/VA mapping, recovered C++ presentation, subsystem navigation, linked evidence, guarded patch requests, and rebuilt-output lineage. It consumes Reverse Core identities rather than inventing a second function/evidence database.

A game function displayed or edited in EXE Editor remains part of the Recovered Game Source Tree.

See [EXE Reconstruction Pipeline](exe/reconstruction-pipeline.md).

### Binary Inspector

Binary Inspector receives bytes, resource/artifact identity, regions, fields, ownership, diagnostics, annotations, diff, entropy, and evidence. It contributes structural observations to Reverse Core but does not resolve game sources independently, become a competing project database, or own game functions.

### Stage Ops

Stage Ops receives `GDStageBundle`/`StageBundle` objects and evolves toward the [Stage Semantic Graph](stage/stage-semantic-graph.md), linking stage, room, geometry, collision, lighting, cameras, transitions, events, effects, audio, and runtime evidence.

### ModViz

ModViz owns visual editing. It has two product modes: Scene/Model Editor and Menu Editor. The first Menu Editor vertical slice is the Red Orb HUD counter. Runtime formatting/limit behavior remains linked through EXE/Reverse Core evidence and guarded patch planning.

See [ModViz Menu Editor](modviz/menu-editor.md).

### Item Editor

Item Editor owns item semantics and typed edit requests, not game-source discovery, recovered game function ownership, or direct executable patching.

### Build & Test Lab

The Trial Chamber owns compile/test/runtime comparison, guarded copied-output execution, rollback, regression evidence, and future release/build validation receipts.

It validates recovered game source but does not semantically own it.

## Reverse reconstruction path

```text
Binary artifact
  -> AddressRange
  -> Function / DataObject
  -> CFG / calls / references
  -> RecoveredType / ABI hypotheses
  -> EvidenceRecord / Experiment
  -> Reconstruction
  -> C++ source unit in Recovered Game Source Tree
  -> isolated build
  -> behavioral comparison
  -> ValidationReceipt
  -> Canon promotion / correction / rejection
```

Mass decompilation is not the first gate. The architecture must first prove one isolated real subsystem through the complete loop.

## Parallel-agent coordination

Agents or contributors must claim canonical reconstruction work before mutating the same function/range/type/subsystem reconstruction. Claims are coordination metadata, not evidence and not game-code ownership. Independent duplicate analysis is allowed only when intentionally requested and must converge through review rather than file/function races.

## Container policy

PAC, PNST, NBZ, and AFS are implementation details inside GDSpaces. Legacy PAC Editor/PAC Manager logic is not part of the top-level architecture.

## Safety policy

All edits operate on working copies. Writes require validation, source hashes, expected bytes, conflict checks, output manifests, rollback information, and explicit export targets. Recovered-source promotion additionally requires exact artifact provenance, ABI/lifetime review appropriate to the subsystem, compilation, and behavioral evidence.
