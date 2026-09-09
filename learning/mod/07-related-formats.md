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

SCM не має MOD-style BLENDINDICES/packed skin stream у підтвердженому path. Не переносити SCM alpha rewrites або `LegacyResourceCode` semantics у MOD header `+0x14`: broader MOD corpus прямо відхилив стару universal decimal identity interpretation для цього поля.

## EFM

EFM — effect-system model family. MOD/EFM share partial runtime infrastructure, зокрема transform initializer `0x1402FA080` та homologous object/material paths.

Новий `mesh+0x38` reverse є хорошим positive control для правила “same offset != same semantic”:

```text
MOD +0x38:
  current corpus 180/180 zero
  canonical MOD runtime builder does not forward it
  corresponding auxiliary runtime positions are disabled

EFM +0x38:
  homologous physical slot is live
  forwarded into COLOR0-facing runtime state
```

Тому EFM допомагає довести, що physical slot реальний, але **не** дає права назвати MOD `+0x38` COLOR0 або padding. MOD raw source value лишається preservation authority.

## MOT / CMotion

MOT/CMotion — animation layer.

MOD дає skeleton domain, rest pose, motion group і skin binding.
MOT/CMotion дає evaluated pose.

Це найближчий runtime partner MOD для анімації, але schemas мають залишатися окремими. Новий main уже має окремий MOT structural reader/contract; це ще сильніше підкреслює, що MOT semantics не треба “запихати” в MOD parser.

## SHW

SHW — self-contained shadow-hull mesh:

- float4 positions;
- triangles;
- adjacency;
- per-vertex transform-matrix selector.

EXE-confirmed selector вибирає `0x40` matrix. Exact selected palette ownership і mapping до MOD node/bone ще open. Тому не робимо shortcut `SHW selector == MOD bone`.

## SO

SO більше не варто описувати лише як cardinality correlation.

У поточному reader/reverse path link-table third byte незалежно ідентифікований як **node selector**: volume records можуть повторно посилатися на один node, а чотири записи, прив’язані до root, мають selector `0`. `analysis::so::mod_binding` зв’язує цей selector із MOD transform domain.

Це підтверджує cross-resource relationship:

```text
SO volume/link node selector -> MOD transform-domain node identity
```

але не робить SO physical records частиною MOD binary format. SO graph/link/volume tables лишаються окремою resource family і окремими parsers.

## TM2 / texture companion

Texture companion — зовнішній model texture source envelope. MOD mesh лише вибирає slot.

TM2 — payload всередині companion allocation.

## DDS / PTX

DDS і PTX — інші texture representations у DMC Rengine. Вони не є MOD mesh fields. Conversion/UI може об’єднувати їх на високому рівні, binary contracts окремі.

Current texture reader також уже розрізняє wrapped DDS і PTX/bundle framing; це допомагає UI/Native Reader, але не змінює serialized MOD texture-slot ABI.

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
