# DMC3 runtime-synthesized PAC/PNST writer

Status: bounded Layer-1 product writer derived from original runtime `.lst` synthesis. It is not Capcom offline packed-file builder equivalence.

## Recovered layout authority

The canonical `.lst` runtime path already proves the synthesized materialized image layout:

- four-byte magic (`PAC\0` by default, or directive-provided such as `PNST`);
- declared slot count;
- `u32` relative offset table beginning at `+0x08`;
- empty / exact `dummy` slot = offset `0`;
- `headerSize = align64((slotCount + 2) * 4)`;
- every populated child begins at a 64-byte boundary;
- child bytes are copied in physical slot order;
- alignment gaps are zero-filled.

`RuntimeSynthRelativeSlotWriter` reuses the already-canonical `LooseContainerListPolicy::header_size()` and `aligned_size()` helpers so writer arithmetic cannot drift from the recovered runtime synthesizer.

## First size-changing tier

This writer starts from an existing valid PAC/PNST and preserves:

- format magic;
- declared slot count;
- populated/empty occupancy of every physical slot;
- physical slot ordering.

It may change populated child byte lengths and therefore recompute output offsets using the recovered runtime-synth layout.

Adding/removing slots or changing occupancy is not included in this tier.

## Duplicate non-zero aliases

A packed source may contain multiple declared slot identities sharing one populated offset. The `.lst` synthesis evidence does not establish a way to emit duplicate non-zero alias offsets.

Therefore any duplicate populated source offset fails `alias_topology_unsupported`.

Use the same-size #140/#142 path for alias-preserving edits.

## Exact intrinsic child bytes

A packed `ContainerEntry.size` is a bounded extraction extent inferred from the next distinct offset or container end. It is not intrinsic file-length authority and may contain alignment/padding bytes.

Runtime-synth rebuild therefore requires one `ExactChildImage` for **every populated source slot**, even if only one child changed.

Accepted authority kinds:

- `format_writer_receipt`;
- `loose_resource`;
- `external_exact_resource`.

Explicitly forbidden:

- `container_extracted_span`.

Every exact child carries authority identity, SHA-256 and intrinsic bytes. The writer recomputes SHA-256 before using the bytes.

If any populated slot lacks exact intrinsic bytes, rebuild fails closed rather than serializing an inferred padded parent span as a child file.

## Deterministic algorithm

1. validate the immutable source through canonical #140 source validation;
2. reject duplicate populated offsets;
3. validate exact child input identity/authority/hash and require all populated slots;
4. compute recovered 64-byte header size;
5. compute a bounded 32-bit output layout;
6. allocate zero-filled output;
7. copy source four-byte magic and declared slot count;
8. preserve empty slots as zero offsets;
9. place each populated exact child in slot order at the current aligned cursor;
10. canonical PAC/PNST reparse;
11. require format, slot count and occupancy to match the source;
12. require reparsed offsets to equal deterministic emitted offsets;
13. return authored bytes + receipt.

Input `ExactChildImage` ordering is not layout authority; output is determined by physical slot index.

## Product safety

Default output budget is 1 GiB. The writer rejects:

- invalid/unreadable source;
- unsupported source format;
- malformed source topology;
- duplicate populated offsets;
- unknown/empty/duplicate child slot input;
- missing exact child bytes;
- forbidden extracted-span authority;
- child SHA mismatch;
- output budget / 32-bit relative-offset overflow;
- canonical output reparse/topology mismatch.

No source `ByteProvenance` is attached to authored bytes.

## Receipt

The receipt records:

- source ResourceId and SHA-256;
- writer mode `runtime-synth-relative-slot`;
- output SHA-256;
- source/output structural fingerprints;
- for every populated slot: slot index, exact-byte authority kind/id, input SHA, intrinsic size and emitted offset.

The receipt requires the source ResourceId size to equal the source structural container size.

## Composition

Root size change:

`exact intrinsic child images -> runtime-synth PAC/PNST -> #141 STORE overlay -> NbzZipSource reopen -> canonical reparse`.

Nested size change:

A child that grows/shrinks cannot pass the same-size #142 reintegrator. Its immediate parent must itself be rebuilt through this runtime-synth tier using exact intrinsic images for **all** of that parent's populated slots. Continue bottom-up until the root is rebuilt.

## Regression

Synthetic regression proves:

- 3-slot PAC with populated/empty/populated topology;
- recovered header `0x40`;
- 3-byte slot0 at `0x40`;
- 70-byte slot2 at `0x80`;
- deterministic zero padding and total output `0x100`;
- input-order-invariant bytes/SHA;
- missing child rejection;
- child-for-empty/unknown/duplicate rejection;
- packed extracted-span authority rejection;
- bad child SHA rejection;
- safety budget rejection;
- duplicate source offset rejection;
- PNST output through the same physical layout;
- zero-slot source rebuilding to the recovered 64-byte runtime image;
- unsupported non-PAC/PNST source rejection.

## Still open

- exact intrinsic-byte providers for broad real DMC3 child families;
- real legal corpus size-changing round-trip receipt;
- nested size-changing bottom-up orchestration;
- controlled original-game consumption receipt;
- Capcom offline packer behavior/equivalence.
