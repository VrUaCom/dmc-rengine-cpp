# DMC3 effect triggers: enemy events, AI commands, weapons, stage keywords

Date: 2026-10-01
Executable: `dmc3.exe`, SHA-256
`e454272ed0fb0247fcbcf300e5d55d7a3e96d50b89b9ffaff81bb978dcbdd082`.
Source: carried over from Native Reader (`VrUaCom/DMC-Native-Reader`, branch
`NR-Luna-v73`, note `dmc3-effect-triggers-2026-10-01.md`) so the reverse is kept with the engine notes. Game
files were analysed locally and are not committed; only ids, offsets and
addresses are quoted. Tools: `research/exe/fx/`.

## Status

| Claim | Status |
| --- | --- |
| Effect ids are global (records byte-identical across banks) | confirmed (corpus hashes) |
| v74 "CEm000: 998 sites, 70 in `0x1401C34C8`" | rejected / corrected (span artefact; see section 2) |
| Enemy event handler `0x1401C3130`, code -> spawn table | confirmed for the no-player path (emulated per code); player branch not run |
| AI command layer, command tables per class | confirmed (disassembly); command 0x52 -> em000 bank 0 action 82 -> MOT 62 checked on data |
| Death schedule 0 / 5 / 10 / 15 / 20 / 25 | confirmed (disassembly of `0x140095E85`) |
| `obj+0x6D8` entry k = body joint k (all 23 entries) | confirmed (model bind `0x14030F850`, see section 3) |
| Death trigger: control code 0x3E7 on the killing hit | confirmed (disassembly of `0x1400675C0`, `0x1401C5CD8`) |
| Nevan state -> motion pairing | open |
| CEfcPub id arithmetic as the source of the remaining plwp ids | hypothesis |
| Stage keywords `beff`, `SET LIGHT`, `DOOR` | confirmed (parsers located, corpus usage counted) |


---

Scope: who asks for FXBANK effects in `dmc3.exe` outside the CEm034 (Lady)
code that is already ported, for the corpus archives em000, em006, em007,
em028, plwp_sword, plwp_2sword and the stages st000..st002. This document is
a map: it fixes a wrong count in the v74 roadmap, records the mechanisms, and
lists what still has to be traced before the Reader can play these effects.
The tools are in `research/exe/fx/` (`spawn_sites.py`, `comcmd.py`,
`linsum.py`, `vcall*.py`, the Unicorn harness `emu/evt.py`); no game bytes are
quoted beyond ids and keywords.

## 1. Effect ids are global

The P / E / G / V id space is shared by every bank. A record with the same
(kind, id) is byte-identical in every PAC that carries it, e.g. V210 in em000,
em006, em007 and plwp_sword; V281..V290, E797, E997, G405 in em000 / em006 /
em007; P44 and P192 in the three enemies and plwp_sword. A bank is the subset
of the global table that its archive needs. The spawn API takes only (kind,
id), so a request resolves in whichever loaded bank holds the id.

## 2. Spawn sites by class (correction of v74)

The v74 roadmap said "CEm000: 998 spawn sites, 70 in `0x1401C34C8`". That
count came from class spans that all started at a shared base-class function
(`0x14004C9E0`), so every site up to the last CEm000 function was counted for
CEm000. Re-attributed by translation unit (a function belongs to the class
whose own vtable functions surround it):

