# DMC3 NBZ retail serialization preservation

Status: bounded GDSpaces L1 implementation; synthetic retail repack exists, representative real-corpus and original-game gates remain open.

This document defines the preservation, artifact-binding and writer seams required for a no-loss retail NBZ path.

## Ownership

`NbzZipSource` remains the materialization authority:

```text
NBZ -> index -> ResourceId -> stored member -> STORE/raw-DEFLATE -> materialized bytes
```

`NbzZipSerializationScanner` is the canonical on-demand serialization authority. Ordinary reads do not pay its metadata-preservation cost and do not inherit writer ownership.

`NbzZipArtifactSerializationBinder` is the exact-artifact read-side trust seam.

`NbzZipRetailRepacker` is the bounded writer-side authority for a previously artifact-bound classic-ZIP/NBZ source. It does not own resource resolution, nested PAC/PNST semantics or original-game compatibility claims.

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
  -> require captured terminal EOCD == indexed NBZ receipt
  -> require captured framing == the indexed physical entry metadata
  -> ArtifactBoundNbzZipSerializationSnapshot
```

`Sha256Accumulator` hashes large archives in bounded chunks and does not require whole-file materialization. Only central/EOCD framing plus local header/name/extra prefixes are captured; member bodies remain source spans.

The critical trust invariant is:

> Every metadata byte stored in the artifact-bound serialization snapshot was observed in the same streaming read whose complete byte sequence produced the accepted SHA-256 receipt.

There is no independent pre-hash / metadata-scan / post-hash path window. A path replacement after the observation cannot alter the already captured metadata, while a replacement during the observation changes the observed byte sequence and therefore fails the expected SHA-256 check unless it is byte-identical.

The captured terminal EOCD is also bound back to the `NbzZipIndexReceipt`: single-disk fields, declared entry counts, central size/offset and comment length must match the receipt used to derive the scanner geometry. This prevents an already-indexed `NbzZipSource` from lending stale traversal authority to a different same-size ZIP even when the caller supplies the replacement artifact's correct SHA-256.

`ArtifactBoundNbzZipSerializationSnapshot` is intentionally non-aggregate with a private constructor. Callers can inspect immutable getters, but cannot self-declare artifact-bound authority by copying expected SHA text into public fields. Only `NbzZipArtifactSerializationBinder` can construct the typed result after the complete verification sequence above.

Binding is not permission to trust the path indefinitely. The writer independently revalidates the complete source artifact at its own I/O boundary.

## Canonical scanner read seam

`NbzZipSerializationScanner::scan_with_reader()` exists so the canonical framing parser can validate an explicit byte view without opening the archive path itself. This is not a second trust authority: the supplied reader carries no artifact identity. Artifact binding becomes trusted only because the binder supplies bytes captured from the same accepted SHA-256 observation.

The ordinary `scan()` API still reads directly from `NbzZipSource::archive_path()` and remains suitable for unbound inspection.

## Retail writer path

`NbzZipRetailRepacker` consumes an exact `ArtifactBoundNbzZipSerializationSnapshot` and writes a distinct new artifact through a bounded streaming path:

```text
bound retail source
  -> stream original source exactly once
       -> every source byte feeds SHA-256
       -> opaque prefix copied
       -> unchanged physical local regions copied byte-for-byte
       -> changed physical regions consume the original bytes but emit rewritten bytes
       -> old central/EOCD consumed for source-artifact verification
  -> require observed source SHA == bound ArtifactIdentity
  -> emit rebuilt raw central records
  -> emit rebuilt EOCD/comment region
  -> finalize temporary output SHA
  -> canonical NbzZipSource reopen of the temporary output
  -> exact writer-receipt metadata comparison
  -> exact materialized-byte comparison for every changed central identity
  -> only then rename temporary output to the requested destination
