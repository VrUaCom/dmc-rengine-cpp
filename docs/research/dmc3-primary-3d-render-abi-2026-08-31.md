# DMC3 primary 3D/render ABI reverse — 2026-08-31

**Status:** CANONICAL RESEARCH ADDENDUM ON PR #268 BRANCH  
**Target:** canonical unpacked DMC3 HD analysis executable  
**SHA-256:** `e454272ed0fb0247fcbcf300e5d55d7a3e96d50b89b9ffaff81bb978dcbdd082`  
**Scope:** distinguish actual geometry-bearing primary resources from render/effect companions by combining original-runtime fixup code, embedded original shader-source/debug material, independent MOD/SCM decoding evidence, and downstream consumers.

## 1. Result

The previous simplified split:

```text
MOD + SCM = geometry
EFM + MRP + SHW = companions
```

is no longer accurate enough.

The strongest evidence-backed classification is now:

| Family | Geometry status | Canonical purpose boundary |
|---|---|---|
| `MOD` | **EXE_CONFIRMED mesh-bearing** | skinned actor/object model geometry |
| `EFM` | **EXE_CONFIRMED mesh-bearing** | effect-model geometry with an extra per-vertex RGBA stream |
| `SCM` | **EXE_CONFIRMED mesh-bearing** | stage/static scene geometry with per-vertex RGBA/baked-lighting-side data |
| `SHW` | **EXE_CONFIRMED geometry-bearing** | shadow geometry / shadow-volume-side resource; not the same document ABI as MOD/EFM/SCM |
| `MRP` | **runtime family confirmed; own geometry not yet proven** | model/render/effect-side family; exact schema still open |

Consequently a DMC3 3D/render implementation should not be designed as only a `MOD/SCM` viewer. `EFM` and `SHW` are also geometry-bearing runtime resources, but SHW requires a distinct adapter rather than reuse of the MOD/EFM/SCM document shell.

## 2. Original-runtime family identity

Three evidence sites are kept separate by `dmc3-runtime-type-evidence-split-2026-08-31.md`.

Registry three-byte probe `0x1402DB1F0`:

```text
MOD -> 0
EFM -> 1
SCM -> 2
MRP -> 3
SHW -> 7
```

Four-byte family-mask probe `0x1402FD650`:

```text
MOD  -> 0x10000000
EFM  -> 0x20000000
SCM  -> 0x30000000
MRP  -> 0x40000000
MCV  -> 0x50000000
SHW  -> 0x60000000
```

Container traversal `0x1401B9FA0` independently reaches the normal fixups for MOD/EFM/SCM/SHW.

This establishes family identity independently from filename extensions or extraction names.

## 3. Embedded shader evidence

The canonical HD executable retains source/debug strings and source text for DMC3-specific vertex shaders. These are not being treated as proof that `.hlsl` files are shipped game resources; they are evidence about the runtime vertex ABI used by the named DMC3 resource families.

### MOD vertex input

`DMC3_MOD.hlsl` family source describes a vertex input containing:

```text
POSITION    float3
NORMAL      float3
TEXCOORD0   float2
BLENDINDICES uint4
PSIZE       float
```

This is consistent with the recovered MOD runtime fixup and independent DMC3 MOD importer behavior: positions, normals, UV coordinates, bone indices/weights and triangle topology.

### EFM vertex input

`DMC3_EFM.hlsl` family source contains the MOD-like inputs plus:

```text
COLOR0      float4 rgba
```

The embedded original source also explicitly describes EFM as models with an additional vertex RGB channel that is modulated in the shader.

That wording plus the dedicated EFM document fixup closes the broad purpose question: EFM is not merely effect metadata. It is a **mesh-bearing effect-model resource**.

### Stage / SCM vertex input

The DMC3 stage shader family (`DMC3_STG.hlsl`) consumes:

```text
POSITION    float3
NORMAL      float3
TEXCOORD0   float2
COLOR0      float4 rgba
```