| Owner | Sites | Ids in corpus banks |
| --- | --- | --- |
| `CNonPlayerDeathEffect` unit: the enemy event handler `0x1401C3130` (section 3) | 72 | em000 / em006 / em007 |
| `CEfcPub` (shared hit / impact effects, `0x14008DCA2`, `0x14008E315`, `0x14008F730`, `0x140090853`) | 61 | some plwp_sword / plwp_2sword (V123, V176, V242..V246, V604, V644..V650) |
| `CEm025`, `CEm035`, `CEm010`, `CEm026`, `CEm013`, `CEm030`, `CEm037`, ... | 20..65 each | none (archives not in the corpus) |
| `CPlDante` and the shared player unit | 27 + 23 | V141, V142 (plwp_sword); V645, V648 (plwp_2sword) |
| `CEm028` (Nevan) and `CEm028Shl00..07` | 18 + 27 | all in em028 |
| `CPl000Shl02` / `Shl03` / `Shl0a` (player projectiles) | 8 / 3 / 5 | V64, V71, G30..G34, V139, V184, V195 (plwp_sword); V602, G318 (plwp_2sword) |
| `CPlWp2Sword` | 4 | G460, V770, V893, V894 (plwp_2sword) |
| `CEm006Shl00` | 3 | V275, V447 (em006) |
| `CEm005Shl00` / `Shl01` | 3 | V750, V751, V439 (em000) |
| `CStageSet*` (Combo, Switch, Mirror, Crystal, Gate, ORBreak, ...) | 2..25 each | none of the corpus stages |

`CComEm000/005/006/008` (enemy AI commands) call only the sound API directly;
their effects go through the event handler of section 3.

## 3. The enemy event channel (em000 .. em008)

`CEm000`..`CEm007` derive from `CNonPlayerDeath`. Their interface at object
`+0x110` has, in slot 16 (`+0x80`), the handler `0x1401C3130(iface, code,
vec3* pos)`: a two-level switch (byte table `0x1401C6CCC`, jump table
`0x1401C6BF4`) over codes 0..0xCE; codes above 0x384 (0x3E4, 0x3E5, 0x3E8..,
0x7D0, 0xBB8) are control events (motion holds, flags), not effects. All event
calls pass constant codes; no event code is read from data.

Effects spawned by a code are placed in one of three ways:

* the given position (`pos`, the actor position `obj+0x80` for most callers);
* actor position and yaw (`obj+0x80`, s16 yaw at `obj+0xC0` x 2pi/65536);
* attached to entry k of a pointer array at `obj+0x6D8` (23 entries, more
  arrays follow at `+0x790` and `+0x7C0`); the effect gets `+0xC8 = entry->+0x110`
  (a matrix pointer) and a mode at `+0xD4`. The second parent resolver
  `0x1402E7FF0` reads them each update: mode 0 copy, 1 identity rotation with
  the parent translation, 2 copy without translation, 3 normalised rows (the
  same four modes as the `+0xC0/+0xD8` live parent of v73).

