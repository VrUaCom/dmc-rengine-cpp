# DMC3 HD — Enemy architecture / `em000` resource ecosystem

**Branch:** `research/enemy-architecture-em000-20260908`  
**Base:** `main@22cfb1073f73d8290a04968dbf0f038048f01990`  
**Primary corpus:** `em000-extract.zip`  
**Corpus SHA-256:** `306130125f09824811289366324f4208c3c1aba880c5a7efa3953a88d566d07b`  
**Canonical reverse executable:** `dmc3.exe` SHA-256 `e454272ed0fb0247fcbcf300e5d55d7a3e96d50b89b9ffaff81bb978dcbdd082`

## 0. Scope

This document defines the current **evidence-backed enemy architecture model** for DMC3 HD Collection using `em000` as the first deeply decomposed enemy/actor package.

It intentionally separates four things that must never be collapsed:

1. **enemy resource package** — serialized assets such as MOD, MOT, PTX, CLT, TSC, EFM and effect resources;
2. **enemy runtime object** — the original game's `CEm*` C++ object family, managers, factories, lifetime and behavior;
3. **spawn/dependency control** — stage/event logic that requests enemy resources and instantiates runtime objects;
4. **authoring authority** — whether DMC Rengine can safely create a new enemy that the original game accepts.

A complete resource pack is not by itself a complete enemy. A parser is not an enemy constructor. A working model preview is not proof of spawn/runtime acceptance.

---

## 1. What an enemy is in DMC3

The recovered runtime is a large C++ object system. The vanilla research baseline independently confirms a `CEm*` enemy RTTI family alongside `CPlayer`, `CStage`, camera, UI and other runtime objects.

Therefore the safest current model is:

```text
Enemy = runtime class/object
      + resource identity/package
      + model/render state
      + animation state
      + effects
      + cloth/secondary motion when present
      + collision/contact/volume relationships
      + spawn/dependency rules
      + AI/action/damage/death behavior
      + lifetime/ownership
```

Only some layers are already structurally recovered. Concrete `CEm*` layouts, enemy manager/factory registration, spawn/despawn, action/command states, damage/death and projectile lifecycle remain explicit gameplay reverse frontiers in the canonical baseline.

---

## 2. Resource-package view: `em000`

The `em000` extraction is not one file format. It is a multi-resource actor ecosystem transported through the DMC3 container system.

At the semantic level the package contains:

```text
em000 actor ecosystem
│
├── PTX texture bundle
│
├── multiple MOD resources
│   ├── primary actor/body variants
│   ├── related model parts
│   └── smaller chain models associated with CLT
│
├── CLT resources
│   └── local cloth/deformation chain references
│
├── one EFM resource
│   └── related model-family effect/cloth-capable model
│
├── TSC resource
│   └── text-serialized motion/control texture-slot state
│
├── three motion PACs
│   └── 82 MOT-family resources total
│
├── three BIN-labelled SO working-family resources
│   ├── graph/control structure
│   ├── compact link table
│   └── spatial volume table
│
└── effect PNST
    ├── manifest
    ├── G records
    ├── V records
    ├── E records
    ├── P records
    ├── T wrapped-DDS texture records
    ├── A records
    └── M grouped records
         ├── MOD primary
         └── optional 16-byte companion
```

The full recursive extraction contains 302 leaf files. Extraction suffixes are not semantic authority: `.bin` can be PTX, MOT, SO-working-family or effect records; `.txt` can be CLT, TSC or an effect manifest.

---

## 3. Known actor/model layer

### 3.1 MOD

MOD is the primary skinned model resource for this actor family. Current canonical reverse already covers:

- 0x40 document header;
- 0x40 outer object records;
- 0x50 mesh records;
- positions, normals, fixed-point UVs;
- blend-index stream;
- packed 5+5+5 skin weights + topology break bit;
- node hierarchy/order domain;
- local transforms;
- motion-group selector;
- world matrices;
- inverse rest/world-at-load matrices;
- current-world and skin-palette composition;
- external texture-slot binding;
- runtime post-load topology workspace;
- default-joint fallback.

