# Rengine Living Surface Genome — Procedural Hyperreal Character Technology

Дата фіксації: 2026-09-16
Статус: long-term R&D vision / not yet an implementation contract

## 1. Мета

Rengine має прагнути до гіперреалістичних персонажів без прямого масштабування обсягу ассетів через 4K/8K/16K текстури, надмірну кількість morph targets або гігантські статичні high-poly ресурси.

Основний принцип:

> Не зберігати кожну дрібну деталь персонажа як готові дані. Зберігати компактний опис, параметри та seed-и, а видимі мікродеталі генерувати детерміновано й адаптивно під час виконання.

Цільова концепція називається **Rengine Living Surface Genome (LSG)**.

```text
Identity Data
+ Character Genome
+ Procedural Rules
+ Shared Decoders
+ Runtime State
        ↓
Adaptive Living Surface
```

## 2. Чому не робити ставку на надвисоку роздільність текстур

Для більшості runtime-сцен унікальна інформація персонажа не потребує безмежного збільшення texture resolution. Базові 2K/Full-HD-class карти можуть зберігати макро- та identity-інформацію:

- колір і тон;
- родимки;
- шрами;
- татуювання;
- великі плями;
- унікальний макрорельєф;
- художньо контрольовані особливості.

Мікродеталі не повинні обов’язково існувати як великі baked textures:

- пори;
- фолікули;
- дрібні зморшки;
- мікроволоски;
- мікронерівність тканини;
- локальні подряпини;
- піт;
- мікрорельєф;
- дрібні варіації roughness.

Вони можуть бути породжені процедурно.

## 3. Character Genome

Character Genome — компактний набір параметрів, seed-ів і профілів, що визначають генерацію деталей персонажа.

Приклад концептуальної структури:

```cpp
struct SkinGenome {
    uint64_t seed;
    uint8_t pore_density;
    uint8_t pore_scale;
    uint8_t oiliness;
    uint8_t roughness_bias;
    uint8_t melanin;
    uint8_t redness;
    uint8_t age_profile;
    uint8_t scar_bias;
};
```

Окремі параметри можуть важити один або кілька байтів, але впливати на мільйони runtime-згенерованих мікродеталей.

Genome не повинен містити самі деталі. Він містить правила їх відтворення.

## 4. Deterministic DNA

Генерація повинна бути детермінованою.

```text
Seed + Parameters + Region
          ↓
Deterministic Generator
          ↓
Same visible result every run
```

Один 64-bit seed може задавати статистичний розподіл великої кількості процедурних деталей:

- пор;
- дрібних волосків;
- мікроплям;
- складок тканини;
- дрібних подряпин;
- поверхневих нерівностей.

Це не замінює художньо унікальні identity-елементи, але суттєво зменшує потребу зберігати випадкові/мікроскопічні деталі явно.

## 5. Shared Generator Principle

Важка технологія має жити один раз у рушії, а не дублюватися в кожному персонажі.

```text
Rengine
└── Universal Human/Surface Generator

Character A → genome + identity data
Character B → genome + identity data
Character C → genome + identity data
```

Умовно сотні мегабайт алгоритмів, моделей, LUT, shader code або shared neural weights можуть бути спільними для всіх персонажів, тоді як конкретний персонаж приносить лише компактний набір унікальних параметрів і ассетів.

## 6. Detail-on-Demand

Rengine не повинен генерувати деталі, які не можуть вплинути на фінальний кадр.

```text
Camera
  ↓
Visible pixel footprint
  ↓
Required physical detail scale
  ↓
Generate only visible detail bands
```

При великій відстані:

```text
macro material only
```

При крупному плані:

```text
pores + micro wrinkles + follicles
```

При екстремальному close-up:

```text
follicles + tiny hairs + moisture + capillary variation + micro-displacement
```

LOD для поверхні повинен визначатися не тільки дистанцією, а тим, скільки екранних пікселів покриває конкретна фізична деталь.

## 7. Multi-Scale Surface

Поверхня розбивається за масштабами:

```text
Identity scale
    ↓
Macro anatomy/material scale
    ↓
Meso detail
    ↓
Microstructure
    ↓
Sub-pixel statistical response
```

Кожен рівень може мати інше джерело даних:

- identity texture;
- geometry/morph/anatomical field;
- procedural displacement;
- generated normal/roughness;
- analytic/statistical BRDF response.

Це дозволяє не витрачати пам’ять на інформацію, яку вигідніше обчислити.

## 8. Surface Function

Кінцева поверхня повинна бути не просто `Texture(U,V)`, а функцією стану персонажа.

