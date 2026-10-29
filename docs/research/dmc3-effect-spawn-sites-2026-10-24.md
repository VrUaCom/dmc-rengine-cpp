# Who starts each effect: the spawn-site catalog (v122)

> Ported 2026-10-29 from the DMC Native Reader (`NR-Luna-v73`, `docs/research/dmc3-effect-spawn-sites-v122.md`). Paths below that start with `app/` or `tools/research/` are the Reader's. In Rengine: the catalog is `profiles/dmc3/effect_spawn_sites.hpp` (data `src/profiles/dmc3/effect_spawn_sites.inc`, test `tests/effect_catalogs_tests.cpp`). `tools/reverse/dmc3_gates.cpp` checks every row: 1023 of 1023 rows are direct calls to their spawn API or kind factory, from the function the row names. The Reader's generator gave a block start instead of the function entry for 125 rows; Rengine's copy uses the `.pdata` entry.

Goal: let the Reader start every effect without per-character exceptions.
Read from the HD Collection's `dmc3.exe` (as dmc3-effect-tool-v121.md). Only
addresses, class names, kinds and ids are recorded.

## Effects are started by code, not by data

- **The motion script triggers nothing.** Its interpreter `0x140058FE0` has
  36 opcodes. None reaches the spawn API or a kind factory within four
  calls:
  - opcodes 16..31 share `0x1400596F0`, which copies joint vectors;
  - opcode 3 sets the five signal channels that a class's code reads.
- **Each effect has a class behind it.** A class's own code spawns it
  through `0x1402E7A90`, `0x1402E7AB0`, `0x1402E7CA0` or a kind factory
  (dmc3-shell-effect-runtime-exe-v73.md). It does so on a signal, a state
  change, an event code or a hit.
- **Data-driven playback is narrow.** Only the stage layout (`eff` /
  `epos`), the stage configuration (STE) and the cutscene clips (`Effect`,
  `EffectI`) start effects from data.

## The catalog

`tools/research/effect_spawn_sites.py <dmc3.exe> <out.inc>` writes
`app/src/main/cpp/modules/effect_spawn_sites.inc`; `effect_spawn_sites.h`
serves it. For every call of the API it records:

- **The owner.** This is the class whose vtable methods (from RTTI: type
  descriptor, complete object locator, vtable) surround the calling
  function. When the function is not a method, the nearest class is taken.
- **The kind and the id.** Constants are folded back along every path into
  the call: `mov`, `xor`, `lea r, [r ± imm]`, `add` and `sub`, across the
  jumps that lead in. A call that several branches reach with different ids
  gets one row per id.
- **The prep mode or the flags**, from the fourth argument.

Results:

| | |
| --- | --- |
| Call sites | 875 |
| Classes with vtables | 385 |
| Sites with kind and id known | 796 (1023 rows; one call can start several ids) |
| Distinct effects | 568 (V 715 rows, P 90, G 89, E 50) |
| Sites with a computed id | 79 |

Sites per owner, highest first: CEm000 72, CEfcPub 61, CEm010 49, CEm035 48,
CEm025 46, CEm026 35, CPlDante 35, CEm013 25, CStageSetCombo 25, CEm030 23,
CEm037 22, CEm028 18.

The 79 computed ids are mostly in CEfcPub (15), CPlDante (9), CEm010 (8) and
CEm026 (7): ids taken from tables or by arithmetic.

The catalog agrees with the sites already traced by hand:

| Owner | Call | Effect | Mode |
| --- | --- | --- | --- |
| CEm034 | `0x1402E7A90` | V423 | prep 3 |
| CEm034Shl04 | `0x1402E7CA0` | V488 | flags 0x10 |

It also lists Lady effects the hand-written profile does not have yet, for
example CEm034Shl02's V543 and V377. The test `effect_sites` keeps these
rows.

## In the Reader

- **⋮ → Effects (play any).** The menu lists every V, G, E and P record
  of the banks loaded with the scene: the file's own, the stage effect bank
  and a cutscene's.
  - Each row gives the parts, the length, the bank and "in the game:" the
    owners from the catalog.
  - The picked effect loops at the model's feet, or at a stage's floor spot.
  - With a free camera or a cutscene's camera, it plays in the middle of
    the picture, just in front of the nearest model there.
  - It is drawn through the same path as stage and cutscene effects.
- **Probe on the corpus.** These counts are effects whose loop visibly
  changed the frame:

  | Scene | Effects | Visible |
  | --- | --- | --- |
  | m03_s00, its cutscene bank | 16 | 9 |
  | st002 with st002_effect.pac | 38 | 33 |

  Most of the rest are E records of 0.0..0.3 s. A few longer ones still
  show nothing: E45, E107, G86, P154, V119. They have not been looked at
  yet.

## Next

1. **Enemy events.** em000..em008 share the event handler `0x1401C3130`
   (dmc3-effect-triggers-recon-v77.md). Emulating it for each code gives
   code → (kind, id, placement) for all of them at once.
2. **Hits.** CEfcPub's id arithmetic: one formula for every weapon's hit
   effects.
3. **The general executor.** Link the catalog's owners to the motion
   script's signal channels and actor states. A single executor then
   replaces the hand-written profiles.
