# DMC3 SEF reverse guide (corpus phase 3)

Status: **versioned structural pass; one truncated resource**

The local validator follows the existing `lib/sefCore.ts` v3 array contract while explicitly preserving older versions as opaque candidates.

## Corpus result

- SEF resources: **183**
- Versioned resources structurally accepted: **182**
- Truncated/invalid: **1** (`st432cfg_002\\st432cfg_002_000.sef`, 16 bytes)
- Warnings: **153** (older versions or preserved overlap/variant notes)

## Confirmed v3 layout

- Magic `SEF\0` at `0x00`; version `u16` at `+0x04`; flags/subtype `u16` at `+0x06`.
- Three array descriptors at `+0x08`, `+0x10`, `+0x18`, each `(count u32, offset u32)`.
- Array elements are 4-byte values; bounds are checked before reading.
- Versions 1/2 and other non-v3 layouts are retained as opaque versioned candidates, not forced through the v3 schema.

Evidence: `evidence/sef_validation.json`.

