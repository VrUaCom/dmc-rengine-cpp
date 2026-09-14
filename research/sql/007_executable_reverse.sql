-- Executable reverse layer.
--
-- Structured index over the deterministic reports produced by
-- `dmc-rengine analyze-exe` and `dmc-rengine map-functions`. Everything here is
-- regenerable from those reports; the database is an index, never authority.
--
-- Cross-cutting questions drove this layer. "Which functions read table X, call
-- a math import and are bound to a class" is awkward over a two-megabyte JSON
-- document and natural in SQL.

PRAGMA foreign_keys = ON;

BEGIN;

CREATE TABLE IF NOT EXISTS exe_image (
    id INTEGER PRIMARY KEY,
    sha256 TEXT NOT NULL UNIQUE,
    size_bytes INTEGER NOT NULL,
    image_base INTEGER,
    entry_point_rva INTEGER,
    machine TEXT,
    notes TEXT
);

CREATE TABLE IF NOT EXISTS exe_function (
    id INTEGER PRIMARY KEY,
    image_id INTEGER NOT NULL REFERENCES exe_image(id) ON DELETE CASCADE,
    begin_rva INTEGER NOT NULL,
    end_rva INTEGER NOT NULL,
    size_bytes INTEGER NOT NULL,
    instruction_count INTEGER NOT NULL DEFAULT 0,
    walk_complete INTEGER NOT NULL DEFAULT 1,
    caller_count INTEGER NOT NULL DEFAULT 0,
    callee_count INTEGER NOT NULL DEFAULT 0,
    export_name TEXT,
    reachable_from_entry INTEGER NOT NULL DEFAULT 0,
    reachable_from_export INTEGER NOT NULL DEFAULT 0,
    -- Prologue facts read from the unwind record.
    prolog_size INTEGER,
    stack_allocation INTEGER,
    pushed_registers INTEGER,
    frame_register INTEGER,
    exception_handler INTEGER NOT NULL DEFAULT 0,
    -- No function is named by this programme; the column exists for promoted
    -- names only, and stays NULL until evidence supports one.
    recovered_name TEXT,
    name_evidence_status TEXT NOT NULL DEFAULT 'PRESERVED_UNDECODED',
    UNIQUE(image_id, begin_rva)
);

CREATE INDEX IF NOT EXISTS idx_exe_function_size ON exe_function(size_bytes);
CREATE INDEX IF NOT EXISTS idx_exe_function_callers ON exe_function(caller_count);

CREATE TABLE IF NOT EXISTS exe_class (
    id INTEGER PRIMARY KEY,
    image_id INTEGER NOT NULL REFERENCES exe_image(id) ON DELETE CASCADE,
    display_name TEXT NOT NULL,
    decorated_name TEXT,
    vtable_slots INTEGER NOT NULL DEFAULT 0,
    slots_bound_to_functions INTEGER NOT NULL DEFAULT 0,
    distinct_functions INTEGER NOT NULL DEFAULT 0,
    install_sites INTEGER NOT NULL DEFAULT 0,
    UNIQUE(image_id, display_name)
);

CREATE TABLE IF NOT EXISTS exe_vtable_binding (
    id INTEGER PRIMARY KEY,
    function_id INTEGER NOT NULL REFERENCES exe_function(id) ON DELETE CASCADE,
    class_id INTEGER NOT NULL REFERENCES exe_class(id) ON DELETE CASCADE,
    vtable_index INTEGER NOT NULL,
    slot INTEGER NOT NULL,
    subobject_offset INTEGER NOT NULL DEFAULT 0,
    UNIQUE(function_id, class_id, vtable_index, slot)
);

CREATE INDEX IF NOT EXISTS idx_exe_vtable_binding_class ON exe_vtable_binding(class_id);

CREATE TABLE IF NOT EXISTS exe_vtable_install (
    id INTEGER PRIMARY KEY,
    function_id INTEGER NOT NULL REFERENCES exe_function(id) ON DELETE CASCADE,
    class_id INTEGER NOT NULL REFERENCES exe_class(id) ON DELETE CASCADE,
    vtable_index INTEGER NOT NULL,
    vtable_rva INTEGER NOT NULL,
    UNIQUE(function_id, class_id, vtable_index)
);

