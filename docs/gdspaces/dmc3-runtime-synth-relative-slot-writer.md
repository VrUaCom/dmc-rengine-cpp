# DMC3 runtime-synthesized PAC/PNST writer

Status: bounded Layer-1 product writer derived from original runtime `.lst` synthesis. It is not Capcom offline packed-file builder equivalence.

Canonical executable authority for the recovered layout below: SHA-256 `e454272ed0fb0247fcbcf300e5d55d7a3e96d50b89b9ffaff81bb978dcbdd082`.

## Recovered layout authority

The canonical `.lst` runtime path establishes the synthesized materialized-image layout used by this writer:

- four-byte magic (`PAC\0` by default, or directive-provided such as `PNST`);
- declared slot count;
- `u32` relative-offset table beginning at `+0x08`;
- empty / exact `dummy` slot = offset `0`;
- `headerSize = align64((slotCount + 2) * 4)`;
- child bytes are copied in physical slot order;
- the complete planned image is zero-initialized before child emission;
- **direct whole-file children reserve `ceil(size / 0x800) * 0x800`;**
- **recursively synthesized complete child images reserve `align64(completeImageSize)`.**

This corrects the older shorthand that treated every child as merely `align64(intrinsicSize)`.

`RuntimeSynthRelativeSlotWriter` now reuses both `LooseContainerListPolicy::direct_transfer_extent()` and the existing 0x40 structural `aligned_size()` so product writer arithmetic cannot drift from the recovered representation distinction.

## Reverse anchors for the corrected extent model

Fresh canonical-EXE reverse establishes:

- `0x1402EF620`: direct whole-file extent helper; opens the resource, obtains `ceil(logicalSize/0x800)` from the whole-file chunk-count path, closes it, and returns `chunkCount << 11`;
- `0x1401B7FD0`: `.lst` planner using the direct helper for ordinary/direct packed children and recursive synthesis size for nested `.lst` fallback;
- `0x1401B85C0`: `.lst` writer using the same placement decisions;
- `0x140337600`: allocation path that zeroes the complete requested image;
- `0x140346BEA`: imported `memset` thunk used by that allocator;
- `0x1402EF4D0`: type-2 materialization **enqueue** entry, not the byte-producing body;
- `0x1402EF790`: queued-job consumer that enters the whole-file read spine.

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

That separation now directly controls placement:

### `intrinsic_resource`

Produced by `from_intrinsic_resource(...)` for `loose_resource` or `external_exact_resource`.

It models a direct whole-file child, so parent placement uses:

```text
ceil(intrinsicSize / 0x800) * 0x800
```

The child SHA still binds only the intrinsic bytes. The remainder of the transfer extent is zero-filled output slack, not part of the intrinsic child identity.

### `writer_defined_complete_image`

Produced only by `from_verified_runtime_synth_result(...)` from a currently valid successful runtime-synth result.

It models the complete image already produced by a recursive synthesis level, so parent placement uses:

```text
align64(completeImageSize)
```

This matches the recovered nested `.lst` fallback branch and prevents a recursively synthesized image from being incorrectly inflated to a fresh direct-file 0x800 extent.

`ExactChildImage` is intentionally **not** a public aggregate. Its constructor is private, so callers cannot self-declare a `format_writer_receipt` capability by filling fields or by supplying a plausible writer-mode string.

## Deterministic algorithm

1. validate the immutable source through canonical #140 source validation;
2. reject duplicate populated offsets;
3. validate every supplied child capability/provider/extent/hash;
4. require one accepted exact child for every populated source slot;
5. compute recovered 64-byte header size;
6. choose each child placement extent from typed extent authority:
   - intrinsic/direct -> 0x800 transfer granularity;
   - verified writer complete image -> 0x40 structural alignment;
7. compute a bounded product-safe 32-bit output layout;
8. allocate zero-filled output;
9. copy source four-byte magic and declared slot count;
10. preserve empty slots as zero offsets;
11. place every populated child in physical slot order;
12. canonical PAC/PNST reparse;
13. require format, slot count and occupancy to match source;
14. require reparsed offsets to equal emitted offsets and each intrinsic/complete image byte span to fit its reparsed bounded extent;
15. return authored bytes + receipt.

