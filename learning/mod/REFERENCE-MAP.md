# MOD Quick Reference Map

## Serialized sizes

```text
Document header  0x40
Outer object     0x40
Inner mesh       0x50
Node block hdr   0x20
Local transform  0x20
Runtime texture descriptor 0x40 (in-memory, not serialized MOD)
Runtime node     0xA0 (in-memory)
```

## Header

```text
00 magic "MOD "
04 f32 version
10 u8 object_count
11 u8 node_count
12 u8 texture mirror
13 u8 default_joint_index
14 u32 preserved runtime metadata
20 u64 node-domain offset
40 object table
```

## Object

```text
00 u8 mesh_count
01 u8 alpha_control
02 u16 aggregate_count
08 u64 mesh_table
10 u32 source_flags
18 f32 runtime parameter A
1C u32 runtime parameter B
30 f32x3 bounds center
3C f32 bounds radius
```

## Mesh

```text
00 u16 count
02 u16 texture_slot
04/06/08/0A GS CLAMP
0C u32 preserved
10 u64 positions
18 u64 normals
20 u64 UV
28 u64 blend
30 u64 weights/topology
38 u64 preserved
40 u64 generated workspace rel (mesh-relative)
48 u32 generated count
4C u32 preserved
```

## Streams

```text
position float3 / 12
normal   float3 / 12
UV       int16x2 / 4 / scale 4096
blend    u8x4
control  u16
```

## Skin

```text
bone = raw_blend / 4
weights = 5+5+5 bits / 31
topology break = 0x8000
influences use blend y/z/w
```

## Node domain

```text
+00 parent_rel
+04 order_rel
+08 motion_group_rel
+0C transform_rel
```

## Transform

```text
00 tx
04 ty
08 tz
0C translation magnitude
10 rx
14 ry
18 rz
1C unresolved
```

## Matrix rules

```text
world[root] = local[root] * rootBase
world[node] = local[node] * world[parent]
skin[node]  = inverseRest[node] * currentWorld[node]
position    = world matrix row 3 XYZ
```

## Texture chain

```text
companion.texture_count = runtime authority
mesh.texture_slot       = selector
header +12              = mirror only
descriptor stride       = 0x40
```

## High-value EXE VAs

```text
1402FA080 MOD/EFM transform init
1402F9700 world update
140030DC0 rigid inverse
140300580 palette loop
1402FE3B0 MOD post-load
1402FE6A0 runtime mesh builder
1403029E0 MOD/EFM object init
1402F9890 material helper
140304B30 companion materializer
14030D040 texture descriptor builder
14030F850..14030F9AE CMotion binding
1402F3D0A blend-index CPU consumer
1400461B0 shader lookup
140561370 MOD tag-5 shader descriptor
14048A8B0 DMC3_MOD.hlsl DXBC
```

## Open

```text
header +14
transform +1C
mesh +0C/+38/+4C semantics
BLENDINDICES.x
full object flag semantics
complete MOT decode/current pose source
SHW matrix-palette ownership
production writer
original-game edited MOD acceptance
```
