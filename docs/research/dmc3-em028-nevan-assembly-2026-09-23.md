# DMC3 em028 (Nevan): four models, node constraints, effect bank

Date: 2026-09-23
Executable: `dmc3.exe`, SHA-256
`e454272ed0fb0247fcbcf300e5d55d7a3e96d50b89b9ffaff81bb978dcbdd082`.
Sample archive (analysed only, not committed): `em028.pac`, 1 888 368 bytes,
SHA-256 `47636f39d60ade101455405ce892a84654ab06cfa34ff20344e25f790ae88fe2`.
Contract: `include/dmc_rengine/profiles/dmc3/enemy_node_constraint_contract.hpp`,
test `tests/enemy_node_constraint_contract_tests.cpp`.

## 1. Archive layout (sample)

| Slot | Payload | Use in CEm028 |
| --- | --- | --- |
| 0 | PTX | texture bank of all four models |
| 1 | MOD, 23 nodes | body (model at `this+0xF00`) |
| 2, 3 | PAC of MOTs | motion banks |
| 4 | MOD, 23 nodes | hair (model `this+0x1680`) |
| 5 | MOD, 14 nodes | bat dress (model `this+0x1E00`) |
| 6 | MOD, 11 nodes | bat sleeves (model `this+0x2580`) |
| 9 | PNST (slot 0 table, slot 1 PNST of PTX/MOD/BIN) | effect bank |
| 13 | small BIN | cloth setup for the dress |

## 2. Code path

- **Factory `0x14012EA80`.** Reads slots 1, 4, 5, 6 through the inline
  relative-slot getter (`pac + u32[pac + 8 + slot*4]`), allocates the object
  (`0x6A00` bytes, ctor `0x14012AFB0`) and fills one joint table per MOD with
  `0x14012AEE0(count = MOD byte +0x11)`: body `this+0xC70`, slot 4 `this+0xD30`,
  slot 5 `this+0xDF0`, slot 6 `this+0xE68`. The resource descriptor goes to
  `this+0xC60`.
- **Init `0x140130480`** (secondary vtable `0x1404D3658`, entry `+0xE8`; `r14 =
  this+0x60`). Caches slot pointers (0, 1, 4, 5, 6, 2, 3, 13), passes slot 9 to
  `0x1402C04C0(pnst, 2)` — the effect-bank loader, which takes the PNST's slot 0
  table and slot 1 resource PNST — and loads the models with `vtbl+0x50(mod,
  ptx0)`: body = slot 1, then slots 4, 5, 6, each with PTX slot 0. Body joint
  table bound with motion bank slot 2 (`vtbl+0x150`).
- **Joint binding `0x14030F850`.** Table entry *i* is node *i*: `+0x110` points
  at the model's world matrix *i*, `+0x108` at the entry's local, `+0x100` holds
  an optional constraint.
- **Constraints.** Init writes twelve `0xC0` constraint objects (ctor
  `0x1400F7AC0`, vtable `0x1404CC1F8`) and stores each in the child joint's
  `+0x100`, with `+0x30 = bodyTable[j]->+0x110`, `+0x80..+0xB0 = identity`
  (`.rdata 0x14035D580..0x14035D5B0`) and `+0x28 = 1`.
- **World update `0x14030E680`.** A joint with an enabled constraint calls the
  constraint's `vtbl[0]` (`0x1402CBBE0`) instead of `0x14030E9B0`. Mode 1:
  `world = offset x *host` via `0x140030E40`; mode 2: `offset x own +0x40`;
  otherwise the normal local x parent.
- **Per frame (`0x140131CF0`).** Body root = actor world (`this+0x2DB0`); models
  of slots 4 and 5 run chain simulation `0x1402C9DC0` (arrays `this+0x3A30`,
  `this+0x4CF0`, 20 entries each) with root = body joint 0 world; slot 6 root =
  actor world.

## 3. Constraint table (child node <- body joint)

| Part | Pairs | Evidence (store of `+0x30`) |
| --- | --- | --- |
| slot 4 hair | 0<-3, 1<-4, 2<-5 | `0x140130911`, `0x14013095D`, `0x1401309A9` |
| slot 5 dress | 0<-1, 1<-14, 2<-2, 3<-3 | `0x1401309F5`, `0x140130A41`, `0x140130A8D`, `0x140130AD9` |
| slot 6 sleeves | 0<-14, 1<-7, 6<-11, 2<-8, 7<-12 | `0x140130B25`, `0x140130B71`, `0x140130BBD`, `0x140130C09`, `0x140130C55` |

Geometry cross-check on the sample: hair node 1 rest (0, 24.9, -4.9) plus body
joint 3 (0, 124.4, 0) equals body joint 4 (0, 149.3, -4.9); sleeve node 1 rest
(-13.4, 37.1, -4.8) plus hip height 107.7 equals body joint 7
(-13.4, 144.9, -4.8).

## 4. Consequences for a viewer

- Assemble slots 1, 4, 5, 6 with PTX slot 0; place the listed nodes at their
  body joint's world, compose the other nodes local x parent.
- A PNST nested in an archive is an effect bank: its MODs are spawned by the
  effect system, not loaded as actor models (same for weapon PNST slot 2).
- The chains (hair strands, dress) keep their rest locals without
  `0x1402C9DC0`; the dress hangs straight down.

## 5. Dress changes in play

- None of the 25 MOTs in slots 2/3 carries non-unit scale on any body joint
  (checked by evaluating every second frame), so the dress does not resize
  through the body motion.
- `0x14012F790` counts the live bat objects (16 pointers at `this+0x64F0`,
  `+0x6580` alive, `+0x6584` flagged) and, through the dress model's MOD
  document (`model+0x80`, `0x140089DE0`), toggles bit 0 of outer objects 2
  and 3 (`0x1402F74E0` sets it, `0x1402F7350` path clears it via
  `0x1402F7400`; records of 0x380 bytes at `document+0x100`). In the sample
  those objects are the small front strip (42 and 40 vertices, the lightning
  seam), not the dress body.
- The dress body (objects 0 and 1, nodes 4-13) is the chain simulated by
  `0x1402C9DC0` from `this+0x4CF0`; its lengthening and shortening is the
  remaining candidate for the size change seen in game.

## 6. Open

- Constraint enable byte `+0x20` (set outside init) and when mode 2 is used.
- Chain simulation parameters (`0x1402C9F40`, `0x1402CA1D0`, `0x1402CA0A0`) and
  slot 13.
- Other enemies: the constraint pattern is per class; only CEm028 is tabled.
