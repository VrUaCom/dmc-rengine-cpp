# DMC3 runtime-synthesized PAC/PNST writer

Status: bounded Layer-1 product writer derived from original runtime `.lst` synthesis. It is not Capcom offline packed-file builder equivalence.

## Recovered layout authority

The canonical `.lst` runtime path establishes the synthesized materialized-image layout used by this writer:

- four-byte magic (`PAC\0` by default, or directive-provided such as `PNST`);
- declared slot count;
- `u32` relative-offset table beginning at `+0x08`;
- empty / exact `dummy` slot = offset `0`;
- `headerSize = align64((slotCount + 2) * 4)`;
- every populated child begins at a 64-byte boundary;
- child bytes are copied in physical slot order;
- alignment gaps are zero-filled.

`RuntimeSynthRelativeSlotWriter` reuses `LooseContainerListPolicy::header_size()` and `aligned_size()` so writer arithmetic cannot drift from the recovered runtime synthesizer.

## First size-changing tier

The writer starts from an existing valid PAC/PNST and preserves:

- format magic;
- declared slot count;
- populated/empty occupancy of every physical slot;
- physical slot ordering.

It may change populated child lengths and therefore recompute relative offsets. Adding/removing slots, changing occupancy, or serializing duplicate non-zero alias offsets is not included.

Duplicate populated source offsets fail `alias_topology_unsupported`. Alias-preserving same-size edits remain the #140/#142 path.

## Byte provider authority is not extent authority

A packed `ContainerEntry.size` is a bounded extraction extent inferred from the next distinct offset or container end. It can include alignment or padding and is not automatically an intrinsic child-file length.

Therefore `ExactChildImage` separates:

- byte-provider authority (`ExactChildAuthorityKind`);
- byte-extent authority (`ExactChildExtentKind`);
- authority identity;
- SHA-256;
- bytes.

The current bounded tier accepts only:

- `loose_resource + intrinsic_resource`;
- `external_exact_resource + intrinsic_resource`.

It explicitly rejects:

- `container_extracted_span`;
- `container_inferred_span` as intrinsic proof;
- `source_span_preserved` as intrinsic proof;
- generic `format_writer_receipt`, even if the caller supplies `writer_defined_complete_image` and a plausible writer-mode string.

The last rule is intentional. A public aggregate containing a writer-mode string is forgeable and therefore cannot prove that a previous writer actually defined the complete child-image extent. Writer-to-parent size-changing composition requires a future typed verified-result factory that consumes a real writer result/receipt instead of accepting a self-declared string.

This prevents an inferred packed span from being laundered through a same-size writer and then reused as an allegedly intrinsic child image.

## Deterministic algorithm

1. validate the immutable source through canonical #140 source validation;
2. reject duplicate populated offsets;
3. validate every supplied child identity/provider/extent/hash;
4. require one accepted exact intrinsic child for every populated source slot;
5. compute recovered 64-byte header size;
6. compute a bounded 32-bit output layout;
7. allocate zero-filled output;
8. copy source four-byte magic and declared slot count;
9. preserve empty slots as zero offsets;
10. place every populated child in physical slot order;
11. canonical PAC/PNST reparse;
12. require format, slot count and occupancy to match source;
13. require reparsed offsets to equal emitted offsets and each intrinsic child size to fit its reparsed bounded extent;
14. return authored bytes + receipt.

Input `ExactChildImage` ordering is not layout authority; physical slot index determines output order.

## Result and receipt integrity

`RuntimeSynthResult::ok()` does not trust construction-time state alone. It requires:

- status `ok`;
- valid receipt;
- output topology size equal to current public byte-vector size;
- SHA-256 recomputed over the current bytes equal to receipt output SHA-256.

`RuntimeSynthReceipt::valid()` additionally requires child receipts to remain in strict increasing unique slot order and every intrinsic size to fit the reparsed emitted slot extent.

Therefore mutating returned bytes or reordering/duplicating child receipt identities invalidates the result.

## Product safety

Default output budget is 1 GiB. The writer fails closed on:

- invalid/unreadable source;
- unsupported source format;
- malformed source topology;
- duplicate populated offsets;
- unknown/empty/duplicate child slot input;
- missing exact child bytes;
- packed extracted-span authority;
- source-span-preserved or container-inferred extent claims;
- generic writer-receipt extent claims;
- child SHA mismatch;
- output budget / 32-bit offset overflow;
- canonical output reparse/topology mismatch;
- invalid authoring receipt.

No source `ByteProvenance` is copied onto authored output bytes.

## Receipt

The receipt records:

- source ResourceId and SHA-256;
- writer mode `runtime-synth-relative-slot`;
- output SHA-256;
- source/output structural fingerprints;
- for every populated slot: slot index, provider kind, extent kind, authority ID, writer-mode lineage field, input SHA, intrinsic size and emitted offset.

The receipt itself currently validates only independently intrinsic standalone child providers; generic writer receipts are not accepted.

## Composition boundary

Root size change can already be composed as:

`independently exact intrinsic children -> runtime-synth PAC/PNST -> #141 STORE overlay -> NbzZipSource reopen -> canonical reparse`.

Nested size-changing propagation is still blocked until a typed verified writer-result-to-child-authority seam exists. The same-size #142 reintegrator remains valid for same-size nested edits because it writes back into the exact existing parent span and does not need to infer intrinsic EOF.

## Regression

The registered CTest regressions cover:

- 3-slot PAC with populated/empty/populated topology;
- recovered header `0x40`;
- 3-byte slot0 at `0x40`;
- 70-byte slot2 at `0x80`;
- deterministic zero padding and total output `0x100`;
- input-order invariant bytes/SHA;
- missing/empty/unknown/duplicate child rejection;
- packed extracted-span rejection;
- `source_span_preserved` and `container_inferred_span` rejection;
- forged-looking generic writer-receipt rejection;
- bad child SHA rejection;
- safety-budget rejection;
- duplicate source-offset rejection;
- PNST through the same physical layout;
- zero-slot source rebuilding to the recovered 64-byte runtime image;
- unsupported non-PAC/PNST source rejection;
- post-construction byte mutation invalidating `ok()`;
- reordered/duplicate/forged receipt child identities failing validation.

## Still open

- typed verified complete-image writer-result -> exact child authority;
- broad real DMC3 intrinsic-byte providers and slot linkage;
- representative real `.lst` corpus receipt;
- real size-changing resource round-trip receipt;
- controlled original-game consumption receipt;
- Capcom offline packer behavior/equivalence.
