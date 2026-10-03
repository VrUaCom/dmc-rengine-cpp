# DMC3 resource and rendering limits — evidence ledger — 2026-10-04

**Branch:** `research/dmc3-resource-rendering-limits-20261004`  
**Canonical executable:** `dmc3.exe`  
**SHA-256:** `e454272ed0fb0247fcbcf300e5d55d7a3e96d50b89b9ffaff81bb978dcbdd082`  
**Scope:** distinguish PS2 hardware limits, serialized-format limits, DMC3 runtime storage limits, observed retail maxima, and HD-Collection/D3D11 resource behavior. Do not collapse these into one "engine maximum".

## Result summary

There is no single valid number for "maximum polygons" or "maximum texture size" in DMC3.

The correct model is layered:

```text
PS2 hardware/GS representable domain
        !=
DMC3 serialized field domain
        !=
DMC3 runtime pool/buffer domain
        !=
largest resource observed in retail corpus
        !=
HD Collection D3D11 backend capability
```

Any editor/exporter that uses one number for all five layers is unsound.

## 1. SCM stage geometry

### Serialized limits

Canonical SCM fields establish:

| Domain | Serialized field | Representable bound | Status |
|---|---|---:|---|
| objects per SCM | header +0x10 u8 | 255 encoded | EXE_CONFIRMED field width |
| scene nodes | header +0x11 u8 | 255 encoded | EXE_CONFIRMED field width |
| texture-slot mirror | header +0x12 u8 | 255 encoded | EXE_AND_CORPUS_CONFIRMED mirror |
| meshes per object | object +0x00 u8 | 255 encoded | EXE_CONFIRMED field width |
| vertices per object | object +0x02 u16 | 65,535 | STRUCTURAL_CONFIRMED |
| vertices per mesh | mesh +0x00 u16 | 65,535 | STRUCTURAL_CONFIRMED |
| texture index | mesh +0x02 u16 | 65,535 encoded | EXE_CONFIRMED, but live companion count is tighter |
| generated index count | mesh +0x48 u32 runtime | 32-bit count field | EXE_AND_CORPUS_CONFIRMED |

The object invariant is:

```text
object.total_vertex_count == sum(mesh.vertex_count)
```

Therefore one object's serialized vertex population cannot exceed **65,535**, even though it may contain multiple meshes.

The generated topology reserve is:

```text
align16(6 * (vertex_count - 2))
```

At the largest u16 vertex count (65,535), the reserve formula evaluates to **393,200 bytes**.

For a single uninterrupted triangle strip, the non-degenerate triangle count is at most `vertex_count - 2`; therefore the representable per-mesh topology ceiling is at most **65,533 non-degenerate triangles**. This is a structural derivation, **not** a runtime-performance claim.

### Effective object-binding limit

SCM's object-binding array is serialized as signed i8 and the confirmed validator/runtime contract accepts only:

```text
-1 = helper/no geometry binding
0..127 = object index
```

Every serialized geometry object is required by the confirmed corpus contract to be bound exactly once.

Therefore the currently confirmed SCM hierarchy encoding can bind at most **128 geometry objects (indices 0..127)**, despite the header's u8 object-count field being able to encode 255.

Status: **STRUCTURAL_CONFIRMED derivation**. Runtime acceptance at exactly 128 objects has not been stress-tested.

The parent array is also signed i8. Scene nodes may be encoded by the u8 order array above index 127, but nodes above 127 cannot be named as parents by this confirmed representation. Do not simplify this to "SCM has only 128 nodes"; the restriction is specifically on parent references.

### Observed retail SCM maxima

Hash-bound 68-unique-SCM corpus:

```text
unique SCM files       68
objects                254 total
meshes                 481 total
scene nodes            328 total
vertices               182,612 total
largest SCM file       st114.scm = 1,038,816 bytes
max vertices / mesh    10,196
texture-slot counts    6 / 16 / 17
```

The consolidated corpus separately records a largest observed per-file vertex population of **27,057 vertices**.

These are **CORPUS_CONFIRMED observed maxima**, not engine ceilings.

## 2. MOD actor/model geometry

The common MOD model ABI uses the same important narrow count classes:

