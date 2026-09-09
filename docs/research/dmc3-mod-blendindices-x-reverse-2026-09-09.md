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

The canonical parser maps serialized mesh `+0x28` to `blend_indices_offset`.

The current bounded MOD corpus contains 20,976 vertices and lane X is zero in 20,976/20,976 samples. This is `CORPUS_CONFIRMED` bounded evidence, not permission to call the byte padding or to normalize it.

## Serialized -> runtime pointer provenance

The canonical MOD runtime builder closes the physical pointer chain:

```text
serialized mesh +0x28
  -> 0x1402FE83A
  -> runtime mesh +0x140[platform] at 0x1402FE83E
  -> mirrored runtime slot at 0x1402FE860 / 0x1402FE868
```

This gives a provenance-confirmed base from which CPU lane reads can be classified without relying on arbitrary four-byte-stride patterns elsewhere in the executable.

## Provenance-confirmed runtime `+0x140` consumers

The model subsystem has four relevant reads of this runtime stream slot in the audited paths.

### `0x1402F3CE6` — CPU skin/matrix path

The pointer comes directly from runtime `+0x140`. At `0x1402F3D0A` the executable reads byte lane `+1` from the four-byte stream and shifts the encoded matrix-row offset right by two.

This is the positive control proving an active lane-Y CPU semantic.

No lane-X read occurs in this consumer.

### `0x1402F55E8` — secondary model path

The runtime `+0x140` pointer is stored to local `rbp+0x120`. Bounded function dataflow shows no subsequent read of that local before function exit. The pointer load therefore does not establish any lane semantic, and in particular it does not establish lane-X use.

### `0x1402F8841` — render-command construction

The whole stream pointer is loaded from runtime `+0x140` and later copied into a generated command record at command `+0x30`. The CPU does not dereference an individual byte lane in this path.

### `0x1402FF171` — render-command construction

The same pattern occurs in the second command builder. At `0x1402FF30B..0x1402FF30F`, the runtime `+0x140` pointer is copied as a whole into command `+0x30`. Again there is no CPU lane dereference.

This distinction matters: forwarding a pointer is an escape edge, not proof that every byte in the pointed stream is consumed.

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

This is `EXE_CONFIRMED` negative evidence for lane-X use in the audited canonical render-skin shader set.

Combined with the runtime-slot census, the current evidence is stronger than a shader-only negative claim: the direct CPU consumers of the provenance-confirmed stream also do not read lane X.

## Rejected false-positive lane-0 candidates

Two whole-executable scaled-four byte-read candidates were reconstructed by pointer provenance and rejected:

- `0x140316726` belongs to an interpolated 4-byte color/control table and performs fixed-point interpolation;
- `0x140325970` is based on a static global table near `0x1405D10F0`, not the MOD runtime blend-index stream.

Offset shape alone is not sufficient evidence.

## C++ contract

`include/dmc_rengine/formats/mod_skin.hpp` deliberately does not call lane X reserved/constant. It states the evidence boundary: zero in the bounded corpus, unused by the audited render-skin shaders, and writer-preserved while global meaning remains unresolved.

`tests/mod_skin_tests.cpp` contains a synthetic non-zero X case. The proven y/z/w skin interpretation must remain unchanged when X changes. This prevents the semantic decoder from converting a corpus observation into an invalid `x == 0` requirement.

## Status

```text
serialized ABI                         STRUCTURAL_CONFIRMED
serialized +0x28 -> runtime +0x140     EXE_CONFIRMED
bounded corpus X == 0                  CORPUS_CONFIRMED
direct provenance-confirmed CPU X use EXE_CONFIRMED (no consumer found)
audited MOD/EFM shader X use           EXE_CONFIRMED (no consumer found)
indirect command/shader escape         PRESERVED_UNDECODED
writer policy                          preserve
```

No Blender/importer behavior is used as authority.

## Rejected hypotheses

- `BLENDINDICES.x` is reserved because an importer ignores it — `REJECTED`.
- Zero in the bounded corpus authorizes writer zero-normalization — `REJECTED`.
- Every four-byte-stride lane-0 read in the executable is a BLENDINDICES consumer — `REJECTED`.
- Forwarding the stream pointer into a render command proves lane X is consumed — `REJECTED`.

## Remaining gate

The direct CPU side is now tightly bounded. The remaining gate is the indirect render-command escape: verify that every canonical shader/input-layout path reachable from command `+0x30` belongs to the audited MOD/EFM families or otherwise classify the additional consumer. Until that last escape is closed, lane X remains `PRESERVED_UNDECODED`, not reserved/padding.
