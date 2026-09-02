# DMC3 HD SCM serialized format

**Status:** structural serialized-core model implemented; writer not promoted.  
**Profile:** `dmc3-hd`.  
**Canonical analysis EXE:** SHA-256 `e454272ed0fb0247fcbcf300e5d55d7a3e96d50b89b9ffaff81bb978dcbdd082`.  
**Primary runtime normalizer:** `0x1403051B0`.  
**Independent fixed-stride SCM mesh consumer:** `0x1402FDD10`.

SCM is a mesh-bearing stage/scene geometry family. It is not treated as a MOD alias. The serialized disk layout below is bounded by canonical executable evidence plus a 68-unique-file corpus sweep.

## Header — 0x40 bytes

| Offset | Type | Meaning/status |
| ---: | --- | --- |
| `+0x00` | `char[4]` | exact magic `SCM ` |
| `+0x04` | `f32` | version; `1.01` on 68/68 confirmed files |
| `+0x08` | `u64` | reserved, zero on confirmed corpus |
| `+0x10` | `u8` | object count |
| `+0x11` | `u8` | scene-node count |
| `+0x12` | `u8` | texture-slot count |
| `+0x13` | `u8` | reserved, zero on confirmed corpus |
| `+0x14` | `u32` | unresolved structured identifier/flag field |
| `+0x18` | `u64` | reserved, zero |
| `+0x20` | `u64` | absolute serialized scene-node-block offset |
| `+0x28/+0x30/+0x38` | `u64` | reserved, zero |

Do not rename `+0x14` to a semantic stage/material id until a direct consumer closes that meaning.

## Object record — 0x40 bytes

Objects begin at file offset `0x40`, fixed stride `0x40`.

| Offset | Type | Meaning/status |
| ---: | --- | --- |
| `+0x00` | `u8` | mesh count |
| `+0x01` | `u8` | unresolved class/flag byte; observed `0x80, 0xC0..0xC5` |
| `+0x02` | `u16` | total vertex count; equals sum of child mesh vertex counts |
| `+0x04` | `u32` | reserved, zero |
| `+0x08` | `u64` | absolute mesh-table offset |
| `+0x10` | `u32` | unresolved object flags |
| `+0x14..+0x2F` | bytes | reserved, zero |
| `+0x30` | `vec3f` | bounding-sphere center |
| `+0x3C` | `f32` | bounding-sphere radius |

The center/radius interpretation is data-confirmed on 254/254 objects: the stored radius equals the maximum distance from the stored center to serialized geometry within float tolerance.

## Mesh record — 0x50 bytes

The physical mesh record is fixed-size `0x50`. This reconciles the old apparent contradiction around `mesh+0x28`:

- helper `0x1402FDD10` addresses mesh records with a fixed `index * 0x50` stride;
- normalizer `0x1403051B0` also reads `mesh+0x28` as an explicit continuation span;
- serialized corpus uses `0x50` on every non-final mesh in an object and `0` on the final mesh.

| Offset | Type | Meaning/status |
| ---: | --- | --- |
| `+0x00` | `u16` | vertex count |
| `+0x02` | `u16` | texture-slot index |
| `+0x04` | `u32` | reserved, zero |
| `+0x08` | `u64` | reserved, zero |
| `+0x10` | `u64` | absolute positions stream offset |
| `+0x18` | `u64` | absolute normals stream offset |
| `+0x20` | `u64` | absolute UV stream offset |
| `+0x28` | `u64` | continuation span: `0x50` except final mesh `0` |
| `+0x30` | `u64` | reserved, zero |
| `+0x38` | `u64` | absolute RGB/topology-flags stream offset |
| `+0x40` | `u64` | mesh-relative offset to generated-index workspace |
| `+0x48` | `u32` | generated index count; zero in serialized corpus before runtime normalization |
| `+0x4C` | `u32` | reserved, zero |

## Vertex streams

Per mesh:

