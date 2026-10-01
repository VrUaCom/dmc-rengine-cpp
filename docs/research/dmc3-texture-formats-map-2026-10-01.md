# DMC3 HD texture formats: names, layers and variants — 2026-10-01

Canonical executable: `dmc3.exe`, SHA-256
`e454272ed0fb0247fcbcf300e5d55d7a3e96d50b89b9ffaff81bb978dcbdd082`.
Samples: `pl000.pac`, `em000.pac`, `id900.pac` (English), `basic.ptx`,
`at.ptx`, `i001_90.tm2` (user-supplied; not in the repository).

This note fixes the vocabulary for texture tooling. Every name below is either
an executable name (RTTI, string, extension test) or a standard name, and the
note says which.

## 1. The layers

A texture on disk is four nested layers:

```text
NBZ volume            DMC3-<n>.nbz          (top-level resource volume)
 └─ PAC               "PAC\0" slot table     (resource container)
     └─ PTX           texture bundle         (.ptx, CPtxManager)
         └─ gfxTexture  0x70 serialized object  (one per texture)
             └─ DDS     Microsoft DirectDraw Surface (BC1/BC3 in the corpus)
```

| Layer | Real name | Source of the name | Magic |
|---|---|---|---|
| Volume | NBZ | string `%sDMC3-%d.nbz` | ZIP-backed |
| Container | PAC | file names `*.pac`, `PAC\0` | `PAC\0` |
| Texture bundle | **PTX** | extensions `.ptx` `.PTX` `.Ptx` tested by the resource registry `0x1402DB3C0`; classes `CPtxManager` / `IPtxManager` | **none** (identified by extension or by the slot role) |
| Texture header | **`gfxTexture`** | RTTI `.?AVgfxTexture@@`, vtable `0x1404C5388` | none (`+0x00` is the vtable slot, zero on disk) |
| Image | **DDS** | Microsoft format; loader checks `DDS ` and size `0x7C` | `DDS ` |
| Compression | DXT1 = BC1, DXT5 = BC3 | DDS FourCC | — |

So "PTX or DDS?" has one answer: **both, at different layers**. A PTX holds
`gfxTexture` records, and each `gfxTexture` carries one DDS.

## 2. PTX: the texture bundle

Read by `0x140336BB0` / `0x140336A70` (called from `CPtxManager` methods
`0x140314E00` / `0x140314FA0`):

```text
+0x000  u32 count
+0x004  u32 span[count]        size of each texture in 0x800-byte sectors
...     zero padding to 0x800
+0x800  texture 0              next texture at previous + span * 0x800
```

The loop is `r15 = base + 0x800`, then `r15 += span << 11` per texture. Each
texture is handed to `0x1403365B0`, which picks one of two paths by its first
dword:

- `TM2\0` (`0x00324D54`): the PS2-era TM2 texture record (header `+0x04`
  picture count, `+0x08` data offset), materialized by `0x140046C40`. The
  branch exists in the PC executable, but **no PC file in the corpus uses
  it**: every texture starts with zero.
- anything else: a serialized `gfxTexture` (section 3).

`CPtxManager` keeps 32 entries of `0x218` bytes. Each entry has a key
pointer, a reference count and a `0x208` descriptor block that holds the
per-texture records.

## 3. `gfxTexture`: the 0x70 header

`0x140046510` turns the on-disk bytes into a live object in place. It needs
no copy, only pointer fixups:

- if `[+0x00] == 0`, it stores the `gfxTexture` vtable there;
- `[+0x20] += &(+0x20)`: a self-relative pointer to the image record;
- `[image + 0x08] += &(image + 0x08)`: a self-relative pointer to the DDS.

Then `gfxTexture` vfunc `+0x10` (`0x140046AF0`) creates the GPU texture:

- `0x1400499C0(device, image.ptr, image.size, &+0x28)` is Microsoft
  DirectXTK `CreateDDSTextureFromMemory`. It checks `DDS `, the `0x7C`
  header and the `DX10` extension, and maps DXT1–DXT5, ATI1/ATI2, BC4U/BC5U
  and RGBG;
- `QueryInterface`, then `ID3D11Texture2D::GetDesc` (vfunc `0x50`);
- `ID3D11Device::CreateShaderResourceView` (vfunc `0x38`) with
  `ViewDimension = TEXTURE2D`, all mips, stored at `+0x30`;
- `+0x58` = loaded.

Field table (on disk / live):

| Off | Size | On disk | Live / meaning | Evidence |
|---|---|---|---|---|
| `+0x00` | 8 | 0 | vtable `gfxTexture` | `0x140046510` |
| `+0x08` | 4 | encoding word (`0x00020988`, `0x00020A88`, `0x00020185`, `0x000201A5`) | not read on the traced PC path | data |
| `+0x0C` | 4 | `0x0000AAE4` | not read on the traced PC path | data |
| `+0x10` | 2 | width | **logical width**: read by `0x1403365B0`; TBW = ceil(w / 64) | EXE |
| `+0x12` | 2 | height | **logical height** | EXE |
| `+0x14` | 4 | 1 | — | data |
| `+0x18` | 4 | row bytes (BC row, or w*4 for `0x201A5`) | — | data |
| `+0x20` | 8 | `0x40` | pointer to the image record (`+0x60`) | EXE |
| `+0x28` | 8 | 0 | `ID3D11Resource*` | EXE |
| `+0x30` | 8 | 0 | `ID3D11ShaderResourceView*` | EXE |
| `+0x38` | 4 | payload bytes (DDS size − 128) | — | data |
| `+0x3C`..`+0x43` | 8 | variant-specific (model PTX: `+0x3C` = 2, `+0x40` non-zero; interface: 0) | not read on the traced PC path | data |
| `+0x44` | 2+2 | secondary width / height | — | data |
| `+0x48` | 4 | f32 1/width | reciprocal of the logical size | data |
| `+0x4C` | 4 | f32 1/height | | data |
| `+0x58` | 1 | 0 | loaded flag | EXE |
| `+0x60` | 4 | format code (0 DXT1, 4 / 5 DXT5) | image record `+0x00`; not read by the loader | data |
| `+0x64` | 4 | DDS byte size (128 + payload) | image record `+0x04`, size passed to the loader | EXE |
| `+0x68` | 8 | 8 | image record `+0x08`, pointer to the DDS | EXE |
| `+0x70` | … | `DDS ` … | the DDS file | EXE |

