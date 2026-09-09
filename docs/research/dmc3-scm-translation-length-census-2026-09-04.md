# DMC3 HD SCM transform `translationMagnitude` — bounded consumer census 2026-09-04

## Target

- executable: `dmc3.exe`;
- SHA-256: `e454272ed0fb0247fcbcf300e5d55d7a3e96d50b89b9ffaff81bb978dcbdd082`;
- size: `6,356,432` bytes;
- ImageBase: `0x140000000`.

## Serialized field

Each SCM scene transform is `0x20` bytes:

```text
+0x00 f32 translation.x
+0x04 f32 translation.y
+0x08 f32 translation.z
+0x0C f32 translationMagnitude
+0x10 f32 rotation.x
+0x14 f32 rotation.y
+0x18 f32 rotation.z
+0x1C f32 reserved
```

The preserved corpus invariant is:

```text
translationMagnitude ~= sqrt(x*x + y*y + z*z)
```

The earlier 68-resource corpus sweep observed the invariant on 328/328 nodes. The two directly materializable validation specimens in the current environment (`st001.scm`, `st114.scm`) also have zero magnitude-invariant violations.

## Canonical transform load path

Function `0x1402FA080` is called from the two recovered SCM setup variants:

```text
0x140303A02 -> 0x1402FA080
0x140303B27 -> 0x1402FA080
```

For every scene node it reads the serialized transform as two 16-byte lanes:

```text
0x1402FA110..117
  source[+0x00..+0x0F]
  -> {translation.x, translation.y, translation.z, translationMagnitude}

0x1402FA127..12E
  source[+0x10..+0x1F]
  -> {rotation.x, rotation.y, rotation.z, reserved1c}
```

The rotation lane is passed to `0x140330450`. The translation lane is passed to `0x140031200`.

No scalar read of the serialized fourth translation lane is present in this node-load loop before the stack slot is reused for later loop state.

## Translation helper `0x140031200`

Relevant instructions:

```text
140031216  movaps xmm2, [r8]          ; input translation vec4
14003121A  addps  xmm2, [rdx+0x30]    ; add to matrix row 3
14003121E  movaps xmm0, xmm2
140031221  xorps  xmm0, [rdx+0x30]
140031225  andps  xmm0, [0x1405D9F30]
14003122C  xorps  xmm0, xmm2
14003122F  movaps [rcx+0x30], xmm0
```

The mask at runtime address `0x1405D9F30` is initialized by `0x140001920..0x140001928` from constant `0x14035D340`.

Canonical executable bytes at `0x14035D340` are:

```text
00 00 00 00
00 00 00 00
00 00 00 00
FF FF FF FF
```

Therefore the four 32-bit SIMD mask lanes are:

```text
{ 0x00000000, 0x00000000, 0x00000000, 0xFFFFFFFF }
```

## Exact algebra

Let:

```text
original = matrix row 3
input    = {tx, ty, tz, magnitude}
x        = original + input
mask     = {0, 0, 0, all-bits}
```

The helper computes:

```text
out = x XOR ((x XOR original) AND mask)
```

Per lane:

```text
X -> x.X = original.X + tx
Y -> x.Y = original.Y + ty
Z -> x.Z = original.Z + tz
W -> original.W
```

Thus the serialized fourth lane is explicitly discarded from the resulting transform row. The helper applies only translation XYZ while preserving matrix W.

## Bounded conclusion

The strongest current classification is:

```text
translationMagnitude
  DATA_CONFIRMED_PRECOMPUTED_LENGTH
  NO_ACTIVE_EFFECT_IN_RECOVERED_TRANSFORM_APPLICATION_PATH
```

Safe semantic description:

> Legacy precomputed length of the serialized translation vector. The canonical HD SCM transform loader carries it in a vec4 load, but the translation helper masks the fourth lane out and preserves matrix W.

This is stronger than merely observing that the local matrix builder ignores W.

It is still intentionally narrower than a mathematical whole-program `unused` claim. No separate non-transform consumer has been proven from the recovered SCM pipeline.

## Authoring policy

For no-edit roundtrip:

```text
preserve original float bits
```

When translation XYZ is edited:

```text
translationMagnitude = length(translation.xyz)
```

This maintains the 328/328 corpus invariant and the historical serialized representation even though the recovered HD transform-application path does not consume the magnitude lane.

## Evidence status

```text
STRUCTURE             EXE_AND_CORPUS_CONFIRMED
LENGTH INVARIANT      CORPUS_CONFIRMED
TRANSFORM W REJECTION EXE_CONFIRMED
GLOBAL UNUSED CLAIM   NOT MADE
WRITER POLICY         EVIDENCE_BACKED
```
