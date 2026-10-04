# DMC3 rendering/resource limits — bounded reverse pass — 2026-10-04

**Branch:** `research/dmc3-render-resource-limits-20261004`  
**Canonical executable:** `dmc3.exe`  
**SHA-256:** `e454272ed0fb0247fcbcf300e5d55d7a3e96d50b89b9ffaff81bb978dcbdd082`

## Rule

This research separates four different ceilings:

1. PS2 GS hardware/register limits.
2. Serialized DMC3 SCM/PTX field limits.
3. Fixed storage and pools in the DMC3 HD executable.
4. Maxima actually observed in preserved retail-derived resources.

A field width is not runtime acceptance. A corpus maximum is not a failure threshold.

## Geometry: fresh SCM corpus census

A fresh pass over exact `DMC 3 RENGINE (6).zip` bytes found **77 SCM paths / 67 unique SHA-256 payloads**.

| Metric | Maximum observed |
|---|---:|
| objects / SCM | 44 |
| scene nodes / SCM | 50 |
| meshes / SCM | 77 |
| vertices / SCM | 23,049 |
| non-degenerate strip triangles / SCM | 14,261 |
| vertices / mesh | 10,196 |
| non-degenerate strip triangles / mesh | 5,696 |
| vertices / object | 14,125 |
| non-degenerate strip triangles / object | 9,619 |
| texture slots / SCM | 17 |

Triangle census uses the recovered retail topology domain `{0,2}`: each vertex index >= 2 whose topology byte does not contain break bit `0x02` contributes one non-degenerate strip triangle.

The expanded 68-unique corpus additionally contains `st114.scm` at **1,038,816 bytes / 27,057 vertices**. It is not inside the historical ZIP used for this fresh triangle census, so **14,261 remains bounded to the 67-unique ZIP until st114 is triangle-censused**.

## SCM serialized ceilings

These are structural ceilings only:

- `header.object_count`: u8 -> 255.
- `header.scene_node_count`: u8 -> 255.
- `header.texture_slot_count`: u8 -> 255.
- `object.mesh_count`: u8 -> 255.
- `object.total_vertex_count`: u16 -> 65,535.
- `mesh.vertex_count`: u16 -> 65,535.
- generated indices are u16.

The canonical invariant is:

`object.total_vertex_count == sum(mesh.vertex_count)`.

Therefore an object cannot serialize more than 65,535 total child-mesh vertices.

With one contiguous mesh and no break flags, 65,535 vertices structurally yield at most **65,533 non-degenerate strip triangles**. Status: `STRUCTURAL_DERIVED`, not an allocation/draw guarantee.

## PTX executable storage

Canonical `dmc3.exe` confirms:

- PTX payload size: `0x208`.
- payload `+0x000..+0x1FF`: **64 qword record pointers**.
- `+0x200`: successful texture count.
- `+0x204`: records-per-texture copied from the first materialized texture.
- global PTX pool: **128 records x 0x50 bytes**.
- first record: `0x140D5FB70`.
- last record start: `0x140D62320`.
- scratch materializer table: **4 pointers**.
- runtime texture descriptor: `0x40` bytes with **4 source-record pointer lanes**.

Status: `EXE_CONFIRMED`.

Do not simplify this to “64 textures maximum.” The loader indexes record pointers using texture index and `records_per_texture`; the practical texture count is still a runtime target.

## DDS corpus: current Rengine profile is too narrow

The same exact archive contains:

- **243 DDS paths**
- **154 unique DDS SHA-256**
- 60 unique DXT1
- 94 unique DXT5

Largest width observed: **1024**.  
Largest height observed: **2048**.

Largest-area specimen:

```text
path   DMC 3 RENGINE/analysis_inputs/stage_drops/plwp_grenade/
       plwp_grenade_000/plwp_grenade_000_000.dds
size   2,796,368
dims   1024 x 2048
mips   12
format DXT5
sha256 00f03a616da7e8ae2a44f5a9a386a21b705ff74118a64bd72734504db23e40c1
```

Current code uses:

```cpp
Dmc3DdsSafety {
    min_dimension = 64;
    max_dimension = 1024;
}
```

That scalar maximum rejects this preserved **1024x2048** DDS. Therefore the current statement that 64..1024 is the envelope derived from all 154 descriptor-backed images is too strong.

Correct evidence boundary:

- 1024x2048 DXT5: `CORPUS_CONFIRMED`.
- universal authored 2048 support: **not proven**.
- writer policy should become axis-aware or resource-family-aware; do not blindly switch to 2048x2048.

## PS2 GS is not the DMC3 HD ceiling

PS2SDK `GS_SET_TEX0` packs `TW` and `TH` in four bits. PCSX2's GS TEX0 handler explicitly documents the **GS spec max exponent as 10**, with fixed texture size limited to **1024x1024**.

References:

- `ps2dev/ps2sdk` — `common/include/gs_gp.h`, `GS_SET_TEX0`.
- `PCSX2/pcsx2` — `pcsx2/GS/GSState.cpp`, TEX0 handler.

This PS2 ceiling must not be copied into the HD profile. The DMC3 HD corpus already contains a 1024x2048 DXT5 image.

Separately, the HD render path is executable-confirmed to reach D3D11 shader binding:

```text
SCM compatibility commands
 -> 0x140044310
 -> 0x140045FE0
 -> ID3D11DeviceContext::VSSetShader / PSSetShader
```

Exact SCM texture state -> exact D3D11 SRV/sampler source binding remains open.

## Rejected shortcuts

- “1024x1024 is the universal DMC3 HD texture maximum.” -> rejected by corpus.
- “PS2 GS maximum equals HD Collection maximum.” -> rejected as an architecture assumption.
- “u16 vertex_count means the engine can render 65,535 vertices in one mesh.” -> not proven.
- “64 PTX pointer slots means exactly 64 textures.” -> not proven.

## Next priority

1. Recover live `records_per_texture` across real PTX bundles.
2. Trace width/height through `0x1403365B0 -> 0x1403366E0 -> 0x14030D040` and catalogue every range check/truncation.
3. Find oversized SCM allocation/normalization/draw failure boundaries.
4. Census a whole loaded room/frame: resident SCM vertices, triangles, objects, meshes and draw packets.
5. Trace PTX publication to exact D3D11 SRV creation/binding.
6. Keep original PS2 DMC3 as a separate hardware/runtime profile.

Machine-readable authority:

`data/reverse/dmc3-render-resource-limits-20261004.json`
