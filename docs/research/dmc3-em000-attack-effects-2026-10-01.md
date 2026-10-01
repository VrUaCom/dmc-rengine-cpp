# DMC3 em000 family: AI commands -> script actions -> effect events

Date: 2026-10-01
Executable: `dmc3.exe`, SHA-256
`e454272ed0fb0247fcbcf300e5d55d7a3e96d50b89b9ffaff81bb978dcbdd082`.
Sample (analysed only, not committed): em000.pac, slots 35 (motions) and 38
(script). Tools: `research/exe/fx/emu/comemu.py`, `action_lengths.py`,
`evt.py`. Builds on `dmc3-effect-triggers-2026-10-01.md`.

## Status

| Claim | Status |
| --- | --- |
| `CComEm000` (vtable `0x1404C6A18`) holds the commands as (update, start) slot pairs: slot 2k+1 update, 2k+2 start | confirmed (disassembly; every pair below runs) |
| Interface slots used by commands: `+0x20` play(cmd), `+0x28` play2(cmd), `+0x40` motion frame, `+0x48` action finished, `+0x80` event | confirmed (CEm000 interface vtable `0x1404C9CD0`: `0x1401C3030`, `0x1401C30B0`, `0x1401C2160`, `0x1401C2450`, `0x1401C3130`) |
| Command tables of CEm000..CEm004 are identical (cmd n -> bank 0 action n; play2 table `0x1405A31E8`) | confirmed (table bytes) |
| (action, frame, code) pairs below | confirmed by emulation; context-free only for the frame-gated code 3 |
| Gates past the MOT end (slot 207, slot 233) | open: they fire only if the motion frame keeps counting after the end |

## Method

`comemu.py` runs each pair with a fabricated command object: `+0xE0` points to
an interface whose vtable slots are stubs, `+0x20` / `+0x28` record a play and
restart the clock, `+0x40` returns the frames since the last play, `+0x48`
(`0x140059590` on the script player: "action finished") is true before the
first play and once the frame reaches the action's MOT length (from
`action_lengths.py`), `+0x80` records codes; the command's own vtable is
stubbed and recorded (hand-over to the scheduler, e.g. the teleport at frame
45 of slot 191). One update call is one tick. Other interface queries return 1,
so branches that depend on the player or the arena take their "true" side;
events at action frame 0 come from such context and are not tied to actions.

`+0x48` follows `0x1401C2450`: `add rcx, 0x2BC0; jmp 0x140059590`, which
returns script player byte `+0xBA` (set while the script runs a block, cleared
at its wait). `+0x40` is `0x1401C2160`: the float at `+0x1B8` of the body
motion object `obj+0x730+0x500`.

## Commands with plays or events (CEm000 table, lengths from em000.pac)

"p2" is a play through the second table. Events are `code @action:frame`.

