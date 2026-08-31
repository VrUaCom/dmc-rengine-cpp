# DMC3 LIG2 light-category routing reverse — 2026-08-31

**Status:** CANONICAL RESEARCH ADDENDUM — CORRECTED  
**Canonical EXE SHA-256:** `e454272ed0fb0247fcbcf300e5d55d7a3e96d50b89b9ffaff81bb978dcbdd082`

## 1. Main result

The first LIG2 record byte is **not** a physical `point / spot / directional` enum in the recovered runtime. Its low values are primarily **lighting routing categories/flags**.

The original StageSet `LIGHT` grammar independently exposes spatial/physical controls (`spos`, `sforce`, `srange`, `srgb`, and matching end values), proving that routing category and physical light parameters are separate concepts.

## 2. Exact selector rules

Light selector:

```text
0x1402EE560
```

Recovered category filtering:

```text
category 1 -> accepted when routing mask contains 0x01 OR 0x02
category 2 -> accepted when routing mask contains 0x02
category 3 -> accepted when routing mask contains 0x01
category 4 -> special shadow category accepted when mask contains 0x10
```

Light source pools are selected independently:

```text
query bit 0x04 -> scan CLightMgr static/list-0 pool at +0x20
query bit 0x08 -> scan CLightMgr runtime/list-1 pool at +0x38
```

The runtime-list branch also treats bit `0x08` in the raw category byte specially, so `record +0x00` must remain modeled as a raw **category/flags** byte.

## 3. Category 1/2/3 semantics are now tied to original grammar

### General CDraw / MOD-EFM default

`CDraw` constructor `0x140089270` initializes routing mask `2` at `CDraw +0x744`.

The normal model-lighting path therefore accepts:

```text
category 1
category 2
```

### CDrawSCM default

`CDrawSCM` constructor `0x140089320` initializes routing mask `1` at `CDrawSCM +0x572`.

The SCM-lighting path therefore accepts:

```text
category 1
category 3
```

### Original StageSet LIGHT `valid` field

`LIGHT` is directly recognized by StageSet classifier `0x140246680` as enum `8`, whose dispatcher entry calls the `CStageSetLight` parser/factory at `0x140263310`.

That parser accepts:

```text
valid obj
valid scr
valid objscr
```

and maps them into the dynamic `CLight` category field as:

```text
valid objscr -> category 1
valid obj    -> category 2
valid scr    -> category 3
```

This is direct grammar-to-runtime evidence.

Therefore the canonical category semantics are:

| Raw low value | Original routing meaning | Status |
|---:|---|---|
| `0` | inactive / not registered | EXE_CONFIRMED |
| `1` | `objscr`: shared object + screen/stage lighting | EXE_CONFIRMED |
| `2` | `obj`: object/general model lighting | EXE_CONFIRMED |
| `3` | `scr`: screen/stage/SCM lighting | EXE_CONFIRMED |
| `4` | SHW shadow-projection category | EXE_CONFIRMED |

Do **not** relabel `1/2/3` as point/spot/directional.

## 4. `Light 1 / Light 2` in Model Set is a lighting-group selector

The Model Set parser recognizes the literal field:

```text
Light <integer>
```

and stores it at parsed config `+0x8F2`.

Downstream model construction converts the values as follows:

```text
Light 1 -> draw routing mask 2 -> categories 1 + 2 (`objscr` + `obj`)
Light 2 -> draw routing mask 1 -> categories 1 + 3 (`objscr` + `scr`)
```

This explains real local corpus entries such as:

```text
Model ss509_15.efm
Texture ss509_m20.ptx
...
Light 2
```

Five EFM Model Set files in the bounded `m20_s00` corpus explicitly use `Light 2`. Their practical meaning is: **light this EFM through the screen/stage light routing group instead of the normal object/model group**.

This is likely the `Light 2` remembered from the stage/demo resources; it is not a second LIG2 file and not a second physical lamp by itself.

## 5. StageSet LIGHT is a real runtime/dynamic light authoring path

RTTI identifies:

```text
.?AVCStageSetLight@@
```

The StageSet `LIGHT` parser accepts at least:

```text
move
valid
minus
life
type
spos
sforce
srange
srgb
epos
eforce
erange
ergb
bwait
await
const
```

The key start/end light controls are:

```text
spos    X Y Z
sforce  float
srange  float
srgb    R G B

epos    X Y Z
eforce  float
erange  float
ergb    R G B
```

