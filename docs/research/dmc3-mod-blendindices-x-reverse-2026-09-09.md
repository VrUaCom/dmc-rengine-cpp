# DMC3 HD MOD — BLENDINDICES.x reverse closure

**Branch:** `reverse/mod-completion-20260907`  
**Canonical executable:** `dmc3.exe`  
**SHA-256:** `e454272ed0fb0247fcbcf300e5d55d7a3e96d50b89b9ffaff81bb978dcbdd082`

## Serialized ABI

The MOD blend-index vertex stream is four bytes per vertex:

```text
+0x00 u8 lane X
+0x01 u8 lane Y
+0x02 u8 lane Z
+0x03 u8 lane W
```

The current bounded MOD corpus contains 20,976 vertices and lane X is zero in 20,976/20,976 samples. This is `CORPUS_CONFIRMED` bounded evidence, not permission to call the byte padding or to normalize it.

## Canonical shader census

The canonical executable embeds/dispatches the audited MOD shader families:

```text
DMC3_MOD.hlsl
DMC3_MOD_SP.hlsl
DMC3_MOD_STX.hlsl
```

The related EFM positive-control families were checked as well:

```text
DMC3_EFM.hlsl
DMC3_EFM_SP.hlsl
DMC3_EFM_STX.hlsl
DMC3_EFM_VA.hlsl
DMC3_EFM_VA_SP.hlsl
```

Whole-executable source-string census:

```text
matIndex.x   0
matIndxX     0
matIndex.y 128
matIndex.z  16
matIndex.w  16
```

This is `EXE_CONFIRMED` negative evidence for lane-X use in the audited canonical render-skin shader set. It is not a whole-program CPU proof.

## CPU positive control

At `0x1402F3D0A` the executable reads byte lane `[1]` from the four-byte blend stream. The following operation divides the encoded matrix-row start by four before indexing the node/world-matrix domain. This closes the active lane-Y mapping and validates the four-byte stream provenance.

No equivalent lane-X semantic is promoted by this pass. The whole-EXE lane-0 CPU dataflow census remains the required next step before declaring lane X globally unused/reserved.

## C++ contract correction

`include/dmc_rengine/formats/mod_skin.hpp` no longer calls lane X reserved/constant. It states the exact boundary: zero in the bounded corpus, unused by audited render-skin shaders, global CPU role unresolved, writer policy preserve.

`tests/mod_skin_tests.cpp` now includes a synthetic `BLENDINDICES.x = 0xA5` case. The test requires the y/z/w skin interpretation to remain identical to the same vertex with X=0. This prevents the semantic decoder from silently turning the corpus observation into an `x == 0` validity rule.

The test is deliberately not evidence that X is globally unused. It only enforces that the currently proven skin semantic does not consume X.

## Status

```text
serialized ABI                   STRUCTURAL_CONFIRMED
bounded corpus X == 0            CORPUS_CONFIRMED
canonical render-skin X use      EXE_CONFIRMED (no consumer found)
global CPU semantic              PRESERVED_UNDECODED
writer policy                    preserve
```

No Blender/importer behavior is used as authority.

## Rejected hypotheses

- `BLENDINDICES.x` is reserved because an importer ignores it — `REJECTED`.
- Zero in the bounded corpus authorizes writer zero-normalization — `REJECTED`.
- Absence of `.x` in render shaders proves absence of all CPU consumers — `REJECTED`.

## Next direct-EXE action

Trace every CPU consumer of the relocated four-byte blend-index stream and classify byte reads at offsets 0/1/2/3 from a provenance-confirmed stream base. Only a complete lane-0 census can authorize an `EXE_CONFIRMED` global unused/reserved promotion. Until then the serialized byte remains `PRESERVED_UNDECODED`.
