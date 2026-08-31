# DMC3 LightSet / dynamic light runtime reverse — 2026-08-31

**Status:** CANONICAL RESEARCH ADDENDUM  
**Canonical EXE SHA-256:** `e454272ed0fb0247fcbcf300e5d55d7a3e96d50b89b9ffaff81bb978dcbdd082`  
**Branch:** `research/dmc3-primary-3d-format-abi-20260831`

## 1. Main findings

This pass closes several open lighting boundaries:

1. StageSet / LightSet `Type` is **not** a point/spot/directional light enum. The original LightSet comments name it `補間タイプ` (interpolation type), and executable code confirms that value `1` enables interpolation from source state to destination state over `Life`.
2. `MoveType` controls clip lifetime behavior. The original exporter documents `0:once`, `1:loop0`, `2:loop1`; the executable additionally accepts `loop2` as value `3`.
3. The raw light `ValidType` / category byte is a **bit-composable routing/role field**, not a strict enum. Routing values 1/2/3 coexist with shadow role bit `0x04` and a special runtime role bit `0x08`.
4. `CDrawShadow` query mask `0x1C` scans **both static and dynamic light pools** and requests the shadow role. Therefore a dynamic LightSet light carrying shadow bit `0x04` can steer SHW shadow projection.
5. Real LightSet corpus contains shadow-only dynamic lights (`ValidType 4`) and a combined model+shadow light (`ValidType 6`).
6. Dynamic CLight registration is not limited to StageSet. Direct callers exist in player/enemy runtime classes including `CPlDante`, `CPlLady` / `CPlNewVergil`, `CPlVergil`, and `CEm030`.

## 2. Real LightSet corpus

Recovered plain-text LightSet documents from the legacy stage-drop corpus:

```text
m20_b00_002_077.txt -> 15 clips
m20_c00_002_080.txt -> 23 clips
m20_s00_002_086.txt -> 26 clips
```

Total bounded corpus: **64 light clips**.

The original file comments explicitly describe:

```text
ValidType  usage flag
IsMinus    minus-light flag
Life       lifespan
Type       interpolation type
SrcPos     source position
SrcForce   source light amount
SrcRange   source application range
SrcRgb     source colour
DstPos     destination position
DstForce   destination light amount
DstRange   destination application range
DstRgb     destination colour
MoveType   0:once, 1:loop0, 2:loop1
```

Observed distributions:

```text
m20_b00: ValidType 2 x9, 8 x4, 3 x2
m20_c00: ValidType 2 x18, 8 x3, 3 x2
m20_s00: ValidType 2 x13, 4 x10, 3 x1, 6 x1, 8 x1
```

All 64 observed stock clips use:

```text
Type     = 0
MoveType = 0
```

and destination fields are zero in this bounded corpus. Therefore interpolation and looping are **engine-supported capabilities**, but are not observed as used by these three shipped LightSet files.

## 3. Interpolation `Type`

The StageSet LIGHT parser reads `type` and stores an interpolation-mode flag. The corresponding runtime CLight field is checked in:

```text
0x1402EE0B0
```

When the field equals `1`, the engine linearly interpolates over the current lifetime between source and destination values:

```text
SrcPos   -> DstPos
SrcRgb   -> DstRgb
SrcForce -> DstForce encoded coefficient
SrcRange -> DstRange encoded coefficient
```

When interpolation is not enabled, source values are used directly.

Evidence-safe editor presentation:

```text
Interpolation
  Off (Type 0)
  Source -> Destination (Type 1)
```

The original numeric field should remain visible for unknown future values.

## 4. MoveType behavior

Parser mapping:

```text
once  -> 0
loop0 -> 1
loop1 -> 2
loop2 -> 3
```

Runtime end-of-life behavior around `0x140263B00`:

- **0 / once:** deactivate after the light lifetime expires.
- **1 / loop0:** reapply/restart the source-side cycle.
- **2 / loop1:** alternate source and destination sides; this is a ping-pong path.
- **3 / loop2:** executable-supported but not described by the recovered exporter header and not observed in the 64-clip corpus. The runtime disables interpolation after the first completion and reapplies the destination side. Keep the polished semantic label `PARTIAL` until a live test or additional corpus example is found.

`bwait` and `await` are independent before/after wait timers around the active lifetime and are decremented by the stage object delta time.

## 5. ValidType is flags/routing, not a physical lamp type

Selector:

```text
0x1402EE560
```

Low routing semantics already confirmed from original `valid` grammar:

```text
1 = objscr = shared object + stage/screen route
2 = obj    = object/model route
3 = scr    = stage/SCM route
```

The selector treats shadow role independently:

```text
raw & 0x04 -> shadow-role candidate
```

Therefore combined values are meaningful. For example:

```text
4 = shadow role only
6 = 2 | 4 = object/model route + shadow role
```

A real `m20_s00` clip uses `ValidType 6`:

```text
SetFrame  440
ValidType 6
SrcPos    3500 8000 3500
SrcForce  5300
SrcRange  5500
SrcRgb    80 20 20
```

This is corpus evidence for a dynamic light that is eligible for both normal object/model lighting and shadow projection.

## 6. Static vs dynamic light pools