So the original engine can describe a light with a start state and end state and update/interpolate its spatial and colour parameters over its clip lifetime.

## 6. StageSet LIGHT feeds the second CLightMgr pool

`CStageSetLight` owns an embedded runtime `CLight`. Its update path synchronizes the authoring values into the runtime light record and, when active, calls:

```text
0x1402EE9F0(CLight, global CLightMgr)
```

`0x1402EE9F0` inserts the light into the list object rooted at approximately:

```text
CLightMgr + 0x30
```

whose first/list-head field is the same second light pool observed by selector query bit `0x08` through `CLightMgr +0x38`.

Therefore:

> **StageSet `LIGHT` is a proven producer of the second/runtime CLightMgr light pool.**

This is stronger than the earlier statement that the second pool merely existed.

It still does **not** prove that bytes inside `st###_effect.pac` directly create those lights. The recovered producer here is the StageSet `LIGHT` runtime/script system.

## 7. Correct range mathematics — prior interpretation corrected

The evaluator `0x1402EE2A0` forms:

```text
delta = lightPosition - queryPosition
```

and calls `0x140030D30(delta, delta)`. For the scalar lane used by the evaluator, this computes:

```text
distanceSquared = dx*dx + dy*dy + dz*dz
```

When `record +0x02 == 0`:

```text
weight = max(1 - distanceSquared * record[+0x20], 0)
```

Therefore the earlier interpretation:

```text
range = 1 / falloff
```

is **REJECTED/CORRECTED**.

The correct zero-contribution radius is:

```text
range = 1 / sqrt(record[+0x20])
```

when `record[+0x20] > 0`.

This is independently corroborated by StageSet LIGHT: `srange` / `erange` are transformed by helper `0x14032E900`, which computes exactly:

```text
1 / value^2
```

before storing the runtime coefficient corresponding to raw light record `+0x20`.

Thus the evidence-safe field name is:

```text
+0x20 = inverse-range-squared coefficient
```

and an editor should expose the human value as `Range`, converting on write with `1 / range^2`.

When `record +0x02 != 0`, the evaluator bypasses distance attenuation and sets distance weight to `1.0`.

## 8. `force` field promoted

The same StageSet path converts:

```text
sforce / eforce -> 1 / force^2
```

and stores it in the runtime field corresponding to raw light record:

```text
+0x1C
```

Therefore `+0x1C` is no longer an anonymous contribution parameter. It is an **inverse-force-squared coefficient derived from the original `force` authoring parameter**.

The exact downstream visual interpretation of `force` beyond this conversion remains to be described carefully, but its source semantic and encoding are EXE-confirmed.

## 9. `minus` flag promoted

StageSet LIGHT accepts:

```text
minus true/false
```

This flows into raw runtime light record byte:

```text
+0x01
```

The evaluator checks this byte and applies a `-1.0` contribution transform to the colour/contribution vector.

Evidence-safe field:

```text
+0x01 = subtractive/minus-light flag
```

## 10. Colour

The evaluator reads:

```text
+0x24 u16
+0x26 u16
+0x28 u16
```

and converts them to three floating contribution channels. StageSet grammar calls the corresponding values:

```text
srgb
ergb
```

Therefore these fields are now safely named **RGB light contribution channels**. Final shader normalization/display colour-space details remain a separate rendering question.

## 11. Correct editor model

Do not expose a misleading physical-type dropdown.

Use:

```text
Routing / Valid For
  Object + Stage   (category 1 / objscr)
  Object / Model   (category 2 / obj)
  Stage / SCM      (category 3 / scr)
  Shadow Projection(category 4)

Position X/Y/Z
Range
Force
RGB
Subtractive / Minus
Distance attenuation mode
```

For animated StageSet lights additionally expose:

```text
Start Position / Force / Range / RGB
End   Position / Force / Range / RGB
Life / interpolation controls
```

## 12. Remaining boundary

The static LIG2 rig is confirmed in `st###cfg.pac`. StageSet `LIGHT` is confirmed as a runtime producer of the second CLightMgr pool.

Still open:

1. trace whether `Effect`/`EffectI` or SEF/effect-bank execution can also create/register `CLight` objects;
2. recover exact meaning of StageSet LIGHT `type` and movement/interpolation modes;
3. validate one edited static LIG2 light and one StageSet dynamic light in-game;
4. implement parser/editor only with source-byte guards and round-trip validation.
