# DMC3 HD GData legacy texture + EventTbl structural pass

Date: 2026-09-12

## Scope

This pass binds reader support to user-supplied retail GData resources without assigning semantics that the bytes do not prove. It covers three MOD 0.80 samples, two legacy single-level PTX bundles, one `.tm2`-named descriptor-wrapped DDS resource, and `EventTbl13.bin` through `EventTbl21.bin`.

The implementation rule is deliberately asymmetric: read-side compatibility is promoted where the supplied bytes close structurally; writer/reflow authority is not expanded by this pass.

## Hash-bound inputs

| file | bytes | SHA-256 |
|---|---:|---|
| `basic.ptx` | 12288 | `a4479f843e3859465102315ebf9be6783a47a7802a0c96b18ada0d810b897ff1` |
| `at.ptx` | 12288 | `4c18c6cd4816ca1a608a12efd9eb1bd1467408152f7c31f8079c3c4aff3ce5ce` |
| `at000.mod` | 3648 | `afbaefa0c6414490dd2e00ff35b36d07b15a38d59230d6d720734343f57b797d` |
| `at002.mod` | 7888 | `7f72c15557c980aa4d505de9057483575ac1af1de110b2a3ac06ff67087dfd18` |
| `at003.mod` | 2848 | `9cb1ac281ae7992859b2576dd06dd54548cff9cd2d74e66f03b1c9b76cc65b6a` |
| `i001_90.tm2` | 65776 | `e0f88dd84d59c4e46987f285dc4c3370464a7cca0f66761376262ce449128cdf` |
| `EventTbl13.bin` | 2272 | `8b70cd2feacfe3770a438e5388b556e56f0655a51c6623ef57b45f5a7d853845` |
| `EventTbl14.bin` | 6880 | `38e6f035328a33da877c89ba248833c3f37c29e6eeeb877886ac23d2b6ceb991` |
| `EventTbl15.bin` | 5760 | `4b8a99b8d802596e5279a91c2e48bdeed742306ac6fbbe23bbbf1176bbb470f3` |
| `EventTbl16.bin` | 5152 | `b29310a26cb72c03402eda82fcbdc9b8e7223f120fa1c5fe0d36c2a8b84d3da8` |
| `EventTbl17.bin` | 4128 | `74986d0584b3889d98d04aa7545496144ea20a164365804bdd7351d128c0d3d0` |
| `EventTbl18.bin` | 6816 | `8370d6187f031023fcefa76ad104f7e9bcbcd37a8b7990d4317b90e5afb9015b` |
| `EventTbl19.bin` | 4256 | `42f36fd5ad8aeca759ee0ff183864384687c80df539a14de9bba26786415d0ee` |
| `EventTbl20.bin` | 416 | `1bc56695cb97bc793bde214da6f76d56e9f7434e654a5476b48b74d9bd32f348` |
| `EventTbl21.bin` | 31968 | `6a65f3842f9125273b1c328917b41f3af14e84dfce2e32f4e0fda874fd1c9b1e` |

## MOD 0.80

All three `at*.mod` payloads carry `MOD ` and version 0.80 while retaining the recovered DMC3 MOD physical ABI:

- 0x40-byte document header;
- 0x40-byte object record;
- 0x50-byte mesh record;
- identical five resource-relative vertex streams;
- the same node-domain relative layout;
- the same packed skin/topology control representation;
- mesh-relative generated topology workspace.

All three have one object, one transform-domain node, one texture slot and one mesh. This promotes `0.80` to `CORPUS_CONFIRMED` structural compatibility. It does not assign a higher-level semantic to the `at` resource family.

## Legacy single-level PTX bundle

`basic.ptx` and `at.ptx` are one-texture bundles:

```text
+0x000  u32 textureCount = 1
+0x004  u32 sectorSpan   = 5
+0x008..+0x7FF zero
+0x800  0x70-byte DMC descriptor
+0x870  DDS
EOF     0x3000
```

