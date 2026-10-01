# DMC3 effect runtime: spawn API, parents, clocks, E records, CEm034 shells, HITS

Date: 2026-10-01
Executable: `dmc3.exe`, SHA-256
`e454272ed0fb0247fcbcf300e5d55d7a3e96d50b89b9ffaff81bb978dcbdd082`.
Source: carried over from Native Reader (`VrUaCom/DMC-Native-Reader`, branch
`NR-Luna-v73`, notes `dmc3-effect-runtime-2026-10-01.md` and `dmc3-motion-effect-runtime-v72.md`) so the reverse is kept with the engine notes. Game
files were analysed locally and are not committed; only ids, offsets and
addresses are quoted. Tools: `research/exe/fx/`.

## Status

| Claim | Status |
| --- | --- |
| Spawn API `0x1402E7A90 / AB0 / CA0`, kind factories, prep modes 0..3 | confirmed (disassembly) |
| Live parent `+0xC0 / +0xD8` (resolver `0x1402E7DE0`) | confirmed (disassembly) |
| Second parent `+0xC8 / +0xD4` (resolver `0x1402E7FF0`, same four modes) | confirmed (disassembly; written by the enemy event handler, see `dmc3-effect-triggers-2026-10-01.md`) |
| Common delta `0x1403261B0`, V-local clock (entry at `floor(a) + 1`) | confirmed (disassembly) |
| E lifetime `+0x80` / held `+0x84` | confirmed (disassembly) |
| E geometry modes (init `0x1402E42EA`, draw `0x1402E5D00` / `0x1402E69E0`) | confirmed for size / pivot / scale / rotation fields; random draws replaced by means in the Reader |
| CEm034 Shl00..Shl05 spawn, flight, explode | confirmed for the no-player paths; steering after tick 10 needs the player (candidate) |
| Kalina control domain (`em+0x4020`) | confirmed (disassembly); the lane0 start of states 0x59 / 0x61 is not paired (candidate) |
| HITS query `0x14005E880`, shell responses, grenade raycast `0x1402C64F0` | confirmed (disassembly) |


### Second parent resolver `0x1402E7FF0` (new in this note)

