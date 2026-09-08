# DMC3 HD PTX — em000 source binding (2026-09-08)

**Branch:** `reverse/ptx-em000-binding-20260908`  
**Base:** `main@da852451d9729d15e3106d87859a83d694d2813b`  
**Corpus:** `em000-extract.zip`  
**Target:** `em000_000.bin`, 487,424 bytes

## Conclusion

`em000_000.bin` is not an unknown BIN. It conforms to the canonical DMC3 PTX texture-bundle framing already implemented in `formats.ptx-dmc3-reader`.

Observed envelope:

```text
texture_count = 4
bundle header = 0x800 bytes
per-texture descriptor = 0x70 bytes
sector alignment = 0x800 bytes
first embedded DDS marker = 0x870
```

The first DDS position is exactly:

```text
0x800 + 0x70 = 0x870
```

Observed sector-span values for the four texture entries are:

```text
43, 86, 86, 22
```

This is a direct em000 binding to the existing PTX module, not a new format invention.

## Classification rule

For this payload:

```text
source/extractor label = .bin
semantic format        = PTX
format evidence        = structural framing
```

The source `.bin` suffix must remain provenance only. DMC Rengine presentation may expose a canonical semantic extension `.ptx` while retaining the original source name separately.

## Architecture

No second PTX parser should be created. The correct route is:

```text
em000_000.bin
  -> TextureSlotFramingParser
  -> texture_bundle
  -> formats.ptx-dmc3-reader
  -> canonical TextureSlotExpander
  -> DDS children
```

This branch exists to bind the em000 resource to the already canonical PTX architecture and to prevent a future fallback classifier from leaving it as generic BIN.

## Separation from wrapped DDS

Effect-pack `T` members use the other evidenced texture framing:

```text
0x70 descriptor
-> DDS at +0x70
```

Those are `wrapped_dds`, not PTX bundles. The two forms must remain distinct even though both ultimately contain DDS images.

## Promotion requirement

The em000 binding can be promoted into semantic classification only if the exact source bytes pass the canonical `TextureSlotFramingParser`; filename position alone is not sufficient authority.
