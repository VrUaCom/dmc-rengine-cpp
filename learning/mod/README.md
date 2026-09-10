# DMC3 `.MOD` Learning Hub

Ця папка — навчальний шар DMC Rengine для формату `.MOD` Devil May Cry 3 HD Collection. Вона пояснює канонічний reverse простими шарами, але **не є окремою технічною authority**: остаточна істина залишається у canonical C++ modules, `docs/research/`, `data/reverse/` і перевіреному `main` після promotion.

Канонічний executable: `dmc3.exe`, SHA-256 `e454272ed0fb0247fcbcf300e5d55d7a3e96d50b89b9ffaff81bb978dcbdd082`.

## Статус курсу

**MOD reverse complete for the canonical DMC3 HD scope.**

Це означає, що всі домени serialized/runtime контракту мають термінальний evidence-статус: typed semantic, preservation-only terminal state, reserved observation або rejected hypothesis. `PRESERVED_UNDECODED` більше не трактується як «ще не дореверсили» — це навмисна фінальна класифікація там, де сильнішої семантичної назви доказів немає.

Окремо продовжується writer/authoring/integration acceptance. Reverse completeness не означає unrestricted writer.

## Evidence baseline

```text
38 unique MOD
166 objects
180 meshes
20,976 vertices
285 transforms
```

Corpus охоплює `em000`, незалежні `pl000` model resources та `id100`. Corpus invariant не стає universal rule без незалежного доказу.

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
        +--> objects / meshes / streams
        +--> node hierarchy + rest transforms
        +--> skin binding
        +--> texture slots
        |
        v
  runtime model manager
        |
        +--> texture companion
        +--> MOT / CMotion evaluated locals
        +--> currentWorld
        +--> inverseRest * currentWorld
        v
     GPU renderer
```

## Що закрито reverse

Канонічно закриті:

- document/object/mesh ABI;
- hierarchy/order domain;
- local/world transforms;
- default joint behavior;
- skin indices/packed weights;
- topology generation;
- texture slot + legacy GS CLAMP;
- texture companion/runtime descriptor binding;
- object runtime projection;
- MOD-side animation-binding boundary;
- post-load relocation;
- preservation policy для всіх непідтверджених семантичних назв.

Ключові closure outcomes:

- header `+0x14` — raw runtime-carried `u32`; universal decimal family/model/sub-index interpretation для MOD **REJECTED**;
- `BLENDINDICES.x` — preserved ABI byte; canonical compiled shader path читає Y/Z/W;
- source flag `0x00100000` — EXE-confirmed legacy GS TEST/ZBUF selector без вигаданого material label;
- source flag `0x00200000` — runtime-carried/restored terminal preservation state;
- mesh `+0x38` — family-sensitive: inactive в audited MOD path, homologous EFM slot live;
- mesh `+0x0C/+0x4C` і transform `+0x1C` — explicit source-preservation terminal states.

## Що вже доведено як writer/container chain

Канонічно promoted:

- **Preserve-Layout Writer Gate 1** (#365): immutable source, source reparse, explicit fixed-size authorized spans, unauthorized-byte rejection, output reparse;
- deterministic writer corpus runner (#367);
- provenance-bound retail no-op parity (#368): **38/38**, 882,736 bytes, exact byte equality, 0 modified bytes, 0 failures;
- один provenance-bound real retail edit (#369): `em000_021.mod`, `object[0].bounding_radius`, span `[124,128)`, рівно 3 changed bytes, решта preserved;
- writer receipt -> `AuthoredChildImage` trust bridge + synthetic PAC reintegration (#369);
- provenance-bound **real retail PNST reintegration** (#372): `m20_s00_012.pac`, physical slot 23, parent size unchanged, slot table unchanged, only expected child bytes changed, canonical reopen returns exact authored MOD;
- synthetic **MOD -> container -> NBZ overlay -> reopen** (#372) через існуючі NBZ writer/source компоненти.

Це сильна bounded authoring chain, але не unrestricted writer і не original-game acceptance.

## Ключове правило

Не змішуй:

- **reverse completeness**;
- **serialized layout**;
- **runtime behavior**;
- **semantic naming**;
- **writer authority**;
- **container reintegration authority**;
- **original-game acceptance**.

MOD reverse already complete. Writer and original-runtime acceptance are separate programs.

## Canonical code entry points

- `include/dmc_rengine/formats/mod.hpp`
- `src/formats/mod.cpp`
- `include/dmc_rengine/formats/mod_writer.hpp`
- `include/dmc_rengine/formats/mod_writer_corpus.hpp`
- `include/dmc_rengine/formats/mod_skin.hpp`
- `include/dmc_rengine/profiles/dmc3/mod_authored_child_bridge.hpp`
- `include/dmc_rengine/profiles/dmc3/nested_relative_slot_reintegrator.hpp`
- `include/dmc_rengine/profiles/dmc3/nbz_overlay_writer.hpp`
- `include/dmc_rengine/formats/mod/transform_domain.hpp`
- `include/dmc_rengine/formats/mod/world_transform.hpp`
- `include/dmc_rengine/analysis/mod/object_flags.hpp`
- `include/dmc_rengine/analysis/mod/mesh_serialized.hpp`
- `include/dmc_rengine/analysis/mod/animation_binding.hpp`
- `include/dmc_rengine/analysis/mod/texture_binding.hpp`

## Що залишається після reverse completion

Це вже не MOD reverse backlog:

- typed-IR-only layout synthesis/reflow;
- transform/skin/material authoring;
- texture-companion rewriting;
- broader mutation authority;
- provenance-bound retail NBZ acceptance;
- original-game authored-MOD acceptance and rollback.

Сайт і курс можуть називати MOD **reverse complete / fully reverse-engineered within the canonical DMC3 HD scope**, але не `full writer` або arbitrary original-game-safe editing.
