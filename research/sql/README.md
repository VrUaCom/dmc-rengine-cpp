# DMC Rengine research SQLite

This directory is the structured evidence store for reverse-engineering facts.

The repository tracks **schema, importers and source evidence**, not a mutable
binary database. A local SQLite database is regenerated from committed evidence
so changes remain reviewable in Git.

## Build the database

From the repository root:

```bash
python research/sql/import_eventtbl_census.py
```

Default output:

```text
research-private/dmc_rengine_research.sqlite3
```

No third-party Python modules are required.

To choose another database path:

```bash
python research/sql/import_eventtbl_census.py --db out/research.sqlite3
```

## Current EventTbl coverage

The importer consumes:

```text
docs/research/dmc3-eventtbl-opcode-census-2026-09-12.json
```

and stores:

- format identity (`EventTbl`, `EVT\0`, stock `.bin` filename extension);
- corpus/source provenance;
- EventTbl file hashes and structural metadata;
- stream roots where present in the census;
- complete opcode census rows;
- observed opcode arity/count/file count;
- semantic registry and evidence status;
- structural begin/end opcode pairs;
- evidence claims with repository source locators.

The schema also reserves normalized `evt_command` and `evt_argument` tables for
the next stage: exporting every parsed command and raw argument from the actual
EventTbl payloads. Unknown argument semantics remain
`PRESERVED_UNDECODED` until evidence promotes them.

## Evidence rule

SQLite is not authority by itself. Every semantic promotion must retain an
evidence status and source. Do not convert a corpus correlation into an
EXE-confirmed semantic merely because it is stored in the database.

Useful statuses include the project evidence vocabulary such as:

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

Legacy research rows may also preserve narrower source strings such as
`CORPUS_STRUCTURAL_CONFIRMED`; those should be migrated deliberately rather
than silently rewritten.

## Example queries

Known and unknown opcodes:

```sql
SELECT opcode_hex, argc, commands, files, name, semantic_class, evidence_status
FROM v_evt_opcode_status;
```

Only unresolved opcodes, ordered by frequency:

```sql
SELECT opcode_hex, argc, commands, files
FROM v_evt_opcode_status
WHERE evidence_status = 'PRESERVED_UNDECODED'
ORDER BY commands DESC;
```

Files and stream counts:

```sql
SELECT * FROM v_evt_file_summary;
```

Confirmed spawn-related opcodes:

```sql
SELECT opcode_hex, name, evidence_status
FROM v_evt_opcode_status
WHERE semantic_class = 'spawn';
```

Evidence attached to EventTbl:

```sql
SELECT subject_key, claim, evidence_status, source_locator
FROM evidence_claim
WHERE subject_type IN ('format', 'corpus')
ORDER BY subject_key;
```

## Direction

The database is intentionally broader than EventTbl. New reverse domains can
add normalized tables/migrations under `research/sql/` while sharing
`source_artifact`, `resource_file` and `evidence_claim`.

For EventTbl the next importer should consume the C++ parser output and fill
`evt_command` + `evt_argument` for every supplied file. That makes queries such
as opcode neighbourhoods, argument-domain clustering, stream-flow analysis and
EXE semantic promotion reproducible instead of living only in prose reports.
