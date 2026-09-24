# DMC3 stage PAC layout and the `# GAME` set text

Date: 2026-09-24.

Executable: `dmc3.exe`, SHA-256
`e454272ed0fb0247fcbcf300e5d55d7a3e96d50b89b9ffaff81bb978dcbdd082`.

Samples:

| File | SHA-256 |
| --- | --- |
| `st000.pac` | `a155af1c18be616c8d203bb67b1751f6fca958d07ed29f65c3782eaa9638c748` |
| `st001.pac` | `43034a2e0596f744ea2d50fd785b2783d319fe5f0c7e15061e7c68b2786a57d9` |

No game file is kept in this repository. Only the structure and the values
quoted below are recorded.

## 1. Top-level slots

Both samples are a `PAC` with seven or eight slots.

| Slot | st000 | st001 | Content |
| --- | --- | --- | --- |
| 0 | 48 B | 48 B | File list text: `st00N.ptx`, `st00N.scm`, `st00N.sch` |
| 1 | PTX | PTX | Stage texture bank (21 textures of 256² / 512² in st000) |
| 2 | SCM | SCM | Stage geometry, including the sky (st000: 22,195 vertices, 11,118 triangles) |
| 3 | `HITS` | `HITS` | Collision (not read here) |
| 4 | text | text | `# GAME` set layout (section 2) |
| 5 | PNST | PNST | Stage objects: EFM / SCM models, with MOT PACs after the breakable ones |
| 6 | `HITS` | `HITS` | Second collision block |
| 7 | — | PAC | One MOT (st001) |

The st000 object PNST holds EFM #0, then SCMs, with a one-MOT PAC after each
intact/broken pair (#1 SCM, #2 SCM, #3 PAC, #4 SCM, and so on). The st001
object PNST holds two SCMs.

## 2. `# GAME` text

The text is line based. `;` starts a comment, `$` ends the text, and
`# GAME_END` closes the layout. Each `# SET <n> <kind>` opens a record. The
kinds seen are `CONFIG`, `STAY` (a static object) and `BREAK` (a breakable
object: `bmodel`, `beff`, `vital`, `hit sphere|box`, `item RED_ORB …`,
`material`).

The record parser is near `0x140247720`. It matches the keys against the
strings at `0x1404E3DE4` (`model`), `0x1404E3E2C` (`pos`) and `0x1404E3E34`
(`scale`) and stores them into the record:

| Key | Record | Read |
| --- | --- | --- |
| `model` | `+0x00` byte | `0x140322A60` (integer) |
| (next 3-char key) | `+0x01` byte | integer |
| `pos` | `+0x10/+0x14/+0x18` | three `0x140322A10` floats |
| `rot` | `+0x20/+0x24/+0x28` | degrees ÷ 180 × π, wrapped by `0x14032E8C0` |
| `scale` | `+0x30…` | floats |

The CONFIG record carries `cam_init x, y, z`. The string is at `0x1404E3DB8`,
and the reference is at `0x14024635A`. Its values are `600,200,600` in st000
and `2500, 100, 2800` in st001. st001 also has `shadow maru`, and its STAY
records have `uv` (a texture scroll) and `ot far 30`.

## 3. `model` index (data-confirmed)

`model k` names the k-th model entry of the slot-5 PNST: EFM, SCM or MOD in
order, with the MOT PACs skipped. This rests on the data:

- st001 has two model entries, and its STAYs use `model 0` and `model 1`.
- In st000, the 15 SETs use `model` values 0-14 and `bmodel` values 2-14. The
  PNST holds 16 model entries: EFM #0 and 15 SCMs. Each intact model is
  followed by its broken `bmodel` and then by the MOT PAC of the break.
- When the intact models are drawn at their `pos` in the stage SCM's
  coordinates, the bottles, guitars, drums and speakers of the Devil May Cry
  office sit on their shelves and on the floor.

The consumer of `+0x00` that turns the index into a model has not been read in
the EXE. The rotation order is also still open, because every sample `rot` is
0.

## 4. Viewer use

Native Reader `stage_room` uses these layouts for the "room" backdrop. The
room holds the stage SCM and every SET's intact `model` at its `pos`, `rot`
and `scale`. The first floor spot is the one nearest the midpoint between
`cam_init` and the placed objects, which lands inside the office in st000. The
stage textures are bilinear-filtered. Textures with soft alpha, such as the
light shafts, are blended after the opaque surfaces.
