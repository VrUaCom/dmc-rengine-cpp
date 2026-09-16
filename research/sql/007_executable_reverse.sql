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
    -- True when the function is not reachable from the entry point even under
    -- the loosest sound assumption: that every virtual call reaches whatever
    -- sits at its slot in any vtable the image carries. Nothing in the file
    -- says the entry point can get here.
    outside_every_closure INTEGER NOT NULL DEFAULT 0,
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

-- The class hierarchy the compiler declared, one row per base a class names.
-- Read from the MSVC class hierarchy descriptors, not inferred from code.
CREATE TABLE IF NOT EXISTS exe_class_base (
    id INTEGER PRIMARY KEY,
    class_id INTEGER NOT NULL REFERENCES exe_class(id) ON DELETE CASCADE,
    base_id INTEGER NOT NULL REFERENCES exe_class(id) ON DELETE CASCADE,
    member_displacement INTEGER NOT NULL DEFAULT 0,
    UNIQUE(class_id, base_id, member_displacement)
);

CREATE INDEX IF NOT EXISTS idx_exe_class_base_base ON exe_class_base(base_id);

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

-- Direct call edges between functions in the inventory, deduplicated. Calls to
-- an import thunk are recorded as import calls instead, and a function calling
-- itself is not an edge.
CREATE TABLE IF NOT EXISTS exe_call_edge (
    id INTEGER PRIMARY KEY,
    caller_id INTEGER NOT NULL REFERENCES exe_function(id) ON DELETE CASCADE,
    callee_id INTEGER NOT NULL REFERENCES exe_function(id) ON DELETE CASCADE,
    UNIQUE(caller_id, callee_id)
);

CREATE INDEX IF NOT EXISTS idx_exe_call_edge_callee ON exe_call_edge(callee_id);

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

-- Arrays in data the code walks with a scaled index, and the offsets it reads
-- inside their elements. Recovered from instruction encodings alone; layout
-- only, never contents.
CREATE TABLE IF NOT EXISTS exe_indexed_array (
    id INTEGER PRIMARY KEY,
    image_id INTEGER NOT NULL REFERENCES exe_image(id) ON DELETE CASCADE,
    base_rva INTEGER NOT NULL,
    element_bytes INTEGER NOT NULL,
    sites INTEGER NOT NULL DEFAULT 0,
    referencing_functions INTEGER NOT NULL DEFAULT 0,
    -- 1 when a register was seen holding this base, so it is where the array
    -- starts. 0 when the array was reached against the image base, where the
    -- start is folded into the displacement and this is the lowest address
    -- observed: an upper bound, with the field offsets relative to it.
    base_measured INTEGER NOT NULL DEFAULT 1,
    UNIQUE(image_id, base_rva, element_bytes)
);

CREATE TABLE IF NOT EXISTS exe_indexed_array_field (
    id INTEGER PRIMARY KEY,
    array_id INTEGER NOT NULL REFERENCES exe_indexed_array(id) ON DELETE CASCADE,
    field_offset INTEGER NOT NULL,
    UNIQUE(array_id, field_offset)
);

-- Class layout read out of constructor stores: which class sits at which
-- offset inside another. Layout only, from instruction encodings and the RTTI
-- the compiler emitted.
CREATE TABLE IF NOT EXISTS exe_class_field (
    id INTEGER PRIMARY KEY,
    image_id INTEGER NOT NULL REFERENCES exe_image(id) ON DELETE CASCADE,
    class_id INTEGER REFERENCES exe_class(id) ON DELETE CASCADE,
    member_class_id INTEGER REFERENCES exe_class(id) ON DELETE CASCADE,
    field_offset INTEGER NOT NULL,
    -- The vtable actually written here. For a base subobject that is a
    -- secondary vtable of the class itself, which is what a call through this
    -- offset dispatches into.
    member_vtable_rva INTEGER,
    site_rva INTEGER NOT NULL,
    -- The member's class differs from the constructor's, so the offset names
    -- something the object contains rather than a base it is.
    embedded_member INTEGER NOT NULL DEFAULT 0,
    -- The RTTI records this vtable's subobject offset and it matches the store:
    -- two independent statements of where the subobject sits.
    offset_confirmed_by_rtti INTEGER NOT NULL DEFAULT 0,
    UNIQUE(image_id, class_id, field_offset, member_class_id)
);

