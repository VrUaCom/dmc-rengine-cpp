# DMC3 SCM scene-transform consumer — corrected canonical EXE record

**Original pass:** 2026-09-03  
**Provenance correction:** 2026-09-05  
**Canonical executable:** `dmc3.exe` SHA-256 `e454272ed0fb0247fcbcf300e5d55d7a3e96d50b89b9ffaff81bb978dcbdd082`

## Correction summary

The transform mathematics recovered by the original pass remains correct, but the owning initializer address was misattributed.

Whole setup-chain reconciliation now proves:

```text
MOD/EFM setup 0x1403039C0
    -> 0x1402FA080

SCM setup 0x140303C10
    -> 0x1402FA360
```

Therefore the previous sentence "SCM transform consumer = `0x1402FA080`" is rejected.

The reason the recovered Euler result remains valid is that **both** format-family initializers consume the same serialized `0x20` transform shell and call the same lower-level helpers:

```text
transform +0x10 XYZ -> 0x140330450
transform +0x00 XYZ -> 0x140031200
```

## Shared scene/node block attachment

`0x1402F1DB0` is common to both setup paths. It reads raw header `+0x20`, resolves the four relative fields and stores:

```text
block +0x00 -> manager +0x08
block +0x04 -> manager +0x10
block +0x08 -> manager +0x18
block +0x0C -> manager +0x20
```

For SCM the third array is the geometry object-binding array. The same offset exists in MOD but has different node-side semantics and must not inherit the SCM name.

## Correct SCM local-transform path — 0x1402FA360

`0x1402FA360` walks the node count from manager `+0xEA`, consumes the serialized transform array reached from raw header `+0x20`, and builds one local matrix per node.

Serialized record:

```text
+0x00 f32 translation X
+0x04 f32 translation Y
+0x08 f32 translation Z
+0x0C f32 precomputed translation length
+0x10 f32 rotation X radians
+0x14 f32 rotation Y radians
+0x18 f32 rotation Z radians
+0x1C f32 zero/reserved
```

The SCM initializer explicitly canonicalizes the temporary fourth lanes before the lower helpers:

```text
translation temp W = 1
rotation temp W    = 0
```

It then calls:

```text
0x140330450(rotation)
0x140031200(translation)
```

## Rotation semantics

`0x140330450` consumes the first three values from the `+0x10` vector as independent radian angles and calls:

```text
0x140030F10 X rotation
0x140030FC0 Y rotation
0x140031080 Z rotation
```

The exact order is X, then Y, then Z. From an identity basis the recovered DMC matrix product is:

```text
Rz * Ry * Rx
```

This remains `EXE_CONFIRMED` for SCM because `0x1402FA360` calls the same helper directly.

## Translation semantics

`0x140031200` applies translation XYZ to row 3 in the recovered row-vector matrix convention. Its mask preserves the pre-existing matrix W lane and excludes serialized transform `+0x0C` from homogeneous W.

Thus:

```text
+0x00/+0x04/+0x08 = translation XYZ
+0x0C              = precomputed length(XYZ), not homogeneous W
```

The SCM corpus invariant remains `328/328` scene nodes matching `length(XYZ)`.

## Cross-family consequence

The corrected MOD/EFM initializer `0x1402FA080` consumes the same two serialized vec4 lanes and calls the same `0x140330450` / `0x140031200` pair. Fresh MOD corpus evidence also confirms the same `0x20` record shape and `+0x0C == length(XYZ)`.

Therefore translation + XYZ Euler rotation are now promoted to the shared Model Family `TransformCoreAbi`.

What remains format-specific is the surrounding initialization behavior. In particular, `0x1402FA080` performs additional inverse/matrix-palette work needed by the MOD/EFM model path, while SCM `0x1402FA360` uses a lighter static/stage initialization path.

## Header initialization note

`0x1402F9570` is also shared rather than SCM-specific. It carries raw header `+0x14`, `+0x10`, `+0x11` and `+0x13` into manager state and obtains the live texture count from the external companion. Semantic interpretation of raw `+0x14` remains format-specific.

## Product state

The clean SCM IR remains correct:

```text
SceneTransform::translation
SceneTransform::translation_magnitude
SceneTransform::rotation_xyz_radians
```

The source-address evidence is corrected to SCM `0x1402FA360`. This document supersedes the previous function ownership claim while retaining the recovered rotation/translation semantics.
