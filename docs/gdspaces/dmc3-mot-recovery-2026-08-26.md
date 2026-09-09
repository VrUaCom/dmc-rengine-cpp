# MOT — the motion payload, read structurally — 2026-08-26

**Image:** `e454272ed0fb0247fcbcf300e5d55d7a3e96d50b89b9ffaff81bb978dcbdd082`
**Payload:** `st001.pac` slot 7 → inner PAC slot 0, 63,440 bytes (`0xF7D0`)

This supersedes §4 and §5 of
`dmc3-animation-semantics-blocked-2026-08-26.md` for `MOT` only. What was
missing there was not a second file. It was reading the one file far enough to
find the repetition **inside** it.

## 1. Why one sample turned out to be enough

The earlier note said a single `.mot` could not distinguish a stride from a
coincidence. That was true of the header's `u16` table, and it stays true of
that table below. It was wrong about the body: the payload contains **69
tracks**, so every claim about a track is a claim tested 69 times by the same
file.

That is the difference between one sample and one file. The file is a corpus of
tracks.

## 2. Layout

```
+0x00  u32    data offset                 = 0x50
+0x04  char4  "MOT\0"                     <- inert; compared nowhere in .text
+0x08  u32    (unread)
+0x0C  f32    duration                    = 650.0
+0x10  u32    (unread)
+0x14  f32    duration mirror             = 650.0
+0x18  u16[28] unresolved table           (see §4)
+0x50  u32    track count                 = 69
+0x54  track[69]
       u32    zero terminator             @ 0xF7CC, file ends 0xF7D0
```

Track:

```
+0x00  u16    size = 32 + 8 * keys
+0x02  u16    key count
+0x04  u32    kind                        = 3 in all 69
+0x08  24 B   floats
+0x20  key[keys]
```

Key, 8 bytes:

```
+0x00  s16    time stamp   (strictly increasing within a track)
+0x02  s16 x3 value
```

## 3. What holds, and how many times

Every one of these is a count over the 69 tracks of the real payload, not a
reading of one header:

| Claim | Holds |
|---|---|
| `size == 32 + 8 * key_count` | **69 / 69** |
| time stamps strictly increase | **69 / 69** |
| `kind == 3` | **69 / 69** |
| `last_stamp - first_stamp == 650` | **69 / 69** |
| first stamp `== -32768` | 69 / 69 |

Total keys: **7,643**. The chain of declared sizes, walked from `+0x54`, lands
exactly on `0xF7CC`, where a zero dword closes it and the file ends four bytes
later. It does not land near there. It lands on it.

The last row of the table is the one that matters most. The header states
`650.0f` twice, at `+0x0C` and `+0x14`. Every track's stamp span, computed from
bytes the header does not touch, is `650`. Two independent regions of the file
agreeing on one number is what raises this from a plausible reading to a
structure — and it is what `MotDocument::duration_matches_stamps` records.

## 4. What is still unresolved

The 28 `u16` values at `+0x18`:

```
0, 24, 24, 0, 448, 56, 56, 56, 56, 56, 56, 56, 56, 56, 56, 56,
56, 56, 56, 56, 56, 56, 56, 56, 56, 56, 56, 0
```

They are not offsets — the sequence is not monotonic. Their sum, 1,784, is not
the track count, the key count, or the payload size, in bytes or divided by the
key stride. One file cannot separate a stride from a coincidence here, and the
earlier note's refusal to guess stands. The parser reads past this table and
claims nothing about it.

Also unclaimed: the 24 bytes of floats in each track header, and the three
`s16` following each stamp. They are read as extents, not as meaning. Naming
them a quaternion or a translation would be invention; the file does not say,
and no call site in the image reads a motion by anything but its name (§1 of
the blocked note, which stands).

## 5. Recognition without a magic

`MOT` at `+4` is compared nowhere in `.text`. So the parser does not identify a
motion by its tag. It identifies one by its arithmetic closing: the size
identity, the terminator landing exactly at the end, and the stamps increasing.
A file that fails any of those is refused whole rather than read part-way.

That makes the classifier's `mot` verdict structural rather than observed — the
same standard already applied to `PAC`, `PNST`, `PTX`, `SCM`, `MOD`.

## 6. Where it lives

- `include/dmc_rengine/profiles/dmc3/mot_contract.hpp` — the constants, with
  `canonical_target_sha256` binding them to the image above.
- `include/dmc_rengine/formats/mot.hpp`, `src/formats/mot.cpp` — `MotParser`.
- `tests/mot_structural_tests.cpp` — the identities above as assertions.
- `src/gdspaces/classifier.cpp` — `mot`, ordered after PTX and before the
  recovered 3-byte tags.
- `src/integration/format_registry.cpp` — maturity `structural`,
  `parser_id = "formats.mot-structural"`.

## 7. What this changes about the plan

Step 3 is no longer blocked for `MOT`. It remains blocked for `MCV`, `HID` and
`TSC`, of which the corpus contains no file at all, and for `EFM` and `SHW` for
the same reason. The lesson is worth keeping: before declaring a format blocked
on data, count the repeating structures **inside** the sample already in hand.
