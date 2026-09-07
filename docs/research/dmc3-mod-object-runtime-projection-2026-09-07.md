# DMC3 HD MOD object runtime projection — 2026-09-07

## Scope

This pass recovers the directly observable runtime projection of serialized MOD object state after the structural object-core pass.

It separates three authorities that must not be conflated:

1. the serialized MOD source bitmap at object `+0x10`;
2. the runtime-object flag word and auxiliary state produced by the MOD/EFM initializer;
3. shared material/render helpers that consume selected source bits independently.

The implementation is read-only analysis. It does not mutate MOD bytes and does not grant writer authority.

## Canonical authority

Canonical executable:

- `dmc3.exe`
- SHA-256: `e454272ed0fb0247fcbcf300e5d55d7a3e96d50b89b9ffaff81bb978dcbdd082`
- MOD/EFM object initializer: `0x1403029E0`
- common render-state helper: `0x140302640`
- shared material helper: `0x1402F9890`
- model-family magic dispatcher: `0x1402FD650`
- MOD-specific post-initializer: `0x1402FF570`

`0x1402FD650` establishes the high family masks directly from document magic:

```text
"MOD " -> 0x10000000
"EFM " -> 0x20000000
"SCM " -> 0x30000000
"MRP " -> 0x40000000
```

Therefore the `0x10000000 -> 0x1402FF570` branch in the object initializer is specifically authorized for MOD rather than inferred from a shared layout.

## Serialized live parameters

Two bytes ranges previously hidden inside the undecoded object middle region are live on the MOD/EFM path:

```text
object +0x18  f32  conditional render parameter
object +0x1C  u32  conditional render parameter
```

When source flag `0x00000200` or `0x00000400` is active, `0x1403029E0` copies:

```text
serialized +0x18 -> runtime +0x16C
serialized +0x1C -> runtime +0x170
```

The same conditions request manager-global bit 21.

Downstream packet construction copies the runtime float quartet `+0x160..+0x16C` into a per-pass/material block and converts/carries the `+0x170` value beside alpha/control state. This proves that `+0x18/+0x1C` are render-path inputs rather than padding.

Semantic labels such as specular, glow, emissive, etc. are **not** claimed by this pass.

## Exact source flag -> runtime flag projection

The runtime object flag word begins with baseline value `3` before these source-bit effects are ORed in.

| Serialized source condition | Confirmed runtime effect |
|---|---|
| low nibble `!= 0` | runtime bit 8; low nibble supplied as helper mode; control forced to `0x80` |
| `0x00000020` | runtime bit 10 |
| `0x00020000` | runtime bit 9; seed runtime float state `(128,128,128,0)` |
| `0x00010000` | runtime bit 7 |
| `0x00000200` | runtime bit 12; request manager bit 21; copy `+0x18/+0x1C` |
| `0x00000400` | runtime bit 13; request manager bit 21; copy `+0x18/+0x1C` |
| `0x00000010` | runtime bit 11 |
| `0x00040000` | runtime bit 4 |
| high nibble domain `0x0F000000 != 0` | runtime bit 15; runtime byte gets `(nibble - 1)` |

The `0x00020000` preset constant is directly recovered as `128.0f`. If a `0x200/0x400` conditional copy is also present, the later `+0x18` copy overwrites the runtime W/component slot while the seeded XYZ values remain part of the preceding state.

## Effects deliberately kept outside the runtime flag word

### `0x00004000` — sampler selection

The shared material helper `0x1402F9890` consumes source/object flag `0x4000` to select the legacy GS TEX1 filtering state. This is recorded by the analysis projection as `nearest_texture_filter`, but it is not invented as another bit in the runtime object flag word.

### MOD source `0x1000` / `0x2000`

MOD-specific helper `0x1402FF570` initializes runtime bytes `+0x05/+0x06` to `4`, then sets both to `0x0C` when either source flag `0x1000` or `0x2000` is present.

This behavior is format-specific evidence and is not transferred to EFM or SCM.

## Additional common render-state evidence

`0x140302640` consumes the low source nibble plus source bits including `0x10000` and `0x100000` while constructing a legacy render-state packet. This confirms that the source bitmap is not merely visibility metadata.

This pass does not promote speculative user-facing names for those packet modes. The analysis API exposes only direct effects that can be stated without guessing downstream artistic semantics.

## Canonical C++

Added:

- `include/dmc_rengine/analysis/mod/object_runtime.hpp`
- `src/analysis/mod/object_runtime.cpp`

The analysis layer provides:

- `ObjectRuntimeSerializedAbi`
- `decode_object_runtime_source_fields()`
- `read_object_runtime_source_fields()`
- `ObjectRuntimeProjection`
- `project_object_runtime()`
- `analyze_object_runtime()`

The parser's preserved `Document::source_bytes` remains byte authority, while `OuterModel::record_offset` provides the exact serialized record origin. This avoids a risky parser rewrite in the same runtime-semantics slice; explicit typed-parser promotion of `+0x18/+0x1C` can occur separately without changing the recovered semantics.

## Compile-time regression

The Core source carries compile-time checks for:

- decoding f32 `12.5` at object `+0x18`;
- decoding u32 `0xDEADBEEF` at `+0x1C`;
- every directly promoted runtime flag bit;
- low-mode and high-mode encoding;
- manager-bit-21/copy conditions;
- `128.0f` preset condition;
- nearest-filter signal;
- MOD-specific `+0x05/+0x06 = 0x0C` condition;
- zero-source baseline behavior.

Because the repository automatically compiles `src/*.cpp` into Core, Ubuntu and Windows CI validate these contracts without a new CMake target.

## Evidence status

- serialized source flags `+0x10`: `EXE_CONFIRMED`
- live parameter `+0x18`: `EXE_CONFIRMED`
- live parameter `+0x1C`: `EXE_CONFIRMED`
- listed runtime flag projection: `EXE_CONFIRMED`
- `0x20000 -> (128,128,128,0)` preset: `EXE_CONFIRMED`
- `0x200/0x400` conditional parameter copy: `EXE_CONFIRMED`
- source `0x4000` nearest-filter material effect: `EXE_CONFIRMED`
- MOD `0x1000/0x2000 -> runtime 0x0C/0x0C`: `EXE_CONFIRMED`
- artistic/visual semantic names for these fields: `PRESERVED_UNDECODED`

## Explicit non-claims

This pass does not establish:

- writer authority for the source bitmap;
- safe mutation ranges for object `+0x18/+0x1C`;
- artistic names for runtime bits 4/7/8/9/10/11/12/13/15;
- complete behavior of source `0x100000`;
- equivalence of MOD, EFM, and SCM per-bit semantics;
- original-game acceptance of edited values;
- no-edit byte-identical MOD rebuild.
