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

### 3.3a Costume groups: how a costume is picked (2026-10-01, later)
- **Loader `0x1401B8F50(mgr, character dl, ?, costume r9b)`.** The group
  table `0x1405B0970` (`.data`) holds one pointer per character:
  - 0 -> `0x1405B08C0`;
  - 1 -> `0x1405B0920`;
  - 2 -> `0x1405B0930`;
  - 3 -> `0x1405B0940`.

  The record is `group[character] + costume * 16`: 16-byte records `{u64
  loaded / state, const char* name}`. The manager slot is `mgr + (word
  [0x140581A20] + character) * 72`; state 3 = already loaded. The record goes
  to `0x1401B84E0`, which loads it.
- **Groups.**
  - Character 0 starts at `pl000`, followed by `pl011`, `pl013`, `pl015`,
    `pl016`, `pl018` (the Dante costumes).
  - Characters 1, 2 and 3 start at `pl001`, `pl002` and `pl021` (`pl021`,
    `pl023`, `pl026` follow).
  - Groups are contiguous runs of the same record array, so a group's length
    is implied by where the next one starts.
- **Second loader `0x1401B8FF0`** (costume `r8b` <= 5, index `character * 7
  + costume`, manager base word `[0x140581A26]`), group table `0x1405B0A40`:
  - 0 and 2 -> `pl005..`;
  - 1 and 3 -> `pl010..`.

  The `pl005`..`pl010`, `pl017`, `pl014`, `pl025` run is probably the devil
  trigger forms.
- **Adding costumes without replacing one.** The group pointers are writable
  `.data`, so a loader DLL can point character 0 at a new, longer record
  array (the original six records first, new names after it), with no exe
  patch for the loading path. Still needed (R4):
  - the manager slot count (`* 72` slots from `[0x140581A20]`) has to cover
    the new indices;
  - the costume menu / selection limit;
  - the per-costume switches in the class code (e.g. costume bytes 5 / 7 at
    `0x1402151F4`) need defaults for new indices;
  - the NBZ lookup of new member names (R5).

### 3.3c R4 progress: the costume path end to end (2026-10-01)
1. **Character config.** `0x1401DF320(character, cfg)` loads a character from
   a 0x84-byte config: bytes `+0..+4` are the five weapon ids, `+0x34` the
   costume code, `+0x35` a flag. The config is copied by `0x1402178B0` from
   `game+0x118` into `game+0x1C4` (mission start, `0x14023A26D`).
2. **Dante's costume codes.** Codes 0..7 select 6 PAC records:
   - codes 0..5 select records 0..5 directly;
   - code 6 loads record 2 (`pl013`);
   - code 7 loads record 5 (`pl018`).

   The code itself still drives class switches:
   - codes 3..5 and 7 load other second-loader banks (`0x1401B8FF0` with
     `r8b = 2`) and style / weapon flags (`0x1401B9160`, `r9b = 2`);
   - code 2, or flag `+0x35`, gives `r9b = 1`;
   - codes 5 and 7 skip the coat shadow (`0x1402151F4`).
3. **Character 3** (`pl021` group): code 1 -> record 1, codes 3..4 ->
   record 2, others -> record 0.
4. **One manager slot per character.** `0x1401B8F50` puts the record into
   slot `word [0x140581A20] (= 0) + character`; the costume only picks which
   record (name) loads into that slot. Manager slot bases (`0x140581A20`):
   `0x0000`, `0x0004` (weapons, `0x1401B90B0`), `0x008C`, `0x00C8` (second
   loader), `0x00E4`, `0x00E5`, `0x0165`, `0x016B`. **Adding costumes needs
   no extra manager slots.**
