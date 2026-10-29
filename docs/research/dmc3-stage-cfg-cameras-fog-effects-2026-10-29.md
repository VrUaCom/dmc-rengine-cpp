# StageCfg CAM, SEF and STE: cameras, fog and placed effects

2026-10-29. This continues `dmc3-stage-cfg-pos-eve-cam-2026-10-10.md` and
`dmc3-stage-cfg-itm-ste-est-sef-2026-10-10.md` with what the DMC Native Reader
worked out afterwards (its `stage_config` and `stage_markers` modules, versions
v86–v97), read from the st000, st001, st002 and st006 configuration archives.
Only offsets, counts and values are recorded here. No reader of these
members in `dmc3.exe` has been traced, so each layout below is `candidate`:
checked against every byte of those files, but not against their consumer.

Code: `formats/stage_cfg.hpp` gives `CamDocument::records`, `read_cam_shots`,
`zone_holds`, `cam_view_at`, `sef_fog` and `ste_effect_kind`. Tests are in
`tests/stage_cfg_tests.cpp`.

## CAM: one record per camera, in a row to the end of the file

st006cfg has five records, from 0x10 to exactly 0x1240. The first word is the
camera's type, and the type fixes the record's head:

| Type | Head | Contents |
| --- | --- | --- |
| 0, fixed | 0xB0 | target at +0x50, eye at +0x90, field of view in radians at +0xA0 (1.05) |
| 0x100, rail | 0x490 | up to 32 eye points at +0x80, 32 target points at +0x200, 32 fields of view at +0x380, point count at +0x480 |
| 0x400, zones only | 0x40 | no camera |

All three types share two fields:

- **Zone count.** The byte at +0x14, not a dword. st002 keeps `02 01 78`
  there, so a 32-bit read gives a huge count and drops its cameras.
- **Flags.** At +0x40. Flag 0x200 on a fixed camera means the target follows
  the player, and +0x50 is the offset from the feet (st006: 0, 105, 0).

**Zones.** The zones close the record, 0x50 bytes each:

- four (x, y, z, 1) floor corners;
- the height the zone reaches;
- 12 zero bytes.

**Picking the camera.** The camera for a player is the one whose zone holds
them. When several zones hold the player, the smallest one wins.

- *Inside a zone:* between the floor and floor + height, and on one side of
  every edge.
- *On a rail:* the player's nearest place on the target rail picks the same
  place on the eye rail and on the field-of-view row.

| Stage | Read |
| --- | --- |
| st000 | no record walk (the older shape scan reads it) |
| st001 | one zone record (0x400), one zone |
| st002 | five cameras: rail (2 zones, 6 points), rail (1, 3), fixed (4 zones), fixed (1), rail (2, 6) |
| st006 | five cameras: rail (2, 6), fixed (3), rail (2, 6), rail (2, 3), fixed (1) |

## SEF section 2: the stage fog

The section of value 2 is laid out like a cutscene's GFG fog:

- f32 distance, f32 distance;
- bytes R, G, B, A, where A = 0x80 is 1.0.

The scene is clear up to the nearer distance and fully fogged at the farther
one. st001 writes the farther distance first.

| Stage | Distances | Colour | Alpha |
| --- | --- | --- | --- |
| st000, st002 | no section 2 | — | — |
| st001 | 2100, 1680 (read as 1680..2100) | 154 145 133 | 0.63 |
| st006 | 1000, 2360 | 128 128 128 | 0.41 |

## STE: the effects a stage places

Each STE record has these fields:

- u16 dispatch kind: 0 P, 1 E, 2 G, 3 V;
- u16 effect id in the stage's own bank (`stNNN_effect.pac`);
- a position, a rotation in degrees and a scale.

The kind letters are the FXBANK kinds (`profiles/dmc3/effect_pack_contract.hpp`).

st006cfg's ten records are all kind 3, id 98, at scale 0.5. That is the
bank's one V: fire E 7, with glow E 154 above it. They are the wall torches
(y 510..580) and the braziers (y 150). st001cfg has no STE records, so its
effect bank only serves effects started by code.

## Not established

- The runtime readers of CAM, SEF and STE.
- The camera's blend between zones.
- The meaning of SEF sections other than 2.
- The 0x400 record's role (zones with no camera).
