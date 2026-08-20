# GDSpaces L1 Pass 79 — Safe intrinsic DDS reintegration

Date: 2026-08-20  
Layer: L1 Resource Materialization  
Base: Pass 78 real child authority / DMC3 texture-slot framing

## Purpose

Close the next bounded materialization seam after Pass 78:

`intrinsic editable DDS -> complete physical DMC3 texture slot -> generic PAC/PNST child writer`

The generic relative-slot writer must not learn DMC3 texture descriptor or bundle semantics. It continues to consume complete physical child images only.

## Accepted Pass 79 write domain

The first texture reintegrator is intentionally same-layout and fail-closed.

An authored DDS is accepted only when all of the following hold:

1. the source physical slot passes `TextureSlotFramingParser`;
2. the authored texture index exists exactly once;
3. the authored request carries the exact SHA-256 of the bounded source DDS;
4. the authored DDS byte size is identical to the source DDS byte size;
5. the complete standard 128-byte DDS header is byte-identical to the source;
6. therefore only compressed DDS payload bytes may change;
7. after insertion, the complete physical slot passes `TextureSlotFramingParser` again;
8. framing kind, descriptor offsets, DDS offsets/sizes, dimensions, mip count, compression and bundle sector spans remain invariant;
9. every byte outside intrinsic DDS ranges remains byte-identical.

This accepts both Pass 78 corpus-confirmed physical shapes:

- `descriptor[0x70] + DDS`;
- `0x800 bundle header + descriptor/DDS records + zero alignment`.

## Composition boundary

The regression suite composes the new seam with `RelativeSlotPackedReflowWriter`:

1. validate a physical texture slot;
2. hash-bind and patch only its intrinsic DDS compressed payload;
3. obtain a complete validated physical child image;
4. supply that complete image as `AuthoredChildImage` to the generic PAC writer;
5. rebuild the PAC without exposing bare DDS bytes to the generic writer;
6. reparse the resulting PAC and verify unchanged relative-slot geometry for the same-size edit.

This is the first code-level editing path from intrinsic DDS bytes back into a parent relative-slot container while preserving the DMC3 physical wrapper.

## Negative gates

The writer rejects:

- invalid source framing;
- empty authored sets;
- duplicate texture indices;
- missing texture indices;
- missing or stale source SHA-256;
- DDS byte-size changes;
- any DDS-header change;
- byte-identical no-op edits;
- any output that fails canonical framing validation;
- any physical framing change;
- any change outside the intrinsic DDS ranges.

## Explicit non-claims

Pass 79 does **not** establish:

- size-changing DDS serialization;
- dimension/mip/FourCC changes;
- semantic authority for the remaining variable descriptor fields;
- DXT3 support;
- original DMC3 runtime texture-parser ABI behavior;
- original-game consumption of rebuilt PAC/PNST/NBZ;
- Layer 1 COMPLETE.

The next promotion boundary for size-changing texture edits remains reverse authority for the still-variable descriptor region and the exact bundle sector-span rewrite rules.
