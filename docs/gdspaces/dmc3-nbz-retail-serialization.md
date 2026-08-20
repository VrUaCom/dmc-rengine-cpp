# DMC3 NBZ retail serialization preservation

Status: bounded GDSpaces L1 implementation; retail repack remains incomplete.

This document defines the read-side preservation and artifact-binding seams required before a no-loss retail NBZ repacker can exist.

## Ownership

`NbzZipSource` remains the materialization authority:

```text
NBZ -> index -> ResourceId -> stored member -> STORE/raw-DEFLATE -> materialized bytes
```

`NbzZipSerializationScanner` is a separate on-demand serialization authority. Ordinary reads do not pay its metadata-preservation cost and do not inherit writer ownership.

`NbzZipArtifactSerializationBinder` is the exact-artifact trust seam. It does not own materialization or repacking.

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

## Exact artifact binding

A serialization snapshot by itself is not trusted as a copier/repacker source. The binder requires a valid canonical `ArtifactIdentity` and performs:

```text
expected ArtifactIdentity
  -> streaming SHA-256 pass A over exact archive bytes
  -> require SHA/size match
  -> NbzZipSerializationScanner
  -> streaming SHA-256 pass B over the same path
  -> require SHA A == SHA B == expected SHA
  -> ArtifactBoundNbzZipSerializationSnapshot
```

`Sha256Accumulator` hashes large archives in bounded chunks and does not require whole-file materialization. The binding receipt therefore proves that the serialization scan was surrounded by two complete reads of the expected artifact identity.

This closes scan-time stale-source/TOCTOU exposure only for that binding window. A future unchanged-region copier/repacker must independently revalidate the artifact at its own I/O boundary before and after consuming source spans.

## Safety

The scanner is fail-closed and uses a separate metadata-memory budget. It does not copy member bodies into the snapshot. Duplicate central identities may alias one local-header offset only when their preserved local framing agrees exactly.

Artifact binding is also fail-closed:

- invalid `ArtifactIdentity` is rejected;
- expected size must equal the indexed archive size;
- hash chunks are bounded product policy;
- exact archive size is checked around each streaming hash pass;
- pre/post scan SHA values must both match the expected artifact;
- a same-size archive replacement after source indexing is rejected by hash mismatch.

## What this does not prove

The artifact-bound snapshot is **not**:

- a writer receipt;
- a Capcom packer model;
- proof that rewritten compressed streams can be byte-identical;
- permission to trust the source path indefinitely after binding;
- proof of original-game consumption.

## No-loss progression

```text
NbzZipSource
  -> NbzZipSerializationScanner
  -> ArtifactIdentity + double streaming SHA binding
  -> artifact-bound serialization snapshot
  -> unchanged-region copier + changed-entry serializer
  -> rebuilt central/local offsets
  -> reopen through NbzZipSource
  -> serialization + materialization comparison
  -> controlled original-game receipt
```

The existing STORE-only next-volume overlay writer remains a different product mode and must not be described as retail-NBZ repack.
