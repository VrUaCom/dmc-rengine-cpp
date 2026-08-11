# DMC3 Vanilla Research Baseline

**Status:** CANONICAL RESEARCH BASELINE (pre-roadmap)  
**Scope:** DMC3 Special Edition / HD Collection vanilla runtime and resource architecture.  
**Purpose:** preserve the current evidence-backed model of vanilla DMC3 before any DMC Rengine roadmap redesign.

This document is a research authority record, not a product roadmap. It reconciles the real HD Collection distribution build, the canonical unpacked reverse target, real Drive resources, Wide Pass 0-36 findings, HITS corpus work, save/progression work, and current claim corrections.

## Evidence policy

Allowed research states remain:

- `HYPOTHESIS`
- `EXE CONFIRMED`
- `DERIVED FROM VERIFIED RUNTIME`
- `GAME VERIFIED`
- `GAME + SAVE VERIFIED`
- `RESEARCH REQUIRED`
- `CORRECTED`
- `REJECTED`

Latest reconciled direct evidence supersedes older documents even when an older file is named `canonical`.

## Executable artifact identities

### Distribution / vanilla build

- File: `dmc3.exe`
- SHA-256: `81c7e61983564113b5105e931d9f185accc14e44ae147d27f720c2d50935c7d6`
- Role: packed/protected HD Collection distribution profile from Google Drive.

This is a real vanilla artifact and must not be treated as byte-identical to the analysis build.

### Canonical reverse target

- File: `dmc3.exe`
- SHA-256: `e454272ed0fb0247fcbcf300e5d55d7a3e96d50b89b9ffaff81bb978dcbdd082`
- Role: unpacked/decrypted target used for VA/RVA/XREF/CFG/DFG evidence.

The two builds are related but are **not interchangeable** for patching, byte identity, or exact address evidence. Cross-build claims must preserve artifact identity.

## Vanilla resource surface

The Drive vanilla corpus includes:

- `dmc3-0.nbz` — 960,358,951 bytes;
- `afs/sound`;
- `Video`;
- `LOADERICON.dds`;
- the real distribution `dmc3.exe`.

The entire NBZ could not be re-downloaded wholesale during the 2026-08-11 research session because of provider download-size limits. Whole-archive claims therefore remain grounded in earlier corpus audits or directly extracted resources rather than a new full-archive scan.

The executable independently exposes the six logical namespace prefixes:

```text
GDataX360.afs/
GData.afs/
Video/
afs/sound/
SAVEDATA/
<empty prefix>
```

The canonical resource path remains:

```text
logical request
-> namespace candidates
-> provider / mount resolution
-> physical or NBZ backend
-> FileBackendHandle
-> FileSlot
-> synchronous / asynchronous read
-> completion callback
```

## HD-port translation layer

Logical game identity is not always the same as the physical HD-port representation.

### Audio

The vanilla EXE contains 154 legacy ADX names while the HD distribution stores corresponding OGG files. The observed ADX and OGG basename sets match.

### Video

The EXE retains legacy SFD catalogs while the HD distribution physically contains WMV files.

### Canonical consequence

`ResourceId` must not collapse logical identity, provider translation, and physical filename into one field.

```text
logical/original identity
-> HD-port translation/provider
-> physical representation
```

## Stage and ST namespace correction

The known runtime stage table at file offset `0x5C30A8` / VA `0x1405C4AA8` is real and stable. It contains **110 rows x 4 resource references** for script/config/effect/sound roles.

A direct vanilla-table read resolves these 110-row groups:

```text
000-012
100-146
200-241
300-307
```

However, the vanilla EXE contains **189 distinct `stNNN` identities** and a separate flat catalog of **730 stage-resource path pointers**. The wider catalog covers:

```text
000-012
100-146
200-241
300-313
400-446
448-449
600-612
900-910
```

For `600-612`, only script and cfg roles are present in the observed flat catalog.

### Canonical correction

> **110-row Stage Table != the complete ST universe.**

`stNNN` is a resource namespace / identity pattern. It is not, by itself, proof that every `stNNN` belongs to the same gameplay-stage semantic category.

Consumer/context ownership for every ST group remains `RESEARCH REQUIRED`.

The extra per-cell field at `+0x08` in the 110x4 table must remain unresolved. It is not safe to call it padding solely because nearly all observed cells are zero.

## Runtime object model

The protected vanilla build independently exposes **408 unique MSVC RTTI TypeDescriptor names**, matching the canonical reverse census. The runtime is a large C++ object system, not a collection of standalone format parsers.

Major families include:

- `CPlayer` and concrete player classes;
- `CEm*` enemy families;
- `CStage`, `CStageSet`, and roughly 35 StageSet types;
- `CCamera*` families;
- `CUID*` UI classes;
- scene classes;
- clip/timeline classes;
- constraints;
- factories;
- draw capability interfaces;
- item roots;
- platform/save compatibility classes.

The canonical resource-completeness unit is:

```text
data/resource
-> loader/parser
-> factory/constructor
-> runtime object
-> ownership/lifetime
-> behavior/consumers
```

A format parser alone is not equivalent to recovered game behavior.

## Items

The EXE contains a **1,029-entry `id*.pac` resource catalog** plus the `endof.afs` sentinel, but only a small RTTI item-class surface.

This supports the model that item identity and properties are substantially table/code-driven rather than one C++ class per item.

## Stage runtime

Known StageSet/TXT/DOOR evidence, stage tables, camera, collision, events, effects, and sound must be modeled as connected runtime behavior.

The main unresolved Stage boundary is:

```text
TXT/resource token
-> parser
-> factory/constructor
-> concrete CStageSet/runtime object
-> lifetime
-> interactions with scene/collision/camera/events/gameplay
```

## Camera

DCA camera research has closed the two-channel mini-demo camera interpretation:

```text
channel 0 -> camera eye/world position
channel 1 -> look-at target
```

The view path constructs `target - eye` and builds the camera view basis.

Still open: camera selection, priority, blending, and transition rules among player, boss, rail, relative, demo, and motion-camera families.

## Collision / HITS

The historical `HITS$` / `0x18060001` model is **REJECTED**.

The current evidence-backed HITS model includes:

- `HITS` magic;
- exact `0x44` header;
- `0x38` triangle-plane records;
- spatial grid lookup;
- source 0 / PAC member 3;
- source 1 / PAC member 6;
- query families;
- reject-mask behavior;
- dynamic-category bridge;
- source switching.

The real 16-resource corpus reproduced **40,789 spatial references with zero missing, zero extra, and zero differing spatial bytes** for the recovered file-level builder.

File-level spatial reconstruction is `GAME VERIFIED`. Runtime acceptance of topology-changing rebuilt files remains `RESEARCH REQUIRED` until controlled repack and game tests.

## Rendering and textures

The vanilla/canonical evidence confirms D3D11/DXGI rendering, shader registries, draw capability interfaces, `CPtxManager`, `gfxTexture`, and PTX/TIM2 runtime handling.

The representation boundary is canonical:

```text
runtime-original PTX/TIM2
!= DDS-bearing PAC representation
!= extracted DDS
!= runtime GPU texture
```

DDS editing alone is not proof of a correct runtime texture write path.

Still open:

- exact TIM2 metadata;
- CLUT/swizzle conversion;
- DDS/PTX conversion boundary;
- GPU-resource ownership details;
- render queues;
- buffer layouts;
- skinning state;
- render-target graph;
- post-processing order.

## Save and progression

`dmc3.sav` is confirmed at `0x4A30` bytes with:

- 21 integrity envelopes;
- ten slot summaries;
- ten detailed payload envelopes;
- one's-complement end-around-carry integrity;
- BCD timestamps;
- a recovered detailed-payload partition.

Wide Pass 33-35 recovered:

- `MissionResultMatrix` structure;
- the six-row `SaveResultModeIndex` mapping;
- the five-category result evaluator;
- perfect-grade promotion;
- monotonic best-result update behavior.

Do **not** infer human-readable rank labels directly from the separate rank-token parser without a direct data-flow edge.

Pass 36 remains blocked for exact `e454...` XREF evidence needed to bind result codes to labels and finish remaining policy semantics. Availability of the protected `81c7...` build does not make canonical addresses interchangeable.

## Canonical corrections and document drift

The project contains older documents that retain superseded claims. Latest reconciled evidence wins over filename-based `canonical` status.

Known example: older Data Formats text still describing `HITS$` and a fixed marker is superseded by the later HITS reverse/core synthesis and real-corpus proof.

Old Open Gaps sheets also contain entries later closed or partially closed by newer passes. Future reconciliation must classify gaps explicitly as:

- still open;
- partially closed;
- superseded / closed.

## Major research frontiers still open

### Resource runtime

`ResourceTypeInfo` / factory dispatch, cache/sharing/refcount/lifetime, provider translation, reload/unload, and remaining `OpenGameResource` branches.

### Stage

Full 189-ST classification, consumer maps, StageSet factory registration, scene/room transition ownership, and EventTbl binary/runtime semantics.

### Gameplay

Concrete Player/Enemy layouts, managers, spawn/despawn, action/command states, damage/death, and projectile lifecycle.

### Animation

MOT/MCV binding, skeleton/bones, constraints, IK, blending, and controller lifecycle.

### Rendering

Vertex/index buffers, state registries, queues, skinning, render-target graph, and post-processing.

### Textures

Exact PTX/TIM2 metadata, CLUT/swizzle, conversion, and transactional reload.

### Effects

Metadata tables, `CGenerator`, pools, ownership, and dispatch.

### Camera

Priority/selection/blending/transitions between camera families.

### UI

HUD/menu hierarchy, draw pipeline, data binding, messages, and font/glyph ownership.

### Audio/video

Exact ADX->OGG and SFD->WMV runtime translation/streaming ABI.

### Save/progression

Remaining payload semantics, mode-progress records, category identities, result labels, `options.sav`, and fallback/no-Steam behavior.

### Lifecycle

Global initialization graph, memory arenas, ownership/destructors, scene reset/switch/shutdown.

### Recompilation

Logical-function ownership, ABI fidelity, source-to-binary mapping, and behavior-equivalent rebuilt units.

## Canonical architecture consequence

This baseline rejects the insufficient mental model:

```text
EXE + PAC/NBZ parsers + editors
```

The evidence-backed vanilla model is:

```text
platform / HD compatibility
-> VFS + translation + async I/O
-> container/resource system
-> resource type/factory/cache/lifetime
-> scene/stage runtime
-> object/gameplay runtime
-> render/audio/input/collision/animation
-> UI/progression/save
-> frame and process lifecycle
```

PAC, PNST, ITM, SCM, DCA, HITS, and related files are **data transported through this runtime architecture**. Correctly opening a file is not equivalent to reproducing the game's resource behavior.

## Preservation rule

This document is the **pre-roadmap DMC3 Vanilla Research Baseline**.

Roadmap decisions must be derived from this reconciled model and later stronger evidence. Do not downgrade newer confirmed/corrected findings to older documentation, and do not promote unresolved semantic names from game familiarity or filename patterns without direct evidence.
