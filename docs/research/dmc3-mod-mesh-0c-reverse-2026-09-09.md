# DMC3 HD MOD — mesh +0x0C reverse closure

**Branch:** `reverse/mod-completion-20260907`  
**Canonical executable:** `dmc3.exe`  
**SHA-256:** `e454272ed0fb0247fcbcf300e5d55d7a3e96d50b89b9ffaff81bb978dcbdd082`

## ABI

```text
mesh +0x00 u16 vertex/element count
mesh +0x02 u16 texture slot
mesh +0x04..+0x0A four u16 GS CLAMP values
mesh +0x0C u32 unresolved
mesh +0x10 first u64 stream pointer
```

The width and serialized position are `STRUCTURAL_CONFIRMED`. The semantic type remains raw `u32`.

## Corpus

Current MOD corpus: `180/180` meshes serialize `+0x0C == 0`. The two currently bound EFM meshes are also zero. This is `CORPUS_CONFIRMED` only.

## Canonical EXE boundary

Common material helper `0x1402F9890` consumes the texture slot and four GS CLAMP words through `+0x0A`. The audited MOD post-load `0x1402FE3B0` and runtime builder `0x1402FE6A0` do not establish a positive semantic transfer for source `+0x0C`. EFM load/build and the SCM comparison likewise provide no evidence that can be safely promoted into MOD semantics.

Physical placement between the 12-byte prefix and the first aligned pointer is not proof of alignment or padding.

## C++ / preservation

`include/dmc_rengine/analysis/mod/mesh_serialized.hpp` preserves the raw `u32`. A registered synthetic regression writes `0xDEADBEEF` at `+0x0C` and requires the preservation projection to return the same value.

Therefore no future writer may zero-normalize the field solely because retail samples are zero.

## Status

```text
serialized offset/width     STRUCTURAL_CONFIRMED
bounded zero histogram      CORPUS_CONFIRMED
global semantic             PRESERVED_UNDECODED
writer policy               preserve
```

## Rejected hypotheses

- padding because the field sits before an aligned `u64` — `REJECTED`;
- reserved because the bounded corpus is zero — `REJECTED`;
- safe writer zero-normalization — `REJECTED`.

A provenance-complete whole-EXE consumer census is still required for a stronger dormant/reserved conclusion.
