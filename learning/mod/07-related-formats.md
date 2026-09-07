# Урок 7 — Як `.MOD` пов’язаний з іншими форматами

## SCM

Спільні model-family частини:

- `0x40` document shell;
- `0x40` object core;
- `0x50` mesh core;
- texture slot;
- GS CLAMP;
- position/normal/fixed UV;
- local-transform core;
- shared material helper.

Але:

```text
SCM = static/stage scene geometry
MOD = skinned model/skeleton geometry
```

SCM не має MOD-style BLENDINDICES/packed skin stream у підтвердженому path. Не переносити SCM alpha rewrites або `+0x14` semantics в MOD.

## EFM

EFM — effect-system model family. MOD/EFM share partial runtime infrastructure, зокрема transform initializer `0x1402FA080` та homologous object/material path.

Правило: shared helper тільки після independent proof. «Однаковий offset» не достатній.

## MOT / CMotion

MOT/CMotion — animation layer.

MOD дає skeleton domain, rest pose, motion group і skin binding.
MOT/CMotion дає evaluated pose.

Це найближчий runtime partner MOD для анімації, але schemas мають залишатися окремими.

## SHW

SHW — self-contained shadow-hull mesh:

- float4 positions;
- triangles;
- adjacency;
- per-vertex transform-matrix selector.

EXE-confirmed selector вибирає `0x40` matrix. Exact selected palette ownership і mapping до MOD node/bone ще open. Тому не робимо shortcut `SHW selector == MOD bone`.

## SO

`analysis::so::analyze_mod_binding()` порівнює MOD domain cardinality із SO link/volume tables.

Це корисно для cross-resource correlation, але не дає semantic identity автоматично.

## TM2 / texture companion

Texture companion — зовнішній model texture source envelope. MOD mesh лише вибирає slot.

TM2 — payload всередині companion allocation.

## DDS / PTX

DDS і PTX — інші texture representations у DMC Rengine. Вони не є MOD mesh fields. Conversion/UI може об’єднувати їх на високому рівні, binary contracts окремі.

## PAC / PNST

Контейнерний parent layer. MOD часто знаходиться в slot. Майбутній writer має rebuild/reintegrate parent container, а не просто записати standalone file.

## NBZ

Volume/distribution layer. Final authoring pipeline може публікувати higher-number overlay NBZ, але тільки після typed child/container rebuild та reopen.

## `.index`

Naming/extraction metadata. Не runtime manifest MOD і не джерело runtime texture/skeleton semantics.

## Формула здорової архітектури

```text
container formats
    !=
resource formats
    !=
runtime descriptors
    !=
cross-resource analysis
```

В UI це може виглядати як одна модельна сцена, але binary authority повинна лишатися модульною.
