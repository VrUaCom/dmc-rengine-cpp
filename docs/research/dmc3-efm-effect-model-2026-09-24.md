# DMC3 EFM: effect model on the MOD document layout

Date: 2026-09-24
Executable: `dmc3.exe`, SHA-256
`e454272ed0fb0247fcbcf300e5d55d7a3e96d50b89b9ffaff81bb978dcbdd082`.

Builds on:
- `dmc3-runtime-type-evidence-split-2026-08-31.md` (EFM post-load handler
  `0x1402F7A90`).
- `dmc3-mod-canonical-exe-unknown-field-closure-2026-09-08.md` (EFM mesh
  `+0x38` = COLOR0).

## Samples (analysed only, not committed)

The user extracted these files with a separate tool. That tool numbers the PAC
slots one lower than the EXE does. For example, it names the em000 EFM
`slot_022`, but `0x1400AD757` reads it from slot 23 (`8 + 23·4 = 0x64`). The
table below uses the EXE numbering.

| File | Slot (EXE numbering) | Bytes | SHA-256 |
| --- | --- | --- | --- |
| em000 EFM | em000.pac 23 | 11 584 | `fdc77673eee672e0d812ec1dd5478435a688004ef572a495723bbbc8c4b409ea` |
| em028 effect 142 | em028.pac 9 / 0 / 142 | 6 624 | `339e808ff2a9ff422b8d64fa8fea465b367454a7db8d9ed1f7e86689a22261e7` |
| em028 effect 156 | em028.pac 9 / 0 / 156 | 2 400 | `7ba9cad98e2f00c360eff2583270d2fc27eb002b1c02fb02eb1c7f3334d8f476` |
| em028 effect 158 | em028.pac 9 / 0 / 158 | 3 984 | `079919ab58536dafb04751425339daf86b8ddeeeccc54d8cdeb3ffedcc9c0d49` |
| em028 effect 162 | em028.pac 9 / 0 / 162 | 6 624 | `ce78fe315426b367d17fc394a541e61e2fff1deb90ddc157cb8f3746d0e3e21b` |

## 1. Layout

The post-load handler `0x1402F7A90` relocates EFM the same way the MOD handler
`0x1402FE3B0` relocates MOD:

- header `+0x20` (transform table);
- objects of 0x40 bytes from `+0x40`, with their count at `+0x10` and their
  mesh table at `+0x08`;
- meshes of 0x50 bytes with stream pointers at `+0x10`, `+0x18`, `+0x20`,
  `+0x28`, `+0x30` and `+0x38`. These are relative to the file base.
- mesh `+0x40`, which is relative to the mesh record itself (the generated
  topology workspace, compared with `0x1212`).

Unlike MOD, EFM relocates `+0x38` too. That stream is COLOR0.

Every sample parses with the canonical MOD parser once the magic is read as
MOD:

- version 1.01;
- all mesh counts, texture indices and stream offsets agree with the file
  size;
- the only diagnostic is `inner-38-nonzero`, which is the COLOR0 pointer.

In every mesh checked, COLOR0 is 4 bytes per vertex (RGBA8, `0x80` = 1.0), and
its last byte ends exactly where the transform table starts:
`0x13F0 + 144·4 = 0x1630` in effect 142, and `0x2350 + 205·4 = 0x2684`
(aligned to `0x2690`) in the em000 EFM.

| Sample | Objects | Nodes | Header `+0x18` (scrolls) | Object flags `+0x10` |
| --- | --- | --- | --- | --- |
| em000 EFM | 2 | 5 | 1 | `0x01100002` (scroll 0, texture 2), `0x00100002` |
| em028 effects | 1 | 1 | 0 | `0x00020013` / `0x00120013` |

## 2. Where EFM is used

**CEm005Shl01** (init `0x1400AD620`, vtable `0x1404CB3D8`, entry 29, RTTI
`.?AVCEm005Shl01@@`) sets up a projectile from em000.pac slots:

| Slot | Content | Use |
| --- | --- | --- |
| 23 | EFM | Loaded into the model at `this+0x260` through the ordinary model loader (vtbl `+0x50`, `0x1400AD77A`). |
| 32 | PTX | Textures of that model. |
| 37 | PAC | Motion bank (vtbl `+0x150`). |
| 22 | `;em005_02.clt` | Tail chain on bones 2-4: axis Z, no gravity, stiffness 0.06, LimitLength 0 (`0x1402CA1D0`). |
| 24 | `.tsc` | A CDrawUV (`this+0xAD0`) with one scroll: type 3, texture 2, down, 450/90 frames, MinimumUV 0.004 / 0.003. It drives object 0. |

**em028.** The EFMs in slot 9 belong to the effect bank that `0x1402C04C0`
loads (bat and lightning effects). They are single-node, single-object models
with vertex colours.

## 3. Blend state

The model's object builders (`0x1403029E0`, `0x140302F10`, `0x1403058F0` and
`0x140305B90`) are reached from the model manager (`0x1402FDED0..0x1402FDF70`).
They call `0x140302640` with the source object flags. For a low nibble
`mode = flags & 0xF` other than 0, `0x1402F17C0(mode)` loads a PS2 GS ALPHA
value from `.rdata 0x1405D0550 + mode·8`. The GS computes
`(A − B)·C >> 7 + D`, where A, B and D select Cs (0), Cd (1) or 0 (2), and C
selects As (0):

| Mode | ALPHA | Result |
| --- | --- | --- |
| 1, 4 | `0x44` | `(Cs − Cd)·As + Cd`: normal alpha blending |
| 2 | `0x48` | `Cs·As + Cd`: additive |
| 3 | `0x42` | `Cd − Cs·As`: subtractive |

Mode 4 also selects the depth state `0x50007`, and flag `0x100000` selects
`0x5010D` over `0x5000D`.

In the samples, most MOD objects use mode 1. The em000 EFM uses mode 2 (a
glowing shell). The em028 EFMs use mode 3 (darkening effects). One em000 MOD
object also uses mode 2.

**COLOR0 modulation.** The PS2 texture function multiplies the texel by the
vertex colour and divides by `0x80` (`Ct·Cf >> 7`). This applies to both
colour and alpha.

## 4. Viewer rule

- Identify EFM by its `EFM ` magic.
- Project it through the MOD path: copy the bytes and replace the magic with
  `MOD `; all offsets stay the same.
- Apply COLOR0 as `texel·colour / 0x80`.
- Apply the object's GS ALPHA mode. Additive and subtractive pixels do not
  write depth.

## 5. Open

- How the PC port's D3D11 path translates the depth states `0x50007`,
  `0x5000D` and `0x5010D`.
- The rest of CEm005: the owner class that spawns Shl01, and the em000.pac
  slots 33 onward.
