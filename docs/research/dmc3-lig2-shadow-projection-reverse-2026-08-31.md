# DMC3 LIG2 -> SHW shadow projection reverse — 2026-08-31

**Status:** CANONICAL RESEARCH ADDENDUM  
**Canonical EXE SHA-256:** `e454272ed0fb0247fcbcf300e5d55d7a3e96d50b89b9ffaff81bb978dcbdd082`  
**Scope:** stage `LIG2` payloads, `CLightStatic` / `CLightMgr`, `CDrawShadow`, SHW projection-direction source.

## 1. Question

Does stage lighting data determine the direction/tilt of an SHW shadow, and if so, which part of the light data is used?

## 2. Answer boundary

**EXE_CONFIRMED: yes.** `CDrawShadow` queries the active `CLightMgr` during shadow update. The recovered query specifically selects the special light category represented by LIG2 record type bit/value `0x04`. The selected light's world-space position becomes the shadow light/projection point. The shadow direction is therefore derived from the vector:

```text
selected LIG2 shadow-light position - model/root world position
```

This closes the previously open direction/tilt ownership boundary for the recovered static-light path.

It does not yet prove names for ordinary LIG2 types `1`, `2`, and `3` such as point/spot/directional. Those exact semantic labels remain open.

## 3. Where the static stage lights live

In the bounded stage corpus:

```text
st001cfg.pac
  slot 1 -> LIG2, size 0x920

st114cfg.pac
  slot 1 -> LIG2, size 0x920
```

The stage effect archives examined separately (`st001_effect.pac`, `st114_effect.pac`) are PNST/effect banks and are not the storage location of these static LIG2 rigs.

Thus for the examined stages the static spatial light rig is in the stage CFG resource set rather than in `_effect.pac`.

## 4. CLightStatic proves the fixed LIG2 layout

`CLightStatic` constructor:

```text
VA      0x14023ECB0
RTTI    .?AVCLightStatic@@
vtable  0x1404E3128
```

The constructor writes `0x3247494C` (`"LIG2"`) and initializes a fixed block corresponding to the raw LIG2 payload.

The raw file size is:

```text
0x920 = 0x20 header + 48 * 0x30 records
```

Therefore LIG2 has a fixed capacity of **48 light records**, each with stride `0x30`.

## 5. Active light count and first-byte semantics

The static-light registration path at `0x140089000` loops exactly `0x30` (=48) records. For each record it tests the first byte and registers the light only when that byte is non-zero.

Evidence-safe field:

```cpp
struct Lig2RecordEvidence {
    uint8_t typeOrFlags;   // +0x00; zero means inactive/disabled record
    // ... total stride 0x30
};
```

Observed active counts:

```text
st001 LIG2: 8 active records
st114 LIG2: 11 active records
```

A record may contain non-zero stale/auxiliary fields while `+0x00 == 0`; it is still skipped by the original registration path.

## 6. Recovered record fields

Runtime light evaluation at `0x1402EE2A0` consumes the `0x30` record directly.

Current field map:

```text
+0x00  u8   type/category/flags; 0 disables the record
+0x01  u8   secondary mode flag; exact semantics OPEN
+0x02  u8   distance-attenuation mode

+0x10  f32  world position X
+0x14  f32  world position Y
+0x18  f32  world position Z
+0x1C  f32  contribution-related parameter; exact semantic name OPEN
+0x20  f32  distance attenuation/falloff scale

+0x24  u16  colour-like channel 0
+0x26  u16  colour-like channel 1
+0x28  u16  colour-like channel 2

+0x2A..     remaining fields partial/open
```

The position interpretation is direct: `0x1402EE2A0` subtracts a supplied world/query point from the three floats at `+0x10/+0x14/+0x18`.

Distance behavior:

```text
if record +0x02 == 0:
    weight = max(1 - distance * record[+0x20], 0)
else:
    weight = 1
```

This proves `+0x20` is a distance attenuation/falloff scale in that path.

The three u16 values at `+0x24/+0x26/+0x28` are converted to floating-point contribution channels and are strongly correlated with light RGB/colour, but exact normalization/range remains to be documented field-perfectly.

## 7. CDrawShadow directly consumes CLightMgr

`CDrawShadow` update path near:

```text
0x14008BCF0
```

obtains the active global light manager from:

```text
0x140C90E38
```

and calls:

```text
0x14031FA80(shadowCore, CLightMgr)
0x14031FB50(...)
```

This is direct original-runtime evidence that SHW shadow projection is not isolated from stage lighting state.

## 8. The shadow query selects the special 0x04 light category

Inside `0x14031FA80`, the shadow code obtains the model/root world position and calls the light-selection routine:

```text
0x1402EE560(CLightMgr, queryPoint, 0x1C, ...)
```

The light selector filters candidates using the first LIG2 record byte plus query-mask bits.

For the recovered static categories:

- ordinary type families `1/2/3` depend on low query-mask bits not present in the shadow request;
- the special `0x04` category is accepted when query flags contain `0x10`;
- the shadow request is `0x1C`, which contains `0x10` and therefore admits the `0x04` category.

In both real stage samples there is exactly one active type-4 record:

```text
st001: type 4 at approximately (5000, 2500, 0)
st114: type 4 at approximately (2500, 99998, 2500)
```

Both use the non-distance-attenuated mode (`+0x02 != 0`).

Evidence-safe semantic promotion:

> LIG2 record category/type `4` is the **shadow-selected light category** in the recovered CDrawShadow path.

Do not yet rename types `1/2/3` to point/spot/directional without further consumer evidence.

## 9. Exact direction/tilt derivation

For a selected LIG2 light, `0x1402EE2A0` produces a vector equivalent to:

```text
lightWorldPosition - modelWorldPosition
```

For the special type-4 records in the examined samples the weight is 1.

`0x14031FA80` then reconstructs/stores the corresponding world-space shadow-light point. The downstream shadow-projection routine around `0x14031F830` subtracts the model/root position again and uses that vector in the projection mathematics.

Therefore the evidence-backed control relation is:

```text
LIG2 type-4 world position
          |
          v
(model/root -> light) vector
          |
          v
SHW shadow projection direction / tilt
```

A very distant type-4 light approximates a directional-light source because the model-to-light vector changes little across a bounded room. The `st114` type-4 Y coordinate near `99998` is consistent with this behavior, but the semantic label `directional light` is not promoted solely from that observation.

## 10. Fallback when no shadow light is selected

The recovered shadow preparation path has a fallback vector/offset using approximately:

```text
(200, 300, 50)
```

when no qualifying light contribution is returned.

This is an engine fallback, not a recommended authoring value and not a field in SHW.

## 11. Complete shadow ownership split

The recovered system can now be separated cleanly:

```text
SHW
  -> shadow-caster geometry
  -> triangles / adjacency
  -> per-vertex transform indices

LIG2 type-4 light
  -> world-space shadow light point
  -> direction / tilt of the projected SHW shadow

Model Set / shadow parameters
  -> ShadowDarkness
  -> ShadowSoftness
  -> ShadowSoftRange
  -> ShadowParamSet
```

In simple terms:

```text
SHW says WHAT shape casts the shadow.
LIG2 says FROM WHERE the shadow light comes.
ShadowDarkness/Softness say HOW the shadow looks.
```

## 12. Stage-editor implication

A future evidence-backed Lighting/Shadow Editor can expose separate resources/components:

```text
Stage Lighting
  Light slots: 48 fixed
  Active lights: type byte != 0

Shadow Projection Light
  selected LIG2 type-4 record
  position X/Y/Z

Shadow Caster
  SHW resource

Shadow Appearance
  darkness
  softness
  soft range
  parameter set
```

For spatial editing, moving the selected type-4 LIG2 light changes the model-to-light vector and therefore changes shadow direction/tilt without rewriting SHW geometry.

## 13. Status table

| Claim | Status |
|---|---|
| LIG2 has 0x20 header + 48 records * 0x30 | DATA_CONFIRMED + EXE_CONFIRMED |
| first record byte controls active registration | EXE_CONFIRMED |
| +0x10/+0x14/+0x18 are world-space light position | EXE_CONFIRMED |
| +0x20 participates in distance falloff | EXE_CONFIRMED |
| +0x24/+0x26/+0x28 are light colour-like channels | HIGH_CONFIDENCE / runtime-converted channels |
| CDrawShadow queries global CLightMgr each update | EXE_CONFIRMED |
| shadow query selects the 0x04/type-4 category | EXE_CONFIRMED |
| selected type-4 position supplies shadow projection point/direction | EXE_CONFIRMED |
| exact semantic names of LIG2 types 1/2/3 | RESEARCH_REQUIRED |
| arbitrary edited type-4 light changes shadow correctly in a live game | GAME TEST REQUIRED |

## 14. Next validation gates

1. Perform a one-field game experiment moving only the active type-4 LIG2 position and record the shadow-direction change.
2. Recover exact semantics of LIG2 types 1/2/3 from their consumers and shader/light setup.
3. Finish header-field semantics and remaining record fields.
4. Test whether dynamic/effect-created lights can also satisfy the same `CLightMgr` shadow query.
5. Keep `_effect.pac` effect banks separate from the static LIG2 stage-rig ownership unless direct evidence links a specific effect light into `CLightMgr`.