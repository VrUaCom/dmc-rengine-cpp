# DMC3 engine upgrade: realism, external shading, physics and resource tables

Date: 2026-10-01
Executable: `dmc3.exe` (PC, Direct3D 11), SHA-256
`e454272ed0fb0247fcbcf300e5d55d7a3e96d50b89b9ffaff81bb978dcbdd082`.
Kind: design consultation and research plan, no integration code. Game files
were analysed locally and are not committed; only ids, offsets and addresses
are quoted. Any mod built from this ships tools and patches, never Capcom data.

## 1. What the reverse already gives us

| Area | Known | Source |
| --- | --- | --- |
| Shaders | 11 HLSL families embedded as DXBC: `DMC3_MOD`, `_SP`, `_STX`; `DMC3_EFM`, `_SP`, `_STX`, `_VA`, `_VA_SP`; `DMC3_SHW`; `DMC3_STG`. The HLSL source is in the PDB blob of the exe (e.g. `DMC3_SHW.hlsl` at file offset `0x4AF800`); `DMC3_MOD_SP` DXBC at `0x140493640`, size `0x96B0`. | `dmc3-shw-shadow-projection-2026-09-23.md`, `dmc3-mod-mesh-preservation-skin-abi-2026-09-07.md` |
| Skinning | 3 influences per vertex (blend index lanes y/z/w), 5+5+5-bit weights summing to 31. | `dmc3-mod-blendindices-x-reverse-2026-09-09.md`, `dmc3-mod-skin-weights-2026-09-04.md` |
| Shadows | SHW = CPU stencil shadow volumes: closed convex hulls, one joint each (Dante: 17 body + 17 coat hulls), silhouette per frame (`0x1403200D0`, `0x1403204F0`), extruded on the GPU and filled with one constant `sdwColor`. Light vector = point `[shw+0x60]` minus a model joint. The hull count follows the MOD node count (`+0x11`). | `dmc3-shw-shadow-projection-2026-09-23.md` |
| SHW authoring | Hulls can be generated from any skinned MOD (dominant-joint grouping, 18-direction extreme points, convex hull, T = 2V - 4, edge adjacency); done for the `pl011` repair. | `dmc3-player-pac-mod-repair-2026-09-25.md` |
| Lighting | Characters: two light colour records per model (`vtbl+0x118..+0x130`, used e.g. for em000's sand tint). Stages: point lights `# SET n LIGHT` (`CStageSetLight`); `DMC3_STG` reads `COLOR0` (alpha confirmed; whether RGB is used is open). | `dmc3-effect-triggers-2026-10-01.md`, `dmc3-scm-material-texture-render-chain-2026-09-04.md` |
| Cloth | CLT chains: springs, gravity, 6 body collision shapes for block 0. Fixed per class: CPlDante 39 coat joints at `+0xA0D0`, chains `+0xA210`, next table `+0xA300` (one chain fits; `ClothNum 2` overwrites the next table); CPlVergil fits two. | `dmc3-cloth-chain-solver-2026-09-23.md` |
| Multi-part characters | Body (slot 1) + coat (slot 12) with its own SHW (slot 14); weapons, Lady components and em000 cloth / weapon attach to joints. | `dmc3-player-coat-attachment-2026-09-23.md`, Reader `attachment_tables` |
| Archives | `DMC3-N.nbz` STORE zip volumes, logical paths `GData.afs/...`; a next-volume overlay that replaces a member is proven (`NbzStoreOverlayWriter`). | `dmc3-mod-nbz-overlay-reopen-gate-2026-09-09.md` |

## 2. Realism options (first consultation)

### Shadows
- **On walls and objects.** A shadow volume shades every surface inside it,
  walls included. If the game shows Dante's shadow on floors only, the cause is
  the light point (`[shw+0x60]`, probably above the character) or the
  extrusion length: research item R2.
- **A shadow for every enemy and object.** Generate SHW from the MOD (tool, no
  game code). Enabling it in game needs a patch: em000's class init does not
  call the SHW builder `0x14031FD30`. Overlapping volumes do not double-darken.
- **Soft edges.** Not possible with stencil volumes alone: a blur pass on the
  shadow mask, or shadow maps.
- **Shadow maps** (largest gain): one depth pass from the dominant light, all
  MOD / SCM models cast and receive (self-shadowing, walls, other enemies),
  PCF / PCSS soft edges, SHW becomes optional. Needs a new render pass plus
  replacement MOD / STG pixel shaders.
- **Stage "shadow files" = baked lighting.** Stages are static: bake ambient
  occlusion and soft indirect light offline into SCM vertex colours (or a
  lightmap). Zero runtime cost; asset-only if `DMC3_STG` multiplies by
  `COLOR0.rgb` (R1 answers this).

### Materials and post-processing
- Post-processing through a `d3d11.dll` proxy (ReShade-style): SSAO, bloom,
  tone mapping, colour grading, SMAA; no asset changes.
- Replacement MOD / STG shaders with the same input layout: specular, Fresnel,
  rim light, normal maps (no tangents in MOD: derive them in the shader).
- Texture upscaling of the PS2-era PTX.

### Models and physics
- **High-poly cutscene Dante in gameplay:** the skeleton must match the
  gameplay one (24 body nodes) or be retargeted; 3 influences per vertex,
  5-bit weights, vertex and matrix-palette limits; SHW regenerated.
- **Fully physical coat, sleeves, hair:** new joints and CLT chains plus a class
  change (the fixed 39-joint / one-chain tables above). Hair as chains (like
  Nevan's dress); trousers / boots better as skinned with corrective joints.
  Best path: a modern solver (XPBD) in rengine first, then replace the game's
  chain step (`0x1402CA1D0` setup, `0x1402CA2F0` shapes) through a hook.
- **Character from parts** (hair, trousers, boots, coat, each with joints and
  SHW): matches the engine's own body + coat pattern.

## 3. External data files (user proposal, 2026-10-01)

The idea: move what the exe hard-codes into editable files that one loader
reads, keeping the original behaviour as the fallback.

### 3.1 Shading pack (`DMC3Shading`)
- **Today:** the shaders are DXBC blobs inside the exe, handed to
  `ID3D11Device::CreateVertexShader` / `CreatePixelShader`.
- **Mechanism:** a proxy `d3d11.dll` hooks those calls, identifies each blob
  by hash, and substitutes the entry from an external pack. New passes (shadow
  depth, shadow mask blur, post) are inserted at draw / `Present` time.
- **Pack layout (proposal):** entries `{family, stage (VS/PS), original
  hash, input signature, constant-buffer layout, DXBC, optional HLSL
  source}`. The original 11 families stay as fallback entries; new families
  are added by name.
- **First step (R1):** extract all embedded HLSL sources and DXBC with their
  input / constant-buffer signatures. That is the contract every replacement
  has to keep.

### 3.2 Universal physics and attachment manifest
- **Today:** fixed per class. Dante 39 coat joints and one chain, Vergil two
  chains; Nevan's dress and sleeves; Lady's hair component; the em000 cloth
  slot + CLT; attach joints and local matrices per class
  (`attachment_tables`).
- **Proposal:** one text file per character (CLT-like) declaring:
  - parts: model slot, host joint, local matrix, SHW slot;
  - cloth groups: e.g. `coat 2 slots`, `sleeves`, `hair 8 rows x 4
    joints`, with gravity, spring, stiffness and collision shapes;
  - collision capsules.
- **Engine side:** one hook replaces the per-class cloth / attach setup with a
  reader of the manifest that allocates its own joint tables, which removes
  the `+0xA300` overlap limit.
- **Order:** rengine already has the schema pieces (`cloth_chain`,
  `attachment_tables`, `collision_shapes`). Implement and preview the manifest
  in Native Reader first, then the game hook.

### 3.3 Editable resource map
- **Found today:** the game's resource table is in `.data` (writable), 16-byte
  records `{const char* name, loaded resource}`:
  - `0x1405B0888..`: `obj\debug\at000.mod` .. `at003.mod`, `at.ptx`,
    `scr\ss900_t.pac`;
  - `0x1405B08C8`: `obj\pl000.pac`, then `pl011`, `pl013`, `pl015`,
    `pl016`, `pl018`, `pl001`, `pl002`, `pl021`, `pl023`, `pl026`;
  - `0x1405B0970`: a list of pointers into the table (`0x1405B08C0`,
    `0x1405B0920`, `0x1405B0930`, `0x1405B0940`), probably a grouping, not
    decoded yet;
  - `0x1405B0998`: `pl005` .. `pl010`, `pl017`, `pl014`, `pl025`.

  The name strings sit in `.rdata` at `0x1404EB370..` (20 player PACs), then
  `motion\pl000\pl000_00_0.pac` .. `_31`, `pl001_00_*`, `pl021_00_*`,
  `pl002_00_*`, then `obj\plwp_*.pac`.
- **Rename without patching the exe:** because the table is in `.data`, a
  loader DLL can rewrite name pointers at start-up from an external map file,
  e.g. costume `pl011` -> `obj\pl000_dmc4.pac`, and add that member to a
  `DMC3-N.nbz` overlay.
- **New entries** (a 21st costume) need more: a relocated, larger table with
  its references patched, the costume count and selection code (R4), and the
  per-costume class data (e.g. `0x1402151F4` skips the coat shadow for
  costume bytes 5 and 7).
- **Overlay with new names:** the overlay writer has proved member
  replacement; whether the NBZ index accepts a member name the base volumes do
  not have is untested (R5).

### 3.4 Costume list file
A file with the costume count and, per costume: PAC, motion banks, coat /
cloth manifest, SHW policy, menu name. Depends on R4.

### 3.5 One loader, four files
A single proxy DLL (`d3d11.dll` or `dinput8.dll`) reads the shading pack,
the physics manifests, the resource map and the costume list. Each file is
optional: when it is absent, the vanilla path runs. rengine is the authoring
side (tools build and validate the files); Native Reader previews them.

## 4. Research items, in order

| Id | Item | Gives |
| --- | --- | --- |
| R1 | Extract the embedded HLSL / DXBC of the 11 families with signatures and constant buffers | shading pack contract; answers `DMC3_STG` `COLOR0.rgb` |
| R2 | Light point `[shw+0x60]` and extrusion length of SHW volumes | shadows on walls with data / constant changes only |
| R3 | SHW generator in rengine + per-class SHW enable points (`0x14031FD30` callers) | shadows for enemies and objects |
| R4 | Costume count, selection menu and per-costume class switches; decode the list at `0x1405B0970` | costume list file |
| R5 | NBZ lookup of member names not in the base volumes | adding new PACs by name |
| R6 | Cloth / attach setup per class (`0x1402CA1D0`, `0x1402CA2F0`, `+0xA0D0..+0xA300`) as one manifest schema | physics manifest |
| R7 | Stage AO / light bake prototype for st000 / st002 in Native Reader | baked stage shadows |

## 5. Lab limits
Native Reader renders on the CPU (own software rasterizer, no GPU API), which
suits bakes, SHW generation and cloth, not shader prototyping. Shader work
needs a GPU backend (a desktop rengine D3D11 / Vulkan viewer).
