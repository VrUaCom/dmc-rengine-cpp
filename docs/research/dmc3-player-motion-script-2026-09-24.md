# DMC3 player motion script (pl000.pac slot 5) and weapon attach states

Date: 2026-09-24
Executable: `dmc3.exe`, SHA-256
`e454272ed0fb0247fcbcf300e5d55d7a3e96d50b89b9ffaff81bb978dcbdd082`.
Sample (analysed only, not committed): `pl000.pac` (SHA-256
`b73b6e8ee2f2088a088f4215a5a13a50700fb4e4f0bef54dc0bcce11ae7b9c5d`), slot 5
(73 120 bytes), together with the user's `pl000_00_N.pac` banks.

Builds on `dmc3-player-weapon-attachment-2026-09-23.md`, which covers the
records and the state-0 table.

## 1. Who picks the weapon's attach record

The sword update `0x140231680` (CPlWpSword) chooses the attach state like
this:
- If the weapon is active (`0x1401FD5C0`), it uses the state byte
  `player+0x39C3`. Otherwise it uses state 1.
- The chosen state goes to `0x1401FDC90(weapon, weapon+0xAC0, state)`. That
  function reads record pointer `[weapon+0x128][state]` and sets the joint
  (`+0x114`), the pose branch (`+0x11A`) and the offset matrix.

The other weapon classes call the same readers (`0x1401FDC90`,
`0x1401FD8F0`, `0x1401FDA80`).

`player+0x39C3` has a single writer, `0x1401F01F0`. For channel 0 of the
player's script object (`player+0x6A70 + form·0x240`), it reads byte 1 with
`0x140059350`, then:

```text
b = obj[0x91 + 5·channel + k]      (0x140059350, channel 0, k = 1)
if b != 0:  player+0x39C3 = b & 0x3F    player+0x3A10 = b & 0xC0
```

A value of 0 leaves the state unchanged.

## 2. The script file

Player init (`0x1401EF461`) loads the file from resource
`0x1401B82C0(list, type 0, character, slot 5)`, which for Dante is
**pl000.pac slot 5**. It binds the file to the script objects with
`0x1400594B0` (mode 0).

**Bank table.**
- Let `T = u16[0]` (6 in the sample).
- The bank list is at `T + u16[T]`. It is a list of u16 offsets, terminated
  by `0xFFFF`, measured from the list's start.
- The sample has 34 banks, which matches the 34 motion files per character in
  `0x1405B0F30`. Bank N is `motion\pl000\pl000_00_N.pac`.
- pl000.pac slots 2, 3 and 4 are byte-identical to `pl000_00_0`, `_1` and
  `_2`.

**Scripts.** The play entry `0x14005A290(obj, bank, motion)` finds the script
with `sub = banks + u16[bank]` and `script = sub + u16[sub + 2·motion]`. It
clears the channels and runs the interpreter.

## 3. Interpreter (`0x140058FE0`)

Opcodes dispatch through the byte table `0x140059240` and the jump table
`0x140059204`:

| Op | Bytes | Meaning |
| --- | --- | --- |
| 0 | 6 | `[00, ?, frame u16, flag, ?]`. Ends the block. The next block runs once the motion frame is past `frame` (`0x140059C2F`); `0x7FFF` means the motion end. |
| 1 | 8 | Play MOT (`0x140059950`). Byte 4 is the bank and byte 5 the MOT index; each script starts with this. |
| 2 | — | Backward jump by s16 `[+2]` (loop). |
| 3 | 6 | Channel 0 bytes, copied to `+0x91..+0x95`. Byte 2 holds the weapon state. |
| 4 | 6 | Channel 1 bytes, copied to `+0x96..+0x9A`. |
| 5 | 2 | Clear both channels. |
| 6, 7, 8, 33, 35 | 2 | |
| 16–31 | `(op−15)·2 + 4` | Conditional on the channel id (`0x1400596F0`). |
| 32, 36 | 4 | |
| 34 | 6 | |

**Example: bank 3 (Rebellion), MOT 3.**

```text
01 00 00 00 03 03 ff 7f     play bank 3 MOT 3
11 01 ff ff 00 00 01 00     conditional block
03 80 00 00 02 00           channel 0 (state byte 0 = unchanged)
00 00 04 00 00 00           wait for frame 4
03 80 02 00 00 00           state 2 (right hand)
00 00 0a 00 00 00           wait for frame 10
03 80 82 00 00 00           state 2 with flag 0x80
```

## 4. States in the sample banks

| Bank | States used |
| --- | --- |
| 0, 1 (common) | 1, which is the back. MOT 15 of bank 0 also uses 2. |
| 3 (Rebellion) | 1, 2 and 3 |
| 13 | 1–4 |
| 10, 12, 14–17, 19 | 22–52: states of the guns and other weapons. These records are empty in the sword table. |

**CPlWpSword states** (table `0x14058C010`):

| State | Record |
| --- | --- |
| 0, 1 | Back: joint 3, T(−14.5, 32, −14) |
| 2 | Right hand: joint 9, T(−7.6, −3, −1), R 0 |
| 3 | Left hand: joint 13, T(7.6, −3, −1) |
| 4 | Joint 9 with pose branch 1 |
| 5–23 | Empty (branch 255) |

**CPlWp2Sword** keeps two parts per record (joints 13 and 9 in hand). Its
states 3–23 hold both blades in the hands with different spins.

**CPlWpGuitar** states 3 and up are play poses (branch = state, joint 0).
They go through the class's own pose code.

## 5. Viewer rule

Port the timeline of channel-0 byte 2 for the playing MOT:
- Look up the MOT by its bank and index.
- On each frame, set the equipped weapon's record to `[table][state]`.
- Keep the previous record when the state is 0, empty (branch 255) or a
  special pose branch (2 or more).

## 6. Open

- The flag bits `0xC0` of `+0x3A10`, and channel 1 (opcode 4).
- Conditions on the active weapon, such as weapon switching or gun banks
  moving a melee weapon to its back.
- The opcode 0 flag byte `+0xB4`, which changes the resume test.
