# DMC Rengine Format Documentation

This directory contains format-specific structural documentation and the canonical DMC3-HD format/purpose inventory.

## Start here

- [DMC3 HD format and resource-purpose catalog](dmc3-hd-format-catalog.md) — canonical human-readable inventory of currently observed/named DMC3-HD families, evidence status, product support and open reverse boundaries. Reconciled through the 2026-08-31 executable format census.
- [DMC3 HD machine-readable format-purpose registry](dmc3-hd-format-purpose-registry.json) — canonical normalized registry separating identity, purpose, schema maturity and product support. It now includes the `VAGp`, direct `DDS `, exact `TM2\0`, owned `LIG2`, `MCV`, `EFE/EFW` and media-capability evidence boundaries.
- [Canonical EXE format census — 2026-08-31](../../data/reverse/dmc3-exe-format-census-20260831.json) — bounded machine-readable census of runtime classifiers, direct content checks, constructor/type tags, extension classifiers, runtime references and explicit exclusions.
- [DMC3 runtime type identification — 2026-08-31](../../data/reverse/dmc3-runtime-type-identification-20260831.json) — canonical split between the three-byte registry probe, container dispatcher, four-byte family-mask classifier, motion/control extension dispatcher and direct content checks.
- [DMC3 primary 3D family evidence — 2026-08-31](../../data/reverse/dmc3-primary-3d-family-20260831.json) — current `MOD/EFM/SCM/MRP/MCV/SHW` identity/purpose boundary.
- [DMC3 runtime type-evidence split — 2026-08-31](../research/dmc3-runtime-type-evidence-split-2026-08-31.md) — historical human-readable correction separating the original classifier paths; machine-readable files above are the current authority when they are more specific.
- [DMC3 primary 3D / render family reverse — 2026-08-31](../research/dmc3-primary-3d-render-family-reverse-2026-08-31.md) — detailed reverse narrative; retained subject to the canonical machine-readable corrections above.
- [DMC3 HD format-purpose closure pass — 2026-08-27](../research/dmc3-format-purpose-closure-pass-2026-08-27.md) — earlier closure pass; historical evidence only where later 2026-08-31 findings supersede it.
- [HITS collision resource](hits.md) — collision-grid/triangle layout. Four-byte `HITS` is `DATA_CONFIRMED` from corpus/parser evidence; the bounded canonical EXE type census does not establish it as an EXE runtime tag. `HITS$` is rejected.
- [PAC read-only structural parser](pac-readonly-parser.md)
- [PNST read-only structural parser](pnst-readonly-parser.md)
- [Generic container foundation](container-foundation.md)
- [Synthetic slot container](synthetic-slot-container.md) — test-only format; never game evidence.

## Authority rule

A filename or extension is never sufficient semantic format authority.

Use the strongest evidence appropriate to the exact claim:

```text
bounded original-EXE classifier/content check/owned type tag
validated byte magic / structural grammar
hash-bound corpus structure
runtime/index/path reference
fallback/unknown
```

Do not flatten those evidence classes. In particular:

- a three-byte runtime registry identity does not automatically define a four-byte file magic;
- the family-mask classifier at `0x1402FD650` has a different byte-width contract from the registry probe at `0x1402DB1F0`;
- constructor/object tags such as `LIG2` prove executable-side type ownership but do not automatically prove the complete on-disk file header/schema;
- runtime filenames such as `basic.ptz` or `snd_sys.phd` prove references, not complete grammars;
- media capability extension checks do not make generic media formats native DMC resource families.

The catalog and registry deliberately separate:

- **resource identity** — what family the bytes belong to;
- **resource purpose** — what subsystem uses the family;
- **binary schema maturity** — how much of the byte layout is understood;
- **original-runtime evidence** — what canonical DMC3 code actually recognizes/consumes;
- **DMC Rengine product support** — what the current C++ repository can recognize, parse, edit or export.

A known purpose does not imply a complete schema, and a parser does not imply original-game semantic equivalence.

## Canonical 2026-08-31 corrections

The current evidence set requires these specific corrections everywhere in the project:

- `HITS`: four-byte corpus/parser identity is real, but its identity status is `DATA_CONFIRMED`, not `EXE_CONFIRMED`, for the bounded canonical EXE type census; zero ASCII `HITS` occurrences were found there.
- `HITS$`: `REJECTED`; never reintroduce it as magic.
- `TM2`: direct canonical content check is exact `TM2\0` at `0x1403365BA`; `TIM2` is only an alias/historical label unless separately evidenced.
- `DDS`: direct canonical first-DWORD checks at `0x140049A8E` and `0x14004AD9D` promote identity to EXE-backed content evidence.
- `VAGp`: direct canonical first-DWORD check at `0x140032970`.
- `LIG2`: constructor `0x14023ECB0` writes the `LIG2` FourCC/type tag to object `+0x08` at `0x14023ECC9`; complete disk schema remains partial.
- `MCV `: EXE-confirmed four-byte family-mask identity `0x50000000`, independently corroborated by `.mcv` class 1 in the motion/control extension dispatcher.
- `EFE` / `EFW`: EXE-confirmed container-dispatch sentinels; semantic schemas remain research-required.

## Evidence vocabulary

- `GAME_VERIFIED` — observed through the original game in a bounded test.
- `EXE_CONFIRMED` — direct canonical executable evidence.
- `DATA_CONFIRMED` — repeatable binary/corpus structure evidence.
- `HIGH_CONFIDENCE` — convergent evidence, but one promotion boundary remains.
- `CANDIDATE` — plausible and evidence-linked; not canonical semantics.
- `RESEARCH_REQUIRED` — identity or label exists, exact purpose/schema unresolved.
- `CAPABILITY_ONLY` — executable/media support exists; shipped DMC3 presence is not claimed.
- `REJECTED` — superseded or contradicted claim; not current authority.

Historical research remains useful acquisition evidence but does not override stronger current product code or later canonical reverse documentation.
