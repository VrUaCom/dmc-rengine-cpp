# DMC3 em000.pac: five enemy classes, cloth and weapon attachment

Date: 2026-09-23
Executable: `dmc3.exe`, SHA-256
`e454272ed0fb0247fcbcf300e5d55d7a3e96d50b89b9ffaff81bb978dcbdd082`.
Sample (analysed only, not committed): `em000.pac`, 2 628 368 bytes,
SHA-256 `10ab4cd0cc83abe4ca4b5ee98e9cc97f953db6f86dac7fddcb4aed540ba1a87c`
(PNST, 42 slots). Contract: `profiles/dmc3/em000_family_contract.hpp`.

## 1. Classes

CEm000-CEm004 share the base vtable entries (update `0x1401C2460` family,
draw `0x1401C6FB0`) and differ in their init, reached through the secondary
vtable at `this+0x60`, entry `+0xE8` (`r15`/`r13` = `this+0x60` inside):

| Class | vtable | init | body | cloth (model <- .clt, joint) | weapon v0-1 / v2-3 | weapon PTX |
| --- | --- | --- | --- | --- | --- | --- |
| CEm000 | `0x1404C9AF8` | `0x140097B40` | 1 | 3 <- 2, joint 14 | 26 / 29 | 25 |
| CEm001 | `0x1404CA1A8` | `0x14009CF70` | 5 | 7 <- 6, joint 14 | 28 / 31 | 25 |
| CEm002 | `0x1404CA4A0` | `0x1400A1D80` | 8 | 10 <- 9, joint 8; 12 <- 11, joint 12 | 26 / 29 | 25 |
| CEm003 | `0x1404CA790` | `0x1400A6BD0` | 13 | 15 <- 14, joint 14; 17 <- 16, joint 14 | 27 / 30 | 25 |
| CEm004 | `0x1404CAA90` | `0x1400A85E0` | 18 | - | 34 | 32 |

Every class also reads slot 41 (effect bank, `0x1402C04C0`), 35 (motion bank
bound to the body joint table) and 38-40 (effect tables). The .clt slots are
text beginning `;em000_01.clt`. Slots 4, 19, 21, 33 and PAC 37 are not read by
any init (candidates for death/sand models). The factory (CEm000:
`0x140094790`) sizes one joint table per model: body `this+0x6D8`, cloth
`this+0x790`, `this+0x7C0`.

## 2. Variant

`[this+0x670]` selects the weapon inside a class (`0x140097EF6`: 0-1 load the
second slot of the pair, 2-3 the first, other values none) and whether the
first cloth model exists (`0x140097980`: only for 0-1).

## 3. Attachment (per frame)

- Cloth models `this+0xFC0` / `this+0x1740` (present flags `this+0x3250` /
  `+0x3251`): root = world of body joint `[this+0x3254]` / `[this+0x3258]`
  (`0x1401C201C`, `0x1401BFFA9`). Init writes the indices (`this+0x3254` via
  `r15+0x31F4`): table above.
- Weapon model `this+0x1EC0`: root = `this+0x28E0` x world of body joint 9
  (`[this+0x720]`, `0x140030E40` at `0x1401C71F6`, `0x1400B6C1F`,
  `0x1400B7C45`). `this+0x28E0` is built in init from T at `this+0x28B0` and R
  at `this+0x28C0` through `0x1403304A0` (Rz, then Ry, then Rx; unlike the MOD
  rest local `0x140330450`), `0x140031200` (translation) and `0x1403304F0`
  (scale 1). CEm000-CEm003: T(-15, -61.3994, -18.9327), R(0.20726, 0, 0);
  CEm004: T(2, 20, -72), R(-0.034907, 0.10472, 1.65806).
- Init also installs look-at constraints on body joints 3, 5 and 14 (sin/cos
  limits, target = player joint world, `0x140098629`...), not attachment.
