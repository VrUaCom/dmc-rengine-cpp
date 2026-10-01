# DMC3 P records: CPtclSprt00, CPtclPoly00, CPtclLine01

Date: 2026-10-01
Executable: `dmc3.exe`, SHA-256
`e454272ed0fb0247fcbcf300e5d55d7a3e96d50b89b9ffaff81bb978dcbdd082`.
Source: carried over from Native Reader (`VrUaCom/DMC-Native-Reader`, branch
`NR-Luna-v73`, note `dmc3-particle-p-records-2026-10-01.md`; port `modules/particle_sprt.cpp`, test `particle_test`) so the reverse is kept with the engine notes. Game
files were analysed locally and are not committed; only ids, offsets and
addresses are quoted. Tools: `research/exe/fx/`.

## Status

| Claim | Status |
| --- | --- |
| P classes 3 / 1 / 4: spawn, update, colour track, blend, draw composer | confirmed: emulated runs of `dmc3.exe` replayed by a C++ port; all 109 corpus records match for 12 ticks (8 expire earlier and match until then) |
| Class byte at definition offset `body[0]` + 1 | corrected (an early "class 2" count read a fixed `+0x21`) |
| Table `0x1405D09E0` = primitive layout, not blend | corrected |
| Blend = GS ALPHA bits of `def+0x65` | confirmed (corpus values 0x44 / 0x48 / 0x41) |
| w = 2 / half-size result of an emulator without static initializers | rejected (the mask at `0x1405D9F30` is written by `0x140001920`) |
| Classes 0 / 2 / 5, Poly00 `+0xFA != 1`, Sprt00 camera-local path | candidate layouts only: no corpus record |


---

Scope: every P record of the corpus (109 records of the uploaded PACs: em000,
em006, em007, em028, em034, plwp_sword, plwp_2sword, st002_effect) is drawn.
The definition offset is `body[0]` (0x10, or 0x20 for the records with three
layers); the class byte follows the definition's version byte.

| Class | Name | Records | Primitive (`obj+0xF0` kind) |
| --- | --- | --- | --- |
| 3 | CPtclSprt00 | 83 | textured camera-facing quad, 4 vertices (kind 2) |
| 1 | CPtclPoly00 (path `+0xFA == 1`) | 12 | untextured free triangle, 3 vertices (kind 1) |
| 4 | CPtclLine01 | 14 | untextured two-segment streak, 4 vertices (kind 0) |

Classes 0 (Line00), 2 (Poly01), 5 (Line02) and the Poly00 path `+0xFA != 1`
have vtables and code (`0x1404E25D0`, `0x1404E2B18`, `0x1404E2878`) but no
record in the corpus; they are not ported. (An early count of "class 2" came
from reading the class byte at a fixed `+0x21`; three-layer records start 0x10
later.) The table at `0x1405D09E0` indexed by the kind is the primitive layout
(2 / 3 / 4 vertices), **not** a blend table.

## How this was checked

`dmc3.exe` functions were run in a Unicorn emulator on fabricated objects (`research/exe/fx/emu/`: `ptcl2.py`, `gt.py`, `gtall.py`, `gt_poly.py`).

* `app/src/test/native/particle_test.cpp` replays the C++ port against
  emulated runs on synthetic records (no game data, embedded in
  `particle_truth.inc`): three classes x two gravity modes, 12 / 16 particles,
  emitter plus two layers, every colour ease mode, both blends, 14 updates,
  and the draw matrices of the emitter and both layers applied to the packet
  vertices. A changed constant fails it.
* Differential run on **all 109 real records** (`emu/realdump.py` dumps; the C++ comparison harness lives with the Reader port): spawn state
  fed from the emulator, 12 updates compared (transforms, colours, velocities,
  positions / vertices, layers). 101 match for every frame; the other 8 live
  fewer than 12 ticks and match until they expire (the emulator never expires).

## Common definition (D = record + body[0] + 0x10, 0x130 bytes; layer 0xB0 bytes)

`record + 0x00` u32 version 2; `body[0]` definition offset, `body[4]` the
pointer-array slot, `body[8 + 4i]` the body offset of layer i (`0x140312DC0`).

| D offset | Meaning |
| --- | --- |
| +0x00 / +0x01 | 2 / class |
| +0x02 | name ("ec021-40p1"); Sprt00 `es400-00p0` takes the camera-local path (`obj+0xF5 = 1`), not ported |
| +0x20 / +0x30 / +0x40 | emitter translation / rotation (rad, Rz*Ry*Rx) / scale, 4 floats each |
| +0x52 / +0x58 / +0x5E | per-tick s16 x3: translation x 1/16, rotation x 2pi/65536, scale x 1/4096 (`0x140312260`) |
| +0x64 | u32: byte 1 (`+0x65`) is the GS ALPHA register bits, see Blend |
| +0x68 | life, ticks (s32) |
| +0x6C | colour track: 4 keys x 20 bytes `{s16 length, s16, 4 RGBA groups}`; `+0xBC` enable, `+0xBD` loop, `+0xBE` ease |
| +0xC0 | layer count (u16) |

Layer: +0x00 / +0x10 / +0x20 translation / rotation / scale, +0x30 / +0x36 /
+0x3C the same s16 motions, `+0x45` blend, +0x48 colour track (enable +0x98,
loop +0x99, ease +0x9A). A layer re-draws the emitter's particles with its own
local transform, colour track and blend.

Class fields (all s16 / 16 unless noted):

