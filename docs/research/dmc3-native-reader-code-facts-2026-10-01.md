# DMC3 facts kept only in Native Reader code (snapshot)

Date: 2026-10-01
Executable: `dmc3.exe`, SHA-256
`e454272ed0fb0247fcbcf300e5d55d7a3e96d50b89b9ffaff81bb978dcbdd082`.
Source: `VrUaCom/DMC-Native-Reader`, branch `NR-Luna-v73`.

## Status

Every block below is quoted verbatim from a Native Reader source comment or
note, for an EXE address that no other note of this repository cites. The
comments were written while porting (disassembly reads, several checked by
emulated runs); treat each as `candidate` unless the quoted text says it was
checked, and promote it to its own note when it is used. Addresses already
covered by `dmc3-effect-runtime-2026-10-01.md`, `dmc3-particle-p-records-2026-10-01.md`,
`dmc3-generator-g-records-2026-10-01.md` and `dmc3-effect-triggers-2026-10-01.md`
are left out. Regenerate the list by scanning the Reader tree for `0x14xxxxxxx`
addresses absent from `docs/`, `src/` and `include/` here.

---

### `app/src/main/cpp/include/dmcresource/collision_shapes.h`

Addresses: `0x14005c7c6` (lines 22-22)

```text
// Index entry (0x14005C7C6).
```

### `app/src/main/cpp/include/dmcresource/effect_bank.h`

Addresses: `0x140312840` (lines 125-128)

```text
// P: 0x140312DC0 validates version 2, rebases the relative pointers stored
// from +0x10, and returns the root object. 0x140312840 dispatches on root+1
// with runtime subtype 0..5. The optional child target list is described by
// root+0xC0 and the relative offsets beginning at raw+0x18.
```

### `app/src/main/cpp/include/dmcresource/motion/part_attachment.h`

Addresses: `0x14013065f` (lines 68-71)

```text
// Texture scroll (.tsc) slot and the model slots its CDrawUV objects drive:
// CEm028 init caches slot 13 (0x14013065F) and hands it to the CDrawUV at
// this+0x2D00 for the dress (slot 5, 0x1401307FD) and at this+0x2D38 for the
// sleeves (slot 6, 0x14013089E).
```

Addresses: `0x140303460`, `0x140303de0` (lines 278-282)

```text
// One selectable position of an archive with several in-game looks: an
// enemy class and its weapon variant (em000.pac), or a model-object state
// (em028.pac: the dress strip, objects 2-3 of slot 5, is drawn only while
// bats are out -- 0x14012F790 sets or clears object bit 0, which the MOD draw
// loops 0x140303460 / 0x140303DE0 require).
```

### `app/src/main/cpp/include/dmcresource/particle_sprt.h`

Addresses: `0x140236aa0`, `0x1404e29c8`, `0x1404e2c68`, `0x1404e2728` (lines 13-27)

```text
// P records (particle emitters). Three definition classes are ported:
//   3 CPtclSprt00  textured camera-facing quads (4 vertices per particle)
//   1 CPtclPoly00  untextured free triangles (3 vertices per particle)
//   4 CPtclLine01  untextured two-segment streaks (4 vertices per particle)
// Classes 0 (Line00), 2 (Poly01), 5 (Line02) and the Poly00 path +0xFA != 1
// are not ported. Reverse authority: dmc3.exe factory 0x140236AA0, per-class
// init / update (vtables 0x1404E29C8, 0x1404E2C68, 0x1404E2728), key track
// 0x1402D30E0 / lerp 0x1402D3200, transform integrator 0x140312260, draw
// composer 0x140312F10; every constant was checked against an emulated run of
// those functions (docs/research/dmc3-particle-sprt00-exe-v75.md).
//
// Every object is a burst: particles spawn once in a hollow box (0x140312450),
// move under friction and gravity, and fade along a 3-segment key track. A
// layer re-draws the emitter's particles with its own local transform, colour
// track and blend.
```

### `app/src/main/cpp/include/dmcresource/ptx_runtime_compat.h`