```

A failed source observation, writer invariant, canonical reopen or changed-member materialization check deletes the temporary output. An unvalidated artifact is never promoted to the requested destination path.

### Identity repack

A zero-replacement invocation is a strict no-loss identity operation. The complete source is streamed to the temporary output and the final source/output SHA-256 and size must match exactly before canonical reopen and commit.

The writer does not normalize metadata merely because it was invoked.

### Unchanged physical regions

An unchanged physical local region is copied as an opaque byte span. Its compression method does not need to be understood by the writer because no bytes or metadata inside that region are re-authored.

This distinction is important: support for changing a compression method is narrower than support for preserving an unchanged member that uses that method.

### Changed STORE and raw-DEFLATE members

The first bounded changed-member tier supports unencrypted classic-ZIP methods:

- method `0` — STORE;
- method `8` — raw DEFLATE.

For method 8, `RawDeflate::deflate_stored()` emits deterministic RFC 1951 `BTYPE=00` stored blocks. This preserves the ZIP method-8 contract and exact materialized bytes but does **not** claim compression-ratio or byte-stream parity with Capcom's original compressor.

The changed member receives a newly computed CRC32, compressed size and uncompressed size. Central records retain their opaque original fields except for the bounded CRC/size/local-offset fields that must change.

### Local metadata and data descriptors

For non-bit-3 entries, the writer requires the local CRC/sizes to mirror the indexed central values before rewriting them.

For bit-3 entries, local CRC/sizes may either be all zero or mirror the central values. The current bounded descriptor writer recognizes and rewrites both classic forms:

- signed 16-byte descriptor: `0x08074B50 + CRC32 + compressed size + uncompressed size`;
- unsigned 12-byte descriptor: `CRC32 + compressed size + uncompressed size`.

Unknown bytes after the recognized descriptor remain opaque and are copied unchanged. Unsupported descriptor forms fail closed rather than being guessed.

### Physical aliases

Multiple central identities may alias one physical local record only when their indexed physical metadata agrees.

For a changed aliased physical record, every central identity in that alias group must explicitly acknowledge the replacement and every replacement byte image must be identical. Partial alias edits or divergent alias payloads fail closed. One physical region is then rewritten and every aliased central record receives the same rebuilt local offset and changed CRC/size tuple.

## Safety

The scanner is fail-closed and uses a separate metadata-memory budget. It does not copy member bodies into the snapshot. Duplicate central identities may alias one local-header offset only when their preserved local framing agrees exactly.

Artifact binding is also fail-closed:

- invalid `ArtifactIdentity` is rejected;
- expected size must equal the indexed archive size;
- observation chunks are bounded product policy;
- exact archive size is checked around the complete streaming observation;
- the complete observed SHA-256 must match the expected artifact;
- required metadata ranges must all be covered by the same observation;
- the captured terminal EOCD must match the indexed receipt that supplied traversal geometry;
- the captured central/local physical fields and names must agree with the indexed source entries before authority is granted;
- a same-size archive replacement after source indexing is rejected by observed SHA mismatch when the expected identity is stale;
- a same-size structurally valid replacement with its own correct SHA is still rejected when its EOCD receipt differs from the stale index;
- typed bound authority cannot be forged as a caller-owned aggregate.

Writer-side safety adds:

- bounded streaming I/O rather than whole-archive materialization;
- complete source SHA revalidation from the exact stream that supplies copied source spans;
- changed-member authoring restricted to bounded understood methods/metadata shapes;
- ZIP32 sentinel/offset/size overflow rejection;
- physical-alias arbitration;
- temporary-output validation before destination commit;
- canonical reopen and exact changed-member materialization comparison.

Memory accounting is explicit rather than implied to be one metadata-budget copy. Captured metadata payload is bounded by `max_metadata_bytes`, and the canonical scanner may simultaneously own a second snapshot payload bounded by the same budget while binding is in progress. Peak metadata byte payload is therefore bounded by at most `2 * max_metadata_bytes`, plus the configured observation chunk and range/entry bookkeeping. The latter is bounded by the already indexed entry count. Captured-range lookup is logarithmic in the number of merged ranges, so repeated scanner reads do not degrade to an O(n^2) range search at large entry counts.

## Current regression boundary

The public synthetic regression now covers:

- byte-identical zero-edit retail repack;
- size-changing STORE member rewrite with a later unchanged local region shifted and preserved;
- canonical reopen and exact changed/untouched materialized bytes;
- method-8 size-changing rewrite through deterministic raw-DEFLATE stored blocks;
- bit-3 signed data-descriptor rewrite;
- physical alias incomplete/conflicting replacement rejection;
- coherent same-byte alias replacement;
- source-artifact revalidation and temporary-output-only validation.

These tests prove the bounded implementation behavior. They are not representative retail DMC3 corpus receipts and are not original-game compatibility evidence.

## What this still does not prove

The current retail writer is **not**:

- a Capcom offline packer reconstruction;
- proof that rewritten compressed streams are byte-identical to Capcom output;
- representative validation of every metadata/descriptor form in retail DMC3 volumes;
- proof that a real size-changing nested PAC/PNST edit can already travel bottom-up into a retail NBZ;
- proof of original-game consumption.

## No-loss progression

```text
NbzZipSource
  -> one-observation SHA-256 + bounded metadata capture
  -> canonical serialization scan over captured bytes
  -> EOCD/index receipt binding + entry framing cross-check
  -> artifact-bound serialization snapshot
  -> artifact-revalidated unchanged-region copier + changed STORE/DEFLATE serializer
  -> rebuilt local offsets + central records + EOCD
  -> canonical temporary reopen + exact changed-member materialization validation
  -> destination commit
  -> nested size-changing PAC/PNST bottom-up composition
  -> representative real-corpus receipt
  -> controlled original-game receipt
```

The existing STORE-only next-volume overlay writer remains a different product mode and must not be described as retail-NBZ repack.