CREATE TABLE IF NOT EXISTS exe_import_call (
    id INTEGER PRIMARY KEY,
    function_id INTEGER NOT NULL REFERENCES exe_function(id) ON DELETE CASCADE,
    module TEXT NOT NULL,
    symbol TEXT NOT NULL,
    UNIQUE(function_id, module, symbol)
);

CREATE INDEX IF NOT EXISTS idx_exe_import_call_symbol ON exe_import_call(module, symbol);

CREATE TABLE IF NOT EXISTS exe_dispatch_site (
    id INTEGER PRIMARY KEY,
    function_id INTEGER NOT NULL REFERENCES exe_function(id) ON DELETE CASCADE,
    displacement INTEGER NOT NULL,
    slot INTEGER NOT NULL,
    UNIQUE(function_id, displacement)
);

-- Recovered fixed-width name tables and multi-field name records.
CREATE TABLE IF NOT EXISTS exe_name_table (
    id INTEGER PRIMARY KEY,
    image_id INTEGER NOT NULL REFERENCES exe_image(id) ON DELETE CASCADE,
    base_rva INTEGER NOT NULL,
    layout TEXT NOT NULL CHECK(layout IN ('STRIDE','RECORD')),
    element_bytes INTEGER NOT NULL,
    entries INTEGER NOT NULL,
    fields_per_record INTEGER,
    longest_name INTEGER,
    pure_name_array INTEGER,
    records_with_payload INTEGER,
    text_payload_records INTEGER,
    payload_offset_consistent INTEGER,
    content_period INTEGER,
    sample_name TEXT,
    -- Elements the scan removed from the end of the run because they were a
    -- packed string pool the grid had absorbed rather than table elements.
    absorbed_elements INTEGER NOT NULL DEFAULT 0,
    -- Instructions that index this run as an array: a register holding its
    -- base, scaled by the run's own element size. Zero throughout the canonical
    -- image, which is the finding rather than a gap in the data.
    indexed_sites INTEGER NOT NULL DEFAULT 0,
    -- Where this run sits inside a record run's field layout, and how many
    -- elements it had to give back at the end of that record's field block.
    interior_of_record_rva INTEGER,
    interior_field_index INTEGER,
    overrun_elements INTEGER NOT NULL DEFAULT 0,
    -- STRUCTURAL_CONFIRMED  layout and extent both stand as read
    -- RECORD_INTERIOR       the run is a record's block of equal-width fields,
    --                       so the record describes the same bytes more fully
    -- EXTENT_TRIMMED        extent corrected against an absorbed string pool
    -- SEMANTIC_CANDIDATE    payload-bearing run whose extent is not settled
    -- EXTENT_UNCLASSIFIED   run known only through the function map, which
    --                       carries its layout but not its payload measurements
    extent_status TEXT NOT NULL DEFAULT 'STRUCTURAL_CONFIRMED'
        CHECK(extent_status IN ('STRUCTURAL_CONFIRMED','RECORD_INTERIOR','EXTENT_TRIMMED',
                                'SEMANTIC_CANDIDATE','EXTENT_UNCLASSIFIED')),
    -- Which report the row was read from. The analysis report lists only the
    -- largest runs, so a table reached by a call site is often known through
    -- the function map alone; both come from the same scan.
    known_from TEXT NOT NULL DEFAULT 'ANALYSIS' CHECK(known_from IN ('ANALYSIS','MAP')),
    UNIQUE(image_id, base_rva, layout)
);

CREATE INDEX IF NOT EXISTS idx_exe_name_table_entries ON exe_name_table(entries);

CREATE TABLE IF NOT EXISTS exe_name_table_field (
    id INTEGER PRIMARY KEY,
    table_id INTEGER NOT NULL REFERENCES exe_name_table(id) ON DELETE CASCADE,
    field_index INTEGER NOT NULL,
    field_offset INTEGER NOT NULL,
    field_width INTEGER NOT NULL,
    extension TEXT,
    UNIQUE(table_id, field_index)
);

