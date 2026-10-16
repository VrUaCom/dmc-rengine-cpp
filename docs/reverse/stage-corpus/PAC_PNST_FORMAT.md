# DMC3 PAC/PNST relative-slot containers (corpus phase 3)

Status: **confirmed structural pass**

The local validator follows `dmc-rengine-cpp` `formats/relative_slot_container.*` for PAC and PNST relative-slot tables.

## Corpus result

- `.pac` resources under `st*`: **1,019**
- Valid relative-slot tables: **1,019**
- Invalid: **0**

## Confirmed layout

- PAC magic `PAC\0` or PNST magic `PNST` at `0x00`.
- Slot count `u32` at `0x04`; offset table begins at `0x08` with 4-byte offsets.
- Zero offsets are empty slots; populated offsets point after the table and delimit entries by the next distinct offset or EOF.
- Safety bound used by the product parser: 1,048,576 slots.

Evidence: `evidence/container_validation.json`.

