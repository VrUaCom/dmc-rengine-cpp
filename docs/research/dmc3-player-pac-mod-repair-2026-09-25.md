# Repairing a community player PAC (`pl011.pac`)

Date: 2026-09-25.

Executable: `dmc3.exe`, SHA-256
`e454272ed0fb0247fcbcf300e5d55d7a3e96d50b89b9ffaff81bb978dcbdd082`.

Sample: the community mod `pl011.pac`, SHA-256
`7d7ec53a7b2c9c64faef2c2b5e7fac76c40286cc0afb6be6ab10cec135800356`, compared
with Dante's `pl000.pac`, SHA-256
`b73b6e8ee2f2088a088f4215a5a13a50700fb4e4f0bef54dc0bcce11ae7b9c5d`. Neither
file is kept here.

## 1. What the mod changed

| Slot | pl000 | pl011 |
| --- | --- | --- |
| 0 PTX | 616,448 | 264,192: new textures, community-tool descriptors |
| 1 MOD body | 216,544 | 416,736: new mesh on Dante's 24-node skeleton (every joint identical) |
| 2-11 | — | byte-identical to pl000 (motion PACs, motion script, tables, collision, **SHW slot 8**) |
| 12 MOD | 35,696 (coat, 33 nodes) | 12,672 (8-node placeholder at ankle height) |
| 13 CLT | 768 | 352 (`pl011_01.clt`, bones 2-5) |
| 14 SHW | 12,560 | **missing** (the PAC has 14 slots) |

## 2. PTX descriptors

Every descriptor field except the dimensions, the DDS size and the `+0x20` /
`+0x68` constants is zero. The zeroed fields are the encoding word, the
constant 0xAAE4, row bytes, payload size, secondary dimensions and their
reciprocals, and the format. Textures 1 and 2 hold only the base mip level.
All of these fields follow from the DDS header (`texture_slot_framing.cpp`).
The canonical descriptors can therefore be regenerated exactly, and the mip
chains completed.

## 3. Slot 14 is not optional

`0x1401B82C0(pack, 0, character, slot)` returns null when `slot + 1 >
count` or when the slot offset is zero (`0x1401B8301` / `0x1401B830E`). The
player init at `0x140215206` (and the second path at `0x140222684`) fetches
slot 14 and hands the pointer, without a null check, to the shadow object's
method `[vtable+8]` = `0x14008BC60`. That method stores it through
`0x1403204B0` (`[obj+0x28] = r8`). `0x14031FC40` then reads `byte [SHW+0x10]`
right away. A PAC without slot 14 therefore dereferences null on any path
that builds the coat shadow. `0x1402151F4` skips that block when the costume
byte is 5 or 7.

## 4. Shadow hulls from the mod's own mesh

Slot 8, copied from Dante, shadows Dante's body on the girl. Hulls can be
built from any skinned MOD, following the SHW layout confirmed in
`dmc3-shw-shadow-projection-2026-09-23.md`:

- **Grouping:** one closed convex hull per joint. The vertices are rest-space
  positions grouped by their dominant skin joint. Small joints (6/10, 18/22,
  23) join their parent's hull.
- **Hull points:** extreme points along 18 directions, then their convex
  hull.
- **Triangles:** counter-clockwise, facing outward. Adjacency entry i is the
  neighbour across edge (vᵢ, vᵢ₊₁), which is the order found in both of
  Dante's files. Reserved words are zero, w = 1, and every block is aligned to
  16 bytes.

The rebuilt file has 17 body hulls (7 to 17 vertices each, T = 2V − 4) and 5
hulls for the slot 12 model. Both parse with no diagnostics, the same as
Dante's own files.

Tools: Native Reader `tools/mod_fix`. The repaired PAC has SHA-256
`de580ff960152640cf51f0b86e1c6e1f369a309c1419863db8cc6309ab435596`, 15 slots
and 1,178,816 bytes.