- header outer/object count: u8;
- per-object mesh count: u8;
- object aggregate element/vertex count: u16;
- mesh element/vertex count: u16;
- texture-slot mirror: u8.

The bounded 38-unique-MOD corpus currently records:

```text
unique MODs             38
objects                 166
meshes                  180
vertices                20,976 total
largest observed mesh   773 vertices
largest supplied model  pl000 main: 5,316 vertices
```

Status: **CORPUS_CONFIRMED for this bounded corpus**. It is not a whole-retail MOD maximum.

## 3. PTX / texture runtime storage

Canonical EXE reverse establishes a fixed runtime payload:

```text
PtxPayloadState
+0x000..+0x1FF  64 qword record-pointer slots
+0x200          texture_count
+0x204          records_per_texture
size            0x208
```

Important correction: **64 is not "64 textures"**. It is 64 **record pointers**.

The loader indexes the table by:

```text
texture_index * first_records_per_texture + record_index
```

so the normal representable population is constrained by the product of texture count and records-per-texture. The exact safe runtime texture count therefore depends on materializer record multiplicity.

Global PTX pool:

```text
records             128
record stride       0x50
first record        0x140D5FB70
occupancy map       pool +0x2800
```

This is **EXE_CONFIRMED** fixed runtime storage.

The materializer scratch state exposes **four scratch record pointers**. Existing differential tests cover 1–4 records. Inputs above that are not promoted as a supported runtime domain.

### Serialized texture bundle count

The texture-bundle framing has a fixed `0x800` byte header:

```text
u32 texture_count
u32 sector_span[texture_count]
```

The physical table itself can encode at most:

```text
(0x800 - 4) / 4 = 511 entries
```

This is a **STRUCTURAL_CONFIRMED serialized framing bound**.

It is not the same as runtime usability: the 64-pointer payload and 128-record global pool are tighter runtime storage domains.

## 4. Retail DDS corpus and the 1024 assumption

A fresh scan of the exact historical archive already bound by the project:

```text
DMC 3 RENGINE (6).zip
size   237,658,858
sha256 7680a9ddb700b958ca1591be0629c2ff1da53efa1b723141bbee0ae4b4c7ff6f
```

finds:

```text
DDS paths                 243
unique DDS SHA-256        154
compression               DXT1 / DXT5
observed dimensions:
  128x64
  128x128
  256x128
  256x256
  256x512
  512x256
  512x512
  1024x1024
  1024x2048
```

Largest observed DDS:

```text
path:
analysis_inputs/stage_drops/plwp_grenade/
plwp_grenade_000/plwp_grenade_000_000.dds

size       2,796,368 bytes
dimensions 1024x2048
mips       12
format     DXT5
sha256     00f03a616da7e8ae2a44f5a9a386a21b705ff74118a64bd72734504db23e40c1
```

Therefore the existing `Dmc3DdsSafety{min_dimension=64,max_dimension=1024}` must **not** be documented as the maximum accepted DMC3-HD texture domain. It rejects at least one preserved corpus DDS.

Status: **CORPUS_CONFIRMED contradiction / product-profile bug**.

Do not fix this by blindly changing the rule to a symmetric 2048x2048 envelope. The corpus currently proves a 1024x2048 specimen, not arbitrary 2048 widths.

## 5. PS2 legacy GS texture domain

The legacy GS TEX0 representation uses logarithmic TW/TH texture dimensions. External GS implementations document the specification maximum exponent as 10, i.e. **1024 texels per axis** for the normal TEX0 dimension domain.

The canonical DMC3 HD executable independently contains the compatibility-side dimension helper at `0x140331070`, whose recovered path enumerates powers through exponent 10. This keeps the legacy compatibility domain aligned with the PS2 GS 1024-axis ceiling.

Status:

- PS2 GS 1024-axis domain: **HARDWARE/SPEC CORROBORATED**;
- DMC3 compatibility helper exponent domain through 10: **EXE_CONFIRMED**;
- statement that every HD DDS must obey 1024x1024: **REJECTED** by the 1024x2048 retail DDS specimen.

This is the key architectural split:

