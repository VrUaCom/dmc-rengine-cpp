# DMC3 LIG2 -> SHW shadow projection reverse — 2026-08-31

**Status:** CANONICAL RESEARCH ADDENDUM — CORRECTED  
**Canonical EXE SHA-256:** `e454272ed0fb0247fcbcf300e5d55d7a3e96d50b89b9ffaff81bb978dcbdd082`

## 1. Core result

`CDrawShadow` queries the active `CLightMgr`. The recovered shadow request selects LIG2/runtime light **category 4**, and the selected light's world-space position is used to derive SHW projection direction:

```text
shadow direction basis = selected category-4 light position - model/root position
```

Therefore the division of responsibility is:

```text
SHW -> shadow-caster geometry
LIG2 category 4 -> shadow light point / direction / tilt
ShadowDarkness / Softness / SoftRange -> visual appearance
```

## 2. Static LIG2 storage

Observed stage CFG resources:

```text
st001cfg.pac -> LIG2
st114cfg.pac -> LIG2
```

Each LIG2 is:

```text
0x20 header + 48 * 0x30 light records = 0x920 bytes
```

The original static registration loop visits all 48 records and skips records with byte `+0x00 == 0`.

Observed active counts:

```text
st001 -> 8
st114 -> 11
```

## 3. Corrected record map

```text
+0x00  u8   category/flags
              0 = inactive
              1 = objscr/shared object+stage
              2 = obj/object-model
              3 = scr/stage-SCM
              4 = shadow projection

+0x01  u8   minus/subtractive-light flag
+0x02  u8   distance-attenuation bypass mode

+0x10  f32  world position X
+0x14  f32  world position Y
+0x18  f32  world position Z

+0x1C  f32  inverse-force-squared coefficient = 1/(force^2)
+0x20  f32  inverse-range-squared coefficient = 1/(range^2)

+0x24  u16  R contribution
+0x26  u16  G contribution
+0x28  u16  B contribution
```

These promotions are backed by the StageSet `LIGHT` grammar and its runtime `CLight` encoder, not by field-shape guessing.

## 4. Correct range formula

The evaluator `0x1402EE2A0` computes the XYZ delta and calls `0x140030D30(delta, delta)`. The scalar result used is:

```text
distanceSquared = dx^2 + dy^2 + dz^2
```

When `+0x02 == 0`:

```text
weight = max(1 - distanceSquared * record[+0x20], 0)
```

Therefore the previous linear-distance wording is rejected.

Correct decode:

```text
range = 1 / sqrt(record[+0x20])
```

The StageSet helper `0x14032E900` independently proves the encoding by converting `srange/erange` with:

```text
rawRangeCoefficient = 1 / range^2
```

When `+0x02 != 0`, distance weighting is bypassed and the evaluator uses weight `1.0`.

## 5. Force and minus

StageSet `LIGHT` exposes:

```text
sforce / eforce
minus true/false
```

Runtime encodes:

```text
+0x1C = 1 / force^2
+0x01 = minus/subtractive flag
```

When the minus flag is set, the evaluator applies a `-1.0` transform to the light contribution vector.

## 6. Exact routing semantics for categories 1/2/3

The original StageSet `LIGHT` parser accepts:

```text
valid objscr -> category 1
valid obj    -> category 2
valid scr    -> category 3
```

This matches the draw consumers:

```text
CDraw default mask 2    -> category 1 + 2
CDrawSCM default mask 1 -> category 1 + 3
```

Thus `1/2/3` are routing categories, not point/spot/directional physical types.

## 7. Category 4 and SHW

`CDrawShadow` update obtains global `CLightMgr` and calls the light selector with query mask:

```text
0x1C
```

The special `0x10` selector bit admits category 4.

Observed category-4 static lights:

```text
st001 -> approximately (5000, 2500, 0)
st114 -> approximately (2500, 99998, 2500)
```

Both bypass distance attenuation.

A very distant category-4 source can approximate directional lighting geometrically, but category 4 should be named **shadow projection**, not generically `directional light`.

## 8. StageSet LIGHT is the dynamic/runtime light path

StageSet classifier `0x140246680` recognizes:

```text
LIGHT -> enum 8
```

and dispatches to `CStageSetLight` parser/factory `0x140263310`.

RTTI:

```text
.?AVCStageSetLight@@
```

Recovered grammar includes:

```text
valid
minus
spos
sforce
srange
srgb
epos
eforce
erange
ergb
life
move
...
```

`CStageSetLight` owns a runtime `CLight` and registers it through `0x1402EE9F0` into the second/runtime CLightMgr list observed by selector bit `0x08` through `CLightMgr +0x38`.

So DMC3 has at least two light sources in the same manager:

```text
static stage rig -> LIG2 / first pool
dynamic StageSet LIGHT -> runtime second pool
```

## 9. Model Set `Light 2`

The Model Set parser also has an independent field:

```text
Light 1 / Light 2
```

This chooses which lighting routing group a model uses:

```text
Light 1 -> draw mask 2 -> objscr + obj categories
Light 2 -> draw mask 1 -> objscr + scr categories
```

In the bounded real `m20_s00` corpus, five EFM clips explicitly contain `Light 2`, meaning those effect meshes are illuminated using the stage/SCM-facing light group.

This is separate from StageSet `LIGHT` creation and separate from LIG2 category 4 shadow projection.

## 10. Effect-bank boundary

`st001_effect.pac` and `st114_effect.pac` are PNST effect banks. They are not the static LIG2 rig.

The second CLightMgr pool is now proven to receive lights from **StageSet `LIGHT`**. It remains open whether `Effect`, `EffectI`, SEF, or other effect-bank execution can additionally create/register runtime `CLight` objects.

Do not claim `_effect.pac -> light` directly until that producer chain is traced.

## 11. Editor implication

An evidence-backed DMC Rengine Lighting Editor can now model:

```text
Static Stage Lights (LIG2)
  48 slots
  category/valid-for
  position XYZ
  range
  force
  RGB
  subtractive flag

Dynamic StageSet LIGHT
  valid obj / scr / objscr
  start position / force / range / RGB
  end position / force / range / RGB
  lifetime/interpolation

Model Lighting Selector
  Light 1 / Light 2

Shadow Projection
  category-4 source
  SHW caster geometry
  darkness / softness / soft range
```

## 12. Remaining validation gates

1. one-field live-game edit of static category-4 position -> record shadow direction change;
2. live-game StageSet LIGHT test with start/end position and range;
3. trace Effect/EffectI/SEF to determine whether they also produce runtime CLight;
4. recover remaining LIG2 header/tail fields;
5. writer only after byte-preserving round-trip and guarded patch validation.
