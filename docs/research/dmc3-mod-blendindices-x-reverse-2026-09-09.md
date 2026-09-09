# DMC3 HD MOD — BLENDINDICES.x reverse closure

**Branch:** `reverse/mod-completion-20260907`  
**Canonical executable:** `dmc3.exe`  
**SHA-256:** `e454272ed0fb0247fcbcf300e5d55d7a3e96d50b89b9ffaff81bb978dcbdd082`

## Result

The former indirect GPU escape is now closed for the canonical executable.

The serialized four-byte blend-index stream is:

```text
+0x00 u8 lane X
+0x01 u8 lane Y
+0x02 u8 lane Z
+0x03 u8 lane W
```

Serialized mesh `+0x28` is the stream pointer. The MOD runtime builder installs it into runtime mesh `+0x140[platform]`.

The bounded corpus still has X = 0 in 20,976 / 20,976 vertices. That remains corpus evidence only and does not authorize writer normalization.

## Runtime pointer chain

Canonical EXE provenance:

```text
serialized mesh +0x28
  -> 0x1402FE83A
  -> runtime mesh +0x140 at 0x1402FE83E
  -> mirrored platform slot at 0x1402FE860 / 0x1402FE868
```

Render construction reads runtime `+0x110/+0x120/+0x130/+0x140/+0x150/+0x160` and projects those pointers into a 0x80-byte draw/input descriptor. In that descriptor:

```text
+0x18 <- runtime +0x110
+0x20 <- runtime +0x120
+0x28 <- runtime +0x130
+0x30 <- runtime +0x140   BLENDINDICES
+0x38 <- runtime +0x160
+0x40 <- runtime +0x150
```

The critical `BLENDINDICES` copy is visible at `0x1402F8841` followed by `0x1402F89F2..0x1402F89F6`. The second render builder repeats the same projection at `0x1402FF171` and `0x1402FF30B..0x1402FF30F`.

This proves that lane X reaches the canonical graphics-input interface as part of the same four-component stream.

## Direct CPU consumer census

A provenance-confirmed CPU consumer at `0x1402F3D0A` reads lane Y (`+1`) and divides the encoded matrix-row offset by four before matrix indexing. This is the active positive control.

No provenance-confirmed direct CPU consumer reads lane X.

The unrelated scaled-four lane-0 candidates `0x140316726` and `0x140325970` remain `REJECTED` after pointer-provenance reconstruction.

## Compiled DXBC census

A fresh whole-image scan found:

```text
DXBC blobs total                           69
DXBC input signatures with BLENDINDICES     8
BLENDINDICES register                       3
BLENDINDICES Mask                         0xF
BLENDINDICES ReadWriteMask                0xE
component type                         uint32
```

All eight compiled input signatures agree exactly.

For a Direct3D input signature, `ReadWriteMask` identifies input components that are read by the compiled shader. Therefore `0xE` means Y/Z/W are read and X is not read.

This independently matches the embedded HLSL source census:

```text
matIndex.x   0
matIndxX     0
matIndex.y 128
matIndex.z  16
matIndex.w  16
```

The canonical source families include the MOD, MOD_SP, MOD_STX and corresponding EFM variants.

## Evidence conclusion

The following is now `EXE_CONFIRMED`:

```text
serialized +0x28
  -> runtime +0x140
  -> draw/input descriptor +0x30
  -> BLENDINDICES uint4 input
  -> compiled DXBC read mask 0xE
  -> canonical shader consumes Y/Z/W, not X
```

Together with the direct CPU census, the canonical runtime has no proven lane-X consumer.

This closes the prior indirect command/shader escape gate.

## Important ABI distinction

Canonical runtime non-use does **not** mean the serialized byte should be renamed padding or forcibly zeroed.

The correct writer contract remains:

```text
read lane X as raw u8
preserve lane X exactly on round-trip
semantic skin decoder ignores lane X
never reject a MOD only because X is non-zero
```

That distinction protects compatibility with non-canonical or future corpus data.

## Status

```text
serialized ABI                         STRUCTURAL_CONFIRMED
serialized +0x28 -> runtime +0x140     EXE_CONFIRMED
runtime +0x140 -> draw descriptor      EXE_CONFIRMED
compiled DXBC BLENDINDICES X use       EXE_CONFIRMED (not read)
direct canonical CPU X use             EXE_CONFIRMED (no consumer found)
bounded corpus X == 0                  CORPUS_CONFIRMED
semantic label                         PRESERVED_UNDECODED
writer policy                          preserve raw byte
```

## Rejected hypotheses

- X is padding merely because corpus X is zero — `REJECTED`.
- X may be normalized to zero — `REJECTED`.
- A `uint4` interface proves all four lanes are used — `REJECTED`.
- External importer behavior is semantic authority — `REJECTED`.

## Closure

The BLENDINDICES.x reverse gate is closed for canonical-runtime consumption. No further shader/input-layout tracing is required before the MOD unknown-field slice can advance to the remaining secondary/header/object/node regions and writer promotion work.
