# DMC3 HITS reverse guide (corpus phase 2)

Status: **confirmed structural pass**

The canonical HITS contract is already implemented in `the repository` (`include/dmc_rengine/formats/hits.hpp`, `src/formats/hits.cpp`). The local independent scan confirms it across all HITS resources in `st*` folders.

## Corpus result

- HITS resources: **281**
- Valid: **281**
- Invalid: **0**
- Warnings: **0**

## Confirmed layout

- Magic: `HITS` at `0x00`.
- Header size: `0x44`.
- Bounds min/max: `0x08` / `0x14` (`Vec3f`).
- Cell size: `0x20` (`Vec3f`).
- Grid dimensions: `0x2C`, `0x30`, `0x34` (`u32`).
- Triangle count: `0x38` (`u32`).
- Spatial-table relative offset: `0x3C`, relative base `8`.
- Triangle-array relative offset: `0x40`, relative base `8`.
- Triangle record size: `0x38`.
- Spatial cell lists terminate with signed `-1`.
- Cell references are byte offsets into the triangle array and must be aligned to `0x38`.

Evidence: `evidence/hits_validation.json`.

