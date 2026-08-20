# GDSpaces L1 Pass 82 — Size-changing texture-slot serializer

Date: 2026-08-21  
Layer: L1 Resource Materialization  
Base: Pass 81 DDS header corpus authority

## Purpose

Close the first bounded **size-changing** authoring path between an intrinsic DDS image and the complete physical DMC3 texture child consumed by the generic PAC/PNST writer.

```text
authored complete DDS
 -> DdsImageParser
 -> reconstruct DMC3 0x70 descriptor
 -> rebuild wrapped record or bundle sector layout
 -> TextureSlotFramingParser
 -> complete validated physical child
```

The writer does not encode pixels and does not invent DDS content. It accepts a complete authored DDS byte image and reconstructs only the DMC3 physical framing around it.

## Authoring envelope

Pass 82 deliberately promotes only the subset for which the preserved corpus provides sufficient write authority.

A changed texture is accepted only when:

1. source physical framing passes `TextureSlotFramingParser`;
2. the request carries the exact SHA-256 of the bounded source DDS;
3. the authored DDS passes `DdsImageParser`;
4. the authored DDS uses the `standard_corpus` profile;
5. source DXT compression is preserved;
6. authored dimensions are one of the compression-specific dimension pairs actually observed in the preserved standard-profile corpus;
7. the changed source descriptor has `auxiliary_mode = 0` and `auxiliary_value = 0`;
8. the source secondary-dimension relation is preserved as either `1x` or `1/2x`.

Across the 154 descriptor/DDS corpus relationships, 112 have zero auxiliary metadata and therefore lie inside this first conservative source-authoring envelope. The other 42 remain read/preserve authority only until their auxiliary metadata receives independent authoring authority.

## Descriptor rebuild

For an accepted authored DDS, the 0x70 descriptor is reconstructed only from Passes 80–81 structural facts:

- encoding field from DXT type + authored mip count;
- dimensions from authored DDS;
- row-related field from width and DXT type;
- exact DXT mip-chain payload size;
- full DDS size;
- source `1x` versus `1/2x` secondary-dimension relation;
- exact float reciprocal bits for the rebuilt secondary dimensions;
- format field from DXT1/DXT5;
- corpus-confirmed constants and zero fields;
- auxiliary fields fixed to zero because Pass 82 rejects changed nonzero-auxiliary descriptors.

No unresolved runtime semantic is guessed.

## Bundle reflow authority

Real-corpus reverse established the record-span rules independently of the writer:

- 71/71 non-final records: `sector_span = ceil((0x70 + DDS_size) / 0x800)`;
- 41/41 aligned final records: same formula;
- 17/17 compact final records: `sector_span = 0`, with the DDS ending exactly at bundle EOF;
- unused bytes in the 0x800 bundle header after the count/span table are zero in 58/58 bundles.

Pass 82 therefore:

- rebuilds a changed aligned record with the minimum corpus-confirmed sector span and zero padding;
- preserves compact-final style when the source final record uses span zero;
- copies every unchanged physical record byte-for-byte even when a preceding changed record moves its descriptor offset;
- patches only the bundle sector-span table in the copied 0x800 header.

## Post-build proof

A result is not returned merely because bytes were emitted.

The complete physical child must be reparsed with `TextureSlotFramingParser`. The writer then proves:

- framing kind and texture count remain valid;
- every authored DDS is reproduced byte-for-byte after canonical reparse;
- the source `1x`/`1/2x` secondary relation is retained;
- changed descriptors have zero auxiliary metadata;
- every unchanged bundle physical record remains byte-identical;
- a receipt binds source/output artifact SHA-256 and per-texture source/output DDS SHA-256, sizes, descriptor offsets and sector spans.

## Regression boundary

Synthetic CI coverage includes:

- wrapped DXT5 DDS growth between observed standard-profile geometries;
- bundle record growth that shifts a following unchanged record;
- exact preservation of that shifted untouched physical record;
- compact-final span-zero preservation;
- stale source SHA rejection;
- malformed authored DDS rejection;
- compression-change rejection;
- parser-valid but corpus-unobserved dimension rejection;
- exact observed depth=1 exception rejection for authoring;
- changed nonzero-auxiliary descriptor rejection;
- no-size-change rejection, leaving same-size editing under Pass 79;
- output-size budget rejection.

## Explicit non-claims

Pass 82 does **not** establish:

- authoring semantics for nonzero auxiliary descriptor metadata;
- DXT1↔DXT5 conversion authority;
- arbitrary power-of-two texture geometry authoring;
- generalized depth=1 texture authoring;
- original DMC3 runtime texture ABI behavior;
- real-corpus execution of the exact current compiled serializer;
- original-game consumption of rebuilt texture/PAC/PNST/NBZ artifacts;
- Layer 1 COMPLETE.

The next useful proof after CI is composition of this complete size-changing physical child with `RelativeSlotPackedReflowWriter`, followed by a controlled real-corpus aux=0 texture case and then the already-established NBZ repack path.
