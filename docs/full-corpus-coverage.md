# Full Extracted corpus reverse coverage

Source: `C:\Users\ADA\Desktop\Moding DMC\Мод проєкти\Progect\Extracted - copia` (ASCII junction `F:\Reverse-Core-Corpus\stage_source_ascii`).

The census covers **79,592 files** in **11,722 directories**, across **44 extensions** and **3,728 top-level families**. It records extension, size bucket, first 16 bytes and a printable 4-byte prefix in `evidence/full_corpus_census.json`.

## Root-family coverage

| Root family group | Files | Top-level dirs |
|---|---:|---:|
| `st*` | 16,790 | 556 |
| `em*` | 4,645 | 24 |
| `m*` (mission/cutscene families) | 22,271 | 85 |
| `GData.afs` | 4,333 | 1 |
| `static_effect` | 327 | 1 |
| Other root families | 31,226 | 3,061 |

## Native Reader pass

Native Reader produced a valid manifest for **79,586/79,592 files**. Six large HITS payloads exceed the 40-second serial bound and remain listed in `evidence/native_reader_full_corpus_summary.json`; all other files were accepted, including the two non-ASCII JSON names via immutable ASCII copy probes.

Selected parser results from the complete pass:

| Extension / family | Parser or identification | Count |
|---|---|---:|
| `.bak` | bak (<none>) | 2 |
| `.bak` | hits / `formats.hits-record-scanner` (structural) | 2 |
| `.bd` | bd (recognized) | 905 |
| `.bin` | bin (recognized) | 2 |
| `.bin` | evt (<none>) | 22 |
| `.cam` | cam (recognized) | 1,393 |
| `.dds` | dds / `formats.dds-dmc3-reader` (structural) | 20,578 |
| `.efm` | efm (recognized) | 508 |
| `.eve` | eve (<none>) | 189 |
| `.ff` | ff (<none>) | 11 |
| `.fon` | fon (recognized) | 4 |
| `.hid` | hid (recognized) | 1 |
| `.hid1` | hid1 (<none>) | 17 |
| `.hid2` | hid2 (<none>) | 46 |
| `.hid3` | hid3 (<none>) | 83 |
| `.hid4` | hid4 (<none>) | 9 |
| `.hid5` | hid5 (<none>) | 26 |
| `.hid6` | hid6 (<none>) | 8 |
| `.hid8` | hid8 (<none>) | 4 |
| `.index` | index (<none>) | 11,721 |
| `.lig` | lig / `formats.lig2-record-scanner` (structural) | 189 |
| `.mod` | mod / `formats.mod-structural-v1` (structural) | 6,483 |
| `.mot` | mot / `formats.mot-structural-v1` (structural) | 5,473 |
| `.mot1` | mot / `formats.mot-structural-v1` (structural) | 5 |
| `.mot2` | mot / `formats.mot-structural-v1` (structural) | 3,849 |
| `.mot2` | mot2 (<none>) | 149 |
| `.mot3` | mot / `formats.mot-structural-v1` (structural) | 1,748 |
| `.mot3` | mot3 (<none>) | 72 |
| `.mot4` | mot / `formats.mot-structural-v1` (structural) | 449 |
| `.mot4` | mot4 (<none>) | 23 |
| `.mot5` | mot / `formats.mot-structural-v1` (structural) | 207 |
| `.mot6` | mot / `formats.mot-structural-v1` (structural) | 194 |
| `.mot7` | mot / `formats.mot-structural-v1` (structural) | 3 |
| `.mot8` | mot / `formats.mot-structural-v1` (structural) | 1 |
| `.pac` | pac / `dmc3-pac-structural-v1` (structural) | 4,590 |
| `.pac` | pnst / `dmc3-pnst-structural-v1` (structural) | 763 |
| `.phd` | phd (recognized) | 1 |
| `.pos` | pos (<none>) | 189 |
| `.ptx` | ptx / `formats.ptx-dmc3-reader` (structural) | 2 |
| `.scm` | scm / `formats.scm-structural-v1` (structural) | 1,718 |
| `.sef` | sef (<none>) | 183 |
| `.shw` | shw / `formats.shw-structural-v1` (structural) | 247 |
| `.so` | so (<none>) | 406 |
| `.so` | so-graph / `formats.so-graph-structural-v1` (structural) | 3 |
| `.so` | so-link / `formats.so-link-structural-v1` (structural) | 21 |
| `.tm2` | tm2 (recognized) | 5 |
| `.tm2` | wrapped-dds / `formats.ptx-dmc3-reader` (structural) | 1 |
| `.tsb` | tsb (recognized) | 33 |
| `.tsc` | tsc (recognized) | 8 |
| `.txt` | clt (recognized) | 318 |
| `.txt` | effect-manifest (recognized) | 265 |
| `.txt` | txt / `formats.stage-txt-lexer` (structural) | 6,586 |
| `.ukn` | dca / `formats.dca-record-scanner` (structural) | 189 |
| `.ukn` | hits / `formats.hits-record-scanner` (structural) | 268 |
| `.ukn` | mot / `formats.mot-structural-v1` (structural) | 9 |
| `.ukn` | so-volume / `formats.so-volume-structural-v1` (structural) | 21 |
| `.ukn` | tsc (recognized) | 38 |
| `.ukn` | ukn (<none>) | 9,336 |

## Boundaries

- `SCM`, `MOD`, `DDS`, `PTX`, `MOT*`, `SHW`, `LIG`, `HITS`, `DCA`, `PAC/PNST`, and stage text are covered by structural readers or validated corpus commands.
- `EFM`, `SEF`, `CAM`, `BD`, `SO`, `EVE`, `POS`, HID variants, sound-bank files and most index/unknown payloads are identified and inventoried but remain read-only/structural where no semantic parser is registered.
- Six HITS payloads are a performance boundary, not missing files: `st100_003`, `st228_003`, `st238_003`, `st417_003`, `st438_003`, `st607_003`.
- A valid manifest is an identification/structure result; it does not claim decoded semantics or game-runtime acceptance.

Evidence:

- `evidence/full_corpus_census.json`
- `evidence/native_reader_full_corpus_summary.json`
- `evidence/native_reader_full_core_families.json`
- `evidence/native_reader_full_remaining_families.json`
- `evidence/native_reader_retry_ukn.json`
- `evidence/mod_writer_corpus.json` (all 6,483 MOD files; 6,482 no-op passes, one unrelated aggregate mismatch)
