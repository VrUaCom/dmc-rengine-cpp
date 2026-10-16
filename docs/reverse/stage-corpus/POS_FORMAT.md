# DMC3 POS reverse guide (corpus phase 3)

Status: **confirmed structural pass**

The local validator follows the existing DMC RENGINE POS reader (`lib/dmc3/posSpawnParser.ts`) and checks every stage POS resource.

## Corpus result

- POS resources: **189**
- Valid: **189**
- Invalid: **0**
- Warnings: **0**
- Every file uses a `0x10` header and `0x30` fixed record stride.

## Confirmed layout

- Magic `POS\0` at `0x00`.
- Version `u16` at `0x04`; active count `u16` at `0x06`.
- Records begin at `0x10`, stride `0x30`.
- Record fields proven for the stage corpus: id `u32` at `+0x00`, position XYZ floats at `+0x04/+0x08/+0x0C`, angle candidates at `+0x10/+0x14`.
- Declared active count matches the number of non-zero position records.

Evidence: `evidence/pos_validation.json`.

