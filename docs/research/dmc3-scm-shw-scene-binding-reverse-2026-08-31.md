# DMC3 SCM -> SHW scene-shadow binding reverse — 2026-08-31

**Status:** CANONICAL RESEARCH ADDENDUM  
**Scope:** original DMC3 HD `Model` configuration, `CDrawSCM`, `CDrawShadow`, SHW attachment and scene compatibility.  
**Canonical EXE SHA-256:** `e454272ed0fb0247fcbcf300e5d55d7a3e96d50b89b9ffaff81bb978dcbdd082`

## 1. Question

Can an SHW resource be attached to an SCM stage/scene model, rather than only to a MOD actor/object?

## 2. Answer boundary

**EXE_CONFIRMED: yes.** The original executable contains an explicit `SCM -> CDrawSCM -> common Shadow attach -> CDrawShadow` path.

This proves engine capability. It does **not** prove that every stock stage uses SHW, nor that an arbitrary existing character SHW is geometrically compatible with an arbitrary SCM.

Observed stock/demo corpus instead shows SHW most often paired with MOD resources. No stock `SCM + Shadow` Set entry was observed in the bounded `m20_s00` sample.

## 3. Model configuration parser

The Model Set parser at `0x1402D83E0` accepts the `Model` resource field and classifies the loaded payload through `0x1402DB1F0`.

Accepted model families:

```text
MOD -> model type 0
EFM -> model type 1
SCM -> model type 2
```

The same Model Set parser independently recognizes:

```text
Texture
Cloth
Shadow
```

`Shadow` is resolved as a separate resource and stored independently of the base Model resource.

Therefore an SCM and SHW are not nested into one binary document. They are separately loaded resources attached at Model-instance construction time.

## 4. Real corpus confirms the configuration model

In the available `m20_s00` extracted corpus, Model Set files include examples such as:

```text
Model   st509.scm
Texture st509.ptx
```

and:

```text
Model   pl102_00.mod
Texture pl102_00.ptx
Shadow  pl002_00.shw
```

The second case also proves that SHW binding is not determined by identical filename stems: `pl102_00.mod` deliberately reuses `pl002_00.shw`.

Bounded Set census in this sample:

```text
SCM Model Set entries observed: 9
SCM entries with Shadow:        0
MOD entries with Shadow:        4
```

This is a stock-content observation only. It does not override the explicit SCM-capable runtime branch described below.

## 5. Concrete SCM construction branch

`CDemoModelWork` constructor: `0x1402DB7B0`.

RTTI identity:

```text
.?AVCDemoModelWork@@
```

Its generic Model-create path starts at `0x1402DBA30`.

Parsed model type is copied into `CDemoModelWork + 0x1730`:

```text
0 -> MOD
1 -> EFM
2 -> SCM
```

At `0x1402DBC03` the function explicitly tests for type 2.

For SCM it uses the object at:

```text
CDemoModelWork + 0x800
```

That object is constructed by `0x140089320`.

RTTI proves its concrete type:

```text
.?AVCDrawSCM@@
```

Vtable:

```text
0x1404C9360
```

This is direct proof that the path being analyzed is the original `CDrawSCM` scene-model path.

## 6. SCM enters the common Shadow attach block

After SCM initialization, execution reaches the Shadow block at approximately `0x1402DBCBC`.

The code:

1. reads the separately resolved Shadow resource from the parsed Model Set;
2. obtains the SCM model-core through the `CDrawSCM` virtual interface;
3. passes that model-core plus the SHW resource to `CDrawShadow`.

For SCM:

```text
CDrawSCM::modelCore()
  vtable +0x08
  -> 0x140089DF0
  -> returns this + 0x50
```

For the MOD/EFM draw implementation:

```text
CDraw::modelCore()
  vtable +0x08
  -> 0x140089DE0
  -> returns this + 0x80
```

Both model-core regions are initialized by the same helper:

```text
0x1400893B0
```

That shared initializer explicitly clears the field at model-core `+0x1A0`, which later becomes the back-pointer to the attached shadow.

