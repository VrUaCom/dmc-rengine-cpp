# DMC3 character collision tables (attack index and shape records)

Date: 2026-09-24
Executable: `dmc3.exe`, SHA-256
`e454272ed0fb0247fcbcf300e5d55d7a3e96d50b89b9ffaff81bb978dcbdd082`.
Samples (analysed only, not committed): pl000.pac, pl011.pac, em000.pac,
em006.pac, em007.pac, em028.pac. The SHA-256 values are in
`dmc3-enemy-motion-script-2026-09-24.md`.

## 1. Where they come from — EXE_CONFIRMED

`0x14005C260(handle, index, shapes, owner_kind, …)` sets up a collision
handle (class derived from `ICollisionHandle`, vtable `0x1404C65A0`):
- `+0x108` = index table;
- `+0x110` = shape table;
- `+4` = owner kind (2 for enemies).

| Archive | Index slot | Shape slot | Call |
| --- | --- | --- | --- |
| pl000 / pl011 | 6 (1008 B, 252 ids) | 7 (20080 B, 251 records) | `0x1401EEFD1` |
| em028 | 11 (16 B) | 12 (160 B) | `0x140130FA8` |
| em000 family | 39 (96 B, 24 ids) | 40 (1840 B, 23 records) | `0x14009823F` |

(em006 and em007 also use slots 39/40, just before their effect bank in slot
41.)

## 2. Attack index — EXE_CONFIRMED

`0x14005C740(handle, id, param, boneMatrices)`, `0x14005C640` for the
variant that takes a bone-pointer list. Each index entry is 4 bytes:

```text
+0 mask    target layers: bits 1/2/4 pick the collision mask by owner kind
           (0x1402CCD60: 0xF00 / 0xCC0 / 0xBC0 / 0x7C0, |0x3F, |0x3D000)
+1 bone    bone index; its matrix is boneMatrices + 64·bone
+2 u16     shape record index
```

The spawned shape takes the `+0x88[16]` slot, the id (`+0x10`) and the param
(`+0x40`). About 235 call sites pass constant ids from AI and player move
code. The motion script only ends shapes: opcode 32
`[20, object, mask, ?]` (`0x140059820`) calls `0x14005C3F0` on every active
shape whose index `mask & byte`. Opcode 6 `[06, x]` (`0x140059650`) calls
`0x14005C840` and `0x14005C870`.

## 3. Shape records (80 bytes) — EXE_CONFIRMED layout

Byte 0 is the type, dispatched 0–6 (`0x1402CC579`), with zero padding up to
+0x10:

| Type | Fields | Code |
| --- | --- | --- |
| 2 sphere | centre +0x10 (vec4, w = 1), radius +0x20 | `0x1402CC3F0` (centre) |
| 3 box | centre +0x10; rotation in degrees +0x1C / +0x20 / +0x24 (X, Y, Z); half size +0x28 / +0x2C / +0x30 | `0x1402CC115`: 8 corners (±1) of the cube `0x1405CEC60`, scaled by the half size |
| 4 capsule | a +0x10, b +0x20 (vec4), radius +0x30 | `0x1402CC300` |

In the samples:
- pl000 has 243 spheres, 7 boxes and 1 capsule;
- the enemies mostly have spheres.

The sphere radius (+0x20) and the capsule radius (+0x30) are confirmed by
the data (40/50 and 120/95 on body-sized models). They are not yet confirmed
by a read in the hit test.

Per-type setup dispatches on byte 0 through the table `0x1402CC5F0`, with 7
entries:

| Type | Function |
| --- | --- |
| 0 | `0x1402CCA30` |
| 1 | `0x1402CCA70` |
| 2 | `0x1402CCB50` |
| 3 | `0x1402CC610` |
| 4 | `0x1402CC890` |
| 5 | `0x1402CCCB0` (three points) |
| 6 | `0x1402CC9A0` (centre, +0x20 → +0x140, +0x24 × scale → +0x144) |

Types 0, 1, 5 and 6 do not occur in the samples.

### Debug meshes `obj\debug\at000–at003.mod`

The system resource list `0x1405B0860` (loader `0x1402EA333`) names four
debug meshes and their texture `obj\debug\at.ptx`. The list also holds
`font\i001_90.tm2`, `basic.ptz`, `basic.ptx` and `scr\ss900_t.pac`. The
build keeps a second, short-name list at `0x140553880`.

The user's samples:

| File | SHA-256 prefix | Mesh | Shape type |
| --- | --- | --- | --- |
| `at000` | `afbaefa0c6414490` | UV sphere, radius 1 (83 vertices) | sphere (2), scaled by the radius |
| `at001` | `9fea695a508cf64e` | cube ±1 (24 vertices), the same corners as `0x1405CEC60` | box (3), scaled by the half size |
| `at002` | `7f72c15557c980aa` | capsule, radius 1, segment y ±0.5, caps to ±1.5 | capsule (4) |
| `at003` | `9cb1ac281ae79928` | octagonal prism, y ±1 | probably type 6 (radius +0x20, height +0x24) |

Textures from the same list (the user's samples, 12288 bytes each, one DDS
texture):

- **`at.ptx`** (SHA-256 `4c18c6cd4816ca1a608a12efd9eb1bd1467408152f7c31f8079c3c4aff3ce5ce`):
  128×64, flat grey 0x7B–0x83. That is the PS2 neutral 0x80, a ×1 modulation.
  The debug meshes take their colour from material or vertex colour, not
  from this texture.
- **`basic.ptx`** (SHA-256 `a4479f843e3859465102315ebf9be6783a47a7802a0c96b18ada0d810b897ff1`):
  the debug font, 128×64 with 16×8 cells of 8×8 pixels.
  - Row 0: hex digits `0–F`.
  - Rows 1–6: ASCII `0x20–0x7E`, with `¥` in place of `\`.
  - The PS2 button glyphs ×, □, △ and ○ stand in for the control codes.

No retail code has been found yet that draws the meshes. The mapping in the last
column comes from the shapes and the record layouts; it is not a read.

## 4. Player parameter blocks

CPlDante init (`0x140212C5C`) stores:
- slot 9 at `player+0x3DE8`;
- slot 10 at `+0x3DF0`;
- slot 11 at `+0x3DF8`.

Slots 9 and 11 are float parameter blocks read at fixed offsets. Examples:
- slot 9 +0x12C (`0x1401DFE96`);
- slot 11 +0x2F4 / +0x2F8 / +0x2FC, limits compared with counters at
  `0x1401CA0FD`.

No reader of slot 10 (u16 pairs) has been found yet.

## 5. Viewer rule

Native Reader:
- identifies shape tables by content;
- identifies index tables only as the slot before a shape table whose shapes
  they all name (`.colidx` routing);
- draws the shapes in bone space from the front and the side;
- lists every attack id.

Float parameter blocks are shown in the raw binary view as a value grid by
offset.