`effect+0xC8` holds a matrix pointer, `effect+0xD4` a mode; null pointer
means "no parent" (returns false). Mode 0 copies the matrix, 1 keeps an
identity rotation with the parent translation, 2 copies it with translation
zeroed, 3 normalises rows 0..2 (`0x140330390`). The enemy event handler writes
`+0xC8 = part->+0x110` and `+0xD4 = 0 / 1` (most of its attached effects use
mode 1: they follow the part's position only).

### E record fields used by the runtime

* `+0x04` T texture id (resolved by `0x1402E3AA0`); `+0x06 == 1` and `+0x08
  != 0xFFFF` bind an A animation (`0x1402E494F`), otherwise `+0x0C..+0x12` is
  the direct rectangle (x, y, w, h).
* `+0x80` lifetime in ticks -> `effect+0x8B0` (`0x1402E4190`); state 1
  `0x1402E47F0` subtracts dt and retires once negative unless `+0x84` is set
  (then the parent retires it, `0x1402E7A40` on parent `+0x60`).
* Geometry: size mode `+0x2C == 0` takes size A = (`+0x3C`, `+0x44`, `+0x4C`)
  and pivot B = (`+0x30`, `+0x34`, `+0x38`); modes 1 / 2 draw A from [min,
  max] pairs at `+0x3C` (per axis / uniform) with B = A x 0.5. Scale `+0xA8`;
  initial rotation `+0x150 + 12 i` (flag, min, max degrees); orientation
  `+0x1F5` (mode 2). Draw mode 1 is a camera-facing quad, mode 2 an oriented
  quad; modes 3 / 4 are not drawn by these paths.
* A (sprite animation, 336 bytes, registrar `0x140322990`): `[1, texture,
  frame time, last frame, loop, loop frame]`, then 10-byte frames `(x, y, w, h,
  0)` in texels; UV reaches the last texel (`0x140313EA0`: (x + w - 1) / W).

### em034 bindings and V child graphs (from the v72 bridge)

| Owner | V | Parent |
| --- | --- | --- |
| Shl00 | V463 | follows the shell |
| Shl02 | V423 (CEm034, slot20 matrix, mode 3), V377 (follows), V543 (explode, copy, y + 2) | |
| Shl04 | V488 (explode), V475 (fuse < 60) | |
| Shl05 | V276 (follows) | |

V276 -> E571, E571 (second with a local Z rotation); V423 -> E752, E887, P337;
V463 -> E741; V475 -> E404; V488 -> P18, P3, P16, P2, E1, V8; V8 -> E10 x 3,
E2.

---

Scope: the generic P/E/G/V spawn, parent and lifetime code of `dmc3.exe`
(canonical SHA-256 `e454272e…dd082`) and its first consumer in the Reader,
the Lady `CEm034Shl02` Kalina Ann shell. The em034 data are from `em034.pac`
SHA-256 `1a5a245c8348dee3fa37ef1a83da39f15f5c1576e252daf5adc897e349f56dff`,
FXBANK slot 28. No game bytes are committed; every constant below was read
from the executable or the bank.

This closes three items that the v72 bridge left open: the V-local clock, the
E lifetime, and the Shl02 post-spawn path. It also corrects the Shl02 spawn
direction, which was the cause of the "vertical, one-frame" rocket.

## Universal spawn API

| Address | Role |
| --- | --- |
| `0x1402E7A90(kind, id, matrix, prep)` | wrapper, flags `0x20` |
| `0x1402E7AB0(kind, id, matrix, prep, flags)` | prepares a copy of `matrix`, dispatches on kind, `obj+0x10 |= flags` |
| `0x1402E7CA0(kind, id, matrix-or-NULL, flags)` | identity when NULL, copy otherwise; translation.y += 2.0 (`.rdata 0x14035D570`) |

Kind factories: 0 = P `0x140312720`, 1 = E `0x1402E3B10` (class `CEffect`,
vtable `0x140507280`, object 0x980), 2 = G `0x1402EBA10`, 3 = V
`0x140324460`. The V entry dispatch in `0x140324680` uses the same map.

Prep modes of `0x1402E7AB0` (the matrix is copied, never referenced):

| Mode | Result |
| --- | --- |
| 0 | copy |
| 1 | identity rotation, source translation |
| 2 | copy with translation zeroed |
| 3 | copy with rows 0..2 normalized (`0x140330390`, xyz only) |

A live parent is a separate mechanism: an owner writes a matrix pointer to
`effect+0xC0` and a mode to `effect+0xD8`. The resolver `0x1402E7DE0(effect,
out)` applies the same four modes to the pointed matrix each update and
returns false without a parent. It is called from the P/E/G update code
(`0x1402E4CC5`, `0x1402E5DAC`, `0x1402E618F`, `0x1402E6B02`, `0x1402E77CF`,
`0x1402EC146`, `0x140232D5D`, ...).

Reader mapping (`EffectBinding::parent`):

- `RuntimeMatrix`: the copied spawn matrix (`DynamicActorEvent::spawn_matrix`),
  never tracked afterwards;
- `DynamicActor` / `ProjectileTransform`: the live actor world, tracked on
  every actor update (`effect+0xC0`).

## Clocks

`0x1403261B0` (common delta) sets `obj+0x14 = 1.0 (.rdata 0x14035D56C) x`
the product of the speed groups selected by `obj+0x10`, times `obj+0x18`.
At unit game speed one update is one 60 Hz tick, the same unit as a script
frame. Children inherit `+0x18` from their V.

V (`0x140324C50` state dispatch, `0x140324A80` update):

- state 0 only sets state 1 and zeroes the accumulator `+0xF0`;
- each update adds dt to `+0xF0` first, then creates every entry whose
  signed i16 threshold (entry `+0x04`; record `+0x08 + i*0x2C`) is below the
  accumulator;
- an entry with threshold `a` therefore appears at V age `floor(a) + 1`
  (age 1 for `a <= 0`);
- the V ends when every created child has ended (`0x140324400` checks the
  child state `+0x1C` via `0x1402E7DB0`), or earlier when its owner retires it.

E (`CEffect`, update `0x1402E48E0`, state 1 `0x1402E47F0`):

- init `0x1402E4190` loads the record's i32 `+0x80` as a float into
  `effect+0x8B0`;
- state 1 subtracts dt and retires once the value is negative, so the E is
  drawn for ages `0..lifetime`;
- when the record byte `+0x84` is set, the countdown is skipped: the E lives
  until its parent retires it (`0x1402E7A40` reads parent `+0x60`).

Reader: `EffectRuntime::advance(frame)` keeps an effect clock for active
instances, including after the owning actor is gone. The presentation walk
spawns V entries at `floor(a) + 1` and culls E records by `+0x80`/`+0x84`
(`effect_bank::EffectDescriptor::lifetime_ticks / held_by_parent`).

## CEm034Shl02

RTTI `CEm034Shl02` (base `CShell`), vtable `0x1404D88E8`, object 0xD80.

- Constructor: `0x1401734C0`.
- Factory: `0x140173620(pos*, dir*, pac*, flag)`.
- Update: `0x140173E60`, dispatches on the state byte `+0x08`.

### Spawn

The spawn site in `CEm034` is `0x140169937..0x1401699C5`. It takes
`[em034+0x8E8]` = slot20 node0:

1. `dir = 0x14016F610`. This copies the slot20 world, replaces row 3 with
   `(0,0,0,1)` (`.rdata 0x14035D5B0`) and multiplies `(1,0,0,1)` by it. The
   result is the slot20 X axis, without the translation.
2. `0x140173620(pos = slot20 world + 0x30, dir, pac, flag)`.
3. `0x1402E7A90(3, 0x1A7 = V423, slot20 world, mode 3)` spawns the muzzle
   flash from a copied, normalized slot20 matrix. It is owned by CEm034's
   call, not by the shell.

The previous Reader multiplied `(1,0,0,1)` by the full matrix. That added the
hand translation (about 85 units of height) to the direction, so the shell
pointed almost vertically. The same translation drop applies to `CEm034Shl03`
at `0x14016CBC8`: both its offset `(87.8,0,4.28)` and its velocity rotate
with node0 only.

### State 0: init (`0x1401738F0`)

- `pos += (18.6, 0, 12)` from `.rdata 0x14057BB20`, as a plain world-axis add.
- Loads the shell model: MOD from PAC slot 25 and PTX from slot 19 (checked
  as `count >= 0x1A` / `>= 0x14`).
- `shell+0x1A0 = 0x14032FD90` align-Z of `dir` with reference `(0,1,0)`:
  - row0 = up × dir;
  - row1 = dir × row0;
  - row2 = dir;
  - translation = pos.
- `V377 = 0x1402E7CA0(3, 0x179, NULL, 0x10)`. The shell then sets
  `effect+0xC0 = &shell+0x1A0` and `+0xD8 = 3`, so the trail follows the shell.
- Flight table `0x14057B4E0`, loaded by `0x140244940`:

  | Field | Value |
  | --- | --- |
  | speed `+0x160` | 30 |
  | lifetime `+0x17C` | 120 |
  | retarget interval `+0x180/+0x184` | 10 |
  | max turn `+0x188` | 1200 |
  | hit distance `+0x18C` | 100 |

  The `CShell` constructor `0x140244720` sets homing `+0x198 = 1`.

### State 1: flight (`0x140173C60`)

- Collision (`+0x278`) → state 2.
- Otherwise `CShell::move 0x140244870(shell, player joint, flags 3, dt)`:
  - lifetime and retarget timer -= dt;
  - at timer ≤ 0 (tick 10, then every 10), steer toward the target with
    `0x140244B50`; otherwise normalize dir;
  - dir *= 30;
  - pos += dir × dt.
- `0x140244810` returns −1 when lifetime ≤ 0, or 1 when within 100 of the
  player. Either → state 2.
- The matrix is rebuilt with align-Z each tick.

### State 2: explode (`0x140173800`)

- `V543 = 0x1402E7CA0(3, 0x21F, &shell+0x1A0, 0x10)`: a copy of the shell
  matrix, with y + 2.
- Blast collision radius 300.
- Countdown `+0xD68 = 3.0`, then 3→2→1→0→−1 → state 3.
- State 3 retires V377 (`0x1403261E0` on `+0xD60`), then the shell.

### Standalone Reader behaviour

Flight is exact for ticks 1..9, where no target is involved. From tick 10 the
EXE steers toward the player joint. The Reader has no player, so it holds the
direction, marks the updates `requires_gameplay_world_context`, and explodes
at the lifetime end:

| Age (ticks from spawn) | Event |
| --- | --- |
| 121 | V543 |
| 125 | shell and V377 retire |

In the game a collision or the 100-unit proximity would end the flight
earlier.

Reader bindings (`em034_effect_bindings`, actor 2):

| Root | Phase | Parent | Lifetime |
| --- | --- | --- | --- |
| V423 | 0 | RuntimeMatrix (slot20 mode 3) | EffectCallback |
| V377 | 0 | ProjectileTransform (shell) | ParentActorRetire |
| V543 | 2 | RuntimeMatrix (shell, y+2) | EffectCallback |

## Kalina (component0) control domain

The entry dispatcher `0x14016A410` is a jump table:

- byte table `0x14016ACAC`;
- rel32 table `0x14016AC48`.

The Kalina (slot20) domain is decided by `em+0x4020`. That byte is the enable
flag (`+0x20`) of the slot20 node0 CCnsMatrix at `em+0x4000`:

- `0x1401713F0(em, component, preset)` rebuilds that constraint for
  component 0 and always writes `+0x4020 = 1`;
- the update `0x140170FB0` advances the em034_013 controller (`em+0x52B0`)
  only while `+0x4020 == 0` (`0x140171072`).

Only one block clears it: `0x14016AA73`. It starts `+0x52B0` with bank 4,
action state − 0x53, and writes `+0x4020 = 0`. It is reached from these
states:

| States | Case | Body actions |
| --- | --- | --- |
| 0x5A..0x5D, 0x64..0x73 | `0x14016AA3D` | 7..10, 17..32 |
| 0x7B | `0x14016A509` | 40 |
| 0x7C/0x7D | `0x14016A52F` | 41/42 |

Bank-4 actions 0..6 (states 0x53..0x59, including the Kalina shots 3/4/5)
keep Kalina on the hand constraint (`0x1401713F0(0, 1)`).

States 0x59/0x61 start the controller later, from their lane0 signal
(`0x14016988B`: stow preset, then action (4, 6)/(4, 14) and
`+0x4020 = 0`). That start is not paired yet.

In the independent domain the component MOT runs in actor space: the
Reader's body part space. Renders of acts 10, 30, 31 and 40..42 put Kalina in
Lady's hands.

## Model-less shells (Shl00, Shl01, Shl04, Shl05)

Their draw slots [2]/[3] are the null stub `0x14024EA30`: they have no model
and are visible only through their effects. Factories:

| Shell | Factory | Spawn sites |
| --- | --- | --- |
| Shl00 | `0x140172240` | pistol states (bank 3), state 0x7F, SMG `0x140171C70` |
| Shl01 | `0x1401729D0` | Kalina states |
| Shl04 | `0x140175210` | state 0x8F |
| Shl05 | `0x140175B10` | state 0x81 |

Common constants:

- body joints are `em+0x7E8 + 8*joint` (`+0x830` = joint 9, `+0x850` =
  joint 13);
- component node pointers are `em+0x8E8` (slot20 × 3), `+0x900` (slot21),
  `+0x908` (slot22), `+0x910` (slot23 × 4), `+0x930` (slot24);
- pistol/Shl05 speed `em+0x59F0` = 35 (init `0x14016FEC2`);
- straight lifetime `+0x52C` = 120.

| Act (state) | Shell | Standalone path from the EXE | Effects |
| --- | --- | --- | --- |
| 44 (0x7F) | Shl00 | no player in the `0x1402C6870` cone → axis (−1,0,0) of slot21 (lane1 ch1 = 0) or slot22; muzzle joint 9; speed 35 | V463 (E741, held) follows |
| 46 (0x81) | Shl05 | no player → slot23 axis (1,0,0); muzzle joint 9 (+0x5A27 = 1); speed 35; hit → V435/V277 | V276 follows |
| 50 (0x85) | Shl00 | lane1 ch0 1/2 toggles `+0x57DD`; 0.9 timer → one shot per tick via `0x140171C70(em, 1)`: slot24 axis (−1,0,0), speed 45, muzzle joint 13; `dl = 1` never aims at the player | V463 |
| 60 (0x8F) | Shl04 | count `[1,2,3,6,3,2][em+0x5A1C]` (Reader: entry 0); velocity (0,0,10) × Ry(yaw) × Rx(pitch), pitch −(rand%30)°, yaw ((rand%100)−50)° + actor yaw (Reader: means −14.5°/−0.5°); joint 9; fuse 120 + 30·i | E765 grenade sprite (held) from init; V475 when fuse < 60; V488 on explode (copy, y + 2) |

Shl04 flight (`0x1401756E0`):

- per tick: pos += vel; vel.y = min(vel.y − 1, 30);
- stage raycast `0x1402C64F0`: mirror about the hit plane,
  v' = 0.5·reflect(v); at rest below |v| = 10;
- fuse < 0 → state 2 (`0x1401753A0`): V488, blast radius 200, 3.0 countdown,
  retire.

With a stage room loaded, the raycast runs on the room's HITS (see
"Stage collision (HITS)" below). Without one, the Reader's floor
(y = 0) stands in for it. Those events carry `requires_gameplay_world_context`.

