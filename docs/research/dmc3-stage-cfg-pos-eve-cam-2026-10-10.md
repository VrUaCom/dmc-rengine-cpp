# StageCfg POS, EVE and CAM: what the files hold

2026-10-10. One stage configuration archive supplied (`st002cfg.pac`): slot 3
CAM (4 672 bytes), slot 4 EVE (784), slot 5 POS (400). The runtime compares
none of these tags (content-tag census), so these are layouts of the files,
not of their consumer. Readers: `formats/stage_cfg.hpp`; structure views in
`integration/resource_structure.cpp`; registry maturity `structural`.

## POS — placements

| offset | type | value seen |
|---|---|---|
| 0x00 | tag | `POS\0` |
| 0x04 | u16 version | 2 |
| 0x06 | u16 count | 3 |
| 0x08 | 8 bytes | zero |
| 0x10 + 0x30·i | record | |

Record (48 bytes): +0x00 u32 (0 in all three), +0x04 position (3 × f32),
+0x10 heading in degrees (180, 0, 0), +0x14..0x2F zero. The file has room for
(size − 16) / 48 = 8 records; the five past the count are zero.

## EVE — event volumes

Header as POS (`EVE\0`, version 0x100, count 6); then exactly `count` records
of 128 bytes, so the size is 16 + 128·count.

| record offset | type | seen |
|---|---|---|
| +0x00 | u32 | 0, 0, 0, 1, 0, 3 |
| +0x04 | u32 kind | 3, 3, 0, 0, 4, 0 |
| +0x08 | u32 argument | 10001, 10002, 0, 0, (u16 0, 1000), 0 |
| +0x0C..0x1F | | zero |
| +0x20 | 4 × (x, y, z, 1.0) | a floor quadrilateral; all four corners share y |
| +0x60 | f32 extent | 500, 400, 800, 1000, 250, 2200 |
| +0x64..0x7F | | zero |

The rengine purpose pass already separates EVE (spatial event volumes) from
`EventTblNN.bin` (mission logic). The meaning of +0x00/+0x04/+0x08 is open.

## CAM — stage camera

Header as above (`CAM\0`, version 1, count 5); records vary in length and
their layout is open. Found by shape, with no assumption about boundaries:

- six point paths (runs of x, y, z), in three pairs of equal length
  (6/6, 3/3, 6/6 points) — read as camera position and look-at paths;
- after each pair, a run of one coefficient per point (0.9599 × 6, × 3, × 6);
- ten areas in EVE's layout: four (x, y, z, 1) corners on one level followed
  by an extent (2000 or 2377);
- 180 non-zero bytes no shape accounts for: counts and flags of the records
  (e.g. u32 6 / 3 / 4 before area groups, 124.0, 91.0, 180.0 and 99.0 fields).

## Open

- POS: which actor each record places (entry points by the stage's numbering?).
- EVE: the three u32 fields.
- CAM: the record header and its counts; whether every record is a pair of
  paths; what the area extent bounds. A second stage's cfg would settle most.