This matches both the SCM fixup layout and independent SCM decoding, where the stage mesh carries positions, normals, UVs and vertex colour/baked-lighting-side data.

### SHW vertex input

`DMC3_SHW.hlsl` consumes at minimum:

```text
POSITION    float3
```

The shader transforms the supplied vertex position into shadow output. Therefore SHW is not merely an abstract render flag table: it has a direct shadow-geometry vertex path.

No equivalent `DMC3_MRP.hlsl` source/path was identified in this pass. Absence of that string is not proof that MRP has no geometry; it only means shader-source evidence does not currently promote it.

## 4. MOD runtime document ABI

Normalizer:

```text
VA 0x1402FE3B0
window SHA-256 2319717d2b827fddf1821832ca8bf12a665317d954d116400151f0e95c60c565
```

Recovered high-level structure:

```text
root
  +0x10 count-like byte
  +0x11 MOD-specific mode byte (compared with 1)
  +0x20 base-relative document/group pointer

groups near +0x40
  stride 0x40
  +0x08 relative inner-record pointer

inner records
  stride 0x50
  +0x00 u16 vertex/count-like field
  +0x10 relative stream -> relocated
  +0x18 relative stream -> relocated
  +0x20 relative stream -> relocated
  +0x28 relative stream -> relocated
  +0x30 relative packed stream -> relocated
  +0x40 record-relative generated/derived topology pointer
  +0x48 generated index/topology count
```

The `+0x30` word stream is scanned for bit `0x8000`; the runtime clears/uses that marker while constructing derived triangle/index topology.

Cross-correlation with the MOD shader and independent decoder supports the following field interpretation at high confidence:

```text
+0x10 positions
+0x18 normals
+0x20 UV coordinates
+0x28 blend/bone indices
+0x30 packed weights / strip-control flags
+0x40 derived triangle/index topology
```

These semantic names are a cross-evidence interpretation of exact relocation positions; complete variant ABI and writer equivalence remain open.

## 5. EFM runtime document ABI

Normalizer:

```text
VA 0x1402F7A90
window SHA-256 0b5ccd9aaa1701fab677ea35bd44924f4d2ad1ab9cbfac754dcf7e246ca1052b
```

EFM shares the MOD-family outer geometry shell but is not byte-identical to MOD semantics.

Recovered structure:

```text
root
  +0x10 count-like byte
  +0x20 base-relative pointer

groups near +0x40
  stride 0x40

inner records
  stride 0x50
  +0x10 relocated stream
  +0x18 relocated stream
  +0x20 relocated stream
  +0x28 relocated stream
  +0x30 relocated packed stream
  +0x38 relocated extra stream
  +0x40 record-relative generated/derived topology
```

The same `0x8000`-marked word-stream logic builds triangle/index topology.

Shader + fixup correlation yields the strongest current interpretation:

```text
+0x10 positions
+0x18 normals
+0x20 UV coordinates
+0x28 blend/bone indices
+0x30 packed weights / strip-control flags
+0x38 vertex RGBA
+0x40 derived triangle/index topology
```

The critical distinction from MOD is the extra `+0x38` vertex colour stream, directly corroborated by EFM shader `COLOR0` input.

Therefore:

```text
EFM geometry-bearing effect model = EXE_CONFIRMED
EFM complete binary schema/writer = NOT YET COMPLETE
```

## 6. SCM runtime document ABI

Normalizer:

```text
VA 0x1403051B0
window SHA-256 5f3923913db171026470d8d15537d58b823f19f9a6770b6508cee778d1fbd321
```

SCM retains the related outer document family but its inner layout differs from MOD/EFM.

Recovered evidence includes relocated streams around:

```text
+0x10
+0x18
+0x20
+0x38
+0x40 derived topology
```

Triangle-strip/control state is read from the fourth byte of a four-byte-per-vertex stream reachable through `+0x38`, using a `& 2` test.

