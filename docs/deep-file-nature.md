# Deep per-file nature reverse

The nature census visits **79,592/79,592 files** in `Extracted - copia`, hashes every payload and records the first 64 bytes, entropy, printable ratio, strings and content-derived family in `evidence/deep_file_nature.jsonl`. The compact aggregate is `evidence/deep_file_nature.json`.

## Content-derived families

| Nature | Files |
|---|---:|
| `dds` | 20,578 |
| `text-like` | 15,578 |
| `opaque-binary` | 12,572 |
| `mot` | 12,182 |
| `mod` | 6,483 |
| `pac` | 4,586 |
| `scm` | 1,718 |
| `pnst` | 1,534 |
| `cam` | 1,393 |
| `efm` | 508 |
| `hits` | 283 |
| `vag` | 250 |
| `shw` | 247 |
| `hid-text-or-tagged` | 205 |
| `dca` | 189 |
| `eve` | 189 |
| `itm-text-or-tagged` | 189 |
| `lig` | 189 |
| `pos` | 189 |
| `ste-text-or-tagged` | 189 |
| `sef` | 183 |
| `est-text-or-tagged` | 112 |
| `tsc-text-or-tagged` | 46 |

## Strong structural observations

- SCM Binary Inspector detail: **1,718/1,718** accepted, no region/ownership conflicts; across the corpus it accounts for **647,942 typed fields**, **40,798 unknown ranges**, and zero conflicts. Per-file region/field counts remain in `evidence/scm_structural_detail.json`.
- PAC/PNST container detail: **5,349/5,353** accepted. Four PNST payloads are deterministic truncation/layout failures (`dmc3.pnst.slot_offset_out_of_bounds`): `m11_b00_012.pac`, `m12_b02_012.pac`, `m17_c00_012.pac`, `m99_s00_012.pac`. Their first-word slot offsets extend beyond the supplied byte span.
- `.index`: **11,721** metadata manifests; 767 begin with `PNST`, 29 are empty, and the remainder are name lists whose first line is a resource name. Aggregate references include 14,145 DDS, 9,439 UKN, 6,275 TXT, 5,272 MOD and 5,270 MOT entries. `.index` remains metadata evidence, not an EXE runtime authority.
- Duplicate fingerprints: **5,442** SHA-256 groups contain more than one path. The complete path groups are in `deep_file_nature.json`.

## Opaque families narrowed by byte structure

- `.ukn`: 9,874 total. Known tags identify HITS 281, DCA 189, ITM 189, STE 189, EST 112, TSC 38, MOT 9, SO-volume 21 and VAG 249. The remaining 8,607 are dominated by exact record-aligned sizes: 544 bytes (3,466), 368 (1,550), 336 (1,250), 16 (566), 0 (316), 528 (309), 96 (223) and 48 (195). These are grouped in the JSONL by prefix, size and family for the next parser pass.
- `.bd`: 900 files are exactly 96 bytes with zero prefix; one `snd_sys.bd` is 374,544 bytes and begins `VAGp`, so the suffix covers at least a fixed effect/behavior record and a VAG sound bank.
- `.so`: all 430 are record-aligned; Native Reader recognizes 3 SO-graph and 21 SO-link payloads, while 406 remain untyped by the current registry. The common 368-byte size equals 23?16-byte records and the existing SO modules are the authority for the three recognized subfamilies.
- `.ff`: 11 payloads are 896 bytes (56?16) and begin with float-looking data (`ff&A`), consistent with a fixed vector table; no magic or writer is claimed.
- `.tsb`: all 33 begin `dbsT` and vary by table length; this is a stable sound-table signature. `.bin` contains 22 `EVT\0` EventTbl records, one `SPUM` map and one zero-filled `lei20_4c.bin`.
- `.tm2`: six zero-prefixed console texture payloads; the EXE contains `.tm2` literals, but no structural reader is registered. `.ptx` has two 12,288-byte bundles accepted by the PTX reader despite no ASCII magic.

## Limits

- Content classification is evidence, not invented semantics. A prefix or stride narrows a family; it does not name fields without a parser or EXE read site.
- The six large HITS timeouts from the previous full pass remain separate; this nature census still fingerprints them without modifying source bytes.

Evidence:

- `evidence/deep_file_nature.json` and `.jsonl`
- `evidence/scm_structural_detail.json`
- `evidence/container_structural_detail.json`
- `evidence/index_nature_census.json`
- `evidence/container_structural_failures.json
