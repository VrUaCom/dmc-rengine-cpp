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
| 3 box | centre +0x10; rotation in degrees +0x1C / +0x20 / +0x24 (X, Y, Z); size +0x28 / +0x2C / +0x30 | `0x1402CC115`: 8 corners of the unit cube `0x1405CEC60` |
| 4 capsule | a +0x10, b +0x20 (vec4), radius +0x30 | `0x1402CC300` |

In the samples:
- pl000 has 243 spheres, 7 boxes and 1 capsule;
- the enemies mostly have spheres.

The sphere radius (+0x20) and the capsule radius (+0x30) are confirmed by
the data (40/50 and 120/95 on body-sized models). They are not yet confirmed
by a read in the hit test.

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