Addresses: `0x140331910` (lines 93-94)

```text
    // EXE 0x140331910 projected into bounded Reader-owned state. Pool bytes are
    // cleared; existing manager state is not implicitly released/reset here.
```

Addresses: `0x140331d90` (lines 98-99)

```text
    // EXE 0x140331D90. Updates reservation/bounds and resets manager keys/counts
    // while preserving pool records/occupancy/tail and manager payload/padding.
```

Addresses: `0x140331520` (lines 125-126)

```text
    // EXE 0x140331520. Normal EXE-style failure/no-space is PlacementStatus::rejected;
    // host-model invalid represented memory/length is RuntimeError.
```

### `app/src/main/cpp/modules/effect_bank.cpp`

Addresses: `0x1402e44fd` (lines 274-274)

```text
            // 0x1402E44FD: B = A * 0.5 (a zero extent keeps a zero pivot).
```

### `app/src/main/cpp/modules/generator.cpp`

Addresses: `0x140371920` (lines 12-12)

```text
constexpr float kTwoPi = 6.28318501F;  // 0x140371920
```

### `app/src/main/cpp/modules/motion/cloth_chain.cpp`

Addresses: `0x1402c9714`, `0x1402c98f4` (lines 194-199)

```text
    // Collision (0x1402C9714..0x1402C98F4): push the node out of every
    // capsule it is inside (0x1402D0630: closest point on a-b, then onto the
    // surface at the radius); any hit zeroes the x and z velocity.
    // Only the first chain of a part collides: 0x140214D50 parses every
    // ClothNo into consecutive chains (+0xA210, +0xA300, ...) but calls the
    // capsule setter 0x1402CA2F0 on +0xA210 alone (0x1402151E7).
```

Addresses: `0x1402c9a8c`, `0x1402c9c63` (lines 236-236)

```text
    // Rest bone length and spring (0x1402C9A8C...0x1402C9C63).
```

### `app/src/main/cpp/modules/motion/effect_profile_registry.cpp`

Addresses: `0x140172380` (lines 92-92)

```text
        // CEm034Shl00 init 0x140172380: V463 follows the bullet (+0xC0, mode 3).
```

Addresses: `0x140175785` (lines 114-115)

```text
        // CEm034Shl04 flight 0x140175785: V475 once the fuse is below 60,
        // following the grenade (mode 3), retired with it.
```

Addresses: `0x140175cc0` (lines 121-121)

```text
        // CEm034Shl05 init 0x140175CC0: V276 follows the shot (mode 3).
```

Addresses: `0x1401754f0`, `0x1402e7a80` (lines 143-144)

```text
        // CEm034Shl04 init 0x1401754F0: 0x1402E7A80(1, 0x2FD) is the grenade
        // sprite E765 (+0x84 held), following the grenade matrix (mode 3).
```

### `app/src/main/cpp/modules/motion/motion_clip.cpp`

Addresses: `0x140310a80` (lines 154-161)

```text
        // The EXE binding loop at 0x140310A80 iterates the initialized
        // CMotion joint count, not the MOT's declared mask count. It advances
        // the mask pointer once per joint. For a shorter MOT domain, the
        // aligned header tail supplies zero masks for the remaining model
        // nodes; em034 slot11 MOTs are the canonical 2-mask/3-node example.
        // Accept that structural case only when the physical zero padding is
        // present. Never synthesize a missing mask outside the serialized
        // header or reinterpret non-zero padding.
```

### `app/src/main/cpp/modules/motion/motion_player.cpp`

Addresses: `0x1404c6440` (lines 523-526)

```text
    // 0x14032FD90(out, dir, ref (0,1,0,1) from 0x1404C6440):
    // row0 = normalize(up x direction)
    // row1 = normalize(direction x row0)
    // row2 = normalize(direction)
```

Addresses: `0x14016cbb0`, `0x14016cc41` (lines 610-611)

```text
    // 0x14016CBB0..0x14016CC41:
    // spawn = (87.8,0,4.28,1) * node0World + node1World.translation.
```

