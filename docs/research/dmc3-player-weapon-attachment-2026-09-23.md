# DMC3 HD — player weapon attachment records (2026-09-23)

Canonical executable: `dmc3.exe`,
SHA-256 `e454272ed0fb0247fcbcf300e5d55d7a3e96d50b89b9ffaff81bb978dcbdd082`.
Canonical data: `include/dmc_rengine/profiles/dmc3/player_attachment_contract.hpp`
(test `tests/player_attachment_contract_tests.cpp`).

Receipts (`.pdata` ranges unless marked leaf):

```text
0x1401FD8F0..0x1401FDA7B  0x1FCCF0  3baf636a4d3631a2f3db3dd0da2cef76af2dfe44e4798c50d2b435adc5c9af69  record -> offset matrix
0x140231505..0x140231541  0x230905  4b67ebdc5e9c55a58ac0fface3ce08a87dddfd37e798a044e55efa1ad769bd24  CPlWpSword root install
0x140231560..0x14023164A  0x230960  dccd013278eaa5e0fa25d7302dd860132ca90ce2d0577b9f2ab86ff0e192bb9f  CPlWpSword vt[1] (table)
0x1401FAA90 leaf 0x25 bytes         f5e5465138286f57fc31da81f1f440d027b2201266899f6c5f706c337edd7d6f  player joint getter
```

## 1. Joint getter — `0x1401FAA90` — EXE_CONFIRMED

`joint(player, i) = player.jointTable[+0x1880 + (formBase[form] + i) * 8]->world(+0x110)`
with `formBase = {0, 24, 48}` from `.rdata 0x1404E0328`, `form = player+0x3E6C`.

## 2. Attach record — `0x1401FD8F0` — EXE_CONFIRMED

`weapon+0x128` points to a per-state array of record pointers (`0x30`-byte
records). For the selected state the function copies `byte +3` to
`weapon+0x114` (joint), builds `offset` in the destination matrix as:

```text
identity (.rdata) -> 0x140330450 XYZ Euler(+0x20,+0x24,+0x28)
                  -> 0x140031200 translation(+0x10,+0x14,+0x18, w=1)
                  -> 0x1403304F0 scale (1,1,1,1)
```

i.e. the same construction as the MOD rest local. The alternate branch reads
`+0x31`, `+0x40`, `+0x50` of the record.

## 3. Root install — e.g. `0x140231505` — EXE_CONFIRMED

```text
m = joint(player = weapon+0x120, byte weapon+0x114)
root = 0x140030E40(offset(weapon+0xAC0), m)   = offset x jointWorld
weapon.model(+0x200)->vtbl[+0x190](root)
```

## 4. State-0 records (strict: written by the class's own vtable[1])

| class | vtable[1] | table | joint | translation | rotation XYZ (rad) |
| --- | --- | --- | --- | --- | --- |
| CPlWpSword | 0x140231560 | 0x14058C010 | 3 | (-14.5, 32, -14) | (-1.65806, 0, 3.40339) |
| CPlWp2Sword | 0x140227EA0 | 0x14058C800 | 3 | (16, -43, -15) | (-1.60570, 0, 0.26180) |
| CPlWpGuitar | 0x14022A8D0 | 0x14058DAE0 | 3 | (-30, -80, -23) | (-1.50098, -0.11345, -0.52360) |
| CPlWpLaser | 0x14022C940 | 0x14058E380 | 8 | 0 | 0 |
| CPlWpFoeceEdge | 0x140229DD0 | 0x14058ED30 | 3 | (-14.5, 32, -14) | (-1.65806, 0, 3.40339) |
| CPlWpNeroSword | 0x14022D290 | 0x14058FAF0 | 3 | (-14.5, 32, -14) | (-1.65806, 0, 3.40339) |
| CPlWpVergilSword | 0x140231EB0 | 0x14058F280 | 13 | (19, -0.5, 11) | (0, 3.83972, 0) |
| CPlWpNewVergilSword | 0x14022DDD0 | 0x14058F770 | 13 | (19, -0.5, 11) | (0, 3.83972, 0) |

`CPlWpShotGun`, `CPlWpRifle`, `CPlWpLadyGun` state 0 is joint 0 with a zero
offset; `CPlWpNunchaku`, `CPlWpFight`, `CPlWpGun` install no table in
vtable[1]. They are not catalogued.

## Open

- Class ↔ `obj\plwp_*.pac` pairing follows the executable's own names; the
  load path is not traced.
- State 0 is taken as the idle/sheathed pose; the state machine that selects
  hand records (e.g. Rebellion joint 9 / joint 13 records) is not reversed.
- Joint-table index = MOD node index is assumed (see the coat note).
