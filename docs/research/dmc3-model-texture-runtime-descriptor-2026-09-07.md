# DMC3 HD model texture runtime descriptor — 2026-09-07

## Scope

This pass documents the in-memory texture descriptor table used by the recovered DMC3-HD model runtime. It is downstream of the TM2-backed model texture companion and upstream of MOD/EFM/SCM material setup.

It does **not** define a serialized file format and does not grant writer authority.

## Canonical authority

Canonical executable:

- `dmc3.exe`
- SHA-256: `e454272ed0fb0247fcbcf300e5d55d7a3e96d50b89b9ffaff81bb978dcbdd082`
- companion/source materializer: `0x140304B30`
- runtime descriptor builder: `0x14030D040`
- shared material consumer: `0x1402F9890`
- dimension-exponent helper: `0x140331070`

## Runtime descriptor ABI

The table selected by model mesh `texture_slot * 0x40` uses a fixed `0x40`-byte entry.

```text
+0x00  qword  pointer to mip/source record 0
+0x08  qword  pointer to mip/source record 1
+0x10  qword  pointer to mip/source record 2
+0x18  qword  pointer to mip/source record 3
+0x20  qword  PlayStation 2 GS TEX0 register image
+0x28  qword  PlayStation 2 GS MIPTBP1 register image
+0x30  dword  mip_record_count - 1
+0x34  dword  base source width copied from source record +0x0A
+0x38..+0x3F  not semantically promoted by this pass
```

The first four qwords reference the intermediate texture/mip records produced by the companion/TM2 materialization path. The builder supports up to four source-record pointers in this descriptor shape.

## Intermediate source record evidence

`0x140304B30` materializes an intermediate `0x50`-byte texture record. Fields directly feeding the runtime descriptor path include:

```text
+0x04  PSM / pixel storage mode field
+0x06  GS base-pointer units
+0x08  TBW / texture-buffer-width field
+0x0A  width
+0x0C  height
+0x0E  calculated allocation/span units
```

The allocation/span value participates in advancing the next GS base address in `0x20`-byte units. Detailed ownership of every byte in the `0x50` record remains outside this pass.

## TEX0 @ descriptor +0x20

The qword assembled by `0x14030D040` matches the PS2 GS TEX0 register layout bit-for-bit:

```text
bits  0..13  TBP0
bits 14..19  TBW
bits 20..25  PSM
bits 26..29  TW
bits 30..33  TH
bit      34  TCC
bits 35..36  TFX
bits 37..50  CBP
bits 51..54  CPSM
bit      55  CSM
bits 56..60  CSA
bits 61..63  CLD
```

The canonical builder explicitly sources the texture base/buffer/format/dimension fields from the intermediate records and sets the recovered constant state used by this path. The C++ API decodes the complete GS register image rather than discarding register fields that are currently zero in the builder path.

`0x1402F9890` selects the descriptor by serialized mesh texture slot and copies descriptor `+0x20` into the runtime material packet. This makes TEX0 consumption `EXE_CONFIRMED`.

## MIPTBP1 @ descriptor +0x28

The adjacent qword matches PS2 GS MIPTBP1:

```text
bits  0..13  TBP1
bits 14..19  TBW1
bits 20..33  TBP2
bits 34..39  TBW2
bits 40..53  TBP3
bits 54..59  TBW3
bits 60..63  unassigned by MIPTBP1
```

The builder obtains these base-pointer/width pairs from mip source records 1..3.

Important authority boundary: `0x14030D040` proves construction of the MIPTBP1 register image. The bounded `0x1402F9890` material path inspected in this pass consumes descriptor `+0x20` TEX0, not `+0x28`. Therefore MIPTBP1 is **builder-confirmed register state**, while its downstream consumer remains a separate trace target.

## Dimension exponent behavior

The helper at `0x140331070` used for TEX0 `TW/TH` does not behave as an unrestricted mathematical `log2`. In the recovered path it searches exponent values in the legacy GS domain and selects the smallest exponent whose power-of-two covers the source dimension. The implementation is therefore effectively a bounded/legacy ceil-log2 behavior for the accepted model texture dimensions.

No generic public encoder helper is introduced by this pass.

## Canonical C++

Added:

- `include/dmc_rengine/formats/model_texture_runtime.hpp`
- `src/formats/model_texture_runtime.cpp`

The header exposes:

- `RuntimeTextureDescriptorAbi`
- `LegacyGsTex0Fields`
- `decode_legacy_gs_tex0()`
- `LegacyGsMiptbp1Fields`
- `decode_legacy_gs_miptbp1()`

The `.cpp` contains compile-time regression vectors so both Ubuntu and Windows Core builds validate all decoded bit positions.

## Evidence status

- descriptor stride `0x40`: `EXE_CONFIRMED`
- source pointer fields `+0x00/+0x08/+0x10/+0x18`: `EXE_CONFIRMED`
- TEX0 at `+0x20`: `EXE_CONFIRMED`
- MIPTBP1 at `+0x28`: `EXE_CONFIRMED` builder state
- `+0x30 = mip_record_count - 1`: `EXE_CONFIRMED`
- `+0x34 = base width`: `EXE_CONFIRMED`
- TEX0 consumption by `0x1402F9890`: `EXE_CONFIRMED`
- MIPTBP1 downstream consumption: open

## Non-claims

This pass does not establish:

- a serialized descriptor file format;
- a descriptor writer or replacement API;
- complete TIM2 internal semantics;
- complete `0x50` source-record semantics;
- downstream consumption of MIPTBP1 by `0x1402F9890`;
- meaning of descriptor `+0x38..+0x3F`;
- production texture replacement authority.