-- Functions the code calls with a constant first argument, and the constants
-- that reach them. What the constant means is not stated: this bounds an
-- enumeration without naming it.
CREATE TABLE IF NOT EXISTS exe_constant_argument_callee (
    id INTEGER PRIMARY KEY,
    image_id INTEGER NOT NULL REFERENCES exe_image(id) ON DELETE CASCADE,
    callee_id INTEGER REFERENCES exe_function(id) ON DELETE CASCADE,
    callee_rva INTEGER NOT NULL,
    call_sites INTEGER NOT NULL DEFAULT 0,
    distinct_arguments INTEGER NOT NULL DEFAULT 0,
    UNIQUE(image_id, callee_rva)
);

CREATE TABLE IF NOT EXISTS exe_constant_argument (
    id INTEGER PRIMARY KEY,
    callee_id INTEGER NOT NULL REFERENCES exe_constant_argument_callee(id) ON DELETE CASCADE,
    argument INTEGER NOT NULL,
    UNIQUE(callee_id, argument)
);

-- Virtual calls resolved to a class, a slot and a target function.
--
-- The receiver is `this`: the dispatch reads the vtable pointer out of the
-- register the Microsoft x64 convention puts the first argument in, or out of a
-- copy of it. The enclosing method's own binding then says which vtable, and the
-- displacement says which slot, so the target is read rather than guessed.
CREATE TABLE IF NOT EXISTS exe_resolved_dispatch (
    id INTEGER PRIMARY KEY,
    image_id INTEGER NOT NULL REFERENCES exe_image(id) ON DELETE CASCADE,
    site_rva INTEGER NOT NULL,
    caller_id INTEGER REFERENCES exe_function(id) ON DELETE SET NULL,
    target_id INTEGER REFERENCES exe_function(id) ON DELETE SET NULL,
    class_id INTEGER REFERENCES exe_class(id) ON DELETE SET NULL,
    displacement INTEGER NOT NULL,
    slot INTEGER NOT NULL,
    -- Offset within the enclosing object the receiver was read from: zero is a
    -- call on the object, anything else a call on what sits there.
    receiver_field_offset INTEGER NOT NULL DEFAULT 0,
    UNIQUE(image_id, site_rva)
);

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

-- Arrays whose element layout is partly recovered: more than one field offset
-- observed inside the element, which is a layout statement rather than a single
-- observation.
CREATE VIEW IF NOT EXISTS v_exe_array_layout AS
SELECT a.id AS array_id,
       printf('0x%x', a.base_rva) AS base_rva,
       a.element_bytes,
       a.sites,
       a.referencing_functions,
       a.base_measured,
       COUNT(f.id) AS fields_observed,
       GROUP_CONCAT(f.field_offset, ',') AS field_offsets
FROM exe_indexed_array a
LEFT JOIN exe_indexed_array_field f ON f.array_id = a.id
GROUP BY a.id
HAVING fields_observed > 1;

-- Virtual call edges, as caller and target addresses with the class the call
-- dispatches on.
CREATE VIEW IF NOT EXISTS v_exe_virtual_call_edge AS
SELECT printf('0x%x', d.site_rva) AS site_rva,
       printf('0x%x', caller.begin_rva) AS caller_rva,
       c.display_name AS class_name,
       d.receiver_field_offset,
       d.slot,
       printf('0x%x', target.begin_rva) AS target_rva,
       target.size_bytes AS target_size
FROM exe_resolved_dispatch d
LEFT JOIN exe_function caller ON caller.id = d.caller_id
LEFT JOIN exe_function target ON target.id = d.target_id
LEFT JOIN exe_class c ON c.id = d.class_id;

-- What each class contains, by offset: members it holds and bases it is.
CREATE VIEW IF NOT EXISTS v_exe_class_layout AS
SELECT c.display_name AS class_name,
       f.field_offset,
       m.display_name AS member_class,
       CASE WHEN f.embedded_member THEN 'MEMBER' ELSE 'BASE' END AS relation,
       f.offset_confirmed_by_rtti,
       printf('0x%x', f.site_rva) AS site_rva
FROM exe_class_field f
JOIN exe_class c ON c.id = f.class_id
LEFT JOIN exe_class m ON m.id = f.member_class_id
ORDER BY c.display_name, f.field_offset;