```text
Surface(
    position,
    body_region,
    pose,
    muscle_tension,
    pressure,
    temperature,
    wetness,
    damage,
    physiology,
    genome
)
→
    color
    normal
    roughness
    displacement
    subsurface
    transmission
```

Тобто surface state змінюється разом із тілом.

При стисканні кулака можуть змінюватися:

- напрямок зморшок;
- локальний натяг шкіри;
- форма/масштаб пор;
- roughness;
- локальна компресія тканин;
- кровонаповнення.

## 9. Living Surface Layers

LSG має працювати разом із Rengine Surface Stack:

```text
Identity Surface
+ Anatomy-driven deformation
+ Procedural Microstructure
+ Physiology
+ Dirt
+ Blood
+ Wetness
+ Burns
+ Cuts
+ Decals
+ DT / Magic effects
```

Шари не повинні обов’язково створювати повні копії texture sets. Перевага надається компактним masks, events, sparse state і процедурній реконструкції.

## 10. Runtime Physiology

Довгостроково персонаж може мати компактний physiological state:

```text
heart_rate
body_temperature
blood_flow
sweat_level
fatigue
stress_response
local_pressure
wetness
```

Ці параметри не є декоративними metadata. Вони можуть керувати shading і surface generation:

```text
physiology
  ↓
capillary colour variation
sweat/specular response
subsurface response
micro-wrinkle state
```

## 11. Hybrid Decoder

Rengine не повинен покладатися лише на нейромережу або лише на ручні procedural shaders.

Ціль — гібрид:

```text
Physical / anatomical rules
        +
Deterministic procedural generation
        +
Small shared neural decoder where useful
        ↓
Controlled reproducible result
```

Neural component повинен бути спільним, детермінованим у production mode і не замінювати artist control.

## 12. Storage Model

Цільова ідея полягає в тому, щоб значна частина видимої деталізації мала **нульовий або майже нульовий per-character storage cost**.

Концептуально:

```text
Base mesh                  MB
2K identity textures        MB
Skeleton / rig              MB or less
Animations                   shared/separate pool
Character genome             KB
Persistent surface state     KB–MB depending on events
Generated microdetail        0 stored bytes in canonical asset
```

Це не означає, що весь високоякісний персонаж реально може важити лише кілобайти. Унікальна геометрія, identity textures, voice, animations та authored assets залишаються. Але мікродеталь не повинна масштабувати storage linearly разом із бажаною візуальною якістю.

## 13. Integration with Rengine Character System

```text
RengineCharacter
├── Skeleton
├── Base Mesh
├── Morph/Anatomy Data
├── Identity Textures
├── Material Set
├── LivingSurfaceGenome
├── Physiology State
├── Surface State
├── Hair/Cloth Profiles
├── Costume Layers
└── CombatDNA
```

Legacy DMC1–DMC4 model formats залишаються import/compatibility formats. LSG належить лише modern Rengine layer і не повинен змінювати доказову legacy-поведінку.

## 14. Potential R&D Modules

Можливі майбутні підсистеми:

- `LivingSurfaceGenome`;
- `MicrostructureGenerator`;
- `AdaptiveDetailScheduler`;
- `AnatomicalField`;
- `PhysiologySurfaceDriver`;
- `ProceduralHairMicroLayer`;
- `ProceduralClothMicroLayer`;
- `SparseDamageState`;
- `SurfaceStateBaker`;
- `SharedSurfaceDecoder`;
- `SurfaceFunctionRuntime`.

## 15. Research Questions

Перед імплементацією потрібно окремо дослідити:

- які detail bands реально вигідніше генерувати, а не зберігати;
- GPU cost проти texture bandwidth/storage;
- temporal stability procedural microdetail;
- deterministic generation across GPU vendors;
- mip/filtering strategy для procedural fields;
- interaction із ray tracing / path tracing;
- memory residency і caching generated tiles;
- authoring workflow для художника;
- save-state compression для persistent damage;
- shared neural decoder size/latency/quality;
- fallback paths для слабкого hardware.

## 16. Головний принцип

**Зберігати не деталі, а компактні причини їх виникнення.**

```text
Traditional approach:
more realism → more textures → more geometry → more storage

Rengine LSG approach:
more realism → better rules + better shared decoder + adaptive generation
```

## 17. Vision Statement

**Rengine Living Surface Genome має перетворити персонажа з набору великих статичних texture/mesh resources на компактно описану, детерміновано відтворювану живу поверхню. Унікальні identity-дані зберігаються явно, тоді як мікрогеометрія, пори, дрібні зморшки, поверхневі варіації, частина фізіологічних і damage-ефектів генеруються процедурно та лише в тій деталізації, яка реально може вплинути на кадр.**
