# DMC3 EVE reverse guide (corpus phase 3)

Status: **confirmed structural pass**

The local validator follows the existing DMC RENGINE EVE reader (`lib/dmc3/eveParser.ts`) and checks every stage event-zone resource.

## Corpus result

- EVE resources: **189**
- Valid: **189**
- Invalid: **0**
- Warnings: **0**

## Confirmed layout

- Magic `EVE\0` at `0x00`.
- Version/family `u16` at `0x04`; record count `u16` at `0x06`.
- Records begin at `0x10`, fixed stride `0x80`; file size is exactly `0x10 + count*0x80`.
- Four polygon corners occupy `+0x20..+0x5F` as vec4 values; height is `f32` at `+0x60`.
- The tail from `+0x64` to the end of each record is preserved as unknown.

Evidence: `evidence/eve_validation.json`.

