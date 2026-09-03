# DMC3 SCM scene-transform consumer — canonical EXE pass 2026-09-03

## Scope

This pass closes the remaining semantic ambiguity around the second vec3 in each serialized SCM scene transform.

Canonical analysis target:

- size `6,356,432` bytes;
- SHA-256 `e454272ed0fb0247fcbcf300e5d55d7a3e96d50b89b9ffaff81bb978dcbdd082`;
- ImageBase `0x140000000`.

The result is direct executable evidence:

> serialized transform `+0x10/+0x14/+0x18` is XYZ Euler rotation in radians. The game applies X, then Y, then Z; from an identity basis the resulting matrix is `Rz * Ry * Rx`.

This supersedes the earlier `rotation_candidate` label.

## 1. Scene-block attachment

Initialization helper `0x1402F1DB0` consumes the scene-node block reached through header `+0x20` and resolves its four relative arrays into manager state:

```text
scene block +0x00 -> parent array
scene block +0x04 -> order array
scene block +0x08 -> object-binding array
scene block +0x0C -> transform array
```

The transform pointer is retained by the runtime manager and consumed by the transform-building path below.

## 2. Per-node transform consumer

Function `0x1402FA080` walks the transform records with the already recovered serialized stride `0x20`.

For every scene node it:

1. takes the second serialized vec4 beginning at transform `+0x10`;
2. passes that vector to `0x140330450`;
3. takes the first serialized vec4 beginning at transform `+0x00`;
4. passes it through the translation-side helper `0x140031200` after the rotation basis exists.

The second vector is therefore not an opaque transform candidate. Its first three floats are consumed as independent angles.

## 3. Angle consumer `0x140330450`

`0x140330450` consumes the three floats in serialized order:

```text
angle X = transform +0x10
angle Y = transform +0x14
angle Z = transform +0x18
```

and calls three dedicated axis-rotation helpers in this exact sequence:

```text
0x140030F10  X rotation
0x140030FC0  Y rotation
0x140031080  Z rotation
```

The helpers call the imported `sinf` / `cosf` functions, proving the values are angular inputs in radians rather than precomputed matrix coefficients or normalized direction values.

## 4. Exact axis matrices

The recovered row-major matrix layouts are:

### X — `0x140030F10`

```text
[ 1   0   0  0 ]
[ 0   c   s  0 ]
[ 0  -s   c  0 ]
[ 0   0   0  1 ]
```

### Y — `0x140030FC0`

```text
[  c  0  -s  0 ]
[  0  1   0  0 ]
[  s  0   c  0 ]
[  0  0   0  1 ]
```

### Z — `0x140031080`

```text
[  c   s  0  0 ]
[ -s   c  0  0 ]
[  0   0  1  0 ]
[  0   0  0  1 ]
```

where `c = cos(angle)` and `s = sin(angle)` for the corresponding axis.

## 5. Composition order

Each axis helper applies the recovered matrix multiplier `0x14002FFE0` to the same destination basis.

The multiplier forms the newly supplied axis matrix against the existing basis. Because `0x140330450` calls X, then Y, then Z, the final basis starting from identity is:

```text
Rz * Ry * Rx
```

Expanded row-major basis:

```text
[ cy*cz,
  cx*sz + cz*sx*sy,
 -cx*cz*sy + sx*sz,
  0 ]

[ -cy*sz,
   cx*cz - sx*sy*sz,
   cx*sy*sz + cz*sx,
   0 ]

[ sy,
 -cy*sx,
  cx*cy,
  0 ]

[ 0, 0, 0, 1 ]
```

This exact reconstruction is implemented in `scm_transform.hpp/.cpp` as `build_rotation_xyz_radians()`.

The module intentionally describes the layout as the recovered DMC3 row-major helper layout. It does not infer an external API convention such as OpenGL/DirectX handedness from matrix storage alone.

## 6. Translation side

The first serialized transform vec4 begins at `+0x00`:

```text
+0x00 X
+0x04 Y
+0x08 Z
+0x0C stored magnitude
```

Corpus evidence already proves `+0x0C == length(X,Y,Z)` on 328/328 scene nodes.

`0x1402FA080` forwards this vec4 to `0x140031200` after the rotation basis is constructed. This directly confirms that X/Y/Z participate in the runtime transform builder. However, the fourth component is engine-specific and is not modeled as a conventional homogeneous translation `w` merely because it occupies the fourth float.

Safe current names remain:

- `translation` for XYZ;
- `translation_magnitude` for `+0x0C`.

## 7. Header +0x14 correction discovered in the same pass

Helper `0x1402F9570` initializes the resource manager from the serialized SCM-like resource.

Exact copy:

```text
0x1402F95BE  read u32 resource +0x14
0x1402F95C5  write u32 manager +0xE4
```

The same function also copies:

```text
resource +0x10 object count     -> manager +0xE8
resource +0x11 scene-node count -> manager +0xEA
resource +0x13 byte             -> manager +0xFA
```

Therefore header `+0x14` is **runtime-consumed metadata**, not merely an offline/build artifact.

Its exact semantic role remains unresolved because this pass does not yet bind downstream `manager+0xE4` uses to one stable higher-level meaning.

Updated status:

> `+0x14`: EXE_CONFIRMED runtime-carried u32 metadata; semantic name RESEARCH_REQUIRED.

## 8. Product consequence

The clean SCM IR is promoted from:

```text
rotation_candidate
```

to:

```text
rotation_xyz_radians
```

A separate transform module reconstructs only the directly proven rotation basis. Full scene/world transform composition is deliberately not overclaimed until the engine-specific translation/fourth-component and hierarchy-composition path is closed.

## 9. Remaining transform frontier

The following are still separate reverse targets:

1. exact semantics/use of serialized translation magnitude in later runtime math;
2. hierarchy/local-to-parent matrix composition order for complete world transforms;
3. coordinate-system/handedness naming relative to external authoring tools;
4. downstream semantic role of header `+0x14` / manager `+0xE4`.

The serialized Euler-angle role and X/Y/Z rotation order are no longer open.
