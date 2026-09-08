# DMC3 HD TSC ↔ EFM ↔ PTX binding — em000 corpus pass (2026-09-08)

**Branch:** `reverse/tsc-em000-20260908`  
**Corpus:** `em000-extract.zip`  
**Corpus SHA-256:** `306130125f09824811289366324f4208c3c1aba880c5a7efa3953a88d566d07b`

## Primary result

The sole bound TSC resource is not only identifiable as a dedicated text-serialized motion/control format. Its `TexNo` value matches both the immediately preceding EFM model's mesh texture selector and the actor PTX texture bundle's valid texture domain.

Top-level cluster:

```text
em000_000.bin  -> PTX texture bundle, 4 textures
...
em000_023.efm  -> EFM, both meshes use texture_slot 2
em000_024.txt  -> TSC, contains TexNo 2
```

The TSC source says:

```text
.TSC
# RELATIVE
<Start
ScrlNo 0
ScrlType 3
TexNo 2
DirUV stay, down
TimeUV 450, 90
MinimumUV 0.004, 0.003
End>
<Finish>
$
# ABSOLUTE
```

## EFM side

`em000_023.efm` has two objects and one mesh in each object.

Both mesh records contain:

```text
mesh.texture_slot = 2
```

The EFM header advertises a texture-slot domain of three entries, making selector `2` the highest valid zero-based slot in that model.

## PTX side

`em000_000.bin` structurally binds to the canonical DMC3 PTX bundle framing and contains four texture payloads, so valid PTX slot indices are `0..3`.

Embedded DDS payloads:

```text
slot 0  DDS @ 0x000870  256x512  DXT1  10 mips
slot 1  DDS @ 0x016070  256x512  DXT5  10 mips
slot 2  DDS @ 0x041070  256x512  DXT5  10 mips
slot 3  DDS @ 0x06C070  256x128  DXT5   9 mips
```

Therefore `TexNo 2` is a valid actor texture-bundle selector and lands on a concrete 256x512 DXT5 texture.

## Safe corpus conclusion

The three independent resources agree on the same selector value:

```text
TSC TexNo 2
    |
    +--> EFM mesh.texture_slot 2      # 2/2 meshes
    |
    +--> PTX texture slot 2           # valid in 4-slot bundle
```

This supports the following promotion:

> In the bound em000 TSC, `TexNo` is a texture-slot selector in the same actor texture domain used by the associated model material binding.

Status: **CORPUS_CONFIRMED** for this resource cluster.

The existing canonical executable separately proves `.tsc` belongs to the motion/control extension dispatcher, but the exact downstream TSC field consumer still needs direct EXE recovery before raising this field relationship to `EXE_AND_CORPUS_CONFIRMED`.

## Why the physical adjacency matters but is not enough alone

TSC is immediately after EFM in this corpus:

```text
slot 023 EFM
slot 024 TSC
```

Adjacency by itself would be weak evidence. The selector equality makes the relationship materially stronger:

- EFM has two independent mesh records and both select slot 2;
- TSC independently says `TexNo 2`;
- PTX independently contains slot 2 in its four-entry domain.

This is a three-way cross-resource invariant, not a filename guess.

## Interpretation boundary

The source tokens strongly suggest texture-coordinate scrolling:

```text
ScrlType
TexNo
DirUV
TimeUV
MinimumUV
```

and the value pair `DirUV stay, down` is consistent with directional UV control. However, this pass does **not** promote a mathematical scroll equation, time unit, UV unit, or `ScrlType 3` enum meaning.

Current safe model:

```text
TSC
  ScrlNo       raw integer selector
  ScrlType     raw integer mode
  TexNo        texture-slot selector      CORPUS_CONFIRMED in em000 cluster
  DirUV        two raw direction tokens
  TimeUV       two raw numeric tokens
  MinimumUV    two raw numeric tokens
```

## Next EXE targets

1. start from the already recovered `.tsc -> type code 5` extension path;
2. follow type 5 to the concrete parser/loader;
3. find literal comparisons or token registration for `ScrlNo`, `ScrlType`, `TexNo`, `DirUV`, `TimeUV`, and `MinimumUV`;
4. trace the stored `TexNo` value to a model/texture descriptor selector;
5. recover the update equation consuming `DirUV`, `TimeUV`, and `MinimumUV`;
6. determine `RELATIVE` versus `ABSOLUTE` section behavior.

## Architectural consequence

TSC analysis should bind to texture/material state outside the raw parser:

```text
formats/tsc
    -> textual syntax / typed raw values

analysis/tsc/texture_binding
    -> TSC TexNo
    -> associated MOD/EFM texture_slot domain
    -> PTX / wrapped texture descriptor domain
```

The generic TXT lexer and PTX/EFM parsers remain separate authorities.