Not materialized standalone:

- **Bank-3 pistol states (0x34..0x52):** they always aim at the player joint
  (`0x14016FC30`) and stay deferred.
- **Shl01 Kalina missiles:** they orbit 16 arena points around (2500, 2400)
  (tables `0x14057B530` / `0x14057B5F0`, radii ≈ 1274/677). They home in on
  the player after three laps; V474 is the trail, V484 the explosion.

## Stage collision (HITS)

### EXE query contract

`0x14005E7A0(manager = [global+0x28]+0x710, from, to, hit_out, record_out, mask)`:

- calls the stage query `0x14005E880`, then two dynamic-object queries
  `0x14005BCF0` (categories 0x0E and 0x11);
- on a stage hit the segment end becomes the hit point (`hit_out`).

`0x14005E880` works on the HITS record list:

- `0x1402D2A10` lists the grid cells the segment crosses;
- each triangle-plane record (stride 0x38) is tested once, deduplicated with a
  bitset;
- a record whose `flags >> 16` shares a bit with `mask` is skipped;
- `0x1402D0F30` intersects the segment with the triangle;
- every hit shortens the segment, so the nearest hit wins;
- the hit record (0x38 bytes) is copied to `record_out`.

Masks used by callers:

| Caller | Mask |
| --- | --- |
| CEm034 line of sight `0x140168ED0` | 0x10 |
| `0x1402C64F0` by object type `+0x74` | 0 → 0x40, 2 → 0x02, 3 → 0x10, 4 → 0x20 |
| `0x1402C64F0` with no object (Shl04) | 0 |

