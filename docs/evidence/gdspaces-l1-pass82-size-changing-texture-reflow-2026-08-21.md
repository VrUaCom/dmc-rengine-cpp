# GDSpaces L1 Pass 82 — Size-changing texture packed reflow

Date: 2026-08-21  
Layer: L1 Resource Materialization

## Purpose

Cross the first evidence-backed size-changing texture boundary:

`authored DDS -> rebuilt DMC3 physical texture slot -> generic PAC/PNST physical-child reflow`

without guessing unresolved texture metadata.

## Safe authoring domain

Pass 80 classified all 154 real descriptor/DDS pairs structurally. Pass 81 made the canonical DDS header authorable. Pass 82 deliberately accepts only the subset whose unresolved auxiliary pair is absent:

- 112/154 descriptors have `auxiliary_mode=0` and `auxiliary_value=0`;
- 42/154 carry unresolved nonzero auxiliary metadata and remain fail-closed;
- within the 112 safe descriptors, 12 use secondary dimensions equal to DDS dimensions and 100 use exact half dimensions;
- the safe set contains all 47 observed DXT1 descriptors and 65 DXT5 descriptors.

Compression switching is not part of this pass. The authored DDS must preserve the source DXT1/DXT5 kind.

## Bundle geometry authority

The physical bundle writer uses the real-corpus rule:

- 71/71 non-final records: `sector_span = ceil((0x70 + dds_size) / 0x800)`;
- 41/41 aligned final records: the same formula;
- 17/17 compact final records: `sector_span = 0` and the DDS ends exactly at EOF;
- unused bundle-header bytes are zero in 58/58 bundles.

Pass 82 preserves the source compact-final vs aligned-final class.

## Real-corpus independent matrix

Every safe descriptor was subjected to a controlled one-step power-of-two resize inside the Pass 81 64..1024 product authoring envelope. DXT kind and source secondary-dimension relation were preserved.

Results:

- 112/112 safe textures resized;
- those textures occur across 45 physical texture slots;
- 45/45 rebuilt direct/bundle slots passed strict structural revalidation;
- zero rebuilt slots failed;
- in mixed bundles, unresolved nonzero-auxiliary records were left byte-exact while safe records were rebuilt.

This is independent real-byte algorithm evidence. It is intentionally not labeled as a current compiled C++ writer corpus execution receipt.

## C++ writer contract

`TextureSlotPackedReflowWriter`:

1. validates the source with `TextureSlotFramingParser`;
2. binds every authored request to the exact bounded source DDS SHA-256;
3. validates authored bytes through `Dmc3DdsProfile`;
4. preserves source compression;
5. rejects changed textures with nonzero unresolved auxiliary metadata;
6. preserves source secondary-dimension class (`same` or `half`);
7. rebuilds the complete 0x70 descriptor for changed textures;
8. rebuilds bundle sector spans and zero padding;
9. preserves source compact-final/aligned-final class;
10. reparses the complete output physical slot;
11. verifies every output DDS against the exact expected bytes;
12. returns only a self-validating receipt.

The regression suite additionally composes the output complete physical child into `RelativeSlotPackedReflowWriter`, proving the intended layer separation for a size-changing PAC parent.

## Non-claims

Pass 82 does not close nonzero auxiliary semantics, DXT compression switching, DXT3, real current-C++ corpus execution, retail NBZ texture-edit repack, original-game consumption, or Layer 1 COMPLETE.