Addresses: `0x14032f1d0` (lines 627-630)

```text
    // 0x14032F1D0:
    // row0 = direction
    // row1 = normalize(ref x direction)
    // row2 = normalize(direction x row1)
```

Addresses: `0x140172590`, `0x140175ed0` (lines 805-807)

```text
// Shl00/Shl05 straight flight (0x140172590 / 0x140175ED0): pos += vel * dt,
// +0x52C = 120 counts down; below zero the state becomes 3 and the next update
// retires the shell together with its trail effect.
```

Addresses: `0x14057b428` (lines 811-811)

```text
// 0x140171C70: SMG shots use a fixed 45.0 (.rdata 0x14057B428).
```

Addresses: `0x14016f780` (lines 854-854)

```text
// 0x14016F780: (axis, 1) * component node world with row 3 = (0,0,0,1).
```

Addresses: `0x1401726a5`, `0x140175fe5` (lines 952-954)

```text
// A straight shell's stage hit is read by the update after the move that
// met the HITS (Shl00 0x1401726A5 / Shl05 0x140175FE5 read the collider
// results after moving): state 2 and the hit effect there, retire next.
```

Addresses: `0x140169b90`, `0x14016f5e0` (lines 1026-1029)

```text
        // 0x140169B90: without a player in the 0x1402C6870 cone the shot uses
        // the pistol axis (-1,0,0) of slot21 (lane1/ch1 == 0) or slot22.
        // The muzzle is 0x14016F5E0 (joint 9 unless +0x5A27 is cleared;
        // state 0x7F leaves it as set by the previous entry: joint 9 here).
```

Addresses: `0x140169d7c`, `0x14016ab5a` (lines 1043-1044)

```text
        // 0x140169D7C: no player in the cone -> slot23 axis (1,0,0);
        // dispatcher 0x14016AB5A sets +0x5A27 = 1 -> muzzle joint 9.
```

Addresses: `0x14016a095` (lines 1057-1061)

```text
        // 0x14016A095: count = [1,2,3,6,3,2][em+0x5A1C] (fight phase; the
        // standalone Reader uses entry 0). Each shell: (0,0,10,1) rotated by
        // pitch -(rand%30) deg and yaw ((rand%100)-50) deg + actor yaw, from
        // joint 9 (em+0x830), fuse 120 + 30*i. The Reader uses the means of
        // the retail uniform draws: pitch -14.5 deg, yaw -0.5 deg.
```

Addresses: `0x140169f98`, `0x14016a090` (lines 1081-1084)

```text
// State 0x85 fire loop (0x140169F98..0x14016A090): while +0x57DD is set the
// 0.9 timer expires every tick and 0x140171C70(em, 1) fires one Shl00: SMG
// axis (-1,0,0) of slot24 node0, speed 45, muzzle joint 13 (the 0x85 entry
// clears +0x5A27). dl = 1 skips the player aim entirely.
```

Addresses: `0x140174d39` (lines 1253-1254)

```text
            // Shl03 state1, 0x140174D39:
            // actor+0x80 += actor+0x140 * actor+0x14.
```

### `app/src/main/cpp/modules/motion/motion_script.cpp`

Addresses: `0x1400596a0` (lines 26-26)

```text
    case 34U: return 6U;              // 0x1400596A0
```

Addresses: `0x1400598c0` (lines 28-28)

```text
    case 36U: return 4U;              // 0x1400598C0
```

### `app/src/main/cpp/modules/motion/part_attachment.cpp`

Addresses: `0x140171240`, `0x1401712a6` (lines 620-624)

```text
    // CEm034 component3/preset1 points its CCnsMatrix at CEm034+0x43C0.
    // Update 0x140171240 copies [em+0x850]->world (+0x110) there - body
    // joint 13 of the +0x7E8 joint table - and 0x1401712A6 scales it by
    // +0x4400: effective parent = S * joint13 world, so the shotgun sits in
    // the hand, not at the body root.
```

Addresses: `0x140169ec2`, `0x140169ee0` (lines 894-895)