`em000` contains 35 MOD files recursively. No new unknown non-zero section was found outside the recovered structural grammar in the full byte-coverage sweep.

### 3.2 EFM

`em000_023.efm` is a real mesh-bearing EFM resource in the related model-document family. Current em000 research strongly binds mesh `+0x38` to a `count * 4` vertex-colour stream matching the EFM shader's extra `COLOR0` input.

EFM must stay a dedicated format adapter; it is not a MOD alias.

---

## 4. Texture/material layer

### 4.1 PTX

`em000_000.bin` structurally matches the canonical DMC3 PTX texture-bundle framing:

```text
0x800 bundle header
-> per-texture 0x70 descriptor
-> DDS payload
-> 0x800 sector alignment
```

For the bound em000 resource:

```text
texture_count = 4
observed sector spans = 43, 86, 86, 22
first DDS marker = 0x870
```

The `.bin` source suffix is therefore only provenance; semantic identity is PTX.

### 4.2 MOD texture binding

MOD mesh `texture_slot` selects an external runtime texture descriptor. Header `+0x12` is only a serialized mirror; the companion/texture domain is the authoritative runtime side.

### 4.3 TSC cross-binding

The single em000 TSC sample contains `TexNo 2`. The associated EFM meshes use `texture_slot = 2`, and PTX slot 2 exists. This creates a strong corpus-level three-way relationship:

```text
TSC TexNo 2
   -> model texture slot 2
   -> PTX texture entry 2
```

The exact mathematics of `ScrlType`, `DirUV`, `TimeUV`, and `MinimumUV` remain open until the original consumer is recovered.

---

## 5. Skeleton and animation layer

### 5.1 MOT binary family

The three motion PACs contain 82 MOT-family resources even though only ten were extracted with `.mot` and seventy-two with `.bin`.

All 82 have `MOT\0` at `+0x04`.

Current recovered envelope includes:

```text
+0x00 u32 header_size
+0x04 "MOT\0"
+0x08 raw u32
+0x0C raw f32
+0x10 raw f32
+0x14 raw f32
+0x18 raw u16
+0x1A raw u16
+0x1C u16 channel_domain_count
+0x1E u16 channel_mask[channel_domain_count]
```

and:

```text
header_size = align16(0x1E + 2 * channel_domain_count)
record_count = sum(popcount(channel_mask[i]))
```

Both identities hold for `82 / 82` em000 MOT payloads.

### 5.2 MOT tracks

Across 5,118 recovered tracks, two compression layouts dominate the em000 corpus:

```text
compression 2:
  prefix 0x10
  key stride 4
  span = 0x10 + 4 * key_count

compression 3:
  prefix 0x20
  key stride 8
  span = 0x20 + 8 * key_count
```

Compression distribution:

```text
2 -> 156 tracks
3 -> 4,962 tracks
```

The low 15 bits of the key time/control word are a monotonic time index. Bit 15 is a separate control bit; signed-time interpretation is rejected.

### 5.3 MOD ↔ MOT domain relationship

The em000 corpus gives a strong relation between MOT channel domains and MOD skeleton domains.

The major actor models share a common `0..21` skeleton core. Some MOD variants add node `22`, whose motion group is `2`, while the common MOT channel domain is only 22 entries (`0..21`).

Safe current interpretation:

```text
MOT channel domain -> common animated/core node domain candidate
extra MOD node 22  -> outside that 22-entry MOT domain in this corpus
```

Do not assign body-part names to motion groups without direct runtime proof.

---

## 6. Cloth / secondary-motion layer

Eight CLT resources are present. They are text-serialized CLT resources, not generic TXT.

All eight physically pair with the immediately following small MOD/EFM chain model:

```text
CLT 002 -> MOD 003 : 5 nodes
CLT 006 -> MOD 007 : 5 nodes
CLT 009 -> MOD 010 : 6 nodes
CLT 011 -> MOD 012 : 6 nodes
CLT 014 -> MOD 015 : 5 nodes
CLT 016 -> MOD 017 : 5 nodes
CLT 020 -> MOD 021 : 5 nodes
CLT 022 -> EFM 023 : 5 nodes
```

