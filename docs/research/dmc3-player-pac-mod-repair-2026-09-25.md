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

## 5. A skirt as the coat model

The skirt of `pl011` was moved from the body into slot 12 as cloth (Native
Reader `tools/mod_fix/skirtcoat.py`). The output has SHA-256
`1640a0d61a883a73f0ec40ba1865fd9ad25c5b751de427498483d6d5fec69a8e`, 15 slots
and 1,168,000 bytes. Both MODs parse with no diagnostics, both SHW slots
parse with no diagnostics, and the viewer reports no non-canonical note.

These constraints come from the EXE:

- **One carrier:**
  - The coat root takes `joint[+0x1898]->world`, which is body joint 3 (the
    chest), every frame. The addresses are listed in
    `dmc3-player-coat-attachment-2026-09-23.md`.
  - No coat node can follow the arms or the pelvis.
  - Sleeves and the shirt therefore stay in the body.
  - A skirt hangs from the chest. Across the 40 motions of `pl011`, the
    waist point carried by joint 3 and the same point carried by joint 14
    differ by 2 to 5 units in idle and run, 8 to 12 in many attacks, and up
    to 25 (`slot_0003.pac/slot_0009.mot`, frame 23).
- **Capsules on block 0 only (`0x1402151E7`):**
  - The joint-3 capsule sits at z +10 in front of the hips, with r 15 and a
    40-unit extent.
  - It pushes the front of a short skirt forward.
  - Putting the front chains in a second `ClothNo` block avoids the push.
    That is the `pl001_02.clt` pattern.
- **Canonical MOD writer:**
  - Streams are grouped by kind across an object's meshes.
  - Each mesh has a generated-topology workspace of `align16(6 · (n − 2))`
    bytes filled with 0x12.
  - The file's last two bytes are zero.
  - Nine retail MODs (pl000/pl001 body and coat, em028, weapons) round-trip
    byte for byte.

## 6. Coat root joint patch

The coat joint is hard-coded in three places in CPlDante:

| Site | Instruction | What follows |
| --- | --- | --- |
| `0x1402120C4` (file `0x2114C4`) | `mov rdx,[rsi+0x1898]` | `call [coat vtbl+0x190](joint->world)` |
| `0x140218EFD` (file `0x2182FD`) | `mov rdx,[rdi+0x1898]` | `call [vtbl+0x198]` |
| `0x140218F67` (file `0x218367`) | `mov rdx,[rdi+0x1898]` | `call [vtbl+0x198]` |

The last two sites are in the CPlDante virtual `0x140218960`, vtable slot
`0x1404DFA38`.

Supporting facts:

- **Joint pointers:** player `+0x1880 + 8·j`, indexed by joint number all
  through the player code.
- **Model objects:** an array at `+0x7540` with a stride of `0x780`. The coat
  is element 0.
- **Coat object:** its vtable is `0x1404C9010`, set by the constructor at
  `0x140089270`. Its `+0x50` entry is `0x140089960`, which loads through
  `0x1402FDF00` and `0x140303AE0` into `0x1402F9570`.
- **MOD manager:** the coat object's manager sits at object `+0x80`.
  `0x1402F960E..0x1402F9616` copies MOD header `+0x13` into the manager at
  `+0xFA` (u16). The byte is therefore at player `+0x76BA`.
- **`0x1402FD040`:** reads only the world translation of the joint named by
  `+0xFA`.

Native Reader's `tools/mod_fix/coatjoint_patch.py` sends each site through a
16-byte subroutine that computes joint `3 + byte [player+0x76BA]`. The
subroutines sit in `int3` padding at `0x140346CF2` and `0x1403455D5`.

- **Retail coats:** pl000 and pl001 carry `+0x13 = 0`, so their behaviour is
  unchanged.
- **The pl011 skirt:** it carries 11, so it hangs from joint 14 (the pelvis).
  Its waist then stays on the hips in all 40 motions of `pl011`.
- **Output hashes:**
  - Patched executable: SHA-256
    `82bc2581b951f2d6f6ac8e1dd3f8f1fad34eb7bd7433a863d9ddaa70e080a9e1`.
  - Pelvis-rooted skirt PAC: SHA-256
    `3b588304437c39a6c9d4bf01f456d58ba04b4d6e52761f043e5392bd08eac947`.
