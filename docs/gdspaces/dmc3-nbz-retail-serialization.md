# DMC3 NBZ retail serialization preservation

Status: bounded GDSpaces L1 implementation; retail repack remains incomplete.

This document defines the read-side preservation and artifact-binding seams required before a no-loss retail NBZ repacker can exist.

## Ownership

`NbzZipSource` remains the materialization authority:

```text
NBZ -> index -> ResourceId -> stored member -> STORE/raw-DEFLATE -> materialized bytes
```

`NbzZipSerializationScanner` is the canonical on-demand serialization authority. Ordinary reads do not pay its metadata-preservation cost and do not inherit writer ownership.

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

A serialization snapshot by itself is not trusted as a copier/repacker source. The binder requires a valid canonical `ArtifactIdentity` and performs one complete byte observation:

```text
expected ArtifactIdentity
  -> derive bounded serialization-metadata capture ranges from the indexed source
  -> one complete streaming read of the archive
       -> every byte feeds incremental SHA-256
       -> overlapping required metadata ranges are copied from those same chunks
  -> require observed SHA/size == expected artifact
  -> canonical NbzZipSerializationScanner over the captured-byte reader
  -> require captured framing == the indexed physical entry metadata
  -> ArtifactBoundNbzZipSerializationSnapshot
```

`Sha256Accumulator` hashes large archives in bounded chunks and does not require whole-file materialization. Only central/EOCD framing plus local header/name/extra prefixes are captured; member bodies remain source spans.

The critical trust invariant is:

> Every metadata byte stored in the artifact-bound serialization snapshot was observed in the same streaming read whose complete byte sequence produced the accepted SHA-256 receipt.

There is no independent pre-hash / metadata-scan / post-hash path window. A path replacement after the observation cannot alter the already captured metadata, while a replacement during the observation changes the observed byte sequence and therefore fails the expected SHA-256 check unless it is byte-identical.

`ArtifactBoundNbzZipSerializationSnapshot` is intentionally non-aggregate with a private constructor. Callers can inspect immutable getters, but cannot self-declare artifact-bound authority by copying expected SHA text into public fields. Only `NbzZipArtifactSerializationBinder` can construct the typed result after the complete verification sequence above.

A future unchanged-region copier/repacker must still independently revalidate the source artifact at its own I/O boundary before consuming source spans. Binding is not permission to trust the path indefinitely.

## Canonical scanner read seam

`NbzZipSerializationScanner::scan_with_reader()` exists so the canonical framing parser can validate an explicit byte view without opening the archive path itself. This is not a second trust authority: the supplied reader carries no artifact identity. Artifact binding becomes trusted only because the binder supplies bytes captured from the same accepted SHA-256 observation.

The ordinary `scan()` API still reads directly from `NbzZipSource::archive_path()` and remains suitable for unbound inspection.

## Safety

The scanner is fail-closed and uses a separate metadata-memory budget. It does not copy member bodies into the snapshot. Duplicate central identities may alias one local-header offset only when their preserved local framing agrees exactly.

Artifact binding is also fail-closed:

- invalid `ArtifactIdentity` is rejected;
- expected size must equal the indexed archive size;
- observation chunks are bounded product policy;
- exact archive size is checked around the complete streaming observation;
- the complete observed SHA-256 must match the expected artifact;
- required metadata ranges must all be covered by the same observation;
- the captured central/local physical fields and names must agree with the indexed source entries before authority is granted;
- a same-size archive replacement after source indexing is rejected by observed SHA mismatch;
- typed bound authority cannot be forged as a caller-owned aggregate.

Memory accounting is explicit rather than implied to be one metadata-budget copy. Captured metadata payload is bounded by `max_metadata_bytes`, and the canonical scanner may simultaneously own a second snapshot payload bounded by the same budget while binding is in progress. Peak metadata byte payload is therefore bounded by at most `2 * max_metadata_bytes`, plus the configured observation chunk and range/entry bookkeeping. The latter is bounded by the already indexed entry count. Captured-range lookup is logarithmic in the number of merged ranges, so repeated scanner reads do not degrade to an O(n^2) range search at large entry counts.

## What this does not prove

The artifact-bound snapshot is **not**:

- a writer receipt;
- a Capcom packer model;
- proof that rewritten compressed streams can be byte-identical;
- permission to trust the source path after binding;
- proof of original-game consumption.

## No-loss progression

```text
NbzZipSource
  -> one-observation SHA-256 + bounded metadata capture
  -> canonical serialization scan over captured bytes
  -> artifact-bound serialization snapshot
  -> artifact-revalidated unchanged-region copier + changed-entry serializer
  -> rebuilt central/local offsets
  -> reopen through NbzZipSource
  -> serialization + materialization comparison
  -> representative real-corpus receipt
  -> controlled original-game receipt
```

The existing STORE-only next-volume overlay writer remains a different product mode and must not be described as retail-NBZ repack.