-- Functions parameterised by a small constant, with the values seen.
CREATE VIEW IF NOT EXISTS v_exe_constant_argument AS
SELECT printf('0x%x', c.callee_rva) AS callee_rva,
       f.size_bytes AS callee_size,
       f.caller_count,
       c.call_sites,
       c.distinct_arguments,
       GROUP_CONCAT(a.argument, ',') AS arguments
FROM exe_constant_argument_callee c
LEFT JOIN exe_function f ON f.id = c.callee_id
LEFT JOIN exe_constant_argument a ON a.callee_id = c.id
GROUP BY c.id
ORDER BY c.call_sites DESC;

-- How widely each base is implemented: the engine's own design map, since an
-- interface is exactly a base many classes name.
CREATE VIEW IF NOT EXISTS v_exe_base_reach AS
SELECT b.display_name AS base_name,
       COUNT(DISTINCT c.id) AS implementors,
       -- An interface here is just a base whose name the compiler recorded with
       -- the project's own I-prefix; nothing structural marks one.
       CASE WHEN b.display_name GLOB 'I[A-Z]*' THEN 1 ELSE 0 END AS named_as_interface
FROM exe_class_base e
JOIN exe_class b ON b.id = e.base_id
JOIN exe_class c ON c.id = e.class_id
WHERE b.display_name <> c.display_name
GROUP BY b.id
ORDER BY implementors DESC;

-- Functions that can reach a given module's imports, and how far away they are.
-- Bounding a subsystem is a question about edges: who, directly or through a
-- few steps, ends up calling it.
CREATE VIEW IF NOT EXISTS v_exe_import_reach AS
WITH RECURSIVE seed(module, function_id, depth) AS (
    SELECT module, function_id, 0 FROM exe_import_call
    UNION
    SELECT s.module, e.caller_id, s.depth + 1
    FROM seed s
    JOIN exe_call_edge e ON e.callee_id = s.function_id
    WHERE s.depth < 3
)
SELECT module,
       COUNT(DISTINCT function_id) AS functions,
       SUM(CASE WHEN depth = 0 THEN 1 ELSE 0 END) AS direct_callers
FROM (SELECT module, function_id, MIN(depth) AS depth FROM seed GROUP BY module, function_id)
GROUP BY module
ORDER BY functions DESC;

-- ---------------------------------------------------------------------------
-- Vtable slot census
-- ---------------------------------------------------------------------------
-- One row per base class and slot: how many classes inherit that slot and what
-- they put in it. The comparison runs against the vtable of the base's own
-- subobject, at the offset the class hierarchy descriptor records, so the two
-- sides describe the same interface.
--
-- `base_kind` is read from the first instruction of the base's own target:
-- 'pure-virtual' reaches the import named _purecall, 'empty-body' starts with a
-- return, 'implemented' is anything else. No slot is named and no body is
-- described; this is layout and linkage only.
CREATE TABLE IF NOT EXISTS exe_base_slot_override (
    id INTEGER PRIMARY KEY,
    image_id INTEGER NOT NULL REFERENCES exe_image(id) ON DELETE CASCADE,
    base_id INTEGER NOT NULL REFERENCES exe_class(id) ON DELETE CASCADE,
    slot INTEGER NOT NULL,
    base_kind TEXT NOT NULL
        CHECK (base_kind IN ('pure-virtual', 'empty-body', 'implemented')),
    base_target_rva INTEGER NOT NULL,
    derived_classes INTEGER NOT NULL CHECK (derived_classes > 0),
    keep_base_target INTEGER NOT NULL CHECK (keep_base_target >= 0),
    empty_bodies INTEGER NOT NULL CHECK (empty_bodies >= 0),
    pure_virtual INTEGER NOT NULL CHECK (pure_virtual >= 0),
    distinct_implementations INTEGER NOT NULL CHECK (distinct_implementations >= 0),
    -- A class can only put one thing in a slot, so the four outcomes partition
    -- the classes carrying it. A row that breaks this is a counting error.
    CHECK (empty_bodies + pure_virtual <= derived_classes),
    CHECK (distinct_implementations <= derived_classes),
    CHECK (keep_base_target <= derived_classes),
    UNIQUE (image_id, base_id, slot)
);

CREATE INDEX IF NOT EXISTS idx_exe_base_slot_override_base
    ON exe_base_slot_override(base_id, slot);