**The array is the body skeleton.** The class init loads the body model
(`obj+0x840`, slot 1 for CEm000) and calls its `vtbl+0x150` (`0x14008A000`)
with `r8 = obj+0x6D8`; the bind `0x14030F850` walks the model's joints
(count u16 `model+0xEA`, 23 for em000's body) and writes into entry k:
`+0xFA = k`, `+0x110 = model+0x188 + 64 k` (the joint's world matrix),
`+0x108 = entry+0x40`, `+0xF0` = the parent entry. The entries themselves
are 0x260-byte joint objects that the factory (`0x140094790`) allocates
(23 at `+0x6D8`, 6 at `+0x790` for the cloth model at `obj+0xFC0`, the rest
at `+0x7C0`). So "object k" below is **body joint k**: the effect follows
that joint's world matrix. (The earlier reading "entry 0 body, 1 weapon"
took the script objects for this array; it is wrong. The weapon model,
`obj+0x1EC0` from slot 26 or 29 with textures 25, hangs on joint 9, which is
why codes 0x18, 0x27 and 0x69 use object 9.)

Event table, from running `0x1401C3130` in the emulator for every code on a
fabricated actor (actor position (1, 2, 3), yaw 90 degrees, `pos` = (100,
200, 300), no player, entry k given translation 10000 + 1000 k). "object k"
is entry k of `obj+0x6D8` (k > 22 continues into the following arrays);
"x2" is a uniform scale on the spawn matrix:

| Code | Spawns (no-player path) |
| --- | --- |
| 0x00 | G75 @ object 5 (mode 0) |
| 0x01 | V105 @ given pos |
| 0x03 | E42 x2 @ object 1 (mode 1); V42 x2 @ object 1 (mode 1) |
| 0x06 | V26 @ object 0 (mode 1) |
| 0x08 | V70 (uses actor fields not set in the run) |
| 0x09 | V84 @ given pos |
| 0x0a | V273 @ actor pos + yaw |
| 0x0b | G152 @ object 0 (mode 1) |
| 0x0c | V274 @ actor pos + yaw |
| 0x0d | V281 @ object 2 translation |
| 0x0e | P44 @ actor pos + yaw |
| 0x0f | V288 @ actor pos + yaw |
| 0x10 | V290 @ actor pos + yaw |
| 0x11 | V282 @ actor pos + yaw |
| 0x13 | G152 @ object 0 (mode 1) |
| 0x15 | V499 @ object 1 (mode 0) |
| 0x16 | V516 @ object 0 (mode 0) |
| 0x18 | V389 @ object 9 (mode 0) |
| 0x19 | V532 @ object 1 (mode 0) |
| 0x1a | V440 @ object 1 (mode 0) |
| 0x1b | V623 @ object 0 (mode 0) |
| 0x1c | V703 @ object 0 (mode 0) |
| 0x1e | V672 @ object 36 (mode 0) |
| 0x1f | V702 @ object 1 (mode 0) |
| 0x21 | V620 @ object 0 (mode 0) |
| 0x23 | V848 @ object 0 (mode 0) |
| 0x24..0x26 | V346 @ object 36 (mode 1) |
| 0x27 | V389 @ object 9, then needs game state |
| 0x29 | V672 @ object 36 (mode 0) |
| 0x2b | V412 @ object 1 (mode 0) |
| 0x2c | V223 @ object 5 (mode 0) |
| 0x66 | P32 @ given pos |
| 0x67 | P35 @ actor pos + yaw |
| 0x68 | V15 and G13 @ object 2 (mode 0) |
| 0x69 | V132 @ object 9 (mode 0) |
| 0x6a | V15 and G13 @ object 3 (mode 0) |
| 0xc8 | V315 @ object 4; P32 @ objects 5, 21 (mode 1) |
| 0xc9 | V316 @ object 3; P32 @ objects 4, 3, 6, 10 |
| 0xca | V317 @ objects 7, 8; P32 @ objects 7, 8, 9 |
| 0xcb | V318 @ objects 11, 12; P32 @ objects 11, 12, 13 |
| 0xcc | V319 @ object 1; P32 @ objects 14, 15, 18 |
| 0xcd | P32 @ object 2 |
| 0xce | P32 @ objects 16, 19, 17, 20 |

For enemy type 0x1A (`obj+0x670`) codes 0xC8..0xCE spawn P302 instead of P32
and skip the V. With a player present, codes 0x0A..0x23 take a different
branch through the player object (not run).

## 4. Who sends the codes

* **AI commands** (`CComEm000` for CEm000..CEm004, `CComEm005`, `CComEm006`,
  `CComEm008`): each command is a small state machine. State 0 calls the
  enemy interface slot 4 (`+0x20`, `0x1401C3030`) with a command number; that
  slot maps it through a per-class table of (bank, action) byte pairs at
  `obj+0x2DF0` and starts both script players (`obj+0x2BB0`, `+0x2CD0`) with
  `0x14005A1F0`. Slot 5 (`+0x28`) uses a second table at `obj+0x2DF8`. Slot 8
  (`+0x40`) returns the motion frame. Later states compare the frame with
  constants and send event codes through slot 16.
  * Command tables are written by each class init: CEm000 `0x1400981B9`
    (`0x1405A3300` / `0x1405A31E8`), CEm001 `0x14009D60F` (`0x1405A4370` /
    `0x1405A4278`), CEm002 (`0x1405A54B0` / `0x1405A53A8`), CEm003
    (`0x1405A6410` / `0x1405A6318`), CEm004 (`0x1405A7050` / `0x1405A6F68`),
    CEm005 (`0x1405A79E0` / `0x140CA80E0`), CEm006 (`0x1405A8320` /
    `0x1405A8218`), CEm007 (`0x1405A8FD0` / `0x1405A8EB8`).
  * CEm000's first table is the identity: command n = (bank 0, action n).
  * Checked on the data: command 0x52 (`0x140066F10`) plays em000 script
    bank 0 action 82, which plays MOT 62 for objects 0 and 1; at frame >= 76
    it sends code 3 (E42 + V42 on the scythe) and sound 0x19.
  * Frame gates in the command units (`research/exe/fx/comcmd.py`): code
    3 at 30, 40, 50, 76, 100, 110, 170, 180; code 0x0C at 30; code 0x22 at
    160; code 0x23 at 140; code 9 at 240. Several attack states have their
    state 0 in another function, so the command -> action pairing still has
    to be read per command.
  * Class inits (between each constructor and the next class's): CEm000
    `0x140093E90`, CEm001 `0x140099100`, CEm002 `0x14009DD30`, CEm003
    `0x1400A2C60`, CEm004 `0x1400A7B80`, CEm005 `0x1400A9230`, CEm006
    `0x1400AE800`, CEm007 `0x1400B0F10`, CEm008 `0x1400B2F00` (constructors).
* **Death trigger.** The damage command of `CComEm000` (command 4, update
  `0x1400675C0`, vtable slot 9) restarts itself on the killing hit and sends
  control codes 0x385 and 0x3E7 (sound 0x1C). The handler case 0x3E7
  (`0x1401C5CD8`) resets the motion state, clears `+0x2DC0`, `+0x2DCC`,
  `+0x2DD8`, and sets `obj+0x2EF4 = 1`, which runs the death state machine
  `obj+0x2EF8` (`0x14009523A`) every update. The sand schedule therefore
  starts with the killing hit, on top of the damage motion.
* **Death** (`CEm000`..`CEm004`, `0x140095E85`; copies per class): on entry
  codes 0x69 and 0xC8, then a timer (`obj+0x2EFC += dt`) sends 0xCA + 0xCB at
  5, 0xC9 at 10, 0xCD at 15, 0xCC at 20, 0xCE at 25 ticks: V315..V319 and
  P32 / P302 over the body, the sand break-up. CEm005 and CEm008 carry the
  same 0xC8..0xCE sequence. The class units of CEm006 and CEm007 send fewer
  codes (CEm006: 8, 0x69, 0xC8, 0x28; CEm007: 8, 0x69, 0xCE, 0x12, 0x67).
* **CComEm006** (its pairing with em006 / em007 is not traced): codes 0x1B,
  0x1D, 0x28 (frames 45, 95), 0x17, 0x27, 0x16, 0x22, 0x0E, 0x68.

## 5. Nevan (CEm028)

No AI command layer: the update `0x140131CF0` runs a 39-state switch
(`0x140131F59`) and per-frame helpers that each spawn one effect through
`0x1402E7CA0` (flags 0x10 / 0x20 / 0x40): V409 `0x14012B710`, V378
`0x14012BC40`, V782 `0x14012D1D0`, E107 `0x14012ED10`, V860 + V869
`0x14012F0E0`, V815 + V816 `0x14012F400`, E346 `0x14012F1D0`, G151 in the
update itself, V220 from two vtable slots. The shells: Shl00 V272, V350, P234,
V1; Shl00/01 V51, V902, P234, P203; Shl01 V364; Shl02 V49, V361; Shl03 G166;
Shl04 V359; Shl05 V191; Shl06 V462, V449, V455; Shl07 V430, V801, P203. Every
id is in the em028 bank. The state -> motion mapping is not traced.

## 6. Dante and the weapons

* The motion script opcodes 7, 8 and 0x24 drive an attack-collision
  controller (`script+0x110`: 16 slots of u16 id + shape pointer, 0x50-byte
  records), not effects. No player effect id is read from the script.
* Constant spawns: CPlDante V141, V142, P44, V144 (x13, not in the corpus),
  V910, V898, V594, V917; the shared player unit V817, V707, G426, V303, ...;
  `CPl000Shl02` V64 + G30..G34 and V71 + P15; `CPl000Shl03` V139, V184, V195;
  `CPl000Shl0a` V602, G318, G30, G31, G34; `CPlWp2Sword` G460, V770, V893,
  V894.
* `CEfcPub` builds some ids from a base plus an index (`?152`, `?240`,
  `?159`, `?163`, `?334` in `0x14008F730` and `0x140090853`). These are the
  likely source of the plwp_sword ids that have no constant (V56, V100, V140,
  V152, V159, V161, V199, V213, V238..V241, V244, V300, V332, V334).
* Player effects mostly answer hits (`CEfcPub`, `CHitMark`) or come from
  projectiles; the Reader has no hit. The swing trails are not FXBANK records
  (see `CAfterImage`, not traced).

## 7. Stage layout

* `# SET n BREAK` blocks are parsed by `0x14024A540` (`CStageSetBreak`):
  `model`, `bmodel` (model after breaking), `eff K id` + `epos` (effect while
  intact), `beff K id` (effect when broken; the `epos` after a `beff` sets its
  own position), `remain on`, `atk es/ps` (who can break it), `hit box`,
  `material`, `lockon`, `special`. Corpus: st000 has seven `beff V 122`, st002
  three `beff V 104` with `bmodel 4`. Breaking needs an attack, so the Reader
  could only offer it as a toggle (show `bmodel` and play `beff` at `epos`).
* `# SET n LIGHT` (`CStageSetLight`): an animated point light. `move loop0 /
  loop2`, `valid scr / obj`, `minus` (subtractive), `life` (ticks), `type`,
  start `spos` / `sforce` / `srange` / `srgb` to end `epos` / `eforce` /
  `erange` / `ergb`, `bwait`, `await`. st002 has one: (2200, 275, 1700),
  force 300, range 6000, black to (233, 228, 239) over 60 ticks.
* `# DOOR n` (in `st002cfg.pac`, parser `0x1401A9E3B`): `BoxIn` (trigger
  box), `BoxLoad`, `NextRoom`, `NextPosId`, `Mission >= / <=`, `Type BtnOn /
  BtnOff`, `FadeType`, `MapModel`, `MapLink`. Room links, no effect.

## 8. Particle and generator leftovers

* P classes 0 (Line00), 2 (Poly01), 5 (Line02), the Poly00 path `+0xFA !=
  1` and the Sprt00 camera-local path have no record in any corpus bank (all
  109 P records are classes 3, 1, 4). Nothing to compare a port against.
* G -> G: one corpus generator spawns a G. The Reader handles it by the same
  recursion as V; the emulator run did not cover the G creator.

## Next steps, by value

1. em000 family attack effects: done in the Reader (code 3 on body joint 1).
2. em000 family death: done in the Reader as a class event "Death" that
   starts the 0 / 5 / 10 / 15 / 20 / 25 schedule on the body joints. The
   body's sand tint (light colours through `vtbl+0x118..+0x130`, `+0x2F24..`
   flags) is not reproduced.
3. Stage: `beff` / `bmodel` toggle; the `SET LIGHT` light if the renderer gets
   a point light.
4. Nevan: trace the 39 states to script actions, as was done for CEm034.
5. CEfcPub id arithmetic and `CAfterImage` for the weapons.
