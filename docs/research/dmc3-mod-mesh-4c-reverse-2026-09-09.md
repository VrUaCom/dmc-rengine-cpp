# DMC3 HD MOD — mesh +0x4C reverse closure

**Branch:** `reverse/mod-completion-20260907`  
**Canonical executable:** `dmc3.exe`  
**SHA-256:** `e454272ed0fb0247fcbcf300e5d55d7a3e96d50b89b9ffaff81bb978dcbdd082`

## ABI

```text
mesh +0x40 u64 generated-topology workspace relative offset
mesh +0x48 u32 generated topology word count
mesh +0x4C u32 unresolved
mesh +0x50 next record
```

The serialized position and width are `STRUCTURAL_CONFIRMED`. The semantic remains raw `u32`.

## Positive control: +0x48

Canonical MOD post-load writes the generated topology count to `mesh +0x48` at `0x1402FE684`. The MOD runtime builder later reads that count at `0x1402FE904` and forwards it to runtime `+0x198` at `0x1402FE907`.

No companion transfer for `+0x4C` is established in this audited chain. The EFM load/build path gives the same bounded negative result for the trailing dword.

## Corpus

Current bounded evidence:

```text
MOD  180/180 meshes -> +0x4C == 0
EFM    2/2 meshes   -> +0x4C == 0
```

This is `CORPUS_CONFIRMED`, not proof of padding/reserved semantics.

## C++ / preservation

`include/dmc_rengine/analysis/mod/mesh_serialized.hpp` preserves the raw value. A synthetic regression writes `0xA5A55A5A` to `+0x4C` and requires exact recovery through the preservation projection.

## Status

```text
serialized offset/width       STRUCTURAL_CONFIRMED
bounded zero histogram        CORPUS_CONFIRMED
global semantic               PRESERVED_UNDECODED
writer policy                 preserve
```

## Rejected hypotheses

- second half of a proven runtime topology-count pair — `REJECTED`;
- padding because it terminates the 0x50-byte record — `REJECTED`;
- safe writer zero-normalization — `REJECTED`.

A provenance-complete whole-EXE census is still required for a stronger dormant/reserved conclusion.
