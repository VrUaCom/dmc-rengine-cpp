# DMC3 layout-preserving PAC/PNST writer

Status: bounded Layer-1 product writer. This document does not claim Capcom offline-packer equivalence.

## Authority

This writer implements the first authoring tier established by issue #100 Passes 54, 55 and 57:

```text
immutable materialized ResourcePayload
        +
revisioned WorkingCopy
        ↓
exact same-size PAC/PNST byte image
        ↓
canonical reparse
        ↓
unchanged structural topology receipt
```

The original materialized packed image remains the layout authority. The writer does not infer intrinsic child-file sizes, alignment rules for retail packing, or an original offline builder.

## Preconditions

`RelativeSlotLayoutWriter::rebuild()` requires:

- readable source payload;
- source `ResourceId.size` equal to the materialized byte count;
- exact source/WorkingCopy `ResourceId` identity;
- WorkingCopy source SHA-256 equal to the supplied immutable source bytes;
- source magic `PAC\0` or `PNST`;
- unchanged total materialized byte count;
- successful canonical source parse;
- successful canonical output parse;
- identical source/output structural topology;
- unchanged container header/table and pre-payload byte region.

`WorkingCopy` may support insertion/deletion for other product workflows. That does not make a size-changing edit eligible for this writer mode.

## Topology receipt

The receipt records:

- canonical resource identity;
- source SHA-256;
- output SHA-256;
- WorkingCopy revision;
- format;
- declared slot count;
- total container size;
- protected prefix size;
- every declared slot's populated state, offset and bounded extracted span.

Duplicate non-zero offsets remain duplicate slot identities over the same span. This writer does not de-alias them.

## Protected prefix

The protected prefix ends at the first populated slot offset. It includes the structural header, slot table, and any pre-payload byte region in the original image.

For a container with no populated slots, the entire image is protected because no evidenced payload domain exists to edit in this writer tier.

## What this writer permits

Same-size edits inside already-materialized populated slot spans may be emitted when the canonical parser proves that the complete container topology is unchanged.

This deliberately permits preserving and editing the full bounded extracted span as bytes. The parser's span extent is not relabeled as an intrinsic child-file size.

## What this writer rejects

- different canonical resource identity;
- stale/copied WorkingCopy source SHA;
- PAC/PNST format conversion;
- size-changing edits;
- invalid rebuilt bytes;
- changes to slot count, sparse topology, offsets, alias topology, bounded spans or total size;
- edits to header/table or pre-payload layout bytes.

## Nested reintegration

Nested child-to-parent reintegration is a separate follow-up seam. Pass 57 requires it to operate over the materialized parent byte domain, preserve same-span alias groups, propagate bottom-up, and fail closed on divergent edits to one shared physical span.

For a DEFLATE parent, child offsets must never be added to compressed NBZ storage coordinates.

## Size-changing tier

Size-changing PAC/PNST output belongs to the separate runtime-synthesized relative-slot writer derived from the original `.lst` in-memory layout. That tier requires exact intrinsic child bytes and must reject unsupported duplicate non-zero alias topology.

## Validation boundary

Synthetic Windows/Ubuntu CI for this writer proves the bounded product contract only. Stronger validation remains:

- clean PNST real-corpus parser receipt;
- representative legal real PAC/PNST same-size edit/reparse receipt;
- nested reintegration receipt;
- generated STORE-only NBZ overlay reopen/selection receipt;
- controlled original-game consumption receipt.