Combined with the stage shader and independent importer evidence, the safest current interpretation is:

```text
+0x10 positions
+0x18 normals
+0x20 UV coordinates
+0x38 vertex RGBA + control-side bytes
+0x40 derived triangle/index topology
```

SCM should therefore share a common abstract mesh layer with MOD/EFM while preserving a distinct static-stage vertex/control adapter.

## 7. SHW is geometry-bearing but structurally distinct

Normalizer:

```text
VA 0x1403204C0
window SHA-256 14dc368e054ef8a7ed686e55de23b0ac1e8d20be66a9909576bee01f34ca008d
```

Unlike MOD/EFM/SCM, its immediate fixup is compact:

```text
count byte @ root +0x10
records walked with 0x40 stride
for each record, four qword fields are rebased:
  +0x30
  +0x38
  +0x40
  +0x48
```

This rejects the older overgeneralization that SHW shares the same generic inner `0x50` mesh-document shell as MOD/EFM/SCM.

However downstream SHW code provides direct geometry evidence. It obtains three indexed spatial points and calls the helper around `0x140320BB0`, which forms vector differences/cross-product style plane data from the three points. Together with `DMC3_SHW.hlsl` consuming a `float3 POSITION`, this establishes a triangle/spatial shadow-geometry path.

Canonical boundary:

```text
SHW geometry-bearing shadow resource = EXE_CONFIRMED
SHW same mesh ABI as MOD/EFM/SCM     = REJECTED
SHW complete record semantics         = OPEN/PARTIAL
```

## 8. MRP remains the major primary-family gap

MRP is confirmed by two independent byte classifiers:

```text
MRP -> registry type 3
MRP  -> family mask 0x40000000
```

But this pass does not find:

- an immediate MOD/EFM/SCM/SHW-style normalizer in the primary registrar/container path;
- an embedded `DMC3_MRP.hlsl` shader-source path;
- a real supplied standalone MRP sample for byte-to-field validation.

Therefore it is incorrect either to call MRP a mesh or to call it non-geometric.

Current status:

```text
MRP runtime 3D/render family identity = EXE_CONFIRMED
MRP exact geometry ownership          = RESEARCH_REQUIRED
MRP schema                            = OPEN
```

The highest-value next reverse is to trace consumers of family mask `0x40000000` and acquire one real retail MRP payload.

## 9. Canonical implementation architecture

The evidence now supports one common logical 3D layer with separate ABI adapters:

```text
                 Dmc3Mesh / render-neutral geometry model
                              |
        +---------------------+---------------------+
        |                     |                     |
      MOD                   EFM                   SCM
 skinned actor        skinned/effect mesh       stage mesh
 adapter              + RGBA adapter          + RGBA/control
        |
        +---- common positions / normals / UV / topology concepts

SHW
  -> separate ShadowGeometry adapter
  -> position/triangle/plane-side data

MRP
  -> unresolved primary render-family adapter
```

Do **not** implement EFM as metadata attached to MOD unless future evidence proves that ownership relation. EFM has its own typed document, fixup and shader family.

Do **not** force SHW through the MOD/EFM/SCM `0x50` inner-record parser. Its recovered layout is different.

## 10. Product status consequence

Current clean product support is still behind the reverse evidence. This pass proves architecture and partial ABI; it does not claim a complete safe writer.

Recommended promotion sequence:

```text
1. shared read-only mesh IR
2. MOD reader adapter
3. SCM reader adapter
4. EFM reader adapter including vertex RGBA
5. SHW shadow-geometry reader adapter
6. real-corpus validation for each family
7. only then writer/rebuild equivalence
8. trace/acquire MRP and add its adapter when proven
```

A future 3D viewer can therefore target MOD + EFM + SCM + SHW as geometry-bearing sources while showing MRP as a linked unresolved primary render resource rather than pretending it is already decoded.
