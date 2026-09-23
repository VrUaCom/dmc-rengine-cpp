# DMC3 HD — IPlayer coat model: PAC slots, texture and joint attachment (2026-09-23)

Canonical executable: `dmc3.exe`,
SHA-256 `e454272ed0fb0247fcbcf300e5d55d7a3e96d50b89b9ffaff81bb978dcbdd082`.

Trigger: in Native Reader the Vergil coat from `pl001.pac` rendered untextured
and stood at the model origin instead of hanging from the body. This note
records how the game loads and places that model.

Receipts (`.pdata` RUNTIME_FUNCTION ranges):

```text
VA range                     file off  sha256
0x140225D70..0x140226D1C     0x225170  69dd3ba2265d3a687f159f0883042da6fbe03afb0d6730fc6412ad55ba801147  CPlVergil load
0x140225A16..0x140225AC6     0x224E16  b461c0ec92f1746e88dda49671d363e3fd586b00edae19eb561d07cf91f529c9  CPlVergil coat update
0x1402120A7..0x14021210A     0x2114A7  a22414b49499dcb6c3a7e21b050a29bfabff2614bd67ef24d2735772e9326a8e  CPlDante coat update
0x1402204E9..0x14022054C     0x21F8E9  acf1cb4e1edf6f4d7eaa1e337a8b67eacc338efeee33923ed6639d0b934c99a9  CPlNewVergil coat update
```

Class identity comes from MSVC RTTI: `CPlVergil` vtable `0x1404DFBA8`,
`CPlDante` `0x1404DF990`, `CPlNewVergil` `0x1404DFFD8`, all derived from
`IPlayer` (`0x1404DF768`).

## 1. PAC slot getter — `0x1401B82C0` — EXE_CONFIRMED

`get(table, 0, playerId, slot)` selects the player's loaded PAC and returns
`pac + u32[pac + 8 + slot*4]` after checking `slot + 1 <= u32[pac + 4]`.
Slot numbers below are therefore direct indices into the player PAC's offset
table. (Leaf function, no `.pdata` entry.)

## 2. Body and coat loading — EXE_CONFIRMED

CPlVergil load (`0x140225D70`):

```text
0x140225E40  tex  = get(..., slot 0)
0x140225E5C  body = get(..., slot 1)
0x140225E8E  model[+0x200]->vtbl[+0x50](body, tex)

0x1402260A9  tex  = get(..., slot 0)
0x1402260C5  coat = get(..., slot 12)
0x1402260F7  model[+0x7540]->vtbl[+0x50](coat, tex)
0x140226112  model[+0x7540]->vtbl[+0x150](0, actor+0xA0F0)     coat joint table
0x14022613D  coatJoint[0]->local(+0x108) = identity (.rdata 0x14035D580)
0x140226169  cloth = get(..., slot 13)   -> ClothNum/ClothNo/Bone parser 0x1402CA1D0
```

The same `slot 0` texture / `slot 12` model pair is loaded by CPlDante
(`0x1402152C6`, `0x1402193E7`) and CPlNewVergil (`0x140222242`,
`0x140222727`).

So the coat has no texture of its own: it uses the body texture in slot 0.

## 3. Per-frame attachment — EXE_CONFIRMED

```text
CPlVergil     0x140225A8D  model[+0x7540]->vtbl[+0x190](joint[+0x1898]->world)
CPlDante      0x1402120E0  model[+0x7540]->vtbl[+0x190](joint[+0x1898]->world)
CPlNewVergil  0x140220522  model[+0x7540]->vtbl[+0x190](joint[+0x1898]->world)
```

`actor+0x1880` is a table of body joint pointers (`+0x110` = world matrix);
`+0x1898` is entry 3. The same `vtbl[+0x190]` call attaches weapons to
`joint[+0x1880 + k*8]`, so it installs the model's root base. Before it, the
cloth objects at `actor+0xA230` (Vergil, 2 × `0xF0`) / `actor+0xA210` (Dante)
are simulated against the coat model (`0x1402C9DC0`).

Result: coat root world = identity root local × body joint 3 current world;
coat children compose normally; the coat mesh is skinned with its own inverse
rest matrices.

## 4. Correction to the 2026-09-23 MOD `+0x13` note

`0x14031FA80`, which reads the linked manager's `currentWorld[+0xFA]`
translation, is called only from `0x14008BD10`, and its object at `+0x30` is
bound through `0x1403204B0` next to the SHW registry entry `0x1403204C0`.
MOD header `+0x13` therefore anchors the model's **shadow**, not a generic
position probe. The conclusion that it never roots geometry stands.

## Open

- `actor+0x1880[i]` is filled by the player model class's `vtbl[+0x150]`
  (the base-class slot `0x1400898A0` is a stub). That `i` equals the MOD node
  index is ASSUMED, not yet bound.
- Cloth simulation (`.clt` text: Gravity, SpringForce, Damping, ClothNo,
  Bone, …; `.c1d`) is not reproduced; without it the coat keeps its rest
  shape while following joint 3.
