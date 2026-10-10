# StageCfg ITM, STE, EST and SEF: what the files hold

2026-10-10. Same archive as the POS/EVE/CAM note (`st002cfg.pac`): slot 6 ITM
(16 bytes), slot 7 STE (176), slot 9 EST (5 920), slot 2 a PAC of one member,
SEF (560). Readers: `formats/stage_cfg.hpp`; structure views in
`integration/resource_structure.cpp`.

## ITM — static item placements

The layout the format catalog already gives (`ITM\0`, u16 version, u16 count,
then 20-byte records: u32 item id, x, y, z, rotation about Y; size padded to a
multiple of 16). This stage places none: count 0, 16 bytes. The reader checks
the padding and that every value is a number.

## STE — scene/effect transforms

Header as POS (`STE\0`, version 1, count 4); then exactly `count` records of 40
bytes:

| offset | type | seen |
|---|---|---|
| +0x00 | u16 | 2 in all four |
| +0x02 | u16 | 10, 12, 181, 181 |
| +0x04 | 3 × f32 | position |
| +0x10 | 3 × f32 | rotation in degrees (0 or 180 about Y) |
| +0x1C | 3 × f32 | scale (1, 1, 3, 5 — uniform) |

What the u16 at +0x02 names (an effect id?) is open.

## EST — a program per row and difficulty

| offset | type | value |
|---|---|---|
| 0x00 | tag | `EST\0` |
| 0x04 | u16 version | 1 |
| 0x06 | u16 count | 6 |
| 0x08 | u32 | 0x16A0: offset of the table |
| 0x0C | 4 bytes | zero |

The table holds `count` rows of five u32 offsets (30 words, then zero padding
to the end). Read as rows of five, the sample is

```
row 0:   32   32   32   32   32
row 1:   96   96   96   96   96
row 2:  160  160  160  160  160
row 3:  320  800 1280 1792 1792
row 4: 2304 3168 4032 4832 4832
row 5: 5696 5696 5696 5696 5696
```

— five columns that match the five difficulty modes (Easy, Normal, Hard, Very
Hard, Dante Must Die): two rows differ by mode, and in both the last two modes
share a program. This is an inference from the shape, not from a consumer.

Each offset starts a program of commands. A command is a u32 whose low byte is
the command and whose next byte is how many i32 arguments follow (the upper
half is zero); a zero word ends the program. Read that way, all 12 distinct
programs end on their zero word and **every non-zero byte between the header
and the table belongs to one of them**. Commands seen: 2 (2 args), 3 (3 args),
5 (1), 8 (2), 9 (2 or 3), 12 (1).

- 3 is always three integers in the stage's coordinate range (2900, 20, 2800):
  a point. 5 follows with one value such as −90 or 90: an angle.
- 2 precedes each point; its first argument takes 0, 8, 12, 16, 31 — perhaps
  an enemy kind (st002 spawns the em000 family); the second 0..5.
- 12 opens the long programs with 3 / 5 / 5 / 10 across the modes.

The rengine census correlated cfg slot 9 with the executable's
dependency/control scan (`0x1401AF000` → `0x1401A9BC0`, enemy-resource
demand); a spawn script per difficulty fits that. What each command does
stays open.

## SEF — section table only

Inside slot 2's PAC: `SEF\0`, u16 4, u16 1, then from +0x08 four pairs of
(u32 value, u32 offset) — (3, 0x28), (7, 0x54), (5, 0xFC), (8, 0x194) — and
the table ends where the first section starts (0x28 = 8 + 4 × 8), so the u16
at +0x04 is the section count. Section sizes 44, 168, 152, 156 bytes; the
values divide only the second evenly (7 × 24), so they are not plain record
counts. Section contents are open; the reader gives the table and sizes.

## Open

- ITM: nothing new here; a stage that places items would exercise the reader.
- STE: what +0x02 numbers.
- EST: the meaning of each command; whether the five columns are the modes.
- SEF: everything inside the sections. A second stage's cfg would settle most
  of this.
