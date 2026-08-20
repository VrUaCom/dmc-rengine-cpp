# DMC3 NBZ retail serialization preservation

Status: bounded implementation candidate for GDSpaces L1.

This document defines the read-side preservation seam required before a no-loss retail NBZ repacker can exist.

## Ownership

`NbzZipSource` remains the materialization authority:

```text
NBZ -> index -> ResourceId -> stored member -> STORE/raw-DEFLATE -> materialized bytes
```

`NbzZipSerializationScanner` is a separate on-demand serialization authority. Ordinary reads do not pay its metadata-preservation cost and do not inherit writer ownership.

## Preserved framing

For a valid indexed classic ZIP/NBZ source the scanner records:

- archive/source identity label and archive size;
- opaque prefix span size before the first local record;
- recovered central-directory start and EOCD offset;
- exact EOCD + archive-comment bytes;
- exact raw central-directory record bytes, including version/time/attributes/extra/comment fields;
- exact local header + filename + local-extra prefix bytes;
- stored member data source span;
- complete local-region source span through the next distinct local header or central-directory start;
- bit-3/data-descriptor presence as an observation only.

Descriptor, padding and unknown gap bytes are deliberately retained through the local-region source span without assigning semantics.

## Safety

The scanner is fail-closed and uses a separate metadata-memory budget. It does not copy member bodies into the snapshot. Duplicate central identities may alias one local-header offset only when their preserved local framing agrees exactly.

## What this does not prove

The snapshot is **not**:

- a cryptographic artifact identity;
- a writer receipt;
- a Capcom packer model;
- proof that rewritten compressed streams can be byte-identical;
- proof of original-game consumption.

A future retail repacker must bind the snapshot/source spans to an exact artifact identity before copying bytes.

## No-loss progression

```text
NbzZipSource
  -> NbzZipSerializationScanner
  -> artifact-bound serialization snapshot
  -> unchanged-region copier + changed-entry serializer
  -> rebuilt central/local offsets
  -> reopen through NbzZipSource
  -> serialization + materialization comparison
  -> controlled original-game receipt
```

The existing STORE-only next-volume overlay writer remains a different product mode and must not be described as retail-NBZ repack.
