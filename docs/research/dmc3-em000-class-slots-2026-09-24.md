# em000.pac: slots per enemy class (CEm000–CEm005)

Date: 2026-09-24
Executable: `dmc3.exe`, SHA-256
`e454272ed0fb0247fcbcf300e5d55d7a3e96d50b89b9ffaff81bb978dcbdd082`.
Sample (analysed only): em000.pac, SHA-256
`10ab4cd0cc83abe4ca4b5ee98e9cc97f953db6f86dac7fddcb4aed540ba1a87c`.

The class init is vtable slot 53. It was found from the script binds and the
`.pdata` primary function, and the RTTI vtables confirm it:

| Class | Vtable | Init |
| --- | --- | --- |
| CEm000 | `0x1404C9AF8` | `0x140097B40` |
| CEm001 | `0x1404CA1A8` | `0x14009CF70` |
| CEm002 | `0x1404CA4A0` | `0x1400A1D80` |
| CEm003 | `0x1404CA790` | `0x1400A6BD0` |
| CEm004 | `0x1404CAA90` | `0x1400A85E0` |
| CEm005 | `0x1404CADA8` | `0x1400AABD0` |
| CEm005Shl00 | `0x1404CB0F0` | `0x1400AC6B0` (slot 8) |
| CEm006 | `0x1404CB610` | `0x1400AF720` |
| CEm007 | `0x1404CBAC8` | `0x1400B2090` |
| CEm008 | `0x1404CBDF0` | `0x1400B57A0` |

The slots each init reads, in code order. Model loads are `vtbl+0x40` /
`+0x50` / `+0x150`; the other targets are the loaders called.

| Class | Slots read |
| --- | --- |
| CEm000 | 41 fx bank · body 1, tex 0, motions 35 · cloth 3 (tex 2) + CLT · 29 26 25 · 39 40 collision · 38 script |
| CEm001 | body 5 · cloth 7 (6) · 31 28 25 · same 35 / 39 / 40 / 38 / 41 |
| CEm002 | body 8 · cloth 10 (9) + 12 (11) · 29 26 25 |
| CEm003 | body 13 · cloth 15 (14) + 17 (16) · 30 27 25 |
| CEm004 | body 18, motions 35 · part 34 (tex 32) with **its own motion PAC 36** |
| CEm005 | body 19 (tex 0), motions **35 + 37** · cloth 3 (2) · 33 32 |
| CEm005Shl00 | model 33 (tex 32), motions **37** · 39 40 · 38 |
| CEm006 (em006.pac) | fx 41 · body 1, motions 35 35 36 36 36 · 6 3 2 · CLT 6 · 26 29 25 · 39 40 · 38 |
| CEm007 (em007.pac) | fx 41 · body 1, motions 35 · 26 29 25 · 39 40 · 38 |
| CEm008 | fx 24 · 16 1 0 19 20 20 · six cloth pairs 3/2 … 13/12 · 17 18 · 22 23 collision · 21 script |

The em000.pac classes share:
- body motions from slot 35;
- script 38, collision 39/40 and effect bank 41.

The extra motion PACs belong to single classes: 36 to CEm004's part, 37 to
CEm005 and its shell. em000.pac holds no SHW; none of these inits calls the
SHW builder `0x14031FD30`.

## Motion script channels and effects

The interpreter `0x140058FE0` spawns no effects. Its opcodes only play MOTs,
wait for frames, write channel bytes (3 and 4), end collision shapes (32),
set the collision parameter (34, which stores a u32 at `handle+0x140`) and
branch. In the samples:
- em000's scripts never write a channel;
- em028's scripts write channel-0 byte 1 = 1…10 at fixed frames. Examples:
  action 22 frame 82 → 1; action 35 frames 107 and 240 → 8 and 9.

These are event numbers that CEm028's code reads. Effects are therefore
started by class code, from these events or from the class's own frame tests.

Open:
- the CEm028 event handler and the effect ids it spawns;
- the E / G / P / V formats.