### Shell responses

| Shell | EXE response | Reader |
| --- | --- | --- |
| Shl02 rocket | state 1 reads collider `+0x278` (count `+0x10`) before moving (`0x140173D14`) → state 2 | segment of each update tested on HITS; state 2 on the update after the hit; V543 at the hit point |
| Shl00 bullet | collider `+0x268` → V10 (`0x1401726D5`); collider `+0x270` with `flags & 3` → V473 (`0x14017273B`); both → state 2 | HITS hit → V473 (actor 0, phase 2), copy of the shell matrix with y + 2 |
| Shl05 shot | collider `+0x268` → V435 if `flags & 0x11040`, else V277; collider `+0x270` with `flags & 3` → V277 (`0x14017607A`) | HITS hit → V277 (actor 5, phase 2) |
| Shl04 grenade | stage raycast `0x1402C64F0` (mask 0) | HITS raycast with the mirror/restitution response |

The Reader maps the stage to the `flags & 3` collider branch. This is an
inference: the collider categories of `+0x268`/`+0x270` are not decoded.
Stage-hit shells stop at the hit point, spawn the hit effect on the next
update and retire one update later.

### Characters

The retail character-versus-HITS response is not decoded. The Reader uses a
documented proxy:

- the vertex centre of the motion-driven parts is a sphere of radius 50;
- each frame it is moved along the root motion through the wall records
  (|normal.y| < 0.7) in sub-steps of r/2, and pushed out horizontally, so it
  slides along walls;
