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

## Executable reverse layer

`007_executable_reverse.sql` indexes the deterministic reports produced by the
`analyze-exe` and `map-functions` commands: functions and their prologue facts,
RTTI classes, vtable bindings and installs, import calls, virtual dispatch
offsets, recovered name tables and the references from code into them. Build it
from reports, never from the executable directly:

```sh
dmc-rengine analyze-exe   <exe> --out analysis.json
dmc-rengine map-functions <exe> --all --out map.json
python research/sql/import_executable_reverse.py \
    --analysis analysis.json --map map.json \
    --db research-private/reverse.sqlite3
```

The importer refuses reports whose artifact SHA-256 differ, and never invents a
table to hang a reference on. Tables hold layout and linkage only, plus one
representative name per run; table contents are not extracted here.

No function carries a recovered name. `exe_function.recovered_name` is NULL
throughout with `name_evidence_status='PRESERVED_UNDECODED'`, and the column
exists for names that evidence later supports.

### Reading a table's extent

`v_exe_table_coverage` answers the question the scan cannot answer alone: does
the code agree with the extent read off the bytes? A constant index is folded
into its displacement, so each reference is an exact byte offset and the spacing
of those offsets measures the element size independently.

```sql
-- Runs the code indexes at a pitch their declared stride cannot express.
SELECT base_rva, element_bytes, entries, interior_references, extent_status
FROM v_exe_table_coverage
WHERE interior_references > 0;

-- Runs whose extent was corrected against an absorbed string pool.
SELECT base_rva, element_bytes, entries, absorbed_elements, sample_name
FROM exe_name_table
WHERE absorbed_elements > 0;

-- Tables that stand on their own, with each record's interior excluded.
SELECT * FROM v_exe_independent_table;
```

`extent_status` records how far each run's extent is settled:

| value | meaning |
| --- | --- |
| `STRUCTURAL_CONFIRMED` | layout and extent both stand as read |
| `EXTENT_TRIMMED` | extent corrected against an absorbed string pool |
| `RECORD_INTERIOR` | the run is a record's block of equal-width fields, so the record describes the same bytes more fully |
| `SEMANTIC_CANDIDATE` | payload-bearing run whose extent is not settled |
| `EXTENT_UNCLASSIFIED` | run known only through the function map, which carries its layout but not its payload measurements |

### Reading a table's access

`exe_name_table.indexed_sites` counts instructions that index a run as an array
— a register holding its base, scaled by the run's own element size. It is zero
throughout the canonical image, and that is the finding rather than a gap:

```sql
-- Runs the code indexes as arrays, as opposed to reaching element by element.
SELECT base_rva, element_bytes, entries, indexed_sites
FROM v_exe_table_coverage
WHERE indexed_sites > 0;
```

A reference whose offset is a whole multiple of the stride is *not* evidence of
indexing: a constant index folded into a displacement and a direct load of the
literal at that offset are the same instruction. `elements_named_by_constant`
says code reaches those bytes; only `indexed_sites` says it treats them as a
table.

### Indexed data arrays

`exe_indexed_array` holds arrays the code walks with a scaled index, and
`exe_indexed_array_field` the offsets read inside their elements. Element sizes
above eight come from the index multiplier, not the scale field.

```sql
-- Arrays whose element layout is more than a single observation.
SELECT base_rva, element_bytes, fields_observed, field_offsets, sites
FROM v_exe_array_layout
ORDER BY fields_observed DESC, sites DESC;
```

A field offset must fall inside its element; the importer refuses one that does
not rather than storing a layout that contradicts itself.

`base_measured` separates the two routes to an array. It is 1 when a register
was seen holding the base, so that is where the array starts. It is 0 when the
array was reached against the image base, where the start is folded into the
displacement: the stored base is then the lowest address observed, an upper
bound, with the field offsets measured relative to it.

### Class layout

`exe_class_field` holds what each class contains at which offset, read out of
what its constructor writes into the object.

```sql
SELECT * FROM v_exe_class_layout WHERE class_name = 'CScene';
```

`relation` is `MEMBER` when the vtable belongs to another class, so the offset
names something the object holds, and `BASE` when it belongs to the same class,
so the offset names a base subobject. `offset_confirmed_by_rtti` marks the
entries where the RTTI's own recorded subobject offset agrees with the store.

