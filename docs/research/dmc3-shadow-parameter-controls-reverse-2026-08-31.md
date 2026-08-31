# DMC3 shadow parameter controls reverse — 2026-08-31

**Status:** CANONICAL RESEARCH ADDENDUM  
**Canonical EXE SHA-256:** `e454272ed0fb0247fcbcf300e5d55d7a3e96d50b89b9ffaff81bb978dcbdd082`

## 1. Boundary

The SHW document owns shadow-caster geometry/topology. The visual tuning parameters recovered here are separate Model Set/configuration fields and are not stored in the SHW geometry document.

The Model Set parser already independently resolves the `Shadow` resource and attaches it to the model runtime. This pass recovers the additional shadow-tuning tokens used by the same configuration system.

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
- `ShadowParamSet` selects a shadow parameter set/preset, but the exact contents selected by each value are not yet recovered. Do not infer angle/direction from this field without a downstream table/consumer.

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

renderer constants
  -> final shadow colour/projection state
```

## 7. Direction / tilt / projection angle remains open

No direct configuration token equivalent to:

```text
ShadowAngle
ShadowDirection
ShadowSlope
```

was recovered in the current bounded token/parser pass.

Therefore shadow direction/tilt must not be assigned to SHW or to `ShadowParamSet` by guesswork. The likely ownership boundary is a separate projection/light/runtime transform path, potentially correlated with stage lighting state, but that exact producer/consumer remains `RESEARCH_REQUIRED` until traced.

`Rot`/model-transform fields in the broader Model Set grammar are model transforms and are not proof of a shadow-light direction control.

## 8. Editor implication

Evidence-safe controls that DMC Rengine can expose once parser/writer support is implemented:

```text
Shadow resource      -> SHW binding
Shadow enabled       -> context-specific enable/disable
Shadow darkness      -> ShadowDarkness
Shadow softness      -> ShadowSoftness
Shadow soft range    -> ShadowSoftRange
Shadow parameter set -> ShadowParamSet (preset/set; exact semantics still partial)
Shadow direction     -> RESEARCH_REQUIRED
Shadow tilt/angle    -> RESEARCH_REQUIRED
```

The next reverse gate is to trace the projection vector/matrix or lighting state consumed by `CDrawShadow`, and identify whether stage LIG data, a global light state, or another runtime parameter owns direction/tilt.