| | Sprt00 (3) | Poly00 (1) | Line01 (4) |
| --- | --- | --- | --- |
| push x3 | +0xE2 | +0xE8 | +0xE8 |
| bias x3 | +0xEE | +0xEE | +0xEE |
| spread x3 | +0x100 | +0xF4 | +0xF4 |
| hollow x3 (/ 32) | +0x116 | +0x100 | +0xFC |
| friction x3 (float) | +0xF4 | +0x108 | none |
| friction decay x3 (float) | +0x11C | +0x114 | none |
| gravity (float) | +0x10C | +0xE0 | +0xE0 |
| world gravity flag | +0x110 == 1 | +0xFE == 1 | +0xFA == 1 |
| size | half w / h u16 +0x108 / +0x10A | corner spread +0xFC | segment lengths +0xE4 / +0xE6 |
| texture | A id +0x106 (A's texture) | none | none |
| frame | +0x128 == 1: one random A frame | - | - |
| particles | 12 | 16 | 12 (object count 24, loop count / 2) |

## Runtime (verified by the emulator replays)

* Burst: particles spawn once (`0x140236D10`, `0x140234CE0`, `0x1402334A0`):
  three uniform draws r in [-0.5, 0.5); position = r x spread; a point inside
  the hollow box (`|p| <= hollow` on every axis) is pushed out on one axis
  (`0x140312450`: random start of six faces, first face whose hollow edge is
  below spread/2: `s/2 * (r + .5) + (.5 - r) * hollow`, sign by face);
  velocity = p x (push / max(spread, 1)) + bias - p.
* Poly00: v2 = p, v0 and v1 = p + six more draws x corner spread; the packet
  vertices are the state (they advance from the previous tick's packet).
* Line01: v1 = p, v0 = v2 = p + dir x L1, v3 = v0 + dir x L2 with dir =
  normalise(velocity) (`0x140330390`, zero for zero); update moves the pivot
  v0 by velocity x dt and rebuilds v1 = v0 - dir x L1, v3 = v0 + dir x L2 from
  the old velocity.
* Update (dt = 1; `0x1402374F0`, `0x140235540`, `0x140233A30`): life -= 1
  (endless below -10000), dies when negative; colour track; translation /
  rotation (wrapped to +-pi) / scale integration; per particle, Sprt00:
  friction, then gravity, then position += velocity; Poly00: vertices +=
  velocity, then friction, then gravity; Line01: pivot += velocity, gravity,
  no friction. Friction `0x140314080` pulls each axis towards zero, never past
  it, and decays after its value is read. A world-space gravity vector goes
  through the inverse world rotation (translation zeroed).
* Colour track `0x1402D30E0` / lerp `0x1402D3200`: segment i lerps key i to key
  (i + 1) & 3 with p = 1 - remaining / length (bytes truncated); the track
  lerps min(class groups, table[ease]) RGBA groups (class groups: Sprt00 4,
  Poly00 3, Line01 2; table at `0x1405CED70` = 1, 2, 4). Ease 0 broadcasts
  group 0 to all four, ease 1 gives (g0, g1, g1, g0), ease 2 keeps four. After
  segment 2 the track stops (or loops). Vertex index w picks the group: quad
  BR 1, BL 0, TL 2, TR 3; triangle 0, 1, 2; line 0 (pivot) and 1 (end).
* Draw `0x140312F10`: local = scale rows x Rz*Ry*Rx with a translation row, then
  x world. The translation helper `0x140031200` adds the vector to row 3 and
  keeps the row's w through a mask (0, 0, 0, -1) that a static initializer
  (`0x140001920`) writes at `0x1405D9F30`; the file image holds zeros there, so an
  emulator that skips the static initializers sees w = 2 (an early version of
  this port did, and halved every size). The emulator now runs the
  6260 constant-copy initializers first.
* Sprt00 vertices (non-camera-local path): corner x camera^-1 x world^-1
  (rotation) x emitter-rotation^-1, plus the particle position, so the emitter's
  own quad faces the camera; a layer with another rotation tilts its copy.
  UV table at `obj+0x164` is (U0,V0) (U1,V0) (U0,V1) (U1,V1) for w = 0..3: the
  image is upright with the corner y axis pointing to the image bottom; UV =
  cell x/W .. (x+w-1)/W. Poly00 and Line01 vertices are in emitter space and
  go through the local matrix as they are.
* Texture: Sprt00 only; the T id is `def + 0xE0`, equal to the A record's
  texture in all 13 checked records.

## Blend

`def + 0x65` (layer `+0x45`) holds GS ALPHA bits (A, B, C, D) = (b0-1, b2-3,
b4-5, b6-7): `0x44` is (Cs - Cd) x As + Cd, normal alpha; `0x48` is Cs x As +
Cd, additive. Corpus (emitters and layers): 0x44 (alpha) 122 times, 0x48 (additive) 56, 0x41
2 (drawn as alpha). Byte 0 of the same u32 (0x01 / 0x03) goes into the
packet state; its meaning is not decoded.

## What the Reader does

`particle::Simulation` (`modules/particle_sprt.cpp`) replays the burst from
tick 0 to the entry's age every frame (the state is a pure function of age and
record). `append_effect_particles` (resource_session.cpp) turns the primitives
into `EffectSprite`s: oriented corners, per-corner colour (Gouraud), additive
or alpha, solid (no texture) for triangles and lines; lines are widened to a
few screen pixels. Sprt00 takes its UV from the A frame.

## Approximations

* The random draws come from a fixed per-record seed: the retail generator
  (`0x140059390`, four-word LCG mix) is shared and unseeded by the effect.
* The camera inverse is the Reader's mirrored camera basis; the world inverse
  uses the world rotation with the rows normalised (the EXE assumes an
  orthonormal matrix).
* `es400-00p0` (Sprt00 camera-local path, no corpus record) is drawn through the
  normal path. Line width is a screen-space choice (PS2 lines are 1 px).
