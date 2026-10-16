# DMC3 DDS reverse guide (corpus phase 3)

Status: **profile pass with one known variant**

The local validator follows `dmc-rengine-cpp` `profiles/dmc3/dds_profile.*` and checks canonical DXT mip chains.

## Corpus result

- DDS resources: **3,755**
- Canonical profile valid: **3,754**
- Variant: **1** (`st904_effect_001\\st904_effect_001_013\\st904_effect_001_013_000.dds`, DXT5 256x128 with a zero mip-count field and non-canonical flags/caps)
- Invalid magic/header: **0**

## Confirmed profile

- `DDS ` magic, 124-byte header, DXT1 or DXT5 compression.
- Power-of-two dimensions from 64 through 1024.
- Complete mip chain and exact payload size.
- Canonical header fields are preserved by the C++ profile parser.

Evidence: `evidence/dds_validation.json`.

