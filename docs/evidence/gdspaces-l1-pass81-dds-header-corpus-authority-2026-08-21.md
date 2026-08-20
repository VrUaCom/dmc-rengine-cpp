# GDSpaces L1 Pass 81 — DDS header corpus authority

Date: 2026-08-21  
Layer: L1 Resource Materialization  
Base: Pass 80 full DMC3 texture descriptor structural authority

## Purpose

Close the next materialization ambiguity below texture-slot framing: the exact DDS image envelope used by the preserved DMC3 corpus.

Before this pass, `TextureSlotFramingParser` independently read a subset of DDS fields while descriptor validation separately reconstructed mip-chain geometry. Pass 81 introduces one lower DDS authority and makes texture framing delegate intrinsic DDS validation to it.

## Corpus

Source artifact: `DMC 3 RENGINE (6).zip`  
Size: 237,658,858 bytes  
SHA-256: `7680a9ddb700b958ca1591be0629c2ff1da53efa1b723141bbee0ae4b4c7ff6f`

The preserved `analysis_inputs/stage_drops` population contains 243 extracted DDS images:

- 154 participate in the descriptor/wrapper or texture-bundle population established by Passes 78–80;
- 89 occur as bare extracted child images;
- 104 are DXT1;
- 139 are DXT5.

## 243/243 structural invariants

Every observed DDS image has:

- `DDS ` magic;
- 124-byte `DDS_HEADER` size;
- flags `0x000A1007`;
- positive power-of-two dimensions;
- a complete mip chain through 1x1;
- exact file size = 128-byte magic+header envelope + exact DXT block-compressed full mip-chain payload;
- eleven zero `dwReserved1` values;
- `DDS_PIXELFORMAT.dwSize = 32`;
- `DDS_PIXELFORMAT.dwFlags = 0x4` (FourCC);
- FourCC DXT1 or DXT5 only;
- zero RGB bit count and channel masks;
- caps `0x00401008`;
- zero caps2/caps3/caps4/reserved2.

## DMC3 standard corpus profile

242/243 images share the same additional profile:

- `depth = 0`;
- DXT1 `pitchOrLinearSize = 0x00010000` in 104/104 cases;
- DXT5 `pitchOrLinearSize = 0x00020000` in 138/138 standard-profile cases.

These values are deliberately treated as **corpus facts**, not replaced with generic DDS linear-size formulas. For many observed dimensions they do not equal the standard top-level compressed byte count.

## One observed depth=1 exception

Exactly one bare DDS is outside the standard profile:

- DXT5;
- 1024x2048;
- 12 mip levels;
- `depth = 1`;
- `pitchOrLinearSize = 0x00200000`.

`DdsImageParser` accepts this only at the exact observed geometry and labels it `observed_depth1_exception`. This is read authority only. One sample is not sufficient to authorize a generalized writer rule.

The 154 descriptor/wrapper DDS relationships all use the standard profile. Therefore texture-slot framing rejects the exception profile if it appears behind a DMC3 0x70 descriptor.

## Code boundary

Pass 81 adds `DdsImageParser` under the DMC3 profile layer.

`TextureSlotFramingParser` no longer owns a second DDS interpretation. Its sequence is now:

```text
descriptor declared DDS span
 -> DdsImageParser
 -> accepted standard DDS structural document
 -> DMC3 0x70 descriptor cross-check
 -> physical texture-slot document
```

This prevents a DDS image from being interpreted differently by the intrinsic-image layer and the wrapper layer.

## Writer boundary

Pass 81 does **not** yet add a size-changing DDS writer.

The next safe authoring step can now distinguish:

1. a standard-profile authored DDS that passes `DdsImageParser` and may become a candidate for descriptor/bundle reserialization; and
2. the one observed exception profile, which remains preserve/read-only until more evidence exists.

The future size-changing texture-slot writer must still preserve a proven secondary-dimension relation and must fail closed on unresolved nonzero auxiliary descriptor metadata unless independent authoring authority is established.

## Non-claims

This pass does not establish:

- the complete original DMC3 DDS parser ABI;
- arbitrary DDS compatibility;
- generalized depth=1 authoring;
- DXT3 support;
- size-changing texture-slot serialization;
- original-game consumption of rebuilt resources;
- Layer 1 COMPLETE.
