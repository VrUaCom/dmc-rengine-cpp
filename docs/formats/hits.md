# HITS$ Record Scanner

The clean C++ repository implements the confirmed, narrow HITS$ record layer without claiming a complete undocumented file schema.

## Confirmed facts used

- file magic: `HITS$`;
- record marker: `0x18060001`;
- record size: 56 bytes;
- record payload: one little-endian `u32` marker followed by thirteen little-endian `float32` values.

These facts are migrated from the historical DMC Rengine format research. The current public repository does not yet contain a sample-hash Evidence Packet for the original game file family.

## API

```cpp
formats::hits::RecordScanner::scan(bytes)
```

The result contains:

- whether `HITS$` magic was recognized;
- zero or more complete confirmed records;
- structured diagnostics.

Each record contains:

- byte offset;
- marker;
- thirteen raw float values.

The floats remain semantically unnamed. Naming them without fresh evidence would convert a structural fact into an unsupported interpretation.

## Scan strategy

1. Require `HITS$` at offset zero.
2. Begin scanning after the five-byte magic.
3. Search byte-by-byte for little-endian marker `0x18060001`.
4. When a marker is found, require the complete 56-byte range.
5. Decode thirteen floats with the shared bounds-checked binary Reader.
6. Advance by one full record after a successful match.

This strategy intentionally permits unknown header/padding bytes before records. It does not assume a record count or fixed first-record offset that has not been confirmed.

## Diagnostics

- `hits.unrecognized` — magic absent;
- `hits.no_records` — magic recognized, but no complete marker/record found;
- `hits.truncated_record` — marker found without 56 available bytes;
- `hits.invalid_record` — complete record range could not be decoded.

No-record is a warning, not a structural error. A truncated confirmed marker is an error.

## Synthetic tests

The test corpus creates original bytes containing:

- HITS$ magic;
- unknown padding;
- two valid 56-byte records;
- deterministic float values;
- wrong-magic input;
- magic-only input;
- truncated-record input.

No original game bytes are committed.

## Current boundary

Implemented:

- recognition;
- confirmed record scanning;
- raw float decoding;
- structured diagnostics;
- cross-platform tests.

Not implemented:

- semantic names for the thirteen floats;
- file-level record count/header interpretation;
- editing or export;
- Stage Ops visualization adapter;
- game-sample Evidence Packet.

The next evidence-backed step is to identify record semantics and sample hashes through local legal files, then attach the scanner output to Binary Inspector regions and StageBundle collision resources.
