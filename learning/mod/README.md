# DMC3 `.MOD` Learning Hub

Ця папка — навчальний шар DMC Rengine для формату `.MOD` Devil May Cry 3 HD Collection. Вона пояснює канонічний reverse простими шарами, але **не є окремою технічною authority**: остаточна істина залишається у canonical C++ modules, `docs/research/`, `data/reverse/` і перевіреному `main` після promotion.

Канонічний executable: `dmc3.exe`, SHA-256 `e454272ed0fb0247fcbcf300e5d55d7a3e96d50b89b9ffaff81bb978dcbdd082`.

## Поточний evidence baseline

Bound multi-corpus pass, який лежить під актуальними уроками:

```text
38 unique MOD
166 objects
180 meshes
20,976 vertices
285 transforms
```

Він охоплює не лише `em000`, а й `pl000` main/cloth-associated model resources та `id100`. Це ширша evidence base, але все одно не привід перетворювати corpus invariant на universal rule без independent proof.

## Як проходити курс

1. [Що таке MOD і де він живе](01-what-is-mod.md)
2. [Document, objects і meshes](02-document-objects-meshes.md)
3. [Скелет, node domain і transforms](03-skeleton-hierarchy-transforms.md)
4. [Skinning, topology і shader ABI](04-skinning-topology-shader.md)
5. [Textures, companion і material runtime](05-textures-materials.md)
6. [Animation, motion groups і MOT/CMotion boundary](06-animation-motion-groups.md)
7. [Зв’язки з SCM, EFM, SHW, SO та контейнерами](07-related-formats.md)
8. [Evidence, unknowns і безпечний reverse](08-evidence-and-unknowns.md)
9. [Практичний reverse workflow](09-reverse-workflow.md)
10. [Практичні вправи та debugging](10-exercises-debugging.md)
11. [Швидка reference map](REFERENCE-MAP.md)
12. [Глосарій](GLOSSARY.md)

## Головна модель

```text
NBZ / PAC / PNST
        |
        v
   serialized MOD
        |
        +--> object + mesh state
        +--> node hierarchy + rest transforms
        +--> skin indices + weights
        +--> texture slots
        |
        v
  runtime model manager
        |
        +--> external texture companion -> runtime descriptors
        +--> CMotion/MOT -> evaluated animated locals
        +--> currentWorld
        +--> inverseRest * currentWorld
        v
     GPU renderer
```

## Що змінив 2026-09-09 closure pass

Курс більше не вчить кілька старих intermediate hypotheses як canonical truth:

- MOD header `+0x14` — raw runtime-carried `u32`; universal decimal family/model/sub-index interpretation **REJECTED** broader corpus evidence;
- `BLENDINDICES.x` — canonical compiled shader path reads Y/Z/W (`ReadWriteMask = 0xE`), а X лишається raw preserved ABI byte;
- source flag `0x00100000` — EXE-confirmed legacy GS `TEST_1` AREF / `ZBUF_1` ZMSK selector, без вигаданого artistic/material label;
- source flag `0x00200000` — runtime-carried/restored state, але terminal semantic consumer ще open;
- mesh `+0x38` — family-sensitive slot: inactive в audited MOD path, але homologous EFM slot live як COLOR0;
- mesh `+0x0C/+0x4C` і transform `+0x1C` — preserved undecoded, не padding лише тому, що bound corpus zero.

## Ключове правило

Не змішуй чотири різні речі:

- **serialized layout** — що лежить у байтах;
- **runtime behavior** — що з цими байтами робить `dmc3.exe`;
- **semantic name** — що поле означає для автора/движка;
- **writer authority** — чи доведено, що ми можемо безпечно генерувати/редагувати його.

Reader success не означає writer authority. Однаковий offset у SCM/MOD/EFM не означає однакову семантику. Runtime image не можна просто записати назад як serialized MOD.

## Canonical code entry points

- `include/dmc_rengine/formats/mod.hpp`
- `src/formats/mod.cpp`
- `include/dmc_rengine/formats/mod_skin.hpp`
- `include/dmc_rengine/formats/mod/transform_domain.hpp`
- `include/dmc_rengine/formats/mod/world_transform.hpp`
- `include/dmc_rengine/analysis/mod/corpus_observations.hpp`
- `include/dmc_rengine/analysis/mod/object_flags.hpp`
- `include/dmc_rengine/analysis/mod/mesh_serialized.hpp`
- `include/dmc_rengine/analysis/mod/secondary_serialized.hpp`
- `include/dmc_rengine/analysis/mod/unresolved_field_evidence.hpp`
- `include/dmc_rengine/analysis/mod/motion_group.hpp`
- `include/dmc_rengine/analysis/mod/animation_binding.hpp`
- `include/dmc_rengine/analysis/mod/object_runtime.hpp`
- `include/dmc_rengine/analysis/mod/texture_binding.hpp`

## Поточна межа

Read/reverse підтримка MOD сильна й evidence-backed. Spatial hierarchy, skin ABI, inverse-rest palette, texture binding, MOD-side animation binding і кілька legacy renderer-state semantics канонізовані або суттєво звужені.

Але **production MOD writer**, broad no-edit byte parity, safe mutation rules, deterministic rebuild/reintegration і original-game edited-MOD acceptance ще не закриті. До проходження цих gates сайт і курс не мають права називати MOD “100% reversed” або довільно editable.