The DDS is 128x64 DXT5 with raw `dwMipMapCount = 0`, standard single-level flags `0x00081007`, caps `0x00001000`, and an 8192-byte BC3 payload. The portable `dds_bc` reader already interprets raw mip count zero as one effective level.

Observed descriptor envelope:

- `+0x08 = 0x00020185`;
- `+0x0C = 0xAAE4`;
- `+0x10 = 128x64` packed;
- `+0x14 = 1`;
- `+0x18 = 0`;
- `+0x20 = 0x40`;
- `+0x38 = 0`;
- auxiliary pair `+0x3C/+0x40 = 0/0`;
- `+0x44 = 128x64` packed;
- `+0x48/+0x4C = 1/128, 1/64`;
- `+0x60 = 4`;
- `+0x64 = 0x2080` DDS bytes;
- `+0x68 = 8`.

This variant is accepted only by the read compatibility layer. The strict full-mip `TextureSlotFramingParser` remains writer/reflow authority.

## `.tm2` logical name with descriptor-wrapped DDS bytes

`i001_90.tm2` is not a Sony TIM2 image: it has no `TIM2` magic. Its physical layout is:

```text
+0x000  0x70-byte DMC descriptor
+0x070  DDS
EOF     0x100F0
```

The DDS is 256x256 DXT5, one effective mip (raw count zero), 65536-byte BC3 payload. The descriptor carries a 128x128 secondary domain:

- `+0x08 = 0x000201A5`;
- `+0x10 = 128x128` packed;
- `+0x18 = 512`;
- `+0x38 = 65536`;
- `+0x44 = 128x128` packed;
- `+0x48/+0x4C = 1/128, 1/128`;
- `+0x60 = 5`;
- `+0x64 = 0x10080` DDS bytes.

The supplied sample shows `DDS dimensions = 2 * descriptor dimensions`. That relation is structurally validated for this variant, but its original runtime meaning remains `PRESERVED_UNDECODED`. File extension is therefore not used as codec authority: this resource classifies by bytes as descriptor-wrapped DDS.

## EventTbl / EVT command grammar

All nine EventTbl payloads share:

```text
+0x00  "EVT\0"
+0x04  0x00010001
+0x08  absolute file offset of terminal command
+0x0C..+0x1F zero in current corpus
+0x20  command stream
```

Each command begins with one little-endian u32 descriptor:

```text
bits  0..7   opcode
bits  8..15  argument_count
bits 16..31  zero in current corpus
```

The descriptor is followed by `argument_count` little-endian u32 values. This grammar walks all nine files to the declared terminal command without desynchronization: 6661 commands total, 105 observed opcode values, maximum observed arity 6.

The terminal offset always points to opcode `0x20` with zero arguments. The immediately preceding command is opcode `0x01` with zero arguments in 9/9 samples. Bytes after terminal `0x20` are zero padding and every current file size is 0x20-aligned.

The parser exposes opcodes and raw arguments losslessly. No semantic opcode names are promoted in this pass. In particular, recurring 0x02/0x09 pairs remain semantic candidates only until an executable interpreter/dispatcher census binds their meanings.

## Reader boundary

Implemented architecture:

- strict `TextureSlotFramingParser`: unchanged authoring/reflow authority;
- `TextureSlotFramingReader`: strict-first read union with the two narrow legacy DXT5 variants above;
- `formats::evt::Parser`: lossless structural EVT reader;
- GDSpaces classifier: recognizes `EVT\0`, legacy PTX bundle bytes and `.tm2`-named wrapped DDS by content;
- portable `ReaderCore`: exports both legacy texture read compatibility and EVT for Native Reader consumers.

## Explicit non-claims

This pass does not establish:

- Capcom writer equivalence for either legacy texture descriptor variant;
- that `.tm2` generally means descriptor-wrapped DDS across all DMC3 resources;
- semantic meaning of descriptor encodings `0x20185` or `0x201A5` beyond identifying these bounded observed variants;
- semantic names for EVT opcodes;
- that EventTbl number always equals mission number for every table;
- runtime purpose of the `at*.mod` family.