- it follows the floor records (normal.y ≥ 0.7) under it, relative to the
  floor it started on; falls are limited to 30 units per frame;
- a loop or seek back restarts from the placed spot.

### Room placement

The room drawn around the model is its collision world. The room is placed
by `stage_room::placement_for`:

- `room → model = Ry(yaw)·(p − spot) + spot + offset`;
- offset = rest centre − spot (x, z) and rest low − spot.y;
- the same transform is used by the renderer, the overlay and the runtime.

The first HITS source (source 0, detailed) is used. st001 has 294 records:
79 floors, 213 walls, 2 ceilings.

## em034 FXBANK data used

Entry format: `activation / local T / R / S`.

```text
V423: E752 a0 T(60,0,0) | E887 a3 T(60,0,0) | P337 a0 R(0,90,0)
V377: E669 a1 T(0,0,-50) S(1,.7,1) | E699 a0 T(0,0,-50) | G214 a7 R(0,180,0)
      E892 a3 T(0,0,60) | E892 a5 T(0,0,60) R(0,0,180)
V543: P18 a18 | P3 a8 T(0,50,0) | P16 a12 | P2 a11 | E1 a5 T(0,150,0) S2.6
      V8 a0 T(0,150,0) S1.5 -> {E10 a0 S2, E10 a1, E10 a2, E2 a3}
      E61 a2 T(0,150,0)
```