-- The override ratio, which is the measurement: distinct implementations per
-- class carrying the slot. Near 1 means per-class behaviour lives here; near 0
-- means the slot is inherited and only a handful of bodies exist for it.
CREATE VIEW IF NOT EXISTS v_exe_slot_override AS
SELECT b.display_name AS base_name,
       o.slot,
       o.base_kind,
       o.derived_classes,
       o.distinct_implementations,
       o.empty_bodies,
       o.keep_base_target,
       ROUND(CAST(o.distinct_implementations AS REAL) / o.derived_classes, 3) AS override_ratio,
       -- Classes that put real code in the slot, whether or not they share it.
       o.derived_classes - o.empty_bodies - o.pure_virtual AS implementing_classes
FROM exe_base_slot_override o
JOIN exe_class b ON b.id = o.base_id
ORDER BY o.derived_classes DESC, b.display_name, o.slot;

-- How each base's own interface is declared: how much of it is left pure, how
-- much is a default body, and how wide it is. An abstract interface and a
-- concrete base look different here without anyone naming either.
CREATE VIEW IF NOT EXISTS v_exe_base_interface_shape AS
SELECT b.display_name AS base_name,
       MAX(o.derived_classes) AS inheritors,
       COUNT(*) AS slots,
       SUM(CASE WHEN o.base_kind = 'pure-virtual' THEN 1 ELSE 0 END) AS slots_declared_pure,
       SUM(CASE WHEN o.base_kind = 'empty-body' THEN 1 ELSE 0 END) AS slots_defaulted_empty,
       SUM(CASE WHEN o.base_kind = 'implemented' THEN 1 ELSE 0 END) AS slots_with_a_body
FROM exe_base_slot_override o
JOIN exe_class b ON b.id = o.base_id
GROUP BY o.base_id
ORDER BY inheritors DESC, slots DESC;

-- ---------------------------------------------------------------------------
-- Function-address runs in data
-- ---------------------------------------------------------------------------
-- Consecutive function addresses sitting in a non-executable section, outside
-- every located vtable's whole extent. A vtable is one of these, so excluding
-- vtables by base alone would report each fragment of a split vtable as a
-- table of its own.
CREATE TABLE IF NOT EXISTS exe_function_pointer_run (
    id INTEGER PRIMARY KEY,
    image_id INTEGER NOT NULL REFERENCES exe_image(id) ON DELETE CASCADE,
    base_rva INTEGER NOT NULL,
    entries INTEGER NOT NULL CHECK (entries >= 3),
    entries_reaching_nothing_else INTEGER NOT NULL CHECK (entries_reaching_nothing_else >= 0),
    referencing_functions INTEGER NOT NULL CHECK (referencing_functions >= 0),
    CHECK (entries_reaching_nothing_else <= entries),
    UNIQUE (image_id, base_rva)
);

-- Reachability as a bracket. The lower bound follows direct calls only; the
-- upper bound assumes a virtual call reaches every vtable's slot. How wide the
-- gap is measures how much of the image's control flow is decided at run time.
CREATE VIEW IF NOT EXISTS v_exe_reachability_bracket AS
SELECT COUNT(*) AS functions,
       SUM(reachable_from_entry) AS direct_calls_only,
       SUM(CASE WHEN outside_every_closure = 0 THEN 1 ELSE 0 END) AS through_any_dispatch,
       SUM(outside_every_closure) AS outside_every_closure,
       ROUND(100.0 * SUM(reachable_from_entry) / COUNT(*), 1) AS lower_bound_percent,
       ROUND(100.0 * SUM(CASE WHEN outside_every_closure = 0 THEN 1 ELSE 0 END) / COUNT(*), 1)
           AS upper_bound_percent
FROM exe_function;

-- What sits outside the bound, and whether anything else refers to it. A
-- function here is not reachable from the entry point by any route the file
-- describes, and is not exported or bound to a vtable either.
CREATE VIEW IF NOT EXISTS v_exe_unreachable_function AS
SELECT printf('0x%x', f.begin_rva) AS begin_rva,
       f.size_bytes,
       f.caller_count,
       f.export_name,
       (SELECT COUNT(*) FROM exe_vtable_binding b WHERE b.function_id = f.id) AS vtable_slots
FROM exe_function f
WHERE f.outside_every_closure = 1
ORDER BY f.size_bytes DESC;