`+0x10/+0x12` are the size the game's UV and layout code works with. The DDS
can be larger: the HD remaster upscaled some art and kept the PS2 logical size
in the header. That is the "DDS = 2 × descriptor" relation of `i001_90.tm2`.
The 0x40 / 0x60 split is the original engine's serialization: header object
first, then image record, then the image.

## 4. The variants in the corpus

All four are PTX framing (or a single `gfxTexture`), with a DDS inside. They
differ only in header constants and mip policy.

| Variant (`TextureSlotReadVariant`) | Where | `+0x08` | `+0x60` | DDS mips | DDS vs logical size |
|---|---|---|---|---|---|
| `canonical` | model PTX (pl, em, wp, st) | `0x00020988` / `0x00020A88` | 0 DXT1, 4 DXT5 | full chain | 1× |
| `legacy_single_mip_bundle_dxt5` | `basic.ptx`, `at.ptx` | `0x00020185` | 4 | base only | 1× |
| `legacy_single_mip_interface_bundle_dxt5` | interface packs `id*.pac` (checked: `id900.pac`) | `0x000201A5` | 5 | base only | 1× |
| `legacy_single_mip_wrapped_dxt5` | single `gfxTexture` file named `.tm2` (`i001_90.tm2`) | `0x000201A5` | 5 | base only | 2× |

`id900.pac` (English) checked byte-exact: PTX slots 0, 2, 4 (2 textures), 6
(2 textures), 8, 11, 14, 16 and 18. Every texture is 256² or 512² DXT5, a
`gfxTexture` at the sector, the DDS at `+0x70` and the pixels at `+0xF0`.
Correction: an earlier note read this as a `0x800` header with pixels at
`+0x800`; that shifted the decode.

## 5. `.tm2` in DMC3 PC is not Sony TIM2

- Sony TIM2 begins with `TIM2`. No `TIM2` (or `2MIT`) constant exists in the
  executable code, and no `.tm2` sample has it.
- The executable knows a PS2-era **TM2** texture record with magic `TM2\0`
  (section 2). No PC file uses it.
- PC `.tm2` files are a single `gfxTexture` plus DDS (variant
  `legacy_single_mip_wrapped_dxt5`). The extension is the PS2 file name kept
  by the port.
- The executable names five `.tm2` files: `font\i001_90.tm2` (font page) and
  `id\id900\id950_01{,f,g,i,s}.tm2`. The second group is a language set at
  `0x1405BD660`: entries 0, 1, 6 and 7 (Japanese, English, Chinese,
  SChinese) share the base file.

## 6. Other texture-side names in the executable

- `basic.ptz`: listed next to `basic.ptx` in the debug asset table
  `0x1405B0870`. No reader, no sample; still open.
- `\data\dmc3\LOADERICON.dds`: a bare DDS loaded directly.
- `/eff/texture/TextureDataNameTbl.bin`: effect texture name table.
- Resource registry `0x1402DB3C0` (demo / cutscene resources), type by magic
  or extension: `MOD` 0, `EFM` 1, `SCM` 2, `MRP` 3, `.ptx` 4, `.clt` 5,
  `.c1d` 6, `SHW` 7. PTX, CLT and C1D have no magic.

## 7. Interface pack names

The executable lists 2690 `id\idNNNN\idNNNN<suffix>.pac` paths. The language
suffixes are:

- `J` Japanese;
- none: English;
- `F` French;
- `G` German;
- `I` Italian;
- `S` Spanish;
- `C` / `Z` Chinese;
- `V*` an alternate (voice / version) set of the same.

Ids group by hundreds (`id200`, `id300`, … `id7100`). `0x1402C07F0(id,
language)` maps ids 0..3999 to these records.

## 8. Tooling consequences

- A texture tool works on three levels:
  - PTX (count + sector spans);
  - `gfxTexture` (0x70, with fixups);
  - DDS (standard).
- Replacing a texture means rebuilding the `gfxTexture` header for the new DDS
  (`+0x64`, spans) and keeping the logical size `+0x10/+0x12` unless the UVs
  are meant to change.
- Because the loader is DirectXTK, a PTX can carry any DDS it accepts,
  including `DX10` headers (BC7 and others). That is the route for
  higher-quality new costumes without a new texture format.
- Reader: the interface variant is read through
  `TextureSlotFramingReader::parse`. Native Reader keeps an equivalent strict
  reader in its compatibility layer until its pinned Rengine copy moves
  forward.