```text
            // 0x140169EC2 writes 1.0 every state81 update; 0x140169EE0
            // promotes it to 1.5 only while channel1 == 1.
```

### `app/src/main/cpp/modules/pac_assembly.cpp`

Addresses: `0x14030afc0` (lines 878-879)

```text
                // JntNo (type 10 facing): defaults to MOD +0x13, clamped to
                // the node count (0x14030AFC0).
```

### `app/src/main/cpp/modules/particle_sprt.cpp`

Addresses: `0x1403228f0` (lines 495-495)

```text
    // Sprt00 +0x128 == 0: the A animation plays (0x1403228F0).
```

### `app/src/main/cpp/modules/ptx_runtime_compat.cpp`

Addresses: `0x140315150` (lines 14-23)

```text
// Provenance: Reader-owned C++23 projection of the EXE-confirmed PTX runtime
// slice recovered in read-only dmc-rengine-cpp commit
// 50d070e158e484937238d9cb02b2bc6affb2f502:
//   0x140331520 placement
//   0x140331910 pool initialization
//   0x140331D90 reservation/configure
// and the reset-key behavior called by configure (0x140315150).
//
// Keep this representation private. It models the represented runtime memory
// required by the confirmed algorithms; it is not a serialized PTX layout.
```

### `app/src/main/cpp/modules/resource_session.cpp`

Addresses: `0x1402e5c70` (lines 895-896)

```text
    // CEffect draw 0x1402E5C70 dispatches modes 0/1/2/5; modes 3 and 4 draw
    // nothing. Modes 0 and 5 keep their undecoded resource contracts.
```

Addresses: `0x1402e5fc2` (lines 940-942)

```text
        // 0x1402E5D00: camera-facing, extents x in [-Bx, Ax-Bx], y in
        // [-By, Ay-By], scaled by |row i| of the effect world times the
        // record scale (0x1402E5FC2 loop).
```

Addresses: `0x1402e6cb8` (lines 982-983)

```text
        // Variants 1..3 rebuild the width axis from the camera (cross
        // products at 0x1402E6CB8): keep the U edge, turn V to face the view.
```

### `docs/PROJECT_AI_CONTEXT.md`

Addresses: `0x140331bd0`, `0x140331a80` (lines 84-91)

```text
Rules:
- existing `TextureSlotFramingParser -> TextureSet -> DDS` remains the only serialized/disk-format PTX authority;
- `PtxRuntimeCompat` models only the explicitly imported confirmed runtime state-machine behavior;
- RuntimeCompat is lazy and is not created by ordinary preview/gallery/PNG flow;
- no second user-facing PTX NativeModule is created;
- no `TextureSet::Slot -> runtime 0x50 record` mapping may be inferred until separately evidenced/reviewed;
- initializer represented storage is `0xCB50`, while placement/configure operate on the confirmed `0xCB48` prefix; the final 8 bytes remain opaque clear/preserved tail only;
- palette `0x140331BD0`, finalizer `0x140331A80`, parser/backend runtime mapping `0x1403365B0`, full graphics-config type/live values, actual caller/teardown ordering and full manager acquire/release lifecycle remain deferred unless separately reviewed.
```

### `docs/reviews/NR_LUNA_V73_ANDROID_ACCEPTANCE_PASS01_2026-09-29.md`

Addresses: `0x14016998e` (lines 146-156)

```text
- ✅ Retail callsite 0x14016998e запускає V423 через
  0x1402e7a90 з mode=3, передаючи raw matrix вибраного
  CEm034 slot20 object.
- ✅ Retail 0x1402e7ab0 копіює цю matrix, нормалізує перші три рядки
  через 0x140330390 і зберігає translation row без зміни.
- ✅ Reader тепер розділяє два домени: world динамічного Shl02 залишається
  actor-render basis, а effect_parent_world для V423 бере окрему
  normalized copy raw slot20 matrix.
- ✅ E752 local T=(60,0,0), V423 graph, FXBANK slot 28 та всі resource
  identities не змінювалися.
- ✅ Додано native regression assertion для row-wise normalization і
```