```text
positions  = f32 x,y,z            stride 12
normals    = f32 x,y,z            stride 12
UV         = i16 u, i16 v         stride 4, scale 1/4096
colorFlags = u8 r,g,b,flags      stride 4
```

In the confirmed corpus, the fourth color/flags byte contains only `0x00` and `0x02`. Runtime topology reconstruction consumes bit `0x02` as a triangle-run break/skip condition. It is therefore not modeled as alpha.

## Exact serialized placement

Given the header object table, the remaining serialized geometry is deterministic:

```text
0x40
  object records [objectCount] x 0x40

for each object in object order:
  mesh records [meshCount] x 0x50
  align16

  for each mesh: positions; align16 after every mesh stream
  for each mesh: normals;   align16 after every mesh stream
  for each mesh: UV;        align16 after every mesh stream
  for each mesh: RGB/flags; align16 after every mesh stream

scene-node block
align16

gfor every mesh in object/mesh order:
  generated-index workspace reserve

EOF
```

This formula reaches exact EOF on 68/68 unique files and is implemented by `scm::build_serialized_layout()`.

## Scene-node block

Header `+0x20` points to the scene-node block:

```text
+0x00 u32 parentRel
+0x04 u32 orderRel
+0x08 u32 objectBindingRel
+0x0C u32 transformRel
+0x10..+0x1F zero
```

For `N = sceneNodeCount`:

```text
parentRel        = 0x20
iorderRel         = 0x20 + align4(N)
objectBindingRel = 0x20 + 2*align4(N)
transformRel     = align16(0x20 + 3*align4(N))
```

Arrays:

- parent: `i8[N]`, `-1` or valid scene-node index;
- order: `u8[N]`, permutation of `0..N-1`;
- object binding: `i8[N]`, `-1` or valid object index;
- transform records: `N * 0x20`.

Scene nodes are not equivalent to objects. 66/68 files use `nodeCount = objectCount + 1`; confirmed exceptions use `+2` and `+6`.

## Scene transform — 0x20 bytes

```text
+0x00 f32 X
+0x04 f32 Y
+0x08 f32 Z
+0x0C f32 translationMagnitude
+0x10 f32 rotationCandidateX
+0x14 f32 rotationCandidateY
+0x18 f32 rotationCandidateZ
+0x1C f32 reserved = 0
```

`+0x0C == length(X,Y,Z)` is data-confirmed on 328/328 nodes. The second vec3 strongly behaves like radians/Euler rotation, but the C++ API deliberately calls it `rotation_candidate` until a direct transform consumer closes the semantic convention/order.

## Generated-index workspace

`mesh+0x40` is not an opaque VIF/GIF command stream. It is a mesh-relative offset to a runtime-generated `u16` index workspace.

Serialized capacity:

```text
capacityBytes = align16(6 * (vertexCount - 2))
```

for `vertexCount >= 3`.

The first serialized word is `0x1212` on all confirmed workspaces; `mesh+0x48` is zero before runtime normalization. `0x1403051B0` reconstructs the u16 index sequence from the per-vertex topology byte and writes the generated word count to `mesh+0x48`.

The DMC Rengine reconstruction is exposed as:

- `scm::index_workspace_capacity_bytes()`;
- `scm::generate_triangle_strip_indices()`.

## Current implementation boundary

Implemented on branch `scm`:

- fail-closed serialized parser;
- typed structural IR;
- deterministic serialized layout builder;
- topology/index generator;
- bounds checks and invariant diagnostics;
- exact object/mesh/scene/index-workspace validation;
- synthetic regression;
- 68-file external corpus validation with zero diagnostics.

Not promoted yet:

- semantic names for header `+0x14`, object `+0x01`, object flags `+0x10`;
- exact Euler order/coordinate convention for scene rotations;
- material/texture-bundle ownership beyond the validated texture index range;
- canonical writer/export support;
- no-edit byte-identical rebuild gate;
- edited resource repack and original-game consumption receipt.
