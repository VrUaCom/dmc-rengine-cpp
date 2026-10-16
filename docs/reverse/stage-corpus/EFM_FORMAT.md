# DMC3 EFM reverse guide (corpus phase 3)

Status: **magic/header structural pass; payload semantics open**

No canonical EFM semantic parser is present in `dmc-rengine-cpp`. The local pass records the confirmed magic and prefix facts without assigning meaning to the opaque payload.

## Corpus result

- EFM resources: **393**
- Valid magic/header prefix: **393**
- Invalid: **0**
- Warnings: **0**

## Confirmed facts

- Magic `EFM ` at `0x00`.
- A finite positive float is present at `+0x04` across the stage corpus.
- Remaining header/payload fields are preserved as opaque until EXE or round-trip evidence identifies record boundaries.

Evidence: `evidence/efm_validation.json`.

