# DMC3 HD (PC) shaders, read from dmc3.exe

> Ported 2026-10-29 from the DMC Native Reader (`NR-Luna-v73`, `docs/research/dmc3-hd-shaders-exe.md`). Paths below that start with `app/` or `tools/research/` are the Reader's. In Rengine: names and rules only, in our own words. The shader sources are Capcom's and are not in either repository. The Native Reader's own GLSL was written clean-room from behaviour specifications (its `docs/cleanroom/`).

Read 2026-10-24 from the HD Collection's `dmc3.exe` (6 356 432 bytes). The
executable and the shader sources are Capcom's and stay out of the
repository; this note records names and the rules the shaders follow, in
our own words.

## The build

- PDB `C:\dev\dmc\dmc3\build\x64\dmc3.pdb`; sources under
  `C:\dev\dmc\dmc3\code\dmc3\source\dmc3nb\` (`dmc3nb`: the port's own tree).
- MSVC 2015 or later (`MSVCP140.dll`), shaders built with the Microsoft HLSL
  compiler 6.3.9600.16384 (Windows 8.1 SDK).
- No Softimage, XSI, Maya or other content-tool string: the executable is
  the port, the PS2 production tools left nothing in it (see
  softimage-pipeline-hypothesis.md).
- 69 DXBC shaders (29 vertex, 38 pixel). 67 keep their debug block (SPDB)
  with the full HLSL source and `Shared.fxh`. One comment in the fog pass
  says it emulates "what the PS3 does": the PC port follows the PS3 HD
  version.

## Vertex shaders

| family | for | variants |
| --- | --- | --- |
| `dmc3_mod` | character models (MOD) | `_sp`, `_stx` |
| `dmc3_efm` | effect models (EFM) | `_sp`, `_stx`, `_va`, `_va_sp` |
| `dmc3_stg` | the stage (SCM) | `_fog` |
| `dmc3_shw` | shadow volumes (SHW) | |
| `dmc3_ptcl` | particles | `_fog` |
| `ocpl`, `vcpl`, `vtpl`, `vtocpl`, `vtvcpl` | 2D / flat geometry (o: object colour, v: vertex colour, t: texture) | `_fog` |
| `fx_*`, `video_frame_v` | full-screen passes | |

### Skinning (`dmc3_mod`, `dmc3_efm`)

- Bones are 4 rows (float4) each in one array of 176 rows: 44 bones a draw.
- A vertex names up to three bones (BLENDINDICES y, z, w: row offsets).
- Its weights are packed in one float (PSIZE): three 5-bit fields, the low
  one first, each read as field / 31. When the higher fields are zero the
  vertex follows its first bone alone ("fixes w=0 source data bug").
- The matrices are blended by the weights, then the position and normal
  are transformed by the blend.

### Model lighting

- Light intensity: the light-direction matrix times (normal, 1), clamped
  at 0: three directional lights and a constant term.
- Colour: the light-colour matrix times the intensities, / 128, times the
  part's base colour; alpha is the part's alpha.
- UVs are PS2 fixed point: (uv + scroll offset) / 4096, times a UV scale.
- The fog colour comes with the vertex; the part's base W is its fog
  amount.

### Stage lighting (`dmc3_stg`)

- Four point lights per vertex (positions as three float4 rows, x / y / z
  of the four lights, and a fourth row of falloff factors).
- Per light: d = light - vertex; intensity = max(dot(d, normal) *
  saturate(1 - |d|² * falloff), 0), with d not normalised.
- Light = the four light colours by their intensities + ambient; times the
  vertex colour / 255.
- Fog amount (the `_fog` variant): saturate(fc1.w + fc2.w / |clip w|), the
  PS2 fog curve.

### Others

- Shadow volumes: positions already in screen space, one solid colour.
- Particles: a 4-corner table of position rows, UVs and colours indexed
  by the vertex's W.

## Pixel shaders: the PS2 texture functions

| shader | rule |
| --- | --- |
| `tex_decal` | texel |
| `tex_col_mod2` | texel × vertex colour × 2 (MODULATE, 0x80 = 1) |
| `tex_col_hl` | texel × colour × 2 + colour alpha (HIGHLIGHT) |
| `tex_col_hl2` | rgb as HIGHLIGHT, alpha the texel's (HIGHLIGHT2) |
| `tex_col_recol2` | recolour: brightness = (r+g+b) × 2/3 picks between a Min and a Max colour; mix by its alpha; then MODULATE |
| `_a1` suffix | punch-through: alpha = 1 at 0.95 and above, else 0 |
| `tex_decal_afull` | alpha forced to 1 |
| `_fog` suffix | rgb = mix(fog colour, colour, fog amount) |
| `col` | vertex colour, alpha × 2 |
| `tex_ptcl_end` | alpha × 64, saturated |
| `fx_decal_clut` | 8-bit index through a palette texture |
| `video_frame` | the movie frame, BGRA |

Alpha test (`Shared.fxh`): a function number 1-7 = never, less, equal,
less-equal, greater, not-equal, greater-equal; anything else passes.

## Pixel shaders: screen effects

- `fx_bloom_glow`, `fx_bloom_glare`, `fx_glow_setup`: glow from four
  offset taps blended 0.5, 0.375, 0.25, × colour × 2.
- `fx_star_glare`: four taps along a direction with their own weights.
- `fx_dir_glow`: two blurred taps mixed by the colour's alpha × 2.
- `fx_dof`: depth through two limits gives the blur amount (cubed), mixing
  in a half-pixel blurred copy.
- `fx_col_corr`, `fx_col_corr2`: colour correction, a scale texture looked
  up by the green channel where an object-id mask allows it.
- `fx_grad_fog`, `fx_proc_fog_gen`: gradient fog, a 256-entry colour table
  looked up by depth (× 257).
- `fx_edge_det`, `fx_edge_det2`, `fx_edge_aa`, `fx_edge_aa2`: edge
  detection and edge smoothing (fixed for 1280 × 720).
- `dx_colour_copy`, `dx_z_copy`, `dx_colour_and_z_copy`: buffer copies.

## What it gives the Reader

- GPU skinning (PERFORMANCE.md step 3) with the game's own bone layout and
  weight packing.
- The game's model and stage lighting instead of our camera-light
  approximation, with LIG2 / the stage lights as the data.
- The PS2 texture functions and alpha tests per batch.
- The cutscenes' screen effects (ScrEfc clips) and the fog table as passes.

Our own shaders with this behaviour, written clean-room from specifications
(no source text): `docs/cleanroom/`, `app/src/main/cpp/shaders/dmc3/`.
