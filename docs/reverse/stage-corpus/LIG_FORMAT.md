# DMC3 LIG2 reverse guide (corpus phase 3)

Status: **confirmed structural pass**

The canonical fixed-record scanner is implemented in `the repository\include\dmc_rengine\formats\lig2.hpp` and `src/formats/lig2.cpp`.

## Corpus result

- LIG resources under `st*`: **189**
- Valid `LIG2` headers: **189**
- Invalid: **0**
- Warnings: **0**
- Every file has 48 complete records and no trailing bytes.

## Confirmed layout

- Magic `LIG2` at `0x00`.
- Fixed header: `0x20` bytes.
- Record size: `0x30` bytes.
- DMC3 corpus record count: 48.
- Record array begins at `0x20`; unknown record fields remain byte-preserved until semantic field ownership is proven.

Evidence: `evidence/lig_validation.json`.