5. **Record and lookup.** A record is `{u16 kind, pad, const char* name}`;
   `0x1401B84E0` stores it at slot `+0x18`. `0x1401B7B90` passes `name` to
   `0x1402EF620`, which turns `/` into `\` and opens the path through
   `0x1400333F0` -> `0x14002FCA0` (open) / `0x14002F9F0` (size, in 2 KB
   sectors). Files are found by path string at load time. Whether that layer
   indexes members that only a new `DMC3-N.nbz` adds is R5.

**Plan for new costumes, from this:**
- a longer record array for the character (originals first, new names
  after), pointed to from the `.data` group table `0x1405B0970`;
- costume codes beyond 7 need one remap hook in `0x1401DF320` (today the
  6 -> 2 and 7 -> 5 mapping) plus default class switches for the new codes;
- the menu that writes the costume code into `game+0x118+0x34` still has to
  be found (UI classes: `CUIDMisSelect` vtable `0x1404E9E50`,
  `CSceneMisSelect` `0x1404E31D8`, `CMisSelect` `0x1404DC508`,
  `CCustomizeData` `0x1404C8790`);
- R5 decides whether new names load from an overlay volume.

### 3.3d R5 answered: new file names load (2026-10-01)
- **Mounts** (`0x14002E930`):
  1. the directory `<exe dir>\data\dmc3\` (`0x140326D20`, type 0, flags
     `0xC`);
  2. `DMC3-0.nbz`, `DMC3-1.nbz`, ... in order until the next number is
     missing (`0x140327720` / `0x140326DA0`, type 1, own member index built
     by `0x140328320` / `0x140327CC0`).

  Every mount is pushed on the head of the list `[0x140CF3180]`
  (`new->next = head`). The search therefore goes: highest volume first,
  lower volumes, and the loose directory last. A later volume overrides an
  earlier one.
- **Open by name** (`0x14002FCA0`): the requested path is cut to its file
  name (everything after the last `\` or `/`). Then `prefix + name` is
  tried for the prefixes `GDataX360.afs/`, `GData.afs/`, `Video/`,
  `afs/sound/`, `SAVEDATA/` and the empty one, in two passes (table
  `0x14055AEF8`, open `0x140327430`). For each mount:
  - an archive looks the logical path up in its index (`0x140328160`, then
    `0x140328290`);
  - the directory joins it to its root (`0x140327160` drops leading
    slashes).
- **Consequence:**
  - a new PAC is found when its logical path `GData.afs/<name>` exists in
    any mounted volume, e.g. a new contiguous `DMC3-N.nbz`, or as a loose
    file `data\dmc3\GData.afs\<name>`;
  - the folder in the table name (`obj\`, `motion\pl000\`) does not take
    part, so file names must be unique within `GData.afs/`;
  - **R5 is closed:** adding costume PACs needs no change to the file layer.

### 3.3e R4: where the costume code is chosen (open)
- **The config belongs to the mission-select scene.**
  - `CSceneMisSelect` (vtable `0x1404E31D8`, constructor `0x14023F2B0`)
    keeps the edited loadout at `+0x118`.
  - Its slot 15 (`0x14023A200`) copies it into `+0x1C4` (`0x1402178B0`) and
    loads the character through `0x1401DF320`. The costume code is
    therefore `scene+0x14C` while it is being edited, `scene+0x1F8` once
    applied.
  - The scene embeds `CCustomizeData` objects (vtable `0x1404C8790`, at
    `+0x368` and in the child at `+0x70`).
- **Not found statically.**
  - No direct byte write to `+0x14C` / `+0x34` exists in the scene or UI
    ranges, so the menu writes through another base.
  - The `bt` / `bts` unlock checks found belong to `CUIDCustomWeapon`
    (`0x14028AF70`), not to costumes.
  - The exe has no costume strings.
- **Fastest next step (runtime).** With the game running, a write watch on
  `CSceneMisSelect+0x14C` (debugger or memory scanner: "find what writes to
  this address") while changing the costume gives the menu code and its limit
  in one step. Statically, the remaining candidates are the `CCustomizeData`
  methods `0x140084E00`..`0x140087BE9` and the `CUIDMisSelect` vtable
  `0x1404E9E50`.

- **UI resources by id** (`0x1402C07F0(id, language)`, emulated for ids
  0..3999). Ids map to resource records:
  - `900` -> `id\id900\id900<lang>.pac`, a language group at
    `0x1405BD450` indexed by the language code;
  - `901` -> `id900USEUJC.pac`;
  - `919`, `920` -> `id919.pac`, `id920.pac`;
  - `960` -> `id960ex.pac`;
  - `990`..`994` -> `id990`..`id994.pac`;
  - `529`..`558` -> `id\id500\id5xx.pac`.

  `CSceneMisSelect` (`0x14023A31E`) requests `920`, so **`id920.pac` is the
  mission-select screen**. `id900.pac` is the localized menu pack (text and
  images per language: J, F, G, I, S, C, Z / USEUJC). Costume names and
  icons, if the menu shows them, would sit in these packs; new costumes then
  also need new UI entries there (to check on the files: `id900*.pac`,
  `id920.pac`).
- **`id900.pac` checked on the data (English).**
  - Layout: 20 slots, pairs of a texture bank and a flat UI MOD. Slots 10
    and 13 are small PACs with one MOT each (menu animation).
  - Texture slots are **PTX** (the game's texture bundle, `.ptx`,
    `CPtxManager`), not a separate format. Layout: `u32 count`, `u32` span in
    2 KB sectors per texture, header padded to `0x800`; per texture at its
    sector a `0x70` descriptor followed by a standard DDS (`DDS ` + 124-byte
    header, DXT5 = BC3, base level only, no mip chain), square 256 / 512.
    Pixel data therefore starts at descriptor `+0xF0`. Correction: an
    earlier version of this note read it as a `0x800` header with data at
    `+0x800`, which shifts the decode; see
    `dmc3-texture-formats-map-2026-10-01.md`.
  - Content: the title screen.
    - DMC3 Special Edition logo, both the 2005 and the 2018 copyright
      versions;
    - VERGIL / DANTE character select with their full-body art;
    - NEW GAME / LOAD GAME / OPTIONS / TUTORIAL, ON / OFF;
    - EASY / NORMAL / AUTOMATIC;
    - INTERNATIONAL / GOLD / YELLOW.
  - It holds **no costume menu**. The costume choice is on the
    mission-select screen, whose pack is `id920.pac`.

- **Not the costume: the language byte.** `[0x1405EA130 + 0x66C]` (getter
  `0x140036E90`) is the language: 0 Japanese, 1 English, 2 French, 3 German,
  4 Italian, 5 Spanish, 6 Chinese, 7 SChinese (setter `0x140036360`). The
  `cmp al, 6` / `cmp al, 7` tests in the `CUIDMisSelect` code
  (`0x140298B3F`, `0x14029B5A2`) are Chinese-text layout cases, not costume
  codes.

### 3.3b `.rdata` / `.data`: what is worth moving out
- **`.rdata`** (2.1 MB, read-only): floats and constants used by code, jump
  tables, vtables and RTTI, strings, and data tables (event / command tables
  such as `0x1405A3300`, effect tables, shader blobs).
- **`.data`** (8.5 MB virtual, 0.5 MB in the file): mutable globals and
  writable tables (the resource and costume tables above).
- **Worth moving out:** only content tables (resources, costumes, command /
  event tables, character parameters, shaders), as data files read through
  hooks.
- **Not worth moving out:** constants and structure that the code itself
  depends on (jump tables, vtables, immediate floats). Moving them gains
  nothing and breaks code.

## 6. Long-term goal: a full C++20 engine
- **Target:** the whole of `dmc3.exe` reimplemented as C++20 in rengine, then a
  standalone runtime that boots the game data without the exe (as OpenMW /
  OpenRCT2 do), with a modern renderer, physics and fully data-driven
  content.
- **Scale.** `.pdata` lists about 12,200 functions (146,820 bytes / 12).
  Reimplement by subsystem (formats, motion, effects, cloth, collision, AI,
  stage, then gameplay), each with emulator-checked tests, as done for P / G
  records and the enemy event table.
- **Other games.**
  - DMC1 and DMC2 share the PS2-era Capcom lineage of formats.
  - DMC4 (MT Framework) and DMC5 (RE Engine) are different families.
  - The engine therefore needs engine-neutral internal assets (skeleton,
    mesh, material, animation, cloth, effects) with one importer per game.
    That is what makes "every costume from every game" possible.
- **Distribution rule:** the engine and tools are shareable; game data always
  comes from the player's own copies.

### 3.4 Costume list file
A file with the costume count and, per costume: PAC, motion banks, coat /
cloth manifest, SHW policy, menu name. Depends on R4.

### 3.6 Old tables and code in the exe: keep them, do not delete
Freeing space inside `dmc3.exe` is not needed and is the riskier path:
- **New code** lives in the loader DLL (no size limit). If a patch must sit in
  the exe, a new PE section is appended; the PE format allows it and nothing
  existing moves.
- **Old tables and code** are tiny (the resource name strings are a few KB of
  the 2.1 MB `.rdata`; `.text` is 3.4 MB) and stay as the vanilla fallback
  when an external file is absent.
- **Hidden references:** a table can be read from places not mapped yet
  (e.g. the grouping list at `0x1405B0970` points into the resource table).
  Removing it would break those paths silently; redirecting a pointer or
  hooking the reader does not.
- **Rule:** hooks redirect, the original stays in place untouched. A function
  is replaced by a jump at its entry, never by rewriting its body in place.

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
| R5 | NBZ lookup of member names not in the base volumes (done, 3.3d: they load) | adding new PACs by name |
| R6 | Cloth / attach setup per class (`0x1402CA1D0`, `0x1402CA2F0`, `+0xA0D0..+0xA300`) as one manifest schema | physics manifest |
| R7 | Stage AO / light bake prototype for st000 / st002 in Native Reader | baked stage shadows |

## 5. Lab limits
Native Reader renders on the CPU (own software rasterizer, no GPU API), which
suits bakes, SHW generation and cloth, not shader prototyping. Shader work
needs a GPU backend (a desktop rengine D3D11 / Vulkan viewer).
