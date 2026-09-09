# DMC3 HD MOD — mesh +0x38 reverse closure

**Branch:** `reverse/mod-completion-20260907`  
**Canonical executable:** `dmc3.exe`  
**SHA-256:** `e454272ed0fb0247fcbcf300e5d55d7a3e96d50b89b9ffaff81bb978dcbdd082`

## Result

`+0x38` is not generic mesh padding. It is a family-specific auxiliary vertex-stream slot whose runtime role differs between MOD and EFM.

## MOD direct EXE evidence

Canonical MOD post-load `0x1402FE3B0` relocates the established source streams at `+0x10/+0x18/+0x20/+0x28/+0x30` and does not relocate serialized `+0x38`.

Canonical MOD runtime builder `0x1402FE6A0` does not forward source `+0x38`. Instead it explicitly disables the corresponding runtime auxiliary-stream positions at `0x1402FE8C4` and `0x1402FE8DD` by writing zero.

Current MOD corpus: `180/180` meshes serialize `+0x38 == 0`.

This closes the MOD-specific canonical runtime role as inactive in the audited path.

## EFM positive control

The homologous EFM slot is live:

```text
0x1402F7BBD  read EFM mesh +0x38
0x1402F7BCC  write relocated +0x38
0x1402F7F88  read relocated +0x38
0x1402F7F8C  forward to runtime +0x160
```

Both currently bound EFM meshes have non-zero `+0x38` values. The EFM shader family independently requires the extra `COLOR0` vertex input.

Therefore EFM `+0x38 -> COLOR0` does not authorize naming MOD `+0x38` COLOR0. Same offset is not same meaning.

## C++ / preservation

`include/dmc_rengine/analysis/mod/mesh_serialized.hpp` describes the shared physical position as a family-specific auxiliary-stream slot and preserves the raw `u64`. A synthetic regression uses `0x0123456789ABCDEF` to ensure non-zero source bytes remain recoverable.

Runtime inactivity still does not authorize a future writer to replace a source value with zero.

## Status

```text
shared physical slot                STRUCTURAL_CONFIRMED
MOD canonical runtime inactivity    EXE_CONFIRMED
MOD bounded zero histogram          CORPUS_CONFIRMED
MOD-specific role closure           EXE_AND_CORPUS_CONFIRMED
EFM +0x38 -> COLOR0                 EXE_AND_CORPUS_CONFIRMED
writer policy                       preserve
```

## Rejected hypotheses

- generic padding — `REJECTED`;
- MOD COLOR0 solely because EFM uses the homologous offset — `REJECTED`;
- cross-family offset equality proves semantic equality — `REJECTED`.
