# GDSpaces L1 Pass 78 — real child authority and DMC3 texture-slot framing — 2026-08-20

## Scope

Layer 1 only: reconcile packed PAC/PNST physical spans against independently extracted child artifacts from the preserved real DMC3 v6 corpus, classify every previously unexplained child-span mismatch, recover bounded texture-slot framing, and produce a controlled real-byte size-changing parent reflow receipt.

This pass does **not** claim original executable texture-parser ABI equivalence and does **not** make GDSpaces Layer 1 COMPLETE.

## Source artifact

- artifact: `DMC 3 RENGINE (6).zip`
- size: `237,658,858` bytes
- SHA-256: `7680a9ddb700b958ca1591be0629c2ff1da53efa1b723141bbee0ae4b4c7ff6f`
- inspected corpus root: `analysis_inputs/stage_drops`

The raw game/extraction bytes are not committed to GitHub. This document preserves only aggregate topology/hash receipts and bounded field relationships.

## Relative-slot corpus relationship census

The stage-drop corpus contains 42 structurally valid `.pac` container instances under the current canonical relative-slot envelope:

- 32 `PAC\0`
- 10 binary `PNST`

Every one has a sibling extraction `.index`. Across their populated physical spans the extraction manifests describe 1,869 non-empty relationships:

- 1,811 direct extracted files
- 58 `folder` relationships containing texture-bundle extractions

The `.index` relationship is extraction metadata, not original runtime slot authority. In sparse containers it enumerates non-empty children by extraction ordinal; it must not be used to renumber declared sparse physical slots.

## Direct child byte authority: 1,811 / 1,811 explained

Comparing each direct extracted child against its corresponding populated physical span in extraction order gives:

- 1,786 / 1,811: physical span is byte-for-byte exactly the extracted child image from offset zero.
- 25 / 1,811: every mismatch is DDS and has exactly one common shape: `0x70-byte descriptor + exact extracted DDS`.
- 0 unexplained direct relationships.
- 0 unknown trailing bytes after the direct child or wrapped DDS in this corpus.

For all 25 wrapped DDS cases:

- DDS starts at physical-span offset `0x70`.
- `span[0x70:]` is byte-identical to the independently extracted `.dds`.
- physical span size is exactly `0x70 + extracted DDS size`.

This closes the previous 25-mismatch bucket as one bounded framing family rather than 25 unrelated extraction failures.

## Texture-bundle physical framing: 58 / 58 explained

All 58 `folder` relationships are structured texture-bundle spans. Together they contain 129 independently extracted DDS images.

Observed invariants across all 58 bundles:

- bundle `u32 +0x00` equals extracted texture count: 58 / 58.
- first descriptor starts at `0x800`.
- first DDS starts at `0x870 = 0x800 + 0x70`: 58 / 58.
- every subsequent descriptor starts on a `0x800` boundary.
- every descriptor is immediately followed by its DDS at `descriptor + 0x70`.
- every extracted DDS is byte-identical to the corresponding bounded DDS image inside the physical span.
- descriptor `u32 +0x64` equals exact DDS byte length for every embedded texture.
- all bytes between DDS end and the next descriptor are zero in this corpus.
- all final alignment tails are zero in this corpus.

For every non-final texture, the bundle-header `u32` following the count equals the exact number of `0x800` sectors from the current descriptor start to the next descriptor start.

For the final texture:

- when the physical bundle ends on a `0x800` sector boundary, the final header value equals the sector count from final descriptor start to bundle EOF;
- when the bundle ends immediately after the final descriptor+DDS image without sector padding, the final header value is zero.

Observed inter-record zero-padding lengths after subtracting the next `0x70` descriptor are `416`, `1088`, and `1112` bytes. Observed final zero-tail lengths are `0`, `416`, `424`, `1088`, and `1112` bytes.

## 0x70 descriptor reconciliation: 154 real descriptor/DDS pairs

The 25 direct wrapped-DDS descriptors plus 129 bundle descriptors produce 154 independent real descriptor/DDS pairs.

The following relationships hold for **154 / 154** samples:

- descriptor `u32 +0x08 = 0x20000 | (DDS mipCount << 8) | compressionCode`, where DXT1 uses `0x86` and DXT5 uses `0x88`.
- descriptor `u32 +0x0C = 0xAAE4`.
- descriptor `u32 +0x10 = (DDS height << 16) | DDS width`.
- descriptor `u32 +0x14 = 1`.
- descriptor `u32 +0x18 = DDS width * 2` for DXT1 and `DDS width * 4` for DXT5.
- descriptor `u32 +0x20 = 0x40`.
- descriptor `u32 +0x38 = exact DDS size - 128`.
- descriptor `u32 +0x60 = 0` for DXT1 and `4` for DXT5.
- descriptor `u32 +0x64 = exact full DDS byte size`.
- descriptor `u32 +0x68 = 8`.
- exact DDS byte size independently equals the standard DXT block-compressed mip-chain size derived from the DDS header.

