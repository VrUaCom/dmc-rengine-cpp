# DMC3 CAM reverse guide (corpus phase 3)

Status: **structural pass; semantic families remain incomplete**

The local validator follows `lib/camCore.ts`. It recognizes the confirmed `CAM\0` header and scans the confirmed `0x00000400` variable record family while preserving candidate polygons and unknown bytes.

## Corpus result

- CAM resources: **189**
- Header/scan errors: **0**
- Warnings: **127** (declared entries not yet decoded as confirmed `0x400` records)
- Confirmed records: recorded per file in evidence; candidate polygons remain read-only.

## Confirmed facts

- Header is `0x10` bytes; magic `CAM\0` at `0x00`, version `u16` at `+0x04`, declared entry count `u16` at `+0x06`.
- A confirmed record has flags `u32=0x400`, zone count at `+0x14`, record size `0x40 + count*0x50`.
- Zone geometry uses four vec4 corners followed by a vec4 tail; semantic meaning of undecoded families is not claimed.

Evidence: `evidence/cam_validation.json`.

