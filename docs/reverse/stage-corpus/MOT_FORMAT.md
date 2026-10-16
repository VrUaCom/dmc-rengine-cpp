# DMC3 MOT reverse guide (corpus phase 3)

Status: **confirmed structural pass**

The canonical parser is implemented in `the repository` (`include/dmc_rengine/formats/mot/abi.hpp`, `src/formats/mot/parser.cpp`). The independent Python validator applies the same bounded layout to every motion resource under `st*` folders.

## Corpus result

- MOT-family resources: **600** (`.mot`: 42, `.mot2`: 492, `.mot3`: 1, `.mot4`: 65)
- Valid: **600**
- Invalid: **0**
- Warnings: **0**
- Parsed tracks: **10,727**
- Parsed keys: **171,741**
- Compression 2 tracks: **1,637**
- Compression 3 tracks: **9,090**

## Confirmed layout

- Header size `u32` at `0x00`; magic `MOT\0` at `0x04`.
- Channel-domain count `u16` at `0x1C`; channel mask table starts `0x1E` and ends at `align16(0x1E + 2*count)`.
- Record count `u32` follows the aligned header.
- Track prefix is 8 bytes: span `u16`, key count `u16`, compression `u16`, start-time raw `u16`.
- Compression 2: 0x10-byte prefix and 4-byte keys.
- Compression 3: 0x20-byte prefix and 8-byte keys.
- Key time is `raw & 0x7FFF`; bit 15 is preserved as a per-key flag.
- Known-compression key times are monotonic; remaining bytes are zero padding.

Evidence: `evidence/mot_validation.json`.

The validator covers the four observed suffixes and preserves unknown compression tracks as bounded records.