E lifetimes (`+0x80`, `+0x84`):

| E record | Lifetime | Note |
| --- | --- | --- |
| E752 | 20 | |
| E887 | 20 | |
| E669 | 4 | held by parent |
| E699 | 4 | held by parent |
| E892 | 20 | mode 5 |
| E1 | 40 | |
| E61 | 30 | |
| E10 | 7 | |
| E2 | 20 | mode 5 |
| E741 | 20 | held by parent |
| E404 | 4 | held by parent |
| E571 | 20 | held by parent |

The V377 trail entries sit at local −Z behind the shell and its front entry
at +Z. The data therefore agree with the EXE align-Z basis, where the shell's
local Z is the flight direction.

## Still open

- P and G rendering, and their lifetimes. A V whose children include P/G
  therefore has no Reader-side completion. `EffectCallback` roots stay active
  until replay/reset; their E children are culled individually.
- The E-local A frame clock (`0x1402E4970`): the first atlas frame is still
  shown.
- The composition of a `0x1402E7CA0(NULL)` local matrix (identity, y+2) with a
  live `+0xC0` parent: V377 is presented directly on the shell matrix.
- Shl02 steering and the player proximity end, which need gameplay context.
- The collider categories of `+0x268`/`+0x270`/`+0x278` and the retail
  character-versus-HITS response (the Reader uses a proxy, see Stage
  collision).
- The exact manager order within one frame (±1 tick on spawn/retire ages).

## Verification

- Native regressions:
  - `effect_runtime` covers phases, copied versus live parents, retire
    ownership and the clock;
  - `motion_playback` covers `shl02_shell_world`: direction, init offset,
    30/tick, 120-tick clamp;
  - `player_coat` covers the E `+0x80/+0x84` decoding;
  - `pac_assembly` covers the seven em034 bindings.
- A scratch harness on the real `em034.pac` (bank 4 action 3) gave this
  timeline:
  - frame 5: V423 and V377 spawn;
  - frames 5..124: the shell flies along the slot20 X axis, (0,0,1) in the
    rest facing;
  - frame 125: V543 spawns;
  - frame 129: the shell and V377 retire;
  - V423 and V543 keep their own clocks.

  Renders show the shell leaving the muzzle horizontally with E752 on the
  muzzle.
