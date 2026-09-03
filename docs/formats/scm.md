# DMC3 HD SCM serialized format

**Status:** structural serialized-core model implemented; writer not promoted.  
**Profile:** `dmc3-hd`.  
**Canonical analysis EXE:** SHA-256 `e454272ed0fb0247fcbcf300e5d55d7a3e96d50b89b9ffaff81bb978dcbdd082`.  
**Primary runtime normalizer:** `0x1403051B0`.  
**Independent fixed-stride SCM mesh consumer:** `0x1402FDD10`.  
**Runtime object-flags addendum:** [`../research/dmc3-scm-runtime-object-flags-2026-09-03.md`](../research/dmc3-scm-runtime-object-flags-2026-09-03.md).

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
| `+0x14` | `u32` | unresolved structured identifier/metadata field |
| `+0x18` | `u64` | reserved, zero |
| `+0x20` | `u64` | absolute serialized scene-node-block offset |
| `+0x28/+0x30/+0x38` | `u64` | reserved, zero |

Do not rename `+0x14` to a semantic stage/material id until a direct producer or consumer closes that meaning.

## Object record — 0x40 bytes

Objects begin at file offset `0x40`, fixed stride `0x40`.

| Offset | Type | Meaning/status |
| ---: | --- | --- |
| `+0x00` | `u8` | mesh count |
| `+0x01` | `u8` | runtime-consumed control/classification byte; exact semantics unresolved |
| `+0x02` | `u16` | total vertex count; equals sum of child mesh vertex counts |
| `+0x04` | `u32` | reserved, zero |
| `+0x08` | `u64` | absolute mesh-table offset |
| `+0x10` | `u32` | runtime-consumed object flags; operational mapping partly EXE-confirmed |
| `+0x14..+0x2F` | bytes | reserved, zero |
| `+0x30` | `vec3f` | bounding-sphere center |
| `+0x3C` | `f32` | bounding-sphere radius |

The center/radius interpretation is data-confirmed on 254/254 objects: the stored radius equals the maximum distance from the stored center to serialized geometry within float tolerance.

### Object `+0x01`

The canonical EXE copies source `object+0x01` verbatim to runtime object `+0x07` at `0x14030303A`. Downstream initialization compares the runtime byte against special values including `0xC4` and `0xEA`, can rewrite it in bounded special cases, and propagates values above `0x80` into adjacent runtime state.

Observed serialized values across 254 objects are `0x80` and `0xC0..0xC5`, dominated by `0x80` (237/254). These observations are not a whitelist.

The serialized C++ IR retains a neutral field name until the higher-level meaning is independently proven.

### Object flags `+0x10`

The flags are no longer structurally opaque. The SCM-like runtime object initializer `0x140302F10` and helper `0x140302640` expose a bounded operational projection:

| Source condition | Runtime operation |
|---|---|
| low nibble nonzero | set runtime flag bit 8; pass low nibble as helper mode |
| `0x00000020` | set runtime flag bit 10 |
| `0x00020000` | set runtime flag bit 9 and initialize runtime float vector `(1,1,1,0)` |
| `0x00010000` | set runtime flag bit 7 and flip a helper boolean |
| `0x00040000` | set runtime flag bit 4 |
| high nibble `0x0F000000` nonzero | set runtime flag bit 15 and store `highNibble-1` in runtime byte `+0x0D` |
| `0x00080000` | set runtime flag bit 5 |
| `0x00100000` with nonzero low mode other than 4 | selects helper numeric state `0x5010D` instead of `0x5000D` |

The exact helper state selectors are reconstructed by `scm::runtime::project()` in `scm_runtime_flags.hpp`. The module deliberately does not rename any source bit to a gameplay/render semantic.

Current 68-file corpus union mask is `0x003A0003`. The EXE supports additional source masks not present in that corpus. Conversely, corpus bit `0x00200000` is observed but remains semantically unresolved in this bounded consumer pass.

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
colorFlags = u8 r,g,b,flags       stride 4
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

for every mesh in object/mesh order:
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
orderRel         = 0x20 + align4(N)
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
- EXE-confirmed neutral runtime flag projection for `object+0x10`;
- synthetic regression including compile-time runtime-flag mapping checks;
- 68-file external corpus validation with zero diagnostics.

Not promoted yet:

- semantic name for header `+0x14`;
- exact semantic name for object `+0x01`;
- semantic names for object flag bits, including unresolved observed bit `0x00200000`;
- exact Euler order/coordinate convention for scene rotations;
- material/texture-bundle ownership beyond the validated texture index range;
- canonical writer/export support;
- no-edit byte-identical rebuild gate;
- edited resource repack and original-game consumption receipt.
