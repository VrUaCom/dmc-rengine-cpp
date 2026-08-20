# GDSpaces L1 Pass 80 — Full DMC3 texture descriptor structural authority

Date: 2026-08-20  
Layer: L1 Resource Materialization  
Base: Pass 79 safe intrinsic DDS reintegration

## Purpose

Promote the DMC3 `0x70` texture descriptor from a partially checked wrapper to a complete **structural** corpus contract without inventing original-runtime semantics for fields whose meaning is still unknown.

The preserved v6 stage-drop corpus contains **154 descriptor/DDS pairs** across direct wrapped DDS slots and **58** multi-texture bundles.

## Complete 0x70 structural map

All 28 little-endian `u32` positions in the `0x70` descriptor are now structurally classified in the observed corpus.

| Offset | Structural relation |
| --- | --- |
| `+0x00` | `0` |
| `+0x04` | `0` |
| `+0x08` | `0x20000 | (fullMipCount << 8) | compressionCode`; DXT1=`0x86`, DXT5=`0x88` |
| `+0x0C` | `0xAAE4` |
| `+0x10` | `(ddsHeight << 16) | ddsWidth` |
| `+0x14` | `1` |
| `+0x18` | `ddsWidth * 2` for DXT1; `ddsWidth * 4` for DXT5 |
| `+0x1C` | `0` |
| `+0x20` | `0x40` |
| `+0x24` | `0` |
| `+0x28` | `0` |
| `+0x2C` | `0` |
| `+0x30` | `0` |
| `+0x34` | `0` |
| `+0x38` | exact block-compressed mip-chain payload size |
| `+0x3C` | bounded auxiliary mode: observed `0`, `1`, `2`; semantics unresolved |
| `+0x40` | bounded auxiliary value; zero iff mode is zero; semantics unresolved |
| `+0x44` | packed secondary dimensions `(secondaryHeight << 16) | secondaryWidth`; semantics unresolved |
| `+0x48` | exact IEEE-754 float32 bits of `1.0 / secondaryWidth` |
| `+0x4C` | exact IEEE-754 float32 bits of `1.0 / secondaryHeight` |
| `+0x50` | `0` |
| `+0x54` | `0` |
| `+0x58` | `0` |
| `+0x5C` | `0` |
| `+0x60` | `0` for DXT1; `4` for DXT5 |
| `+0x64` | exact full DDS byte size |
| `+0x68` | `8` |
| `+0x6C` | `0` |

## Corpus invariants

Across all **154/154** descriptor/DDS pairs:

- DDS mip count is the complete chain: `floor(log2(max(width,height))) + 1`;
- `+0x38` equals the mathematically exact DXT1/DXT5 block-compressed full mip-chain payload size;
- `+0x64 == 128 + +0x38`;
- all 13 descriptor positions classified as zero are exactly zero;
- secondary dimensions are non-zero and are either exactly the DDS dimensions or exactly half in both axes;
- **50/154** use the same-scale relation;
- **104/154** use the half-scale relation;
- `+0x48/+0x4C` are exact float32 reciprocal bit patterns for the secondary dimensions;
- auxiliary mode distribution is `0:112`, `1:33`, `2:9`;
- auxiliary value is zero iff mode is zero;
- every non-zero auxiliary mode appears on a DXT5 texture in this corpus.

The corpus contains **47 DXT1** and **107 DXT5** descriptor/DDS pairs.

A duplicate-content check found **43 DDS SHA-256 duplicate groups**; within every group the entire `0x70` descriptor is byte-identical. No same-DDS/different-descriptor counterexample exists in the preserved corpus.

## Parser promotion

`TextureSlotFramingParser` now validates the full structural envelope above rather than only the earlier DDS-derived subset. `TextureSlotEntry` exposes `secondary_width`, `secondary_height`, `auxiliary_mode`, and `auxiliary_value` as structural fields only.

The Pass 79 reintegrator framing-equivalence gate is correspondingly expanded so same-layout payload edits must preserve these fields too.

## Important serializer boundary

This pass does **not** make size-changing DDS serialization safe.

Two descriptor families are not uniquely derivable from DDS alone:

1. secondary dimensions can be either `1x` or `1/2x` relative to DDS dimensions;
2. the auxiliary mode/value pair has a proven bounded relation but unresolved semantics and cannot be synthesized from DDS bytes with evidence-backed authority.

Therefore a future dimension/mip-changing serializer must either preserve a proven source relation or require explicit authored metadata. It must not guess.

## Explicit non-claims

Pass 80 does **not** establish:

- semantic names for secondary dimensions or the auxiliary pair;
- size-changing texture serialization;
- DXT3 support;
- original DMC3 runtime descriptor-parser ABI behavior;
- original-game consumption of rebuilt PAC/PNST/NBZ;
- Layer 1 COMPLETE.
