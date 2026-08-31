# DMC3 main gameplay Stage -> SHW binding boundary — 2026-08-31

**Status:** CANONICAL RESEARCH ADDENDUM  
**Canonical EXE SHA-256:** `e454272ed0fb0247fcbcf300e5d55d7a3e96d50b89b9ffaff81bb978dcbdd082`

## 1. Why this addendum exists

The companion pass `dmc3-scm-shw-scene-binding-reverse-2026-08-31.md` proves that an SCM instantiated by the generic Model Set runtime can bind a separate SHW through `CDrawSCM -> CDrawShadow`.

This addendum answers the narrower question:

> Does the stock main gameplay stage object automatically bind an SHW to its main `st###.scm`?

## 2. Main CStage owns CDrawSCM

RTTI identifies the gameplay stage class:

```text
.?AVCStage@@
```

Concrete vtable:

```text
0x1404E3D18
```

The `CStage` constructor at `0x140245A50` constructs a `CDrawSCM` at:

```text
CStage + 0x60
```

using:

```text
0x140089320 -> CDrawSCM constructor
```

Therefore the main gameplay map uses the same concrete SCM renderer class that was proven shadow-compatible in the generic Model Set path.

The `CDrawSCM` model-core getter returns:

```text
CDrawSCM + 0x50
```

so the main stage's shadow-compatible model core is located at approximately:

```text
CStage + 0xB0
```

## 3. Stock CStage does not own CDrawShadow

The bounded `CStage` constructor/destructor path constructs and destroys its `CDrawSCM`, but does not construct a `CDrawShadow` member.

The concrete `CDrawShadow` constructor is:

```text
0x14008BB20
```

and no call to that constructor occurs in the `CStage` constructor.

The original executable contains one normal low-level shadow binder:

```text
0x1403204B0
```

Its only direct caller is:

```text
0x14008BC60 -> CDrawShadow attach path
```

No separate `CStage -> BindShadow` caller was recovered.

## 4. No automatic `st###.shw` naming rule

The literal configuration token:

```text
Shadow
```

occurs at `0x140506DB0` and is consumed by the Model Set parser described in the companion pass.

No second `Shadow` configuration-token parser was found for `CStage`.

Therefore there is no evidence for an automatic stock rule such as:

```text
st001.scm -> look for st001.shw
```

or:

```text
main stage PAC contains SHW -> auto-attach to stage SCM
```

Simply placing an SHW beside the main SCM is not sufficient under the recovered stock path.

## 5. Precise capability split

### Generic SCM Model instance

```text
Model custom.scm
Shadow custom.shw
```

-> **EXE_CONFIRMED supported path**.

### Main gameplay CStage SCM

```text
CStage
└─ CDrawSCM
```

-> has a shadow-compatible ModelCore, but stock CStage does not create/attach CDrawShadow.

Therefore:

> The geometry/runtime ABI is compatible, but the stock main-stage ownership path lacks the attachment component.

## 6. Modding options

### Option A — asset/config path, no new main-stage binder

Instantiate the desired SCM (or a shadow-casting scene sub-object) through a Model Set path that already supports:

```text
Model  *.scm
Shadow *.shw
```

This uses original engine behavior without adding a new CStage shadow owner.

Boundary: whether the desired Model Set mechanism is available in every gameplay-stage context must be verified separately. Do not claim this as a universal no-code stage injection yet.

### Option B — clean CStage shadow sidecar

Add a small runtime-owned `CDrawShadow` sidecar for the main CStage.

Recovered original primitives are sufficient to define the architecture:

```text
CStage + 0x60 -> CDrawSCM
CDrawSCM + 0x50 -> ModelCore

allocate/own CDrawShadow
-> CDrawShadow constructor 0x14008BB20
-> attach 0x14008BC60(ModelCore, SHW)
-> normal CDrawShadow update/draw lifecycle
-> detach/destruct on stage teardown
```

This does not require modifying SCM's binary schema. The binding belongs in runtime ownership/configuration, not inside the SCM file.

For DMC Rengine, this should be modeled as a resource relationship/scene component rather than as an invented SCM chunk.

## 7. Recommended authoring model

Do **not** design:

```text
SCM file
└─ embedded SHW
```

Design:

```text
SceneModelInstance
├─ SCM geometry resource
├─ texture/material resources
└─ optional SHW shadow-caster resource
```

For the main stage:

```text
StageScene
├─ main SCM
└─ optional ShadowComponent
    └─ SHW
```

This mirrors the original engine's separate-resource ownership model.

## 8. Practical warning for whole-map SHW

Engine compatibility does not imply that a full-resolution level should be copied into SHW.

SHW is a specialized shadow-caster mesh with its own vertices, triangles, adjacency and matrix indices. A practical stage authoring tool should generate a simplified caster mesh or allow selecting only geometry that should cast this shadow type.

A huge one-to-one SCM -> SHW conversion may be wasteful and may exceed assumptions/performance budgets that have not yet been recovered.

## 9. Status table

| Claim | Status |
|---|---|
| main gameplay CStage owns CDrawSCM | EXE_CONFIRMED + RTTI |
| CStage CDrawSCM exposes same shadow-compatible ModelCore ABI | EXE_CONFIRMED |
| stock CStage constructs CDrawShadow | NOT OBSERVED / negative bounded evidence |
| stock CStage automatically searches `st###.shw` | NOT OBSERVED |
| SCM can bind SHW through Model Set runtime | EXE_CONFIRMED |
| main CStage can be extended with a CDrawShadow sidecar using recovered ABI | HIGH_CONFIDENCE implementation consequence |
| direct main-stage custom SHW renders correctly in game | GAME TEST REQUIRED |

## 10. Next game-validation gate

The cleanest test is two-stage:

1. **Original path proof:** load a simple SCM via Model Set with an explicit valid `Shadow *.shw` and verify rendering.
2. **Main-stage sidecar proof:** attach a purpose-built SHW to `CStage + 0xB0` through a controlled runtime hook and compare behavior/lifecycle.

Only after those receipts should `main gameplay SCM + custom SHW` be promoted to `GAME_VERIFIED`.
