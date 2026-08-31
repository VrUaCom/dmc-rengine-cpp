# DMC3 shadow parameter controls reverse — 2026-08-31

**Status:** CANONICAL RESEARCH ADDENDUM  
**Canonical EXE SHA-256:** `e454272ed0fb0247fcbcf300e5d55d7a3e96d50b89b9ffaff81bb978dcbdd082`

## 1. Boundary

The SHW document owns shadow-caster geometry/topology. The visual tuning parameters recovered here are separate Model Set/configuration fields and are not stored in the SHW geometry document.

The Model Set parser independently resolves the `Shadow` resource and attaches it to the model runtime. A later LIG2 reverse pass additionally proves that shadow projection direction/tilt comes from the active light manager rather than from SHW geometry or `ShadowParamSet`.

Companion direction evidence:

- `dmc3-lig2-shadow-projection-reverse-2026-08-31.md`
- `data/reverse/dmc3-lig2-shadow-projection-20260831.json`

## 2. Direct executable tokens

Canonical executable strings:

```text
ShadowParamSet   @ 0x140507028
ShadowDarkness   @ 0x140507038
ShadowSoftness   @ 0x140507048
ShadowSoftRange  @ 0x140507058
```

Base resource token:

```text
Shadow           @ 0x140506DB0
```

The four tuning tokens are consumed in the Model Set/config parser region around `0x1402D78D0`.

## 3. Parsed storage and value type

Recovered parser/storage mapping:

```text
ShadowParamSet
  integer/byte read
  -> config +0x80

ShadowDarkness
  integer/byte read
  -> config +0x81

ShadowSoftness
  integer/byte read
  -> config +0x82

ShadowSoftRange
  float read
  -> config +0x84
```

Evidence-safe semantic boundary:

- `ShadowDarkness` is the shadow darkness/intensity-style control named directly by the original executable.
- `ShadowSoftness` is the shadow softness control named directly by the original executable.
- `ShadowSoftRange` is a floating-point softness/range control named directly by the original executable.
- `ShadowParamSet` selects a shadow parameter set/preset, but the exact contents selected by each value remain partially open.

## 4. Recovered defaults

The configuration initializer around `0x1402D736B` establishes:

```text
ShadowParamSet  = 0
ShadowDarkness  = 0x60 (96)
ShadowSoftness  = 2
ShadowSoftRange = 2.1f
```

These are engine defaults for the recovered configuration object; they are not claimed as universal desired values for every model/stage.

## 5. Visibility toggle is separate

Observed configuration/script material also uses `Shadow 0` / `Shadow 1` as an enable/disable-style runtime visibility/control operation. This must be kept separate from the `Shadow <resource.shw>` resource-binding form and from `ShadowDarkness` / `ShadowSoftness` tuning.

Exact context distinguishes the forms; tooling must not collapse all occurrences of `Shadow` into one field.

## 6. Shader corroboration

Embedded `DMC3_SHW.hlsl` uses a shadow colour constant (`sdwColor`) and a position-only vertex input. This is consistent with the recovered architecture:

```text
SHW document
  -> caster geometry/topology

Model Set / shadow runtime
  -> darkness / softness / range / parameter-set controls

LIG2 / CLightMgr
  -> shadow projection light point / direction

renderer constants
  -> final shadow colour/projection state
```

## 7. Direction / tilt / projection source — CLOSED for the recovered static-light path

There is still no configuration token named `ShadowAngle`, `ShadowDirection`, or `ShadowSlope` because the direction is not authored as an independent angle field in the recovered path.

`CDrawShadow` update near `0x14008BCF0` obtains the active global `CLightMgr` and calls `0x14031FA80`. That function queries `CLightMgr` through `0x1402EE560` with mask `0x1C`.

The recovered light-filter logic causes this shadow query to select the special LIG2 type/category `4` while ordinary type-1/2/3 categories are not selected by this query mask.

The selected LIG2 record provides its world-space position from:

```text
+0x10 f32 X
+0x14 f32 Y
+0x18 f32 Z
```

The shadow projection direction/tilt is derived from:

```text
selected type-4 LIG2 position - model/root world position
```

and consumed in the downstream shadow-projection math around `0x14031F830`.

Therefore:

```text
Shadow direction / tilt source = LIG2 type-4 light position
```

**Status: EXE_CONFIRMED** for the recovered static-light path.

Observed examples:

```text
st001 type-4 shadow-selected light ~= (5000, 2500, 0)
st114 type-4 shadow-selected light ~= (2500, 99998, 2500)
```

The very distant coordinate in `st114` is consistent with a nearly directional vector across a bounded stage, but `type 4 == directional light` is not promoted as an exact general-purpose type name until the wider light-type system is fully recovered.

If no qualifying light is returned, the shadow preparation path has a fallback offset approximately `(200, 300, 50)`.

`Rot`/model-transform fields remain model transforms; they are not the shadow-light direction control.

## 8. Editor implication

Evidence-safe controls that DMC Rengine can expose once parser/writer support is implemented:

```text
Shadow resource      -> SHW binding
Shadow enabled       -> context-specific enable/disable
Shadow darkness      -> ShadowDarkness
Shadow softness      -> ShadowSoftness
Shadow soft range    -> ShadowSoftRange
Shadow parameter set -> ShadowParamSet (preset/set; exact semantics partial)

Shadow direction     -> derived from selected LIG2 type-4 light position
Shadow tilt/angle    -> derived from the same model-to-light vector
```

A professional editor should not store a fake `ShadowAngle` field. Instead it should expose the actual light source spatially:

```text
Shadow Projection Light
  X / Y / Z
  gizmo in scene
  derived azimuth/elevation preview
```

Moving that light changes the projection vector while SHW itself continues to describe caster geometry.

## 9. Ownership summary

```text
SHW
  = shape of the shadow caster

LIG2 type 4
  = where the shadow light is
  = direction / tilt source

ShadowDarkness / ShadowSoftness / ShadowSoftRange
  = how the shadow looks
```

## 10. Remaining validation gates

- one-field live-game test moving only the active type-4 LIG2 position;
- exact semantic names of general LIG2 types 1/2/3;
- exact normalization of LIG2 colour channels;
- exact contents selected by `ShadowParamSet`;
- whether dynamic/effect-created lights can feed the same `CLightMgr` shadow query.