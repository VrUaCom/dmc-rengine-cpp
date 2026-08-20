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

## Size-changing tier

The writer starts from an existing valid PAC/PNST and preserves:

- format magic;
- declared slot count;
- populated/empty occupancy of every physical slot;
- physical slot ordering.

It may change populated child lengths and therefore recompute relative offsets. Adding/removing slots, changing occupancy, or serializing duplicate non-zero alias offsets is not included.

Duplicate populated source offsets fail `alias_topology_unsupported`. Alias-preserving same-size edits remain the #140/#142 path.

## Byte provider authority is not extent authority

A packed `ContainerEntry.size` is a bounded extraction extent inferred from the next distinct offset or container end. It can include alignment or padding and is not automatically an intrinsic child-file length.

`ExactChildImage` therefore separates:

- byte-provider authority (`ExactChildAuthorityKind`);
- byte-extent authority (`ExactChildExtentKind`);
- authority identity;
- writer-mode lineage where applicable;
- SHA-256;
- bytes.

`ExactChildImage` is intentionally **not** a public aggregate. Its constructor is private, so callers cannot self-declare a `format_writer_receipt` capability by filling fields or by supplying a plausible writer-mode string.

There are two public authority factories:

1. `from_intrinsic_resource(...)`
   - accepts only `loose_resource` or `external_exact_resource`;
   - assigns `intrinsic_resource` extent authority;
   - computes SHA-256 internally;
   - rejects empty authority IDs and empty byte images.

2. `from_verified_runtime_synth_result(slot, result)`
   - accepts only a live `RuntimeSynthResult` for which `result.ok()` is true;
   - requires the canonical `runtime-synth-relative-slot` writer receipt;
   - derives the output SHA and bytes from the verified result itself;
   - emits a typed `format_writer_receipt + writer_defined_complete_image` child capability.

This prevents an inferred packed span from being laundered through a same-size or self-declared writer receipt. A writer-defined complete image can feed a size-changing parent only through the typed verified-result factory.

## Deterministic algorithm

1. validate the immutable source through canonical #140 source validation;
2. reject duplicate populated offsets;
3. validate every supplied child capability/provider/extent/hash;
4. require one accepted exact child for every populated source slot;
5. compute recovered 64-byte header size;
6. compute a bounded 32-bit output layout;
7. allocate zero-filled output;
8. copy source four-byte magic and declared slot count;
9. preserve empty slots as zero offsets;
10. place every populated child in physical slot order;
11. canonical PAC/PNST reparse;
12. require format, slot count and occupancy to match source;
13. require reparsed offsets to equal emitted offsets and each child image size to fit its reparsed bounded extent;
14. return authored bytes + receipt.

Input `ExactChildImage` ordering is not layout authority; physical slot index determines output order.

## Result and receipt integrity

`RuntimeSynthResult::ok()` does not trust construction-time state alone. It requires:

- status `ok`;
- valid receipt;
- output topology size equal to current public byte-vector size;
- SHA-256 recomputed over the current bytes equal to receipt output SHA-256;
- every child receipt's embedded `(offset, intrinsic_size)` span to remain inside the current output;
- SHA-256 recomputed over every embedded child span to remain equal to the immutable child receipt SHA.

`RuntimeSynthChildReceipt` is non-aggregate and non-default-constructible. `RuntimeSynthReceipt::valid()` additionally requires child receipts to remain in strict increasing unique slot order and each child image size to fit the reparsed emitted slot extent.

Therefore mutating returned bytes, mutating an embedded child while rewriting only the top-level output hash, or reordering/duplicating child receipt identities invalidates the result.

## Nested size-changing composition

Typed complete-image composition is now mechanically supported:

```text
independently intrinsic leaf bytes
    -> ExactChildImage::from_intrinsic_resource()
    -> child RuntimeSynthRelativeSlotWriter
    -> verified RuntimeSynthResult
    -> ExactChildImage::from_verified_runtime_synth_result(parent_slot, child_result)
    -> parent RuntimeSynthRelativeSlotWriter
    -> verified parent RuntimeSynthResult
```

This may be repeated bottom-up through multiple PAC/PNST levels and the rebuilt root can then feed #141 STORE-overlay authoring.

The typed factory proves only **complete-image writer provenance and extent**. It does **not** prove that a child belongs to a particular semantic parent slot. Real child-to-slot linkage remains a separate evidence/representation authority and must not be inferred from synthetic `slot_NNNN.bin` names or packed parser extents.

## Product safety

Default output budget is 1 GiB. The writer fails closed on:

- invalid/unreadable source;
- unsupported source format;
- malformed source topology;
- duplicate populated offsets;
- unknown/empty/duplicate child slot input;
- missing exact child bytes;
- packed extracted-span authority;
- unsupported or self-declared provider/extent combinations;
- child SHA mismatch;
- invalid or mutated verified runtime-synth child result;
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
- for every populated slot: slot index, provider kind, extent kind, authority ID, writer-mode lineage, input SHA, complete child-image size and emitted offset.

A writer-defined child receipt can only enter through a typed `ExactChildImage` factory that consumed a currently valid successful runtime-synth result.

## Composition boundary

Root size change is available as:

`exact child capabilities -> runtime-synth PAC/PNST -> #141 STORE overlay -> NbzZipSource reopen -> canonical reparse`.

Nested size-changing mechanics are available through the verified-result factory described above. Same-size #142 remains the preferred path when physical parent layout/aliases must stay unchanged.

Still **not** implied by these product mechanics:

- semantic child-to-slot linkage;
- arbitrary packed `ContainerEntry` span as intrinsic child bytes;
- broad real-game intrinsic-byte discovery;
- Capcom offline-packer behavior or equivalence;
- successful original-game consumption of an authored size-changing resource.

## Regression

The registered CTest regressions cover:

- 3-slot PAC with populated/empty/populated topology;
- recovered header `0x40`;
- 3-byte slot0 at `0x40`;
- 70-byte slot2 at `0x80`;
- deterministic zero padding and total output `0x100`;
- input-order invariant bytes/SHA;
- missing/empty/unknown/duplicate child rejection;
- rejected construction of packed-span or self-declared writer-receipt capabilities;
- safety-budget rejection;
- duplicate source-offset rejection;
- PNST through the same physical layout;
- zero-slot source rebuilding to the recovered 64-byte runtime image;
- unsupported non-PAC/PNST source rejection;
- post-construction byte mutation invalidating `ok()`;
- embedded child mutation remaining detectable even if the caller recomputes the public top-level output hash;
- immutable/non-aggregate child capability and child receipt type boundaries;
- invalid/mutated runtime-synth results rejected by the typed writer-child factory;
- two consecutive size-changing container levels: child runtime-synth result -> verified child capability -> parent runtime-synth result;
- parent receipt and parent embedded span remaining bound to the exact child output SHA/bytes.

## Still open

- representative real child-to-slot linkage authority for size-changing nested authoring;
- broad real DMC3 intrinsic-byte providers;
- representative real `.lst` corpus receipt;
- real size-changing resource round-trip receipt on representative game resources;
- controlled original-game consumption receipt;
- Capcom offline packer behavior/equivalence.
