# Opaque family classification

## Scope

`opaque_family_classification.json` classifies all 11,477 payloads whose extension is one of `.ukn`, `.bd`, `.so`, `.ff`, `.tsb`, `.tm2`, `.bin`, or `.hid*` under the ASCII source junction `F:\Reverse-Core-Corpus\stage_source_ascii`. The classifier is content based and records one row per file. It does not modify source files.

## Confirmed routes

| Classification | Count | Evidence |
|---|---:|---|
| `hits-records` | 281 | `HITS` tag in the payload; matches the Native Reader HITS family |
| `vag-audio` | 250 | `VAGp` chunk stream; EXE decomp shows the VAGp/FM0D sound path |
| `hid-dispatch-payload` | 194 | `HID\0` tag at byte 4; the EXE extension registry includes `.hid` dispatch |
| `itm-tagged` | 189 | `ITM` tag; one per stage configuration family |
| `ste-tagged` | 189 | `STE` tag; payload family is tagged, while the EXE `STE` string hit is a Steam API false positive |
| `dca-records` | 189 | `DCA\0` tag; matches DCA record family |
| `ukn-tsc-text` | 38 | `.TSC` text payload, including `# RELATIVE` records |
| `tsb-sound-table` | 33 | `dbsT` header, 32-byte records; declared count agrees with `(size-16)/32` |
| `event-table` | 22 | `EVT\0` header; EventTbl payloads in `GData.afs` |
| `so-link` | 21 | leading `u32=6`, 4-byte records, reserved fourth byte zero |
| `ff-float-table` | 11 | `ff&A`, exactly 56 records ? 16 bytes |
| `tm2-console-texture` | 6 | 112-byte console-texture header followed by a DDS payload |
| `spu-map` | 1 | `SPUMAPDT` header in `SpuMap.bin` |

The eight exact `bd-effect-record` matches are the strict subset with a 96-byte payload and 48 zero prefix bytes. The remaining 892 96-byte `.bd` records have the same family size but populated vector fields, so they are retained as `.bd` sound/effect payloads without an invented field schema.

## What remains opaque

`opaque-unclassified` contains 10,045 payloads. This is expected: many `.ukn` files are fixed-layout stage records with no magic, and the large `.so` payloads are not the compact link table. The report preserves size, prefix, sibling extensions and path for clustering. Existing C++ readers currently prove only the compact SO link route plus bounded graph/volume structures; no writer is inferred.

## Reproduction

```powershell
python F:\Reverse-Core-Corpus\dmc-rengine-cpp\tools\reverse\opaque_family_record_census.py F:\Reverse-Core-Corpus\stage_source_ascii --out F:\Reverse-Core-Corpus\dmc3-stage-reverse\evidence\opaque_family_record_census.json
python F:\Reverse-Core-Corpus\dmc-rengine-cpp\tools\reverse\opaque_family_classify.py F:\Reverse-Core-Corpus\stage_source_ascii --out F:\Reverse-Core-Corpus\dmc3-stage-reverse\evidence\opaque_family_classification.json
```

The classifier is a research aid. It reports structural observations and known runtime tags; it does not claim semantic names for unparsed binary fields.