In all eight samples, the `Bone` rows select the tail of the local linear node chain.

The em002 pair proves CLT carries two node domains:

```text
Bone       -> local cloth/deformation model node domain
WindParent -> external/owning actor skeleton domain candidate
```

because `WindParent 9` / `13` are impossible inside the six-node local cloth model but valid inside the surrounding 22-node main actor skeleton.

This distinction belongs in `analysis/clt/model_binding`, not in the raw CLT parser.

---

## 7. Effect layer

`em000_041.pnst` is an effect pack built as an outer PNST containing:

```text
slot 0 -> ASCII manifest
slot 1 -> inner PNST physical record storage
```

Manifest census:

```text
G 12
V 50
E 45
P 34
T  8
A 11
M 13
---
 173 logical entries
```

The inner PNST has 186 physical slots and 183 populated payloads.

### 7.1 Grouped M records

`M` disproves the old universal one-manifest-line/one-physical-payload assumption.

Each M entry maps to two physical slots:

```text
M logical entry
├── primary MOD slot
└── optional 16-byte companion slot
```

There are 13 primary MODs, ten populated companions and three empty companion slots.

### 7.2 E reference graph

Current corpus analysis recovers subtype-dependent references:

```text
E subtype 1/2
  +0x04 low16 -> T manifest ID
  +0x08 low16 -> A manifest ID or 0xFFFF

E subtype 5
  +0x28 u32   -> M manifest ID
```

This creates real effect-resource graph edges:

```text
E subtype 1/2 -> T
               -> optional A

E subtype 5   -> M -> MOD + optional companion
```

The high-level words behind `E/T/A/M` are not invented here; only kind IDs and reference edges are promoted at corpus level.

### 7.3 P grammar

P is not one fixed-size record. The four observed extents `336 / 528 / 704 / 896` follow one parameterized structure:

```text
relative-offset table
-> primary 0x130 block
-> optional repeated 0xB0 auxiliary blocks
```

The primary block carries internal ASCII identities such as `ee000-21p1` and related names.

---

## 8. SO working-family / collision-volume candidate layer

Three top-level BIN-labelled resources currently form the SO working family:

```text
em000_038.bin -> graph/control structure, 6144 bytes
em000_039.bin -> compact 4-byte link table, 96 bytes
em000_040.bin -> 23 x 0x50 spatial volume records, 1840 bytes
```

Important boundary:

> `SO` is a DMC Rengine working family name. The original Capcom filename extension is not yet proven.

There is strong cardinality correlation:

```text
23 post-prefix link records
23 spatial volumes
23 MOD transform selectors in the companion actor domain
```

This is useful cross-resource evidence, but gameplay names such as hitbox/hurtbox/attack/collision are not yet authorized without the original consumer switch.

---

## 9. Stage / dependency / spawn layer

The stage subsystem is the other half of an enemy.

Canonical format-purpose research already establishes:

- `EventTblNN.bin` is mission/event control bytecode with dynamic-spawn behavior at a high-confidence/EXE-backed subsystem level;
- StageCfg physical slot 9, when present, is passed to `0x1401A9BC0`, whose dependency/control scan contributes enemy-resource demand;
- an EST dependency/control interpretation is a candidate, not a universal `slot9 == EST` law.

Therefore the current enemy-loading model is:

```text
stage / mission / event state
      -> dependency scan / enemy resource demand
      -> enemy resource package becomes available
      -> enemy factory / CEm* runtime construction
      -> runtime initialization
```

The exact numeric mapping:

```text
spawn opcode / enemy id
    -> emNNN resource identity
    -> concrete CEm* class/factory
```

is **not yet closed**. This is one of the highest-value reverse targets for actual new-enemy authoring.

---

## 10. Runtime gameplay layer

A real enemy must also implement/receive runtime behavior that is not serialized in MOD/MOT/PTX alone.

Open canonical gameplay frontiers include:

```text
CEm* concrete layouts
Enemy manager
factory / constructor registration
spawn / despawn
AI/action state machine
target selection
movement/control
damage intake
stagger / reactions
death / cleanup
drops/rewards
projectile lifecycle
collision/contact-category ownership
resource acquire/release
```

Until these are recovered, DMC Rengine can describe and edit resource-side anatomy more completely than it can create an entirely new gameplay class.

---

## 11. Enemy lifecycle model

Current safe system model:

```text
1. stage/mission selects enemy demand
2. resource resolver finds enemy package
3. PAC/PNST recursively materialize children
4. semantic classifier identifies PTX/MOD/CLT/TSC/EFM/MOT/effect members
5. resource manager builds runtime model/texture/animation state
6. CEm* factory/constructor creates gameplay object
7. actor model + animation + cloth + effect resources bind to runtime object
8. currentWorld / skin palette update each pose
9. collision/contact/volume systems participate in gameplay
10. AI/action state advances
11. damage/death/despawn execute
12. runtime resources are released/reused according to manager lifetime rules
```

Steps 3-5 are much more recovered than steps 6-12.

---

## 12. Three different meanings of "create a new enemy"

### Level A — Resource variant of an existing enemy

Goal: same original `CEm*` gameplay behavior, changed visual/resource content.

Potential changes:

- textures;
- mesh/geometry;
- skeleton-compatible model parts;
- animations compatible with the same channel/node contract;
- CLT/TSC/effect assets where compatible.

This is the lowest-risk new-enemy path, but even it still requires writer authority for the edited formats and container reintegration.

### Level B — New resource identity using an existing enemy runtime class

Goal: a new `emNNN`-like actor package but reuse an existing CEm gameplay implementation.

Additional gates:

- discover enemy ID -> resource-path mapping;
- discover enemy ID -> factory/class mapping;
- make dependency scanner request the new package;
- make stage/EventTbl spawn the new identity;
- ensure runtime managers accept the new resource-domain counts.

This is likely the first practical route to a genuinely new selectable enemy resource family once the mapping tables are recovered.

### Level C — Entirely new enemy behavior/class

Goal: new AI, actions, damage rules and lifecycle, not merely new assets.

Requires all Level B work plus:

- new or replaced `CEm*` class behavior;
- factory registration / dispatch;
- constructor/destructor/lifetime contract;
- AI/action state implementation;
- damage/death integration;
- collision/contact integration;
- effects/audio/projectiles/rewards where relevant;
- either source-level recompilation/replacement or a controlled binary extension strategy.

This is the true full-authoring endpoint.

---

## 13. What a future `EnemyDefinition` should look like in DMC Rengine

The project should not define an enemy as a single flat struct. It should be a graph/manifest that references independently parsed resources and runtime bindings.

Conceptual model:

```text
EnemyDefinition
  identity
    logical_enemy_id
    resource_family / emNNN identity
    runtime_factory_id      # unresolved until EXE-bound

  visuals
    primary_models[]        # MOD/EFM references
    texture_bundle          # PTX
    texture_control[]       # TSC

  skeleton
    primary_node_domain
    auxiliary_node_domains[]

  animation
    motion_sets[]           # MOT containers/resources
    motion_group contract

  cloth
    clt_resources[]
    local_model_bindings[]
    external_parent_bindings[]

  effects
    effect_pack
    logical effect graph

  spatial
    so_working_resources[]  # until original identity/semantics are closed

  spawn
    dependency references
    event/spawn selectors

  runtime
    factory/class binding
    lifecycle binding

  evidence
    per-edge evidence status
    source ResourceId / slot / hash provenance
```

This representation belongs above the individual format parsers.

---

## 14. Required modular architecture

Each format remains independent:

```text
formats/mod
formats/efm
formats/ptx
formats/mot
formats/clt
formats/tsc
formats/effect_pack
formats/so/*          # working identity
formats/pac
formats/pnst
formats/dds
```

Enemy composition belongs in a higher analysis/domain layer, for example:

```text
include/dmc_rengine/analysis/enemy/
    identity.hpp
    resource_graph.hpp
    model_binding.hpp
    animation_binding.hpp
    effect_binding.hpp
    spawn_binding.hpp
    validation.hpp
```

Raw parsers must not know that a neighboring file happens to belong to one enemy.

---

## 15. Dedicated reverse branches

Current format ownership is intentionally separated:

```text
MOD
  reverse/mod-completion-20260907

CLT
  reverse/clt-em000-20260908

TSC
  reverse/tsc-em000-20260908

MOT
  reverse/mot-em000-20260908

EFM
  reverse/efm-em000-20260908

Effect pack / G,V,E,P,T,A,M
  reverse/effect-pack-em000-20260908

SO working family identity
  reverse/so-em000-identity-20260908

PTX em000 binding
  reverse/ptx-em000-binding-20260908

Cross-format leaf census
  reverse/em000-format-census-20260908

Enemy system composition
  research/enemy-architecture-em000-20260908
```

Existing canonical PAC/PNST/DDS/PTX/MOD infrastructure is reused rather than duplicated.

---

## 16. Definition of success for a real "Create New Enemy" pipeline

The feature is not complete until all of the following are demonstrated:

1. enemy resource ID and resource package mapping recovered;
2. factory/class mapping recovered;
3. spawn/event selector mapping recovered;
4. every required child format structurally parsed;
5. writer authority for every edited child format;
6. exact container topology preserved/rebuilt;
7. resource graph validates all references;
8. actor skeleton / MOT / cloth / effects validate coherently;
9. dependency scanner requests the new resource;
10. game instantiates the intended runtime class;
11. enemy renders correctly;
12. animation works;
13. collision/contact works;
14. AI/action state works;
15. damage/death/despawn works;
16. stage reload and resource cleanup work;
17. deterministic rollback restores original behavior;
18. original `dmc3.exe` acceptance is recorded as game evidence.

Until then the project may support partial authoring levels A or B without claiming full new-enemy authority.

---

## 17. Highest-priority reverse targets from here

### Runtime identity / spawn

- locate enemy resource-path/catalog tables;
- recover enemy ID -> `emNNN` mapping;
- recover enemy ID -> CEm factory/class mapping;
- recover EventTbl spawn opcode(s) and arguments;
- recover StageCfg/EST dependency contribution format;
- trace create -> init -> destroy lifecycle.

### Gameplay

- enumerate concrete CEm RTTI/class hierarchy;
- recover common enemy base layout;
- recover action/state dispatch;
- recover damage/death paths;
- recover dynamic collision/contact ownership.

### Resource graph

- finish MOT evaluation and CMotion binding;
- finish CLT runtime field consumers;
- finish TSC UV/texture-control semantics;
- finish EFM semantics;
- decode effect G/V/E/P/A/M records;
- identify SO original family and spatial-volume consumer;
- determine audio ownership for enemy-specific sound resources.

### Authoring

- establish no-edit byte-exact writers first;
- then bounded edits;
- then complete actor package rebuild;
- then dependency/spawn integration;
- finally original-game acceptance.

---

## 18. Evidence boundary

What is already strong:

- em000 leaf-format accounting;
- MOD structural/skin/world/runtime model;
- MOT envelope/channel-domain/compression grammar;
- CLT local chain association and two-domain evidence;
- TSC texture-slot correlation;
- EFM model-family payload structure;
- effect-pack grouping and several internal reference edges;
- SO working-family structural/cardinality correlation;
- existence of `CEm*` enemy runtime class family;
- stage/event systems contribute dynamic spawn/dependency behavior.

What is not yet proven:

- exact original names/meaning of all effect kind letters;
- original `SO` extension/name;
- exact enemy resource ID table;
- exact CEm factory table;
- full spawn opcode grammar;
- complete AI/action/damage/death lifecycle;
- full collision/hit/hurt semantic ownership;
- production writer for a new enemy;
- original-game acceptance of a genuinely new enemy identity.

The project must preserve these boundaries explicitly rather than filling them with game-familiarity assumptions.
