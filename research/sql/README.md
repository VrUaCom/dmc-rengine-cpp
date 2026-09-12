# DMC Rengine research SQLite

This directory is the structured evidence store for reverse-engineering facts.
The repository tracks schema, importers and machine-readable evidence; the mutable
`.sqlite3` database is generated locally and is not committed.

## EventTbl authority model

The canonical EventTbl scope is **all 22 runtime slots**:

```text
EventTbl00.bin ... EventTbl21.bin
```

The database always creates 22 `evt_runtime_slot` rows. A slot is never removed
from the research model merely because its current byte payload is unavailable.
At the 2026-09-12 corpus state:

- 22/22 runtime slots are represented;
- 19/22 byte payloads are available and parsed (`00..09`, `13..21`);
- `10..12` are represented as `MISSING_BYTES`;
- 12,012 command instances are imported;
- 16,927 raw u32 arguments are imported;
- 121 opcode values are observed in the available bytes.

## Build the database

Place available stock EventTbl payloads in:

```text
research-private/eventtbl/
```

using their stock names (`EventTbl00.bin` ... `EventTbl21.bin`), then run:

```bash
python research/sql/import_eventtbl_runtime_family.py
```

Default database:

```text
research-private/dmc_rengine_research.sqlite3
```

The legacy command still works and routes to the same full importer:

```bash
python research/sql/import_eventtbl_census.py
```

To require byte-complete `00..21` coverage instead of allowing explicit missing
slots:

```bash
python research/sql/import_eventtbl_runtime_family.py --require-all-bytes
```

## What is stored

`001_schema.sql` defines the shared research/evidence tables. `002_eventtbl_runtime_family.sql`
adds the fixed `00..21` runtime-slot layer.

For every available valid EventTbl payload the importer stores:

- stock slot index/name/runtime path;
- exact size and SHA-256;
- revision, stream count, terminal offset;
- every stream root;
- every command instance with offset, raw descriptor, opcode and arity;
- every raw u32 argument;
- opcode command/file counts;
- known semantic registry entries and evidence status;
- structural scope-pair evidence.

Missing byte payloads remain queryable through `evt_runtime_slot` with
`bytes_status='MISSING_BYTES'`.

## Semantic reverse workflow

`analyze_eventtbl_semantics.py` performs reproducible corpus analysis over the
normalized command/argument tables. It reports file coverage, argument domains,
immediate predecessor/successor distributions and leaves semantics neutral by
default.

Example:

```bash
python research/sql/analyze_eventtbl_semantics.py \
  --opcode 0x2C --opcode 0x31 --opcode 0x48 --opcode 0x3D \
  --output research-private/eventtbl-semantic-pass01.json
```

Current evidence migrations and reports are:

```text
003_eventtbl_semantic_pass01.sql  -> 0x2C, 0x31, 0x48, 0x3D
004_eventtbl_semantic_pass02.sql  -> 0x4D, 0x4F, 0x36, 0x49
005_eventtbl_semantic_pass03.sql  -> 0x4C, 0x58, 0x47, 0x72, 0x37, 0x65
006_eventtbl_semantic_pass04.sql  -> 0x46, 0x6E, 0x6F, 0x38, 0x71, 0x66
```

with corresponding Markdown reports under `docs/research/`.

Corpus facts and semantic hypotheses are stored separately. A `SEMANTIC_CANDIDATE`
never becomes `EXE_CONFIRMED` merely because a repeated motif is strong.

## Evidence rule

SQLite is an index, not authority by itself. Unknown opcode/argument semantics
remain `PRESERVED_UNDECODED`. A semantic name is promoted only when supported by
EXE/runtime or explicit corpus evidence.

Project evidence vocabulary includes:

```text
EXE_CONFIRMED
CORPUS_CONFIRMED
EXE_AND_CORPUS_CONFIRMED
STRUCTURAL_CONFIRMED
SEMANTIC_CANDIDATE
PRESERVED_UNDECODED
RESERVED_OBSERVED_ZERO
REJECTED
```

## Useful queries

Full 00..21 coverage:

```sql
SELECT * FROM v_evt_runtime_coverage;
```

Slots whose bytes are still missing/invalid:

```sql
SELECT * FROM v_evt_runtime_missing;
```

Highest-frequency unresolved opcodes:

```sql
SELECT printf('0x%02X', opcode) AS opcode,
       observed_argument_count AS argc,
       observed_command_count AS commands,
       observed_file_count AS files
FROM evt_opcode
WHERE evidence_status='PRESERVED_UNDECODED'
ORDER BY observed_command_count DESC;
```

Every occurrence of one opcode with file/stream/offset:

```sql
SELECT f.stock_name, c.stream_ordinal, c.sequence_index,
       printf('0x%X', c.file_offset) AS file_offset
FROM evt_command c
JOIN resource_file f ON f.id=c.file_id
WHERE c.opcode=0x2C
ORDER BY f.corpus_index, c.sequence_index;
```

Raw argument domain for one opcode:

```sql
SELECT a.argument_index, a.value_u32, COUNT(*) AS n
FROM evt_command c
JOIN evt_argument a ON a.command_id=c.id
WHERE c.opcode=0x2C
GROUP BY a.argument_index, a.value_u32
ORDER BY a.argument_index, n DESC;
```

## Canonical machine-readable evidence

`docs/research/dmc3-eventtbl-opcode-census-2026-09-12.json` is regenerated from
the actual available binary payloads and describes the 22-slot runtime family,
including explicit `MISSING_BYTES` slots. The Markdown report is
`docs/research/dmc3-eventtbl-corpus-reverse-2026-09-12.md`.
