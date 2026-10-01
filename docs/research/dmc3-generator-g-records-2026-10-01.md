# DMC3 G records: CGenerator

Date: 2026-10-01
Executable: `dmc3.exe`, SHA-256
`e454272ed0fb0247fcbcf300e5d55d7a3e96d50b89b9ffaff81bb978dcbdd082`.
Source: carried over from Native Reader (`VrUaCom/DMC-Native-Reader`, branch
`NR-Luna-v73`, note `dmc3-generator-g-records-2026-10-01.md`; port `modules/generator.cpp`, test `generator_test`) so the reverse is kept with the engine notes. Game
files were analysed locally and are not committed; only ids, offsets and
addresses are quoted. Tools: `research/exe/fx/`.

## Status

| Claim | Status |
| --- | --- |
| G record fields, spawn schedule, RNG order, jitter, scale ramp, drift, follow mode | confirmed: all 74 corpus G records reproduce every emulated spawn event (tick, kind, id, follow, matrix) |
| C clip B-spline `0x1402D3660` | confirmed (4 corpus clip generators + synthetic samples) |
| G -> G child path | candidate (one corpus record; the G creator was not emulated) |


---

Scope: the 74 G records of the corpus PACs (em000, em006, em007, em028, em034,
plwp_sword, plwp_2sword, st002_effect). A G record is an invisible spawner
(vtable `0x1405073F8`, tick `0x1402EC840`: state byte `+8`, 0 init
`0x1402EBC10`, 1 update `0x1402EBDC0`). It creates child effects of kind P, E, G
or V (the same dispatch kinds as the V entries) through the creators
`0x1403127B0` (P), `0x1402E3B90` (E), `0x1403244E0` (V).

## Record (96 bytes)

| Offset | Meaning |
| --- | --- |
| +0x00 | 1: children follow the generator (compose with its live world) |
| +0x01 | motion: 0 drift along yaw / roll, 1 C clip, 2 still |
| +0x02 | C record id (motion 1) |
| +0x04 / +0x08 | drift speed / deceleration per tick (motion 0); clip rate per tick (motion 1) |
| +0x10 / +0x14 | drift direction yaw / roll, degrees (`0x1402EC9E0`: row 0 of Rz*Ry*Rx) |
| +0x18 / +0x1C | ticks before the first spawn / base interval |
| +0x20 | life, ticks (unless +0x30 != 0: endless) |
| +0x24 / +0x26 | child kind (0 P, 1 E, 2 G, 3 V) / id |
| +0x28 / +0x2C | child yaw / roll, degrees; +0x59 random yaw spread (degrees, wrapped to +-180) |
| +0x34 | interval mask: next interval = (rand & mask) + base |
| +0x38 / +0x3C / +0x40 | scale start / end / ramp ticks |
| +0x44..+0x4C | spawn offset range per axis: rand % n - n / 2 (n = int value, 0 = none) |
| +0x50 / +0x54 | random scale range: ((rand % \|int(range * 10)\|) * sign) * 0.1, 1.0 when int(range) == 0 |

## Runtime

* Tick 1 is the init tick (timers, drift state, the one random scale with a
  ramp); spawns begin at tick 2. Per tick: life -= 1 (dies below 0), scale
  (ramp start -> end over `steps` ticks, or a fresh random factor), yaw
  spread, then the local matrix `Rz*Ry*Rx(0, yaw, roll)` with the drift
  position as translation and uniform scale rows, times the owner's world.
  `timer -= 1`; at `timer <= 0` a child is created and the timer reloaded
  (interval draw, then the three jitter draws, in that order). The drift /
  clip position advances at the end of the tick.
* Spawn matrix: the composed matrix with row 3 += jitter; in follow mode the
  creator receives an identity matrix with row 3 = jitter (`0x1402EC256`
  resets the scratch matrix first) and the child composes it with the
  generator's live world.
* Clip motion (`0x1402D3660`): uniform cubic B-spline over n + 1 points,
  s = clamp(t) * (n + 2) - 1, weights (3u^3 - 6u^2 + 4) / 6 for u < 1 and
  (2 - u)^3 / 6 for u < 2, index clamped; t += +0x04 per tick.

## How this was checked

* `app/src/test/native/generator_test.cpp` replays synthetic records (no game
  data, `generator_truth.inc`) against emulated runs of the EXE: three
  scenarios (drift + ramp + jitter, still + follow + endless, clip motion),
  100 ticks, every spawn tick / kind / id / follow / matrix; the B-spline is
  sampled at seven t.
* Differential run on all 74 real records (`research/exe/fx/emu/gen_gt.py`): 70
  non-clip and 4 clip records reproduce every emulated spawn event exactly.
* Not covered by the emulator run: the G -> G child path (one corpus record)
  and a clip rate other than 0 on real data (the synthetic scenario covers it).

## What the Reader does

`append_effect_generator` (resource_session.cpp) replays the generator from
tick 0 to the entry's age and draws each spawn through the usual
`collect_effect_children` at its own age (`ticks - spawn.tick - 1`, the child
first updates on the next tick). The 160 newest spawns are kept. `effect_extent`
counts delay + life (endless: 300) + the child's extent.

## Approximations

* The random draws come from a fixed per-record seed (the retail generator
  `0x140059390` is shared and unseeded); the spawn count and jitter match the
  distribution, not a particular retail run.
* Earlier ticks are replayed under the entry's current world, so a moving
  owner drags non-follow children with it.
