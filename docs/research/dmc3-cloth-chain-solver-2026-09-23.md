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
6. Collide against the capsule lists at `c+0x48` and `c+0x58`. This step is not
   ported.
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
frames when the model is attached. Collisions (step 6) are not ported.

## 6. Open

- Capsule collision lists (`c+0x48` / `c+0x58`) and where they are filled.
- WindType semantics.
- Whether the world of WindParent is taken from the chain's own model or from
  its host.