The selector request mask uses separate bits for pools:

```text
0x04 -> scan static CLightMgr pool (LIG2/static rig)
0x08 -> scan dynamic/runtime CLightMgr pool
0x10 -> request shadow-role lights
```

`CDrawShadow` uses mask:

```text
0x1C = 0x04 | 0x08 | 0x10
```

Therefore the shadow renderer intentionally searches **both** the static and dynamic pools for a shadow-role light.

This promotes the following claim to `EXE_CONFIRMED`:

> A StageSet/LightSet dynamic light whose raw routing/role byte includes bit `0x04` can participate in SHW shadow-light selection.

Practical consequence: SHW shadow direction does not have to be controlled only by the static LIG2 type-4 point. A runtime light can move/change and become the shadow projection source.

## 7. Special raw bit 0x08

Dynamic-pool selection strips bit `0x08` before applying the ordinary routing/shadow tests.

CLight registration at `0x1402EE9F0` also treats raw bit `0x08` specially: it writes the associated light node into a dedicated `CLightMgr +0x70` reference. The selector has a separate request path for this special reference.

Real corpus has `ValidType 8` records with mostly zero RGB/force/range and meaningful positions.

Exact gameplay semantic name remains `RESEARCH_REQUIRED`; do not label this as ambient, camera, target, or directional light without another direct consumer trace.

## 8. Dynamic light registration outside StageSet

Callers of CLight registration `0x1402EE9F0` are widespread. RTTI/vtable correlation identifies direct runtime methods for at least:

```text
CEm030
CPlDante / CPlayer
CPlLady / CPlNewVergil
CPlDante / CPlVergil
```

These methods update an embedded `CLight` with `0x1402EE0B0` and register it into the global `CLightMgr` while active.

This proves the lighting technology is a reusable engine subsystem, not only a stage-file feature. Exact gameplay meaning of each character/enemy light instance still requires per-caller tracing before naming them as weapon flash, aura, etc.

## 9. Authoring fields and encoding

The recovered authoring vocabulary is spatial and radial rather than a proven point/spot/directional enum:

```text
Position X/Y/Z
Force / light amount
Range
RGB
Minus / subtractive
Routing / valid-for roles
Life
Interpolation
Move / loop behavior
Before wait
After wait
```

`SrcRange` / `DstRange` are encoded into runtime coefficient form with helper `0x14032E900`:

```text
coefficient = 1 / range^2
```

The evaluator forms world-space `lightPosition - queryPosition`, computes XYZ distance squared, and applies the range coefficient for distance falloff when attenuation is enabled.

`SrcForce` / `DstForce` are also transformed by the same helper before entering the runtime light state. The original file comments call this value `光量` (light amount). Preserve the human authoring value in tooling and perform the recovered encoding/decoding internally.

RGB fields are directly named by the original authoring format. `IsMinus` is explicitly a minus/subtractive-light flag.

## 10. What is now possible in a DMC Rengine Lighting Editor

Evidence-backed controls can be designed as:

```text
STATIC ROOM LIGHTS (LIG2)
- fixed 48 slots
- enabled/disabled
- routing/role flags
- position gizmo
- force
- range gizmo
- RGB
- subtractive light
- shadow-role flag

DYNAMIC LIGHT CLIP (LightSet / StageSet LIGHT)
- SetFrame / Life
- routing/role flags
- source position/force/range/RGB
- destination position/force/range/RGB
- interpolation on/off
- once / restart / ping-pong / experimental loop2
- before/after delay

SHADOW
- SHW caster geometry
- static or dynamic shadow-role light selection
- ShadowDarkness
- ShadowSoftness
- ShadowSoftRange
- ShadowParamSet
```

This supports, at the recovered engine level:

- moving coloured lights;
- lights that grow/shrink in influence range;
- intensity/force changes over time;
- colour transitions;
- timed one-shot lights;
- looping and ping-pong light animation;
- subtractive/darkening lights;
- model-only, stage-only, or shared lighting;
- lights that both illuminate models and steer SHW shadows (`ValidType 6` pattern);
- shadow-only dynamic light points (`ValidType 4` pattern).

## 11. Important boundaries

Not yet promoted:

- no proven classic spotlight cone/direction field in this recovered light record;
- no proven physical `point/spot/directional` enum in `ValidType` or LightSet `Type`;
- `loop2` polished semantic label remains partial;
- exact semantic role of raw `0x08` special reference remains open;
- `const` transform-binding grammar is only partially reversed;
- edited-light live-game behavior still requires one-field validation;
- safe production writers/repack paths must follow existing source-byte guards and round-trip policy.

## 12. Next reverse gates

1. Finish `const` transform binding to determine whether lights can follow an object/bone/attachment point through authoring alone.
2. Classify the dedicated `ValidType 8` / `CLightMgr +0x70` role.
3. Trace character/enemy CLight setup callers and recover which gameplay effects use dynamic lights.
4. Trace SEF/effect execution for additional CLight producers.
5. Build a read-only LightSet/LIG2 semantic visualizer before enabling writes.
6. Perform a controlled live test: move one static shadow light, then author one dynamic shadow-role light and record SHW direction response.
