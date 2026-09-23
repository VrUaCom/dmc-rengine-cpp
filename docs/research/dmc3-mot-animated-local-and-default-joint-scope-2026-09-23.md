# DMC3 HD — MOT animated local matrix closed; MOD `+0x13` scope corrected (2026-09-23)

Canonical executable: `dmc3.exe`,
SHA-256 `e454272ed0fb0247fcbcf300e5d55d7a3e96d50b89b9ffaff81bb978dcbdd082`.

This closes the one link left open in
`dmc3-mot-runtime-channel-path-2026-09-11.md` (T/R/S → animated local matrix),
closes compression-2 evaluation, and narrows the cross-model reading of MOD
header `+0x13` recorded on the Native Reader line
(`dmc3-mod-cross-model-default-joint-2026-09-15.md`).

Receipts are `.pdata` RUNTIME_FUNCTION ranges (a chunk, not necessarily a
whole function body):

```text
VA range                     file off  sha256
0x140310310..0x1403104BA     0x30F710  6e39557e2c63e82b1fac44017be0575cb25294d2429d1c0dab4907e98b506cc2
0x14030E9B0..0x14030EB24     0x30DDB0  11c615a1c31da3a816b9d28de1b85eba1964a2a9b24875a566c415daefb88987
0x14030F8DF..0x14030FAA8     0x30ECDF  eecaf2c42be83d92e3a75c5f5f8da7f24ba7449046e5fe882e2be730926229ea
0x1402E9170..0x1402E91A2     0x2E8570  9af66194e2f39da53e12d2903d5ffb5b73c0bf5909e5b4b7f1b8a9a6c5b97e71
0x1402E8C80..0x1402E8E11     0x2E8080  6d454aa01c9e36db04225e48952dcc8c603aa9895834344d18cf4f1800590ac2
0x1402E8FB0..0x1402E9141     0x2E83B0  190d41cef4e5f987ba9ef5caf78bf39ee601320a83e11de160ecd2a08ac940c5
0x1403025E0..0x140302610     0x3019E0  0f2fdbce95022e67fc2c1a22c1b5e562b927e36ec02ae2f81aa71fdf57440eb4
0x140302610..0x140302639     0x301A10  b21ba01eab0e162c9539e0ad26237eddfdb1a5491584da9f8ced7c18068565a2
0x14031FA80..0x14031FB48     0x31EE80  a081c9b684d016a7f99188fb31d38c3e823ed67ba0c666f221b5b8e9fc95c648
0x1402FD040..0x1402FD2B0     0x2FC440  970f7329125f4aeee20fc78505c03b6a6fc88e88322af2fceb57034b2ad40e09
0x1402DCBAC..0x1402DD8DF     0x2DBFAC  d82ca2eeec6f19a52b71bf130d3ee6fdb31534ca6d8ededbdbffa8891074da0d
0x140330450..0x140330494     0x32F850  f11c66a7b9910dea4feb1072dd84efbafd0ff605c8c144ac30b9469cff07ac65
```

## 1. Animated local matrix — `0x140310310` — EXE_CONFIRMED

Per motion group `g`, for every joint with `CMotionJoint+0xF8 == g`:

1. nine channels at `joint+0x120` stride `0x20`: if the track pointer `+0x08`
   is non-null, `0x1402E9170(channel, time = group[+0x1B4 + g*4])`; otherwise
   `channel+0x00 = channel+0x04` (current = default);
2. `joint+0x108` is overwritten with identity (`.rdata 0x14035D580..0x14035D5BF`);
3. rotation `+0x180/+0x1A0/+0x1C0` → `cvttss2si(r * 10430.377)` (`0x4622F982`
   at `0x140507AA8`), stored as a word (int16 wrap), re-expanded with
   `* 9.587381e-5` (`0x38C90FDC` at `0x140507AA4`); the result is the rotation
   angle modulo 2π;
4. `0x140330450(local, local, &angles)` — the XYZ Euler helper the MOD rest
   initializer uses (`rotX → rotY → rotZ`);
5. translation `+0x120/+0x140/+0x160` → `local+0x30/+0x34/+0x38`,
   `local+0x3C = 1.0`.

Scale does not enter the local build. `0x14030E9B0` is a later pass over
`joint+0x110`, reached only when a factor leaves
`(0.99999, 1.00001)` (`0x3F7FFF58`, `0x3F800054`); it scales basis rows via
`0x14032ED30` and compensates the parent's scale. Not reproduced here.

Rest defaults (`0x14030F9D0..0x14030FA65`): `+0x124/+0x144/+0x164` ← record
`+0x00/+0x04/+0x08`, `+0x184/+0x1A4/+0x1C4` ← record `+0x10/+0x14/+0x18`,
`+0x1E4/+0x204/+0x224` ← `1.0`.

With this, the chain MOT → channel → **animated local** → currentWorld →
inverseRest × currentWorld is closed end to end.

## 2. Compression-2 evaluation — EXE_CONFIRMED

`0x1402E9170` jump table at `0x1402E962C`: `3 → 0x1402E91D8`,
`2 → 0x1402E9338`, `0 → 0x1402E945F`, `1 → 0x1402E94EA`, `6 → 0x1402E95D5`,
`7 → 0x1402E9406`, `4,5 → 0x1402E960B` (writes 0).

Case 2 calls `0x1402E8FB0`, whose instruction stream equals the compression-3
search `0x1402E8C80` except for the key stride (4 vs 8 bytes). Value =
`u16 * q[1] / 65535 + q[0]`; inside a segment
`u = (t_local - t_left) / (t_right - t_left)`, `v = (1-u)·left + u·right`,
with key times `control & 0x7FFF`. No Hermite branch.

## 3. MOD `+0x13` is a translation probe, not a geometry root — EXE_CONFIRMED

- `manager+0x198` is a linked-manager pointer: getter `0x1403025E0`, setter
  `0x140302610` (only callers `0x1402DCEAF`, `0x1402DDD1A`).
- `0x14031FA80`: with a linked manager, `index = own manager+0xFA`,
  `matrix = linked+0x188 + index*0x40`, reads row `+0x30` only, writes a
  position to `+0x50`.
- `0x1402FD040`: same `+0xFA → +0x188` pattern, translation row only.
- Actor attachment `0x1402DCBAC` selects the host joint from actor state
  `+0x1648` (clamped to the host joint count) and copies that joint's world
  matrix — it never reads the MOD header.

Consequence: `resolve_default_joint(child, host)` returning the host joint's
full world matrix is a reader-side policy with no EXE consumer. Companion MOD
geometry (hair, coat) must be shown in its own model space. The Native Reader
now does so by default.

## Non-claims

- the parent-scale compensation inside `0x14030E9B0`;
- compression forms 0, 1, 6, 7 (6/7 produce boolean channel bytes);
- the meaning of MOT header `+0x10` (loop-start reading remains community);
- blending between motion groups and motion-to-motion interpolation;
- cloth (CLT/C1D) driving of coat/hair bones.