```text
legacy GS-compatible texture state
    <= 1024 axis encoding

HD Collection physical DDS/D3D11 resource path
    can contain a preserved 1024x2048 DDS
```

The exact D3D11 runtime acceptance ceiling is still open.

## 6. PS2 memory/performance ceiling is a budget, not a file-format number

The PS2 Graphics Synthesizer has **4 MiB embedded video memory**. This is the dominant physical residency budget for frame/depth buffers, textures/CLUTs and render targets; assets can be streamed/reuploaded, so total stage texture data is not limited to 4 MiB on disk.

Public PS2 performance specifications quote very high synthetic primitive/vertex rates, but those values are **not valid as a DMC3 scene polygon limit**. Real throughput depends on VU work, skinning, clipping, packet construction, texture uploads, fill rate, alpha, Z, overdraw, effects, and frame target.

For DMC Rengine, theoretical PS2 marketing/peak figures must remain separate from DMC3 engine limits. They are useful as hardware context only.

## 7. Current limit matrix

| Resource | Observed retail | Serialized/structural | EXE/runtime | Status |
|---|---:|---:|---:|---|
| SCM vertices / mesh | 10,196 | 65,535 | exact stress ceiling open | corpus + structural |
| SCM vertices / object | corpus-dependent | 65,535 summed | exact stress ceiling open | structural |
| SCM geometry objects | observed <=66 in fresh stage subset | u8 says 255; signed binding makes 128 bindable | exact stress ceiling open | structural |
| SCM nodes | observed <=67 in fresh subset | u8 255; parent refs only 0..127 | exact stress ceiling open | structural |
| SCM texture-slot mirror | 17 observed | 255 | live companion/runtime tighter | EXE + corpus |
| MOD mesh vertices | 773 in 38-MOD set | 65,535 | exact stress ceiling open | corpus + structural |
| PTX payload records | normal corpus not yet globally censused | 64 pointer slots | fixed 0x208 payload | EXE_CONFIRMED |
| PTX global records | — | — | 128 x 0x50 | EXE_CONFIRMED |
| texture bundle entries | 17 in current model samples | 511 table entries | PTX storage tighter | structural + EXE |
| DDS dimensions | 1024x2048 max observed | descriptor uses 16-bit dimensions | HD backend ceiling open | corpus |
| legacy GS axis | <=1024 | TW/TH exponent <=10 normal domain | helper 0x140331070 matches | hardware + EXE |

## 8. What is still OPEN

The following must not yet be represented in UI as hard game maxima:

1. maximum total triangles visible/drawn in one DMC3 frame;
2. maximum total SCM vertices that can be loaded in one room before allocation failure;
3. maximum MOD vertices accepted by the original runtime under stress;
4. exact maximum simultaneously resident PTX records under real stage+actor load;
5. exact HD D3D11 texture dimension/byte-size ceiling;
6. exact heap/allocation ceilings for manager object/mesh/node arrays;
7. exact draw packet/VIF/DMA chunk-size ceilings inherited from the PS2-origin compatibility pipeline;
8. original-game acceptance of deliberately stress-authored assets at the structural maxima.

## 9. Required next experiments

Priority order:

1. instrument or emulate SCM manager allocation paths around `0x1402F9570`, `0x140303C10` and downstream object/mesh materialization to recover allocation-size arithmetic and failure gates;
2. run bounded synthetic SCM stress series: mesh vertices 10,196 -> 16k -> 32k -> 65,535, preserving all layout invariants;
3. stress object count up to the signed-binding boundary 128 and explicitly reject/characterize 129;
4. census PTX `texture_count * records_per_texture` across representative stage/player/enemy bundles;
5. trace the 1024x2048 grenade DDS through parser -> PTX materializer -> finalizer -> D3D11 resource creation;
6. recover D3D11 `CreateTexture2D` descriptor width/height path and any DMC3-side clamp/check before the API call;
7. only after those experiments publish a user-facing "safe authoring budget".

## Product rule

DMC Rengine should expose at least three values instead of one:

```text
Observed retail maximum
Confirmed structural maximum
Confirmed runtime-safe maximum
```

When the runtime-safe maximum is not proven, display **UNKNOWN / NOT STRESS-CONFIRMED** rather than substituting the field-width maximum.