CREATE TABLE IF NOT EXISTS exe_table_reference (
    id INTEGER PRIMARY KEY,
    function_id INTEGER NOT NULL REFERENCES exe_function(id) ON DELETE CASCADE,
    table_id INTEGER NOT NULL REFERENCES exe_name_table(id) ON DELETE CASCADE,
    offset_in_table INTEGER NOT NULL,
    constant_index INTEGER NOT NULL DEFAULT 0,
    element_index INTEGER,
    field_index INTEGER,
    offset_in_element INTEGER NOT NULL DEFAULT 0,
    UNIQUE(function_id, table_id, offset_in_table)
);

CREATE INDEX IF NOT EXISTS idx_exe_table_reference_table ON exe_table_reference(table_id);

-- Coverage of a table by constant indices: trustworthy for a pure name array,
-- an upper bound for a payload-bearing run whose extent may be over-stated.
CREATE VIEW IF NOT EXISTS v_exe_table_coverage AS
SELECT t.id AS table_id,
       printf('0x%x', t.base_rva) AS base_rva,
       t.layout,
       t.element_bytes,
       t.entries,
       t.pure_name_array,
       COUNT(DISTINCT r.function_id) AS referencing_functions,
       COUNT(DISTINCT CASE WHEN r.constant_index=1 THEN r.element_index END) AS elements_named,
       MAX(CASE WHEN r.constant_index=1 THEN r.element_index END) AS highest_element_named,
       -- References the scan could only place inside an element. A run with
       -- several of these at a uniform sub-multiple of its stride is indexed at
       -- a pitch its declared stride cannot express, which means the stride is
       -- wrong or the run spans more than one structure.
       COUNT(CASE WHEN r.offset_in_element != 0 THEN 1 END) AS interior_references,
       t.absorbed_elements,
       t.overrun_elements,
       t.indexed_sites,
       CASE WHEN t.interior_of_record_rva IS NULL THEN NULL
            ELSE printf('0x%x', t.interior_of_record_rva) END AS interior_of_record,
       t.extent_status
FROM exe_name_table t
LEFT JOIN exe_table_reference r ON r.table_id = t.id
GROUP BY t.id;

-- Functions carrying at least one structural attribution.
CREATE VIEW IF NOT EXISTS v_exe_function_attribution AS
SELECT f.id AS function_id,
       printf('0x%x', f.begin_rva) AS begin_rva,
       f.size_bytes,
       f.instruction_count,
       f.caller_count,
       f.callee_count,
       f.export_name,
       (SELECT COUNT(*) FROM exe_vtable_binding b WHERE b.function_id=f.id) AS vtable_bindings,
       (SELECT COUNT(*) FROM exe_vtable_install i WHERE i.function_id=f.id) AS vtable_installs,
       (SELECT COUNT(*) FROM exe_import_call c WHERE c.function_id=f.id) AS import_calls,
       (SELECT COUNT(*) FROM exe_table_reference r WHERE r.function_id=f.id) AS table_references,
       f.recovered_name,
       f.name_evidence_status
FROM exe_function f;

-- Imports ranked by how many functions call them.
CREATE VIEW IF NOT EXISTS v_exe_import_usage AS
SELECT module, symbol, COUNT(DISTINCT function_id) AS calling_functions
FROM exe_import_call
GROUP BY module, symbol;

COMMIT;

-- Tables that stand on their own: a record run, or a stride run that is not
-- some record's interior. Counting every recovered run as a table counts the
-- same bytes twice wherever a record is made of equal-width name fields.
CREATE VIEW IF NOT EXISTS v_exe_independent_table AS
SELECT id, printf('0x%x', base_rva) AS base_rva, layout, element_bytes, entries,
       fields_per_record, sample_name, extent_status
FROM exe_name_table
WHERE interior_of_record_rva IS NULL;