Other descriptor fields remain opaque unless independently resolved. In particular, this pass does not assign semantic names to the variable fields around `+0x3C..+0x5C` merely from correlation.

## Historical product-source corroboration

The same preserved v6 source artifact contains historical product code at:

`lib/gdspaces/readers/textures/GDTextureBundleReader.ts`

That reader scans embedded DDS images and records the 0x70 bytes immediately before DDS as texture metadata (`metadataOffset = ddsOffset - 0x70`, `metadataSize = 0x70`). It also derives bounded DDS size from dimensions, mip count and DXT compression.

This is historical GDSpaces product behavior, not original-game ABI authority. The value of this pass is that the preserved implementation observation is now independently corroborated by real physical/extracted-byte relationships.

## Architecture consequence

A generic PAC/PNST `ContainerEntry.size` is a **physical span extent**, not automatically an intrinsic editable child-file size.

For the evidenced DMC3 texture cases the required Layer-1 split is now explicit:

- ordinary direct child: physical span == intrinsic child image;
- wrapped DDS child: physical span == 0x70 descriptor + intrinsic DDS image;
- texture-bundle child: physical span == 0x800 bundle header + one or more descriptor/DDS records + alignment padding.

Therefore `RelativeSlotPackedReflowWriter` must continue to accept complete physical child images. An editor that operates on intrinsic DDS bytes requires a format/profile-specific serializer to reconstruct the descriptor/bundle framing before parent reflow. Passing a bare extracted DDS as the replacement for one of these physical spans would be incorrect.

Pass 78 introduces `TextureSlotFramingParser` as a conservative DMC3 profile seam for these two corpus-confirmed physical framings. Its strict checks are product/corpus validation and must not be mislabeled as the exact original executable rejection policy.

## Controlled real-byte size-changing reflow receipt

A real dense two-slot PNST was selected because both extracted children are independently byte-exact and require no texture wrapper:

`analysis_inputs/stage_drops/em035/em035_057.pac`

Source topology:

- format: PNST
- declared/populated: 2 / 2
- source size: `363,200` (`0x58AC0`)
- source SHA-256: `9a164d0e7534ef7776126aa4a2a9b6a3d21ddc12e2ecf312cc6bc0f513505a1c`
- slot 0 offset/size: `0x10 / 0x340` = 832-byte `em035_057_000.txt`
- slot 0 SHA-256: `a647f848f3dd012034958be8ffd65a722957fb29f06770566c10f8b8154d8b2e`
- slot 1 offset/size: `0x350 / 0x58770` = 362,352-byte nested binary PNST `em035_057_001.pac`
- slot 1 SHA-256: `bb0b36d3daf7ffa5953f07380cff68b7059aa13ab8ddd52071f88c4ef83b843c`

Controlled byte-level edit: append 16 zero bytes to the exact slot-0 child image. This is a writer/materialization test, not a claim of original-game semantic acceptance of the edited text payload.

Independent bottom-up reflow result:

- slot 0 output size: `0x350` = 848 bytes
- slot 1 output offset: `0x360`
- slot 1 output size/hash remain exactly unchanged
- output size: `363,216` (`0x58AD0`)
- output SHA-256: `35f3e70a3376ca6a752d9f9862b909650ee85bb1c4fb4445470f997e1ae06a84`
- canonical relative-slot structural reparse succeeds with 2 populated slots at `0x10` and `0x360`.

This is an **independent real-byte algorithm receipt**. It is not yet labeled an execution receipt of the current compiled C++ `RelativeSlotPackedReflowWriter`; that exact external-corpus runner remains a separate validation step.

## Remaining boundaries

- Exact current-C++ writer execution over externally supplied real corpus bytes remains to be captured without committing copyrighted game bytes.
- Original-game consumption of a rebuilt real PAC/PNST remains open.
- Original-game consumption of a repacked/overlay NBZ carrying the rebuilt resource remains open.
- Descriptor fields not listed above remain opaque.
- DXT3 or other texture encodings are not promoted by this corpus; only DXT1/DXT5 are evidenced here.
- Sparse `.index` ordinal metadata remains non-authoritative for declared physical slot numbering.
- Real `.lst` corpus acquisition remains open.

## Gate impact

This pass materially strengthens intrinsic-child authority and removes the previous unexplained direct-child mismatch bucket. It also converts the texture-bundle physical-span problem from opaque data into a bounded, testable framing model. Layer 1 remains open until real writer/repack/original-consumption receipts are promoted at their exact claimed scopes.
