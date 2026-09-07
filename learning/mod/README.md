# DMC3 `.MOD` Learning Hub

Ця папка — навчальний шар DMC Rengine для формату `.MOD` Devil May Cry 3 HD Collection. Вона пояснює канонічний reverse простими шарами, але **не є окремою технічною authority**: остаточна істина залишається у `main`, canonical C++ modules, `docs/research/` та `data/reverse/`.

Канонічний executable: `dmc3.exe`, SHA-256 `e454272ed0fb0247fcbcf300e5d55d7a3e96d50b89b9ffaff81bb978dcbdd082`.

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
- `include/dmc_rengine/analysis/mod/motion_group.hpp`
- `include/dmc_rengine/analysis/mod/animation_binding.hpp`
- `include/dmc_rengine/analysis/mod/mesh_serialized.hpp`
- `include/dmc_rengine/analysis/mod/object_runtime.hpp`
- `include/dmc_rengine/analysis/mod/texture_binding.hpp`

## Поточна межа

Read/reverse підтримка MOD сильна й evidence-backed. Spatial hierarchy, skin ABI, inverse-rest palette, texture binding і MOD-side animation binding канонізовані. Production MOD writer, broad no-edit byte parity, safe mutation ranges і original-game edited-MOD acceptance ще не закриті.
