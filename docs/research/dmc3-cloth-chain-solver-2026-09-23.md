# DMC3 chain/cloth solver (.clt): parser, per-frame step, bindings

Date: 2026-09-23
Executable: `dmc3.exe`, SHA-256
`e454272ed0fb0247fcbcf300e5d55d7a3e96d50b89b9ffaff81bb978dcbdd082`.
Samples (analysed only, not committed): `pl000.pac` (SHA-256
`b73b6e8ee2f2088a088f4215a5a13a50700fb4e4f0bef54dc0bcce11ae7b9c5d`),
`em028.pac` (`47636f39…8fe2`), `em000.pac` (`10ab4cd0…`).
Contract: `include/dmc_rengine/profiles/dmc3/cloth_chain_contract.hpp`,
test `tests/cloth_chain_contract_tests.cpp`.
Builds on `dmc3-em028-nevan-assembly-2026-09-23.md` (open item "chain
simulation parameters") and `dmc3-em000-family-assembly-2026-09-23.md`.

## 1. Text format

A `.clt` slot is plain text whose first line is `;<name>.clt`:

```
ClothNum	1
ClothNo     0
ClothId     0
Gravity     0.000000  -0.010000  0.000000
SpringForce 0.020000
MaxSpeed    50.000000
Stiffness   0.250000
Wind        0.000000  0.000000  0.000000
WindLocal   1
WindParent  0
WindType    1
Bone      2    Y
...
End
$
```

The block parser is `0x1402CA345`, and the `Bone` parser is `0x1402CA42A`.
Defaults (`0x1402CA000`) apply to keys a block leaves out:

| Key | Offset | Default |
| --- | --- | --- |
| Stiffness | `+0x6C` (`+0x70` also 0.3) | 0.3 |
| MaxSpeed | `+0x74` | 50 |
| SpringForce | `+0x78` | 0.05 |
| WindParent | `+0x7C` | 0 |
| WindLocal | `+0x80` | 0 |
| FloorLevel | `+0x84` | -1e6 |
| damping (no key) | `+0x88` | 0.99 |
| WindType | `+0x8C` | 0 |
| Wind | `+0xA0` | (0, 0, 0) |
| Gravity | `+0xB0` | (0, -0.2, 0) |
| dt (no key) | `+0xE0` | 1.0 |
| LimitLength | `+0xE4` | 1 |

`Bone <node> <axis>` turns node `<node>` of the model into a chain constraint
and stores the axis id at joint `+0x240`. The axis ids are X, Y, Z, NX, NY, NZ,
numbered 0 to 5.

## 2. Per-node step (`0x1402C9450`)

The world update calls this step once per game frame for each chain joint, in
hierarchy order. In the steps below, `c` is the constraint, `j` is the joint,
`P` is the parent world, and `T = local × P` is the rest target.

1. If `dt == 0`, skip the step.
2. Let `S` be the previous result (`j+0xA0`) and `v` the velocity (`j+0x250`).
   Move `S.t += dt·v`.
3. Set `d = P.t − S.t`, and negate it for axis ids 3 to 5.
4. Rebuild `S`'s rotation so that the chosen axis equals `d`. The reference
   row is `T.row1` for X and Z, and `T.row0` for Y.
   - X (`0x14032EEE0`): `C = d×r`, and the rows are `(d, C×d, C)`.
   - Y (`0x14032F4C0`): `C = r×d`, and the rows are `(d×C, d, C)`.
   - Z (`0x14032FD90`): `C = r×d`, and the rows are `(C, d×C, d)`.

   All rows are normalized.
5. Blend with `0x14032DA20`, where `s = Stiffness`:
   - `z = s·T.row2 + (1−s)·S.row2`
   - `y` is blended the same way from row 1, and `t` from the translation.
   - `W` is then Z-aligned to `z`, with `y` as the reference.
6. Collide (section 6): push `W.t` out of every capsule it is inside. Any hit
   zeroes the x and z velocity (`j+0x250`, `j+0x258`).
7. Apply wind:
   - `w = Wind`. When WindLocal is set, `w` is rotated by the world of joint
     WindParent.
   - Scale it: `w *= 10·(1 − |cos(w, d)|)`.
   - Update the velocity: `v += dt·w + dt·Gravity`.
8. Enforce the bone length:
   - `e = W.t − P.t`, and `L` is the rest bone length, the length of the
     local translation (`j+0x120/0x140/0x160`).
   - With LimitLength, set `e = e/|e|·L`, then `W.t = P.t + e`, then
     `v −= e·(|e|−L)/L·SpringForce·dt`.
   - Without LimitLength, use the same formula on the squared lengths.
9. Clamp `|v| ≤ MaxSpeed`, then damp: `v *= 0.99`.
10. Clamp to the floor: `W.t.y = max(W.t.y, FloorLevel)`.
11. Store the result: `j+0xA0 = W`.

This data check supports steps 3 and 4: in every simulated bone of the three
samples, the rest Y axis points at the parent (dot product 1.000).

## 3. Bindings in the samples

| Archive | CLT slot | Model slot | Bones | Gravity y | Stiffness |
| --- | --- | --- | --- | --- | --- |
| pl000 | 13 `pl000_02` | 12 coat | 26 (2-5, 7-10, 12-15, 17-20, 22-26, 28-32) | -0.01 | 0.25 |
| em028 | 7 `em028_01` | 4 hair | 16 | -0.02 | 0.3 |
| em028 | 8 `em028_02` | 5 dress | 9 (5-13) | -0.07 | 0.3 |
| em000 | 2 / 6 / 9 / 11 / 14 / 16 | 3 / 7 / 10 / 12 / 15 / 17 | 3-4 each | -0.05 | 0.1 |

In em000, each cloth model is the slot after its `.clt` slot, as listed in the
class inits of the em000 family doc. em000 adds `Wind (0, -0.01, 0)`.

## 4. Nevan's dress

The dress body (slot 5, nodes 5-13) is a chain that pulls down with gravity
-0.07. It swings out when Nevan turns and settles back when she stops. What the
game shows as the dress growing and shrinking is this simulation combined with
the bat strip toggle (objects 2-3, bit 0).

## 5. Viewer port

Native Reader (`motion/cloth_chain.cpp`) runs this step with `dt = 1` for
every elapsed motion frame (at most 6 per update). When a motion starts, the
chains restart from the rest pose and settle for 30 frames; they settle for 60
frames when the model is attached. The player coat collides with the six body
capsules in section 6. The `+0x48` object list and the enemy capsule tables are
not ported.

## 6. Collision

`0x1402CA2F0(chain, joints, entries, shapes, count)` stores the data the
collision step reads:

| Offset | Content |
| --- | --- |
| `+0x30` | Host joint pointer table. Each joint's world is at `+0x110`. |
| `+0x48` | Collision objects. The setter clears this list. |
| `+0x50` | Entry count. |
| `+0x58` | Entries of 4 bytes: flags (bit `8` enables the entry), the host joint, then the shape index as u16. |
| `+0x60` | Shapes of 0x50 bytes: point A at `+0x10`, point B at `+0x20` (both in joint space), radius at `+0x30`. |

For each enabled entry, the step (`0x1402C97F0`) transforms A and B by the
joint world (`0x14032DB90`). It then calls `0x1402D0630`:

1. Find the closest point on the segment A-B (`0x1402CE760`).
2. If the node is closer to it than the radius, move the node onto the capsule
   surface along that direction (`0x14032ED80`).

**Dante's coat.** Every Dante coat setup (for example `0x140213E6E`,
`0x140214A32`, `0x1402151E7` and `0x1402210A4`) passes the entry table
`0x14058B380` with a count of 6, the joint table of the body model, and the
shapes at `player+0xB630`. `0x140214E17` fills those shapes from `.rdata`
`0x14058B260`:

| Body joint | A | B | Radius |
| --- | --- | --- | --- |
| 3 (chest) | (0, 20, 10) | (0, -40, 10) | 15 |
| 2 | (0, -5, 0) | (0, -15, 0) | 18 |
| 15, 16, 19, 20 (legs) | (0, 0, 0) | (0, -50, 0) | 10 |

**Nevan's hair.** CEm028 calls `0x140130D9A` on the hair chain (`this+0x3A30`,
slot 4 with `.clt` 7). It passes the body joint table (`this+0xC70`), the
entry table `0x140576110` with a count of 3, and shapes built at
`this+0x5FB0` from `.rdata 0x140576120`:

| Body joint | A | B | Radius |
| --- | --- | --- | --- |
| 5 | (0, 4.65, 0) | (0, −4.65, 0) | 9.3 |
| 4 | (0, 6, 0) | (0, −6, 0) | 12 |
| 3 | (0, 9.5, 0) | (0, −9.5, 0) | 19 |

The dress chain (`this+0x4CF0`, slot 5 with `.clt` 8) only goes through the
reset `0x1402CA0A0`, which clears velocities and copies worlds. It gets no
collision. No CEm000–CEm004 init calls `0x1402CA2F0`, so the em000 cloaks have
no collision either.

## 7. Open

- The `c+0x48` collision objects, and the capsule tables of the other
  `0x1402CA2F0` callers in the `0x140182…` and `0x1402DD…` classes.
- WindType semantics.
- Whether the world of WindParent is taken from the chain's own model or from
  its host.
