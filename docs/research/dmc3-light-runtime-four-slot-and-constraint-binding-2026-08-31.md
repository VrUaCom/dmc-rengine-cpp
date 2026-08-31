# DMC3 lighting: four selected lights + transform constraint binding — 2026-08-31

**Status:** CANONICAL RESEARCH ADDENDUM  
**Canonical EXE SHA-256:** `e454272ed0fb0247fcbcf300e5d55d7a3e96d50b89b9ffaff81bb978dcbdd082`

## 1. Four-light selection boundary

`CLightMgr` selector `0x1402EE560` maintains four ordered candidate records of stride `0x30` in its result scratch. A better candidate is inserted while up to three existing records are shifted.

Embedded HLSL independently corroborates the same rendering width:

```hlsl
struct MDL_LIGHT_MAT
{
    matrix Lc;
    matrix Lv;
};
```

Model/EFM shaders evaluate:

```hlsl
float4 lightI = mul(light.Lv, float4(netNormal.xyz, 1.0f));
float4 lCol   = mul(light.Lc, lightI);
```

The four components of `lightI` correspond to four selected light contributions.

Evidence-safe conclusion:

> Normal per-draw lighting is built from **up to four selected light contributions** at a time. A stage may contain/register many more lights, but each draw query is reduced to the best/applicable four before shader evaluation.

This is important for authoring: placing 20 active lights does not mean one model receives 20 simultaneous direct contributions.

## 2. Dynamic light transform/constraint support

`CStageSetLight` parser recognizes:

```text
const <int> <int>
```

and stores the two operands in its light object. Transform update `0x140263CF0` uses them as an external transform lookup path.

When the first operand is not `-1`, the runtime:

1. resolves a stage/runtime object through `0x140246CA0` using the first integer;
2. calls that object's virtual transform accessor at vtable offset `+0x38` with the second integer;
3. copies the returned matrix into the light's transform matrix;
4. `CLight` subsequently transforms its local source/destination position through that matrix every update.

When the first operand is `-1`, the alternate path obtains a matrix from the global/current player/model-side transform storage, optionally indexed by the second integer.

This behavior matches the recovered original Model Set vocabulary:

```text
CnsPos <int> <int>
```

whose source comments identify the two operands as:

```text
constraint destination track
constraint destination bone number
```

Therefore the evidence-safe semantic for StageSet LIGHT `const` is:

> **constraint/attachment target: track/object + transform/bone index.**

Exact naming differences between `const` and Model Set `CnsPos`, and sentinel `-1` behavior, should remain visible to expert tooling, but the underlying transform-attachment capability is confirmed.

## 3. Modding consequence

This opens an important class of dynamic lighting without an EXE hook:

- a light can be authored in local coordinates and follow another moving runtime object/track;
- a light can use a selected transform/bone matrix rather than a fixed world position;
- combined with `Type 1` interpolation, RGB/Range/Force start/end values, and MoveType timing, an attached light can animate while its parent moves;
- combined with shadow role bit `0x04`, an attached/moving runtime light can in principle become the light point used by SHW shadow projection because `CDrawShadow` scans the dynamic pool.

The last combined behavior is `EXE_CONFIRMED` at the subsystem routing level but still requires a controlled live-game authoring test before being labeled `GAME_VERIFIED`.

## 4. Practical engine limit vs capability

The lighting architecture now separates two limits clearly:

```text
Room/static capacity: 48 LIG2 records
Runtime/dynamic pool: additional registered CLight objects
Per draw/model lighting: best/applicable 4 contributions
Shadow projection query: separate static+dynamic shadow-role selection
```

Thus DMC Rengine should visualize **all** authored lights, but show a per-selected-object diagnostic indicating which four currently win the normal-light selection and which light wins the SHW shadow query.