This is strong ABI evidence that `CDrawSCM` deliberately exposes a shadow-compatible model core, not an unrelated SCM-only object.

## 7. CDrawShadow identity and attach ABI

RTTI:

```text
.?AVCDrawShadow@@
base: .?AVIDrawShadow@@
```

Concrete vtable:

```text
0x1404C97C0
```

The common virtual attach entry is:

```text
vtable +0x08 -> 0x14008BC60
```

It calls the low-level binder:

```text
0x1403204B0
```

Recovered minimum ABI:

```cpp
void BindShadow(ShadowRuntime* shadow,
                ModelCore* model,
                ShwDocument* shw)
{
    shadow->model = model;       // shadow + 0x10
    shadow->shw   = shw;         // shadow + 0x28
    model->shadow = shadow;      // model  + 0x1A0
}
```

This is a real bidirectional model-shadow relationship.

## 8. Downstream renderer does not reject SCM

The SHW update/draw path later dereferences the linked model through `shadow + 0x10`.

The observed readiness check does not test `MOD` versus `EFM` versus `SCM`; it checks a generic model-runtime flag.

SHW transform processing also reads the linked model's matrix/transform palette and applies the SHW per-vertex matrix indices.

Therefore no downstream family gate was recovered that rejects an already linked `CDrawSCM` model core.

## 9. What this means for modding

The original engine supports this conceptual resource graph:

```text
Model Set
├─ Model   custom_scene.scm
├─ Texture custom_scene.ptx
└─ Shadow  custom_scene.shw
              |
              v
          CDrawShadow
              |
              <-> CDrawSCM model core
```

A custom SHW can therefore be attached to an SCM Model instance.

However, the SHW must be geometrically compatible with the target model-core transform palette:

- SHW has its own vertices and triangles;
- SHW has per-vertex transform-matrix indices;
- those indices select matrices supplied by the attached Model core;
- for a static scene-oriented SHW, the safe initial experiment should use the simplest valid matrix mapping (normally matrix index 0 unless further SCM-specific evidence proves otherwise);
- reusing a character SHW on a stage SCM is not expected to produce a meaningful stage shadow merely because the attachment succeeds.

## 10. Important stock-content distinction

Current bounded corpus does **not** show Capcom attaching SHW to the examined SCM Set entries.

That suggests:

- SHW is heavily used for actor/object shadow geometry;
- stock static stage shading can rely on other rendering/light/vertex-colour techniques;
- SCM+SHW is an engine capability that is available to modding even when stock data does not commonly exercise it.

Do not rewrite this as `all map shadows are SHW`.

## 11. Evidence status

| Claim | Status |
|---|---|
| Model parser accepts SCM | EXE_CONFIRMED |
| Model parser accepts independent Shadow resource | EXE_CONFIRMED |
| type 2 constructs/uses CDrawSCM | EXE_CONFIRMED + RTTI |
| SCM path enters common Shadow attach block | EXE_CONFIRMED |
| CDrawShadow binds bidirectionally to SCM-compatible ModelCore | EXE_CONFIRMED |
| downstream SHW path has no MOD-only family rejection | EXE_CONFIRMED bounded |
| stock m20 SCM Set entries use SHW | NOT OBSERVED |
| arbitrary custom SCM+SHW renders correctly in the game | GAME TEST REQUIRED |

## 12. Next validation gate

The minimal game experiment should be deliberately small:

1. use a copy of an existing Model Set that loads a simple SCM;
2. add `Shadow <known-valid-shw>`;
3. first choose/build an SHW whose matrix indices are compatible with the SCM model core;
4. boot the scene and record whether the shadow component initializes and renders;
5. only then move to generating a purpose-built SHW from SCM geometry.

If successful, promote:

```text
SCM + SHW binding
EXE_CONFIRMED -> GAME_VERIFIED
```

A later authoring pipeline can then implement:

```text
SCM geometry
-> select simplified shadow-caster geometry
-> build SHW vertices/triangles/adjacency
-> assign SCM-compatible matrix indices
-> emit SHW
-> add Shadow binding to Model Set
-> game validation
```
