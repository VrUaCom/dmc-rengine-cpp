# DMC Rengine — Vision and Long-Term Direction

Дата фіксації: 2026-09-16

## 1. Мета

DMC Rengine — не просто редактор ресурсів і не лише проєкт декомпіляції Devil May Cry 3. Кінцева мета — створити власний сучасний character-action engine, який виростає з доказового реверс-інжинірингу DMC1–DMC4, але не залишається обмеженим архітектурою оригінальних ігор.

Ключова ідея:

```text
DMC1 ─┐
DMC2 ─┤
DMC3 ─┼──► Reverse / Recompilation Knowledge
DMC4 ─┘                  │
                         ▼
                Compatibility Layer
                         │
                         ▼
                    DMC Rengine
                         │
         ┌───────────────┼───────────────┐
         ▼               ▼               ▼
      Runtime         Editors        Tooling
         │               │               │
         └───────────────┴───────────────┘
                         ▼
                  New Games / Mods
```

Реверс гри, її рекомпіляція та розвиток самого рушія — це окремі, але пов’язані програми роботи.

## 2. Три головні шари

### 2.1 Reverse / Reconstruction

Для кожної гри відновлюються:

- serialized formats;
- runtime data structures;
- loaders, allocators, managers;
- renderer-facing behavior;
- animation, combat, AI, camera, scene logic;
- resource lifecycle;
- container and rebuild paths.

Відновлений код повинен бути evidence-aware і, де можливо, диференційно перевірений проти оригінального EXE.

### 2.2 Compatibility / Recompilation

Відновлена поведінка переноситься у власний C++ так, щоб можна було підтримувати максимально точну сумісність з оригінальною грою.

Цей шар має залишатися окремим еталоном і не повинен змішуватися з модернізаціями.

```text
Original EXE behavior
        ≈
Compatibility C++ behavior
```

### 2.3 Modern Rengine Layer

Поверх compatibility layer будується незалежний сучасний рушій:

- unified resource model;
- modern renderer;
- animation system;
- character system;
- combat framework;
- material system;
- scene/runtime system;
- scripting;
- editor ecosystem;
- deterministic build pipeline.

Compatibility layer відповідає на питання «як це робила оригінальна гра». Modern layer відповідає на питання «як ми хочемо робити це в Rengine».

## 3. Єдина гра з чотирма кампаніями

Довгострокова ціль — перенести DMC1, DMC2, DMC3 і DMC4 на один runtime DMC Rengine.

Не чотири EXE в одному launcher, а один рушій і чотири кампанії:

```text
DMC Rengine
│
├── Main Menu
│   └── Campaigns
│       ├── Devil May Cry 1
│       ├── Devil May Cry 2
│       ├── Devil May Cry 3
│       └── Devil May Cry 4
│
├── Shared Renderer
├── Shared Input
├── Shared Audio
├── Shared Save/Profile
├── Shared Character System
├── Shared Costume System
├── Shared Mod System
└── Shared Toolchain
```

Після міграції кампанії перестають залежати від власних старих runtime-движків і стають наборами даних/правил для Rengine.

## 4. Єдина система персонажів

Старі формати типу `.MOD` залишаються compatibility/import formats.

Новий canonical character asset Rengine має бути незалежним:

```text
RengineCharacter
├── Skeleton
├── Meshes
├── Skin
├── MorphSet
├── Materials
├── AnimationSet
├── CostumeSet
├── WeaponSet
├── PhysicsMetadata
├── VFXStyle
└── CombatDNA
```

### 4.1 Що покращуємо відносно legacy MOD

- 4/8 bone influences замість legacy 3;
- float/half weights;
- tangents/bitangents;
- multiple UV channels;
- vertex colors / masks;
- morph targets / blend shapes;
- LOD chain;
- collision/shadow meshes;
- sockets / attachment points;
- retarget metadata;
- cloth/hair physics metadata;
- 32-bit indices;
- mesh streaming / GPU-friendly clusters;
- modular body/head/hair/coat/gloves/boots/weapons/accessories.

## 5. Costume Layer System

Костюм не повинен бути монолітною моделлю.

```text
Character
├── Body
├── Head
├── Hair
├── Coat
├── Shirt
├── Gloves
├── Boots
├── Weapons
└── Accessories
```

Це дозволить комбінувати частини костюмів між кампаніями й поколіннями, за умови сумісності rig/animation/attachment contracts.

Ціль: усі костюми з усіх частин можуть бути доступні в одному Rengine character inventory і використовуватися в різних кампаніях.

## 6. Combat DNA — фірмова технологія Rengine

Combat behavior відділяється від моделі персонажа.

```text
CombatDNA
├── MovementRules
├── ComboGraph
├── CancelRules
├── Startup/Active/Recovery
├── AirBehavior
├── ParryRules
├── StyleLogic
├── WeaponBindings
├── EnemyInteractionRules
└── VFXHooks
```

Це дозволяє мати різні бойові профілі одного персонажа:

- DMC1-style;
- DMC3-style;
- DMC4-style;
- custom Rengine style.

У перспективі тіло, анімації, combat logic, weapon behavior і VFX можуть бути незалежними змінними.

## 7. Modern Animation Stack

Rengine має підтримувати:

- animation graph;
- state machines;
- blend trees;
- additive animation;
- retargeting;
- full-body IK;
- foot placement;
- motion warping;
- procedural pose correction;
- animation layers;
- facial morph animation;
- optional motion matching.

## 8. Modern Rendering Stack

Один renderer для всіх кампаній дозволяє модернізувати графіку одразу в DMC1–4.

Цільові можливості:

- PBR materials;
- HDR;
- modern shadowing;
- SSAO / modern ambient shading;
- volumetric effects;
- modern anti-aliasing;
- GPU skinning / compute skinning;
- texture streaming;
- virtual texturing where useful;
- mesh streaming / cluster-based rendering;
- GPU particles;
- advanced hair/cloth rendering;
- scalable material layers.

## 9. Surface Stack / Persistent Damage

Персонажі та сцени можуть мати багатошаровий runtime surface state:

```text
Base Surface
+ Dirt
+ Blood
+ Wetness
+ Burns
+ Cuts
+ Decals
+ Damage Masks
+ Magic / DT Effects
```

Для світу окремо планується realtime destruction / persistent damage system: сліди куль, мечів, вибухів, маски прозорості, локальна деформація та збереження стану між відвідуваннями локації.

### 9.1 Living Surface Genome

Окремий R&D-напрям Rengine — компактно описувати гіперреалістичні поверхні персонажів через genome/seed/parameter model замість прямого зберігання всіх мікродеталей у великих texture sets.

Повний концепт зафіксований у:

`docs/vision/RENGINE_LIVING_SURFACE_GENOME.md`

Ключовий принцип:

```text
identity data
+ compact genome
+ deterministic procedural generation
+ shared decoder
+ detail-on-demand
        ↓
living adaptive surface
```

Ціль — щоб збільшення візуального реалізму не вимагало пропорційного збільшення per-character storage.

## 10. Asset Migration Pipeline

Legacy formats не повинні диктувати внутрішню архітектуру Rengine.

```text
DMC1 formats ─┐
DMC2 formats ─┤
DMC3 formats ─┼──► Importers / Converters ─► Rengine Canonical Assets
DMC4 formats ─┘
```

Потрібно розділяти два класи інструментів:

### Exact Rebuild Tools

Для реверсу і compatibility:

```text
Legacy format
→ parse
→ edit
→ rebuild
→ validate
→ original runtime acceptance
```

### Migration Tools

Для Rengine:

```text
Legacy format
→ importer
→ canonical IR
→ Rengine native asset
```

Це повинно зняти більшість legacy-обмежень усередині нового рушія.

## 11. Tooling

Rengine повинен мати набір інтегрованих інструментів:

- Scene Editor;
- Model/Character Editor;
- Skeleton/Rig Editor;
- Animation Graph Editor;
- Combo/Combat Graph Editor;
- Hitbox/Hurtbox Editor;
- Material Graph;
- VFX Graph;
- Costume Editor;
- Weapon Editor;
- Encounter Editor;
- Camera Editor;
- Timeline/Event Editor;
- Asset Dependency Graph;
- Validation tools;
- Hot Reload;
- Build/Packager;
- Mod Manager.

## 12. AI / Combat Systems

Майбутні системи:

- Behavior Trees;
- Utility AI;
- Combat Navigation;
- attacker-slot coordination;
- Encounter Director;
- difficulty profiles;
- gameplay recording and replay validation.

## 13. Unified Profile / Save / Content

Один глобальний профіль може містити:

```text
Profile
├── DMC1 progress
├── DMC2 progress
├── DMC3 progress
├── DMC4 progress
├── Shared Costumes
├── Shared Gallery
├── Unlocks
├── Mods
└── Global Settings
```

Це відкриває можливість cross-campaign character/costume/loadout systems.

## 14. Scripting and Extensibility

Rengine має мати:

- native C++ extension layer;
- visual scripting for missions/triggers;
- runtime scripting layer for gameplay logic;
- package manifests;
- dependencies;
- sandboxed mod APIs where appropriate.

## 15. Після DMC1–4

Кінцева ціль не обмежується ремастером/рекомпіляцією старих ігор.

Після накопичення технологій DMC1–4 Rengine повинен дозволяти створювати повністю нові character-action games з нуля.

```text
Reverse DMC1–4
      ↓
Verified Compatibility Cores
      ↓
Unified Rengine Runtime
      ↓
Editors + Toolchain
      ↓
4 migrated campaigns
      ↓
New original campaigns / new games
```

## 16. Основний принцип розвитку

Не ламати доказову compatibility-частину заради модернізації.

```text
legacy / compatibility
        │
        ├── exact behavior
        ├── evidence
        └── rebuild/validation

modern rengine
        │
        ├── new APIs
        ├── new formats
        ├── new renderer
        ├── new gameplay systems
        └── new games
```

Це дозволяє одночасно зберігати точність реверсу і необмежено розвивати новий рушій.

## 17. Vision Statement

**DMC Rengine має стати єдиним сучасним character-action engine, який перетворює знання, отримані з реверс-інжинірингу DMC1–DMC4, у відкриту для розвитку систему runtime, редакторів, asset pipeline та gameplay-технологій. Чотири оригінальні гри мають бути перенесені на один Rengine як чотири кампанії, після чого той самий рушій повинен дозволяти створювати власні нові ігри.**
