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

## Current bound corpus

```text
38 unique MOD
166 objects
180 meshes
20,976 vertices
285 transforms
families/contexts: em000 + pl000 main + pl000 cloth + id100
```

## Header

```text
00 magic "MOD "
04 f32 version
10 u8 object_count
11 u8 node_count
12 u8 texture mirror
13 u8 default_joint_index
14 u32 runtime_metadata_u32
20 u64 node-domain offset
40 object table
```

`+0x14`:

```text
serialized -> manager +0xE4       EXE_CONFIRMED
universal decimal identity formula REJECTED
high-level semantic                PRESERVED_UNDECODED
writer                              preserve exact source u32
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

### Important source flags

```text
0x00100000:
  active low-mode GS selector
  TEST_1 AREF 0 <-> 16
  ZBUF_1.ZMSK 1 <-> 0
  technical semantic EXE_CONFIRMED
  artistic/material category open

0x00200000:
  baseline/effective runtime carriage EXE_CONFIRMED
  restoration participation EXE_CONFIRMED
  local GS packet consumer: none
  high-level semantic PRESERVED_UNDECODED
```

## Mesh

```text
00 u16 count
02 u16 texture_slot
04/06/08/0A GS CLAMP
0C u32 preserved undecoded
10 u64 positions
18 u64 normals
20 u64 UV
28 u64 BLENDINDICES
30 u64 weights/topology
38 u64 family-sensitive auxiliary slot
40 u64 generated workspace rel (mesh-relative)
48 u32 generated count
4C u32 preserved undecoded
```

Bound zero histograms:

```text
+0C 180/180
+38 180/180 MOD; canonical MOD path inactive, EFM homologous slot -> COLOR0
+48 180/180 serialized before runtime generation
+4C 180/180
```

Zero histogram does not authorize writer normalization.

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
influences use blend Y/Z/W
```

`BLENDINDICES.x` closure:

```text
compiled DXBC signatures = 8
Mask                      = 0xF
ReadWriteMask             = 0xE
canonical shader reads    = Y/Z/W, not X
direct proven CPU X use   = none
corpus X                  = 0 in 20,976/20,976
writer                     = preserve raw X
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
1C preserved undecoded
```

`+0x1C`:

```text
MOD corpus 285/285 = 0.0f
bound EFM 5/5 = 0.0f
canonical local rotation helper reads XYZ only
local-matrix non-consumption EXE_CONFIRMED
global semantic PRESERVED_UNDECODED
writer preserve
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
1402F9570 model manager init / header +14 -> manager +E4
1402FA080 MOD/EFM transform init
140330450 XYZ rotation helper; does not consume transform +1C
1402F9700 world update
140030DC0 rigid inverse
140300580 palette loop
1402FE3B0 MOD post-load
1402FE6A0 runtime mesh builder
1403029E0 MOD/EFM object init
140302640 legacy GS object-state packet builder
1402F9890 material helper
140304B30 companion materializer
14030D040 texture descriptor builder
14030F850..14030F9AE CMotion binding
1402F3D0A BLENDINDICES Y CPU consumer
1400461B0 shader lookup
140561370 MOD tag-5 shader descriptor
14048A8B0 DMC3_MOD.hlsl DXBC
```

## Closed / narrowed in 2026-09-09 pass

```text
BLENDINDICES.x canonical consumption -> not read by compiled shader; no direct proven CPU consumer
source flag 0x00100000 -> GS TEST_1 AREF + ZBUF_1 ZMSK selector
mesh +38 -> inactive in canonical MOD path; EFM positive-control COLOR0
transform +1C -> not consumed by canonical local rotation helper
header +14 -> old universal decimal semantic rejected
```

## Still open

```text
header +14 high-level manager semantic
transform +1C global semantic outside audited path
mesh +0C/+4C global semantics
mesh +38 semantics outside audited canonical MOD path / alternate variants
source flag 0x00200000 terminal semantic consumer
complete MOT decode/current pose source
SHW matrix-palette ownership
production writer
original-game edited MOD acceptance
```