Input `ExactChildImage` ordering is not layout authority; physical slot index determines output order.

## Zero-padding authority

Padding is no longer only a deterministic product choice for this path. The recovered allocation route converges on `0x140337600`, which calls `memset(buffer, 0, requestedSize)` through `0x140346BEA` before the writer emits header/children.

Therefore the original runtime-synth image has zero bytes in:

- header alignment padding;
- direct whole-file transfer slack up to the next 0x800 boundary;
- structural alignment slack after recursively synthesized complete images.

The product writer intentionally preserves that behavior by allocating `std::vector<std::byte>(total_size, std::byte{0})` before writes.

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

Typed complete-image composition is mechanically supported:

```text
independently intrinsic leaf bytes
    -> ExactChildImage::from_intrinsic_resource()
       [parent reserves 0x800 transfer extent]
    -> child RuntimeSynthRelativeSlotWriter
    -> verified RuntimeSynthResult
    -> ExactChildImage::from_verified_runtime_synth_result(parent_slot, child_result)
       [parent reserves 0x40-aligned complete-image extent]
    -> parent RuntimeSynthRelativeSlotWriter
    -> verified parent RuntimeSynthResult
```

This may be repeated bottom-up through multiple PAC/PNST levels and the rebuilt root can then feed #141 STORE-overlay authoring.

The typed factory proves only **complete-image writer provenance and extent**. It does **not** prove that a child belongs to a particular semantic parent slot. Real child-to-slot linkage remains a separate evidence/representation authority and must not be inferred from synthetic `slot_NNNN.bin` names or packed parser extents.

## Product safety vs original arithmetic

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
- transfer/structural extent overflow;
- output budget / 32-bit offset overflow;
- canonical output reparse/topology mismatch;
- invalid authoring receipt.

The original planner performs important size arithmetic in 32-bit registers and does not expose the same explicit product overflow guards. GDSpaces intentionally fails closed rather than reproducing unsafe wraparound. That hardening is not relabeled as original behavior.

No source `ByteProvenance` is copied onto authored output bytes.

## Receipt

The receipt records:

- source ResourceId and SHA-256;
- writer mode `runtime-synth-relative-slot`;
- output SHA-256;
- source/output structural fingerprints;
- for every populated slot: slot index, provider kind, extent kind, authority ID, writer-mode lineage, input SHA, complete/intrinsic child byte size and emitted offset.

The extent kind is now operational layout authority, not descriptive metadata only.

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
- direct intrinsic 3-byte slot0 at `0x40` with a reserved 0x800 transfer extent;
- direct intrinsic 70-byte slot2 at `0x840`;
- deterministic zero transfer slack and total output `0x1040`;
- mixed extent authority: verified writer-defined `0x40` complete image followed by a direct 0x800-granular leaf;
- input-order invariant bytes/SHA;
- missing/empty/unknown/duplicate child rejection;
- rejected construction of packed-span or self-declared writer-receipt capabilities;
- safety-budget rejection;
- duplicate source-offset rejection;
- PNST through the same physical extent policy;
- zero-slot source rebuilding to the recovered 64-byte structural image;
- unsupported non-PAC/PNST source rejection;
- post-construction byte mutation invalidating `ok()`;
- embedded child mutation remaining detectable even if the caller recomputes the public top-level output hash;
- padding mutation invalidating writer authority;
- immutable/non-aggregate child capability and child receipt type boundaries;
- invalid/mutated runtime-synth results rejected by the typed writer-child factory;
- consecutive size-changing container levels with typed complete-image authority.

## Still open

- representative real child-to-slot linkage authority for size-changing nested authoring;
- broad real DMC3 intrinsic-byte providers;
- representative real `.lst` corpus receipt;
- final original failure-propagation reconciliation for malformed/truncated/enqueue-failure paths;
- controlled real size-changing resource round-trip and original-game consumption receipt;
- Capcom offline packer behavior/equivalence.

Layer 1 remains **INCOMPLETE / NOT 100%** until those evidence gates are closed.
