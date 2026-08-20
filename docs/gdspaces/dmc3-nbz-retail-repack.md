# DMC3 NBZ metadata-preserving retail repack

Status: bounded GDSpaces L1 implementation candidate.

This is a DMC Rengine product serialization tier built on the artifact-bound retail snapshot from Pass 73. It is **not** Capcom compressor or offline-packer equivalence.

## Input authority

The writer requires all of the following:

```text
valid NbzZipSource
+
ArtifactBoundNbzZipSerializationSnapshot for that exact source artifact
+
optional exact central-entry replacements
```

A replacement is identified by physical `central_index` plus the expected original logical path. Filename-only discovery is not writer authority.

## No-edit tier

With zero replacements the writer reconstructs the archive from preserved serialization framing and source spans.

Acceptance is strict:

```text
source artifact SHA/size
    ==
output artifact SHA/size
```

A parse-equivalent but byte-different no-edit archive is a failure. This is the first no-loss identity repack gate.

## Changed-entry tier

The first changed-entry tier supports one or more non-directory, non-aliased entries whose original local record does **not** use ZIP bit 3 / a data descriptor.

For a changed entry:

- filename and local extra bytes are preserved exactly;
- local version/flags/time/date remain preserved;
- payload is emitted as ZIP method 0 STORE;
- CRC32, compressed size and uncompressed size are recomputed;
- opaque post-data padding/gap bytes are preserved from the original local region;
- the corresponding central record preserves version/time/attributes/extra/comment/name metadata;
- central method/CRC/sizes and local-header offset are rewritten as required.

For an unchanged entry its complete local region is copied byte-for-byte, including opaque data-descriptor/padding/gap bytes.

Every central record keeps its original raw metadata except fields whose values must change because a local region moved or was replaced.

The EOCD/archive comment is preserved; the central-directory offset is rebuilt.

## Artifact boundary

The source artifact is independently SHA-256 verified during the writer I/O pass. The earlier artifact-bound snapshot is not indefinite permission to trust a mutable source path.

The output is hashed while written, then reopened through canonical `NbzZipSource` and rebound through `NbzZipArtifactSerializationBinder` before a success receipt is issued.

Failure removes the newly-created output path where possible. Existing output paths are never overwritten by this tier.

## Explicit hard freezes

This tier does not yet serialize a **changed** bit-3/data-descriptor member. Such a replacement fails closed because the descriptor belongs to the old payload metadata. Unchanged descriptor-bearing local regions remain safe because they are copied opaque.

A changed local region referenced by multiple central identities also fails closed. Duplicate physical local-region ownership requires a separate alias-aware authoring contract.

The writer does not attempt to reproduce original DEFLATE bytes. A changed member is STORE by product policy.

## Validation target

Regression must prove both:

1. zero-edit output is byte-identical to the source archive;
2. a size-changing replacement preserves unrelated local/central/EOCD metadata, reopens through `NbzZipSource`, materializes changed and unchanged members correctly, and produces a new artifact-bound serialization snapshot.

A synthetic fixture intentionally includes:

- opaque prefix bytes;
- a DEFLATE source member changed to STORE;
- local extra bytes;
- central extra bytes and entry comments;
- archive comment;
- an unchanged bit-3 member with data descriptor;
- opaque inter-region tail/gap bytes.

## Remaining L1 boundary

Even after this tier is canonical, L1 is not complete until representative real-corpus retail repack, representative real child-to-slot/intrinsic-byte authority, real size-changing bottom-up round-trip and original-game consumption receipts are closed. Changed descriptor/alias support is mandatory only if representative supported resources require those shapes for the chosen no-loss path.
