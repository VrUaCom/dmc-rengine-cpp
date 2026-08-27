# L1 nested DDS `.index` identity pass — 2026-08-27

## Scope

This pass closes the remaining nested DDS naming acceptance item recovered from the retained GDSpaces v6 audit:

- `st001_001.index` is confirmed to supply 17 DDS names for the texture bundle beneath `st001.pac` slot 1.
- DDS child names are external `.index` evidence, not physical/write authority.
- Texture physical identity is parent texture-slot resource + `TEXTURE[n]` + exact DDS byte span.

## Implementation

`TextureSlotExpander` expands a validated DMC3 texture slot into virtual DDS children using the already evidence-backed `TextureSlotFramingParser`.

Each child carries:

- stable logical lineage `...::TEXTURE/slot-NNNN` for layout-preserving images;
- `container_chain` suffix `TEXTURE[n]`;
- exact DDS offset and size;
- copied DDS bytes;
- parent-bound byte provenance;
- synthetic `texture_NNNN.dds` presentation until name evidence is applied.

The existing sealed `.index` pipeline from the parent pass is reused unchanged:

`IndexManifestParser -> IndexSlotNameBinder -> IndexNameOverlayBuilder`.

No second DDS-specific `.index` parser is introduced.

## Regression boundary

The deterministic regression models the confirmed 17-entry `st001_001.index` topology and proves:

1. 17 validated physical DDS children expand from the texture bundle.
2. The 17-line companion `.index` binds one-to-one by physical texture index.
3. Applying names changes display metadata only; DDS `ResourceId` and bytes stay unchanged.
4. A rename-only 17-line `.index` observation does not redirect the physical target.
5. A same-size DDS edit rebuilds and reopens the bundle with the same `TEXTURE[7]` identity.
6. After applying renamed labels, a second edit still targets the same physical texture and reopens correctly.
7. A 16-line manifest against a 17-DDS bundle fails closed; partial name authority is rejected.

## Non-claims / remaining boundary

- The deterministic fixture does not claim the exact historical spelling of all 17 real DDS labels; retained evidence confirms the count and naming role, not the full text list in this checkout.
- Size-changing texture reflow can legitimately change physical byte offsets/sizes; this pass only claims exact `ResourceId` preservation for layout-preserving/same-size rebuilds.
- `.post` remains unresolved and is not aliased to PNST.
- Historical `.index` grammar beyond the already confirmed narrow grammar remains unresolved.
- No L1 completion claim is promoted by this pass.
