# DMC3 enemy motion scripts and motion resource table

Date: 2026-09-24
Executable: `dmc3.exe`, SHA-256
`e454272ed0fb0247fcbcf300e5d55d7a3e96d50b89b9ffaff81bb978dcbdd082`.

Samples (analysed only, not committed):

| Archive | SHA-256 |
| --- | --- |
| `em028.pac` | `47636f39d60ade101455405ce892a84654ab06cfa34ff20344e25f790ae88fe2` |
| `em000.pac` | `10ab4cd0cc83abe4ca4b5ee98e9cc97f953db6f86dac7fddcb4aed540ba1a87c` |
| `em006.pac` | `4296f2f1131095fe04472871e971b79415e9b720549ba5fa77c4a00a5f032173` |
| `em007.pac` | `7b8e2767f6d96c5d07f808527ec79ece782a50bd37b819f563069b5b2ee8175e` |
| `pl000.pac` | `b73b6e8ee2f2088a088f4215a5a13a50700fb4e4f0bef54dc0bcce11ae7b9c5d` |

Extends `dmc3-player-motion-script-2026-09-24.md`: the enemy script slots use
the same file format and interpreter.

## 1. Bind — `0x1400594B0` — EXE_CONFIRMED

`bind(obj, file, object_index r8, mode r9w, [motion PAC array], [actor
motion controller], [+0xA0], [+0x78], [+0x110])`:

```text
obj+0x08 = file + u16[file+0]      A: scripts
obj+0x10 = file + u16[file+2]      B: motion resources
obj+0x18 = file + u16[file+4]      C (0xFFFF in every sample)
obj+0x60 = r8 (object index)       obj+0x64 = mode
mode 0:  obj+0x20 = A + u16[A]     obj+0x30 = B + u16[B]
mode 1:  obj+0x20 = A              obj+0x30 = B
obj+0x88 = entries of obj+0x20 before 0xFFFF (bank count)
```

The player binds mode 0 (`0x1401EF040`, r9 = r12w). Enemies bind mode 1, with
two objects (r8 = 0 and 1), e.g. em028 `0x140131037` / `0x1401310AF`, and the
em000-family classes `0x1400982D9`, `0x14009D72F`, `0x1400A26CB`, ….

## 2. Play and motion resources — EXE_CONFIRMED

`0x14005A290(obj, bank, action)`:
- `sub = banks + u16[banks + 2·bank]`;
- `script = sub + u16[sub + 2·action]`;
- then run `0x140058FE0`.

Opcode 1 (`01 ?? ?? ?? bank action ff 7f`) hands its bytes to `0x14005A360`,
which reads table B:

```text
r10 = B + u16[B + 2·bank];  rec = r10 + u16[r10 + 2·action]
rec[0] = count; entries at rec + 2, 6 bytes each:
  +0 object (matches obj+0x60)   +1 loop: 0 once, 1 loop, 2 ask the actor (vtbl+0xB0)
  +2 flag (==1 -> obj+0xB6)      +4 u16 id
pac = motionArray[id / 100]      (obj+0x68, from the bind)
mot = pac + u32[pac + 8 + 4·(id % 100)]   (slot count checked)
actor->vtbl+8(object, mot, …)    then vtbl+0x88 (loop) and vtbl+0x98
```

So an action number is not a MOT number. For example:
- player bank 3, action 2 plays id 300 (`pl000_00_3` slot 0);
- em028 actions 1–9 all play MOT 0 (looped idles).

Every bank and every record list ends at `0xFFFF`. A bank can have more
script actions than records: em000 bank 0 actions 78–92 have none.

## 3. Motion PAC arrays per class — EXE_CONFIRMED where listed

The array is built by each class before the bind:

| Class init | Array | Groups (id / 100) |
| --- | --- | --- |
| em028 `0x140131037` | actor+0xF08 | 0 → slot 2, 1 → slot 3 |
| em000 family `0x1400982D9` | actor+0x670 | 0 → slot 35 (written at `0x140097C8D`) |
| `0x1400AB350` | +0x38E0 | 0, 1 → slot 37 |
| `0x1400B6525` | +0x4968 | 0, 1, 2 → slot 20 |
| `0x14013C5FF` | +0x1130 | slots 11, 12, 17, 21, 25 |
| `0x140170BEB` | +0x5410 | slots 2 … 7 |

The script slot itself is the class's constant slot read: em028 `[pac+0x30]`
= slot 10, and the em000 family `[pac+0xA0]` = slot 38.

**Data check.** Table B ids per group against the MOT PACs in the samples:
- em028: every group-0 id (0, 10–17, 24, 25, 30, 40–51) is populated in slot
  2 (52 slots).
- em000: group 0 = 0–70 matches slot 35 (71 slots).
- em000, em006, em007: bank 1 plays ids 100–109 and bank 2 plays 200–204;
  bank *b* uses group *b*.
- Groups 1 and 2 of the em000 family are not stored at `+0x678` or `+0x680`
  by the code read so far. The data would fit slot 37 (10 MOTs) for group 1.
  No archive slot covers group 2. Open.

## 4. Other enemy slots next to the script

In `em028 0x140130F7E`, slots 11 and 12 go to `0x14005C260(obj +0x6580,
slot 11, slot 12, 2, …)`. In the em000 family `0x14009823F`, slots 39 and 40
go to the same function. These are the 96-byte (`06 00 00 00 02 03 01 00`)
and 1840-byte (`02 00 00 00 …`) tables. See the next note.

## 5. Viewer rule

Native Reader:
- identifies the file by structure in both modes;
- lists action → MOT id, loop flag and weapon states;
- in an assembled enemy PAC, labels each MOT with the actions that play it.

It binds groups with the EXE table where one is known. Otherwise it uses the
first unused MOT PAC whose slots cover the group, marked "data". It never
reuses a pack for a second group. The player's weapon states now go through
table B: a MOT (bank *N*, slot *k*) takes the states of the action in bank
*N* that plays id `N·100 + k`.
