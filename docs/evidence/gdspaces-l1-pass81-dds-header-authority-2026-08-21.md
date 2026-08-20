# GDSpaces L1 Pass 81 — DMC3 DDS header authority

Date: 2026-08-21  
Layer: L1 Resource Materialization

## Purpose

Close the DDS-header side of the size-changing texture-authoring boundary established by Passes 78-80.

The preserved v6 corpus contains 154 DDS images that are independently bounded by validated DMC3 `0x70` texture descriptors. Pass 81 asks whether their 128-byte DDS headers can be reconstructed deterministically rather than copied as opaque metadata.

## Real-corpus result

For all 154 descriptor-backed DDS images:

- magic is `DDS `;
- `dwSize = 124`;
- `dwFlags = 0x000A1007`;
- width and height are power-of-two dimensions in the observed 64..1024 axis domain;
- `dwDepth = 0`;
- mip count is the complete chain down to 1x1;
- all eleven `dwReserved1` values are zero;
- pixel-format size is 32 and pixel-format flags are 4;
- FourCC is DXT1 (47 cases) or DXT5 (107 cases);
- RGB bit count and all RGBA masks are zero;
- `dwCaps = 0x00401008`;
- caps2/caps3/caps4/reserved2 are zero;
- `dwPitchOrLinearSize = 0x10000` for all 47 DXT1 cases;
- `dwPitchOrLinearSize = 0x20000` for all 107 DXT5 cases.

A canonical 128-byte header generated only from `(width, height, DXT1/DXT5)` matched the real source header byte-for-byte in **154/154** cases. There were zero mismatches.

The complete DDS byte size is also independently determined by the standard block-compressed full mip chain and matches the descriptor-backed DDS extent in the whole population.

## Code boundary

`Dmc3DdsProfile` adds:

- exact-profile parsing;
- deterministic canonical DDS construction from dimensions/compression plus exact compressed mip-chain payload;
- strict product authoring dimension safety;
- fail-closed rejection of header drift, unsupported compression, invalid mip chains and wrong payload lengths;
- build self-validation through the same canonical parser.

The 64..1024 product authoring envelope is intentionally labeled product-side evidence. It is **not** asserted as an original-runtime maximum.

## Size-changing serializer consequence

DDS header generation is no longer the principal blocker to size-changing texture authoring.

The remaining bounded descriptor issue is the auxiliary pair at descriptor `+0x3C/+0x40`:

- 112/154 descriptors have `auxiliary_mode = 0` and `auxiliary_value = 0`;
- 42/154 have nonzero auxiliary metadata whose runtime semantics are not yet established.

Therefore the first size-changing physical texture serializer should accept only source descriptors with auxiliary mode zero, preserve the source secondary-dimension relation (`same` or `half`) and fail closed on unresolved auxiliary cases rather than guessing.

## Non-claims

Pass 81 does not establish DXT3, non-power-of-two runtime support, dimensions above the product authoring envelope, semantics of nonzero auxiliary descriptor metadata, original-game consumption of rebuilt textures, or Layer 1 COMPLETE.