### Call graph and subsystem reach

`exe_call_edge` holds the direct call edges between functions in the inventory.
`v_exe_import_reach` uses them to size a subsystem: which functions reach a
module's imports within three calls.

```sql
SELECT * FROM v_exe_import_reach;
```

The figures are lower bounds — unresolved indirect jumps are holes in the graph.

### Class hierarchy

`exe_class_base` holds the hierarchy the compiler declared, one row per base a
class names, read from the MSVC class hierarchy descriptors rather than inferred
from code.

```sql
SELECT * FROM v_exe_base_reach LIMIT 20;
```

A base named by many classes is an interface in practice, whatever it is called.
Bases carrying no vtable of their own are created here so the hierarchy is not
truncated at exactly the interfaces that make it worth having.

### Functions parameterised by a constant

`exe_constant_argument_callee` and `exe_constant_argument` record which
functions are called with a constant first argument and which values reach them.

```sql
SELECT * FROM v_exe_constant_argument LIMIT 20;
```

The meaning of a constant is not stored, because nothing in the file states it.
`callee_id` is NULL where the address is not a function in the exception
directory.

### Virtual call edges

`exe_resolved_dispatch` holds virtual calls whose receiver is `this`, resolved
to a class, a slot and a target function.

```sql
SELECT * FROM v_exe_virtual_call_edge ORDER BY class_name, slot;
```

Only calls on `this` in a method bound into exactly one vtable are here. Every
other dispatch site is counted in the report's summary and left unresolved:
nothing in the file says what an argument or a heap pointer points at.

### Reachability as a bracket

`exe_function.outside_every_closure` marks a function the entry point cannot
reach even when every virtual call is assumed to reach whatever sits at its slot
in any vtable the image carries. That assumption is unsound as an answer and
sound as a bound, so the column is a statement about what is *not* reachable.

```sql
SELECT * FROM v_exe_reachability_bracket;
SELECT * FROM v_exe_unreachable_function LIMIT 20;
```

Build with `map-functions --all`. Without it the report carries a subset of the
functions and the bracket's percentages describe that subset, not the image.

`exe_function_pointer_run` holds runs of consecutive function addresses in data
that fall outside every located vtable's whole extent — excluding vtables by
base alone reports each fragment of a split vtable as a table of its own.

### Class size floors

`exe_class_size_floor` holds a floor on each class's object size, never a size.
Two sources: where the hierarchy descriptor places base subobjects carrying a
vtable, and how far into the object the class's own bound methods reach. Neither
reads what is at any offset.

```sql
SELECT class_name, floor_bytes, deepest_base, functions_reaching_half, support
FROM v_exe_class_size WHERE support = 'CORROBORATED' LIMIT 20;
```

`support` says how many functions independently reach at least half the floor,
so a number one function alone supports is visibly a weaker claim than one a
dozen agree on.

### Vtable slot census

`exe_base_slot_override` holds one row per base class and slot: how many classes
inherit that slot and what they put there. The comparison runs against the
vtable of the base's own subobject, at the offset the class hierarchy descriptor
records for it, so both sides describe the same interface — a base at a non-zero
displacement has a different vtable from the derived class's primary one.

`base_kind` says what the base itself puts in the slot, read from the first
instruction of its target: `pure-virtual` reaches the import named `_purecall`,
`empty-body` starts with a return, `implemented` is anything else.

```sql
SELECT base_name, slot, derived_classes, distinct_implementations, override_ratio
FROM v_exe_slot_override WHERE base_name = 'CWork' ORDER BY slot;

SELECT * FROM v_exe_base_interface_shape WHERE inheritors >= 15;
```

`override_ratio` is distinct implementations per class carrying the slot. Near 1
means per-class behaviour lives there; near 0 means the slot is inherited and
only a few bodies exist for it. The linker folds identical functions, so the
distinct count is a lower bound on distinct behaviour.

Layout and linkage only. No slot is named, and the ratio is not evidence of what
a slot is for.

Every table carrying a CHECK is inserted with a conflict clause scoped to its
own unique key rather than `INSERT OR IGNORE`, which suppresses CHECK failures
as well as uniqueness conflicts and would drop a malformed row silently.

Guardrails for the importer are in `test_import_executable_reverse.py` and run
in CI.
