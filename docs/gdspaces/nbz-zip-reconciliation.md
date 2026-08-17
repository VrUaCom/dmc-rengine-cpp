# NBZ / ZIP Reader — Reconciled Original Walk and Product Safety

**Date:** 2026-08-18  
**Status:** IMPLEMENTED / WHOLE-HEAD CI REQUIRED BEFORE PROMOTION

## Purpose

Promote a clean read-only NBZ source without collapsing recovered DMC3 traversal behavior and GDSpaces product-hardening policy into one claim.

The reader keeps two dimensions explicit:

1. **Recovered walk authority** — what current executable evidence says controls central-directory traversal.
2. **Safe product validation** — stricter checks DMC Rengine deliberately applies while reading user-supplied archives.

## Recovered central-directory walk

The central-directory start is derived as:

```text
computedCentralStart = absoluteEOCDOffset - centralDirectorySize
```

The EOCD `centralDirectoryOffset` field is preserved in `NbzZipIndexReceipt`, but it is not used as seek authority. A mismatch produces a compatibility warning and is preserved as evidence.

Central records are then walked from the computed start until the EOCD boundary. The EOCD declared entry count is recorded and compared with the actual number of walked records, but it does not terminate traversal.

This corrects the historical clean-product implementation that sought directly to EOCD `centralDirectoryOffset` and iterated exactly `entryCount` records.

## Index receipt

`NbzZipIndexReceipt` preserves:

- archive size;
- absolute EOCD offset;
- computed central start;
- EOCD-declared central offset;
- central size;
- declared entry count;
- walked entry count;
- offset-match flag;
- count-match flag.

This lets validation compare product metadata against recovered traversal without rewriting one into the other.

## Safe product validation

The current reader deliberately hardens archive handling with checks that are **not** promoted as original DMC3 acceptance rules:

- classic single-disk subset only;
- ZIP64 sentinels rejected until a bounded ZIP64 product contract exists;
- local/central flags and compression method must agree;
- local/central filename bytes must agree;
- encrypted entries are not materialized;
- STORE requires equal stored/materialized size;
- materialized bytes must match central CRC32;
- product materialization currently supports STORE (`0`) and raw DEFLATE (`8`) only.

Unknown non-zero compression methods can remain indexed as evidence but are not materialized by this product slice. This does not claim the original executable used the same method whitelist.

## Materialization and provenance

STORE members receive:

```text
ByteOriginKind::direct_source_span
ByteTransform::zip_stored
```

DEFLATE members receive:

```text
ByteOriginKind::transformed_source_span
ByteTransform::zip_deflate
```

DEFLATE uses the independently reviewed `core::RawDeflate` product primitive.

After materialization, classification is magic-first. A compressed member that materializes to PAC/PNST is therefore surfaced as a container.

Children expanded from a transformed member inherit `materialized_parent_span` provenance; container-relative offsets are never fabricated as `compressedDataOffset + childOffset` physical archive coordinates.

## Regression controls

The synthetic NBZ test intentionally includes two metadata contradictions that historical code mishandled:

1. EOCD central offset is wrong while central bytes and central size are valid. The clean reader must still walk the computed central start and preserve a mismatch warning.
2. EOCD entry count says one while the central byte domain contains two valid records. The clean reader must walk both records and preserve a count warning.

Additional tests cover STORE, DEFLATE -> nested PAC -> DDS expansion, CRC rejection, ZIP64 sentinel rejection, local/central name mismatch and an indexed-but-not-materialized unknown compression method.

## Boundary

This slice does not yet implement DMC3 provider precedence, archive-key normalization, duplicate normalized-key winner policy, contiguous `DMC3-N.nbz` mount probing, six-prefix resolution or `.lst` reconstruction. Those remain the subsequent resolver/reconstruction slices.

It also does not move original `FileSlot`, `LoadedResource`, async I/O, typed ready-state or scene-lifecycle ownership into GDSpaces.