| Slot | Update / start | Plays (action @ tick) | Events (code @ action frame) |
| --- | --- | --- | --- |
| 103 | `0x140065a00` / `0x140068ec0` | p2 17 @0 |  |
| 105 | `0x140066cc0` / `0x140077330` | p2 17 @0 |  |
| 107 | `0x1400737a0` / `0x140077040` | p2 58 @0 | 0x68 @58:0 |
| 109 | `0x140065fd0` / `0x140068c50` | p2 60 @0, p2 26 @0, p2 27 @35, p2 62 @154, p2 21 @262 | 0x68 @60:0, 0xd @26:0, 0x11 @27:0 |
| 111 | `0x140066600` / `0x140068de0` | p2 28 @0 | 0xe @28:0 |
| 113 | `0x140066990` / `0x140082360` | p2 28 @0, p2 21 @51 |  |
| 115 | `0x140066d90` / `0x140082020` | p2 41 @0, p2 21 @144 | 0xa @41:0 |
| 117 | `0x14007f5c0` / `0x140082210` | p2 22 @0 |  |
| 119 | `0x140066a80` / `0x140081ee0` | p2 24 @0, p2 29 @0, p2 62 @76, p2 21 @143 | 0xa @29:0 |
| 121 | `0x140065b50` / `0x140082150` | p2 29 @0 | 0xe @29:0 |
| 123 | `0x14007f610` / `0x140077270` | p2 30 @0, p2 62 @84, p2 21 @155 | 0xe @30:0 |
| 125 | `0x140073450` / `0x140081f80` | p2 37 @0, p2 63 @32, p2 20 @103 |  |
| 127 | `0x14007f570` / `0x140068bc0` | p2 33 @0 |  |
| 129 | `0x140065dd0` / `0x140068330` | 42 @0 | 0x6 @42:0 |
| 137 | `0x140062d30` / `0x140081eb0` | 48 @0 |  |
| 139 | `0x1400632c0` / `0x140076e40` | 49 @0 |  |
| 141 | `0x14006efd0` / `0x140068a70` | 75 @0 | 0x13 @75:0 |
| 143 | `0x14007a470` / `0x140081bb0` | 70 @0, 50 @34 | 0xc @50:0 |
| 149 | `0x1400830a0` / `0x140069b60` | 9 @0 |  |
| 151 | `0x1400830a0` / `0x140077df0` | 10 @0 |  |
| 153 | `0x14006a290` / `0x140068f70` | 75 @0 | 0x13 @75:0 |
| 161 | `0x1400678d0` / `0x140069850` | 13 @0 |  |
| 163 | `0x1400759f0` / `0x140069730` | 14 @0 |  |
| 165 | `0x1400759f0` / `0x140069930` | 52 @0 | 0x0 @52:0 |
| 167 | `0x140067d30` / `0x140069900` | 53 @0 | 0x2 @53:81, 0x2 @53:82, 0x2 @53:83, 0x2 @53:84, 0x2 @53:85, 0x2 @53:86, ... |
| 171 | `0x140063430` / `0x140069570` |  | 0x9 @-:0, 0x9 @-:1, 0x9 @-:2, 0x9 @-:3, 0x9 @-:4, 0x9 @-:5, ... |
| 177 | `0x140078480` / `0x140069880` | 55 @0 |  |
| 179 | `0x140075ed0` / `0x140069dd0` | 46 @0, 56 @9, 57 @32 |  |
| 181 | `0x14006b190` / `0x140082ce0` | 9 @0 |  |
| 183 | `0x140083240` / `0x140077bf0` | 5 @0 |  |
| 189 | `0x14006ae00` / `0x140069d90` | 68 @0 | 0x3 @68:40, 0x3 @68:50 |
| 191 | `0x14006aa70` / `0x140069d90` | 68 @0 | 0x3 @68:40, 0x3 @68:50 |
| 193 | `0x14006b190` / `0x140069a40` | 62 @0 | 0x3 @62:40, 0x3 @62:50 |
| 195 | `0x140082ff0` / `0x140069ad0` | 63 @0, p2 33 @71 |  |
| 197 | `0x140077f00` / `0x140068e90` | 64 @0, p2 37 @0 |  |
| 203 | `0x140068090` / `0x140077ce0` | 30 @0 |  |
| 205 | `0x1400765d0` / `0x140077ec0` | 66 @0 |  |
| 207 | `0x14006bbe0` / `0x140069ed0` | 68 @0 | 0x3 @68:100, 0x3 @68:110 |
| 217 | `0x14006a430` / `0x140082570` | 45 @0, 78 @170 | 0x6 @45:0 |
| 219 | `0x14007ff40` / `0x140069070` | 5 @0, 39 @62 |  |
| 225 | `0x140067500` / `0x140069030` | 81 @0 |  |
| 227 | `0x140067070` / `0x140068fd0` | 81 @0, 44 @0 |  |
| 229 | `0x140066f10` / `0x140069110` | 83 @0, 82 @254 | 0x3 @82:76 |
| 231 | `0x140067120` / `0x140082620` | 84 @0 | 0x3 @84:180 |
| 233 | `0x140067200` / `0x140069e10` | 87 @0 | 0x3 @87:170 |
| 235 | `0x14006b900` / `0x140082fb0` | 66 @0 | 0x3 @66:30 |
| 237 | `0x14006b520` / `0x140082f80` | 85 @0, 87 @104 | 0x3 @85:100, 0x3 @87:30 |
| 243 | `0x140062ea0` / `0x140069190` | 81 @0, 89 @0 |  |
| 251 | `0x14006c9b0` / `0x140069f00` | p2 58 @0, p2 29 @0, 62 @76 | 0x22 @62:0 |
| 253 | `0x14006bf70` / `0x14006a1d0` | 91 @0, p2 26 @0, p2 27 @35, p2 62 @154 | 0x2b @91:0, 0xd @26:0, 0x11 @27:0, 0x22 @62:0 |

MOT lengths of the actions with code 3 (em000.pac slot 35): 62 -> MOT 55, 109
frames; 66 -> MOT 60, 124; 68 -> MOT 46, 60; 82 -> MOT 62, 124; 84 -> MOT 64,
300; 85 -> MOT 65, 232; 87 -> MOT 47, 142.

## Result: code 3 by action (what the Reader plays)

| Script action (bank 0) | Code 3 at frame | From slot |
| --- | --- | --- |
| 62 | 40, 50 | 193 |
| 66 | 30 | 235 |
| 68 | 40, 50 | 189, 191 (207's 100 / 110 are past the 60-frame MOT) |
| 82 | 76 | 229 (after action 83 finished) |
| 84 | 180 | 231 |
| 85 | 100 | 237 |
| 87 | 30 | 237 (233's 170 is past the 142-frame MOT) |

Code 3 spawns E42 and V42 (V42 = P93, E30, E37) scaled x2 at object 1 with
`+0xD4 = 1` (position only). Object 1 is body joint 1, not the weapon: the
array `obj+0x6D8` is the body model's joint list (`0x14030F850`, see
`dmc3-effect-triggers-2026-10-01.md` section 3). On the data it is a sand whirl around the enemy;
slot 191 teleports the enemy at frame 45 (`self +0xC8 / +0xD0`, random offset
+-600 on x / z).

## Native Reader

`modules/motion/enemy_effects.cpp` (branch `NR-Luna-v73`): profile "em000"
for archive positions CEm000..CEm004, bindings E42 / V42 (slot 41), and a
Script Play step that emits a spawn when the current script action (slot 38)
passes a frame of the table, then follows body joint 1's translation for 90
ticks. The same module plays the death schedule (codes 0x69, 0xC8..0xCE on
body joints 1..21) from a class event "Death" (control code 0x3E7). Test
`enemy_effects_test`.
