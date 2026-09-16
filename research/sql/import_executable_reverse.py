#!/usr/bin/env python3
"""Import the deterministic executable-reverse reports into the research SQLite.

Reads the JSON produced by:

    dmc-rengine analyze-exe   <exe> --out analysis.json
    dmc-rengine map-functions <exe> --out map.json

and indexes functions, classes, vtable bindings and installs, import calls,
dispatch offsets, recovered name tables and the references from code into them.

The database is an index, not authority. Every row here is regenerable from the
reports, and the reports are regenerable from the artifact; nothing is promoted
to a semantic name by being stored.
"""

from __future__ import annotations

import argparse
import json
import sqlite3
from pathlib import Path

ROOT = Path(__file__).resolve().parents[2]
DEFAULT_SCHEMA = ROOT / "research" / "sql" / "007_executable_reverse.sql"
DEFAULT_DB = ROOT / "research-private" / "dmc_rengine_research.sqlite3"


def parse_rva(value) -> int:
    """Report RVAs are hex strings; counts are plain integers."""
    if isinstance(value, int):
        return value
    return int(value, 16)


def load_image(con: sqlite3.Connection, analysis: dict) -> int:
    artifact = analysis["artifact"]
    image = analysis.get("image", {})
    con.execute(
        """INSERT INTO exe_image(sha256, size_bytes, image_base, entry_point_rva, machine)
           VALUES(?,?,?,?,?)
           ON CONFLICT(sha256) DO UPDATE SET size_bytes=excluded.size_bytes""",
        (
            artifact["sha256"],
            artifact["size"],
            parse_rva(image.get("image_base", 0)),
            parse_rva(image.get("entry_point_rva", 0)),
            image.get("machine"),
        ),
    )
    row = con.execute("SELECT id FROM exe_image WHERE sha256=?", (artifact["sha256"],)).fetchone()
    return int(row[0])


def load_name_tables(con: sqlite3.Connection, image_id: int, analysis: dict) -> dict:
    """Returns a base RVA -> table id map for both layouts."""
    tables = analysis.get("name_tables", {})
    ids: dict[tuple[int, str], int] = {}

    for run in tables.get("largest", []):
        base = parse_rva(run["base_rva"])
        interior = run.get("interior_of_record_rva")
        interior = parse_rva(interior) if interior is not None else None
        con.execute(
            """INSERT OR IGNORE INTO exe_name_table(
                   image_id, base_rva, layout, element_bytes, entries, longest_name,
                   pure_name_array, records_with_payload, text_payload_records,
                   payload_offset_consistent, content_period, sample_name,
                   absorbed_elements, interior_of_record_rva, interior_field_index,
                   overrun_elements, extent_status)
               VALUES(?,?, 'STRIDE', ?,?,?,?,?,?,?,?,?,?,?,?,?,?)""",
            (
                image_id,
                base,
                run["stride"],
                run["entries"],
                run.get("longest_name"),
                1 if run.get("pure_name_array") else 0,
                run.get("records_with_payload", 0),
                run.get("text_payload_records", 0),
                1 if run.get("payload_offset_consistent", True) else 0,
                run.get("content_period"),
                run.get("sample_name"),
                run.get("absorbed_elements", 0),
                interior,
                run.get("interior_field_index") if interior is not None else None,
                run.get("overrun_elements", 0),
                # An interior says what the run *is*, so it wins over how its
                # extent was arrived at. A trimmed run keeps that provenance even
                # though trimming usually leaves a pure array behind: its extent
                # was corrected, not merely read.
                "RECORD_INTERIOR"
                if interior is not None
                else (
                    "EXTENT_TRIMMED"
                    if run.get("absorbed_elements", 0)
                    else (
                        "STRUCTURAL_CONFIRMED"
                        if run.get("pure_name_array")
                        else "SEMANTIC_CANDIDATE"
                    )
                ),
            ),
        )
        row = con.execute(
            "SELECT id FROM exe_name_table WHERE image_id=? AND base_rva=? AND layout='STRIDE'",
            (image_id, base),
        ).fetchone()
        ids[(base, "STRIDE")] = int(row[0])

    for run in tables.get("records", []):
        base = parse_rva(run["base_rva"])
        con.execute(
            """INSERT OR IGNORE INTO exe_name_table(
                   image_id, base_rva, layout, element_bytes, entries,
                   fields_per_record, sample_name, extent_status)
               VALUES(?,?, 'RECORD', ?,?,?,?, 'STRUCTURAL_CONFIRMED')""",
            (
                image_id,
                base,
                run["record_bytes"],
                run["records"],
                run["fields_per_record"],
                run.get("sample_name"),
            ),
        )
        row = con.execute(
            "SELECT id FROM exe_name_table WHERE image_id=? AND base_rva=? AND layout='RECORD'",
            (image_id, base),
        ).fetchone()
        table_id = int(row[0])
        ids[(base, "RECORD")] = table_id

        for index, field in enumerate(run.get("fields", [])):
            con.execute(
                """INSERT OR IGNORE INTO exe_name_table_field(
                       table_id, field_index, field_offset, field_width, extension)
                   VALUES(?,?,?,?,?)""",
                (table_id, index, field["offset"], field["width"], field.get("extension") or None),
            )

    return ids


def load_referenced_tables(con: sqlite3.Connection, image_id: int, mapping: dict,
                           ids: dict) -> None:
    """Adds runs the function map reaches but the analysis report does not list.

    The analysis report caps its run list, so a reference from code often names
    a run that is absent from it. Both reports project the same scan, and the
    map carries each reached run's layout, so the row can be created from the
    map with its payload measurements left unknown rather than the reference
    being dropped.
    """
    for entry in mapping.get("name_table_usage", []):
        base = parse_rva(entry["table_base_rva"])
        layout = "RECORD" if entry.get("record_layout") else "STRIDE"
        if (base, layout) in ids:
            continue

        con.execute(
            """INSERT OR IGNORE INTO exe_name_table(
                   image_id, base_rva, layout, element_bytes, entries,
                   indexed_sites, extent_status, known_from)
               VALUES(?,?,?,?,?,?, 'EXTENT_UNCLASSIFIED', 'MAP')""",
            (image_id, base, layout, entry["element_bytes"], entry["entries"],
             entry.get("indexed_sites", 0)),
        )
        row = con.execute(
            "SELECT id FROM exe_name_table WHERE image_id=? AND base_rva=? AND layout=?",
            (image_id, base, layout),
        ).fetchone()
        ids[(base, layout)] = int(row[0])


def apply_indexed_sites(con: sqlite3.Connection, image_id: int, mapping: dict) -> None:
    """Records, for every run the map describes, how often code indexes it.

    Kept separate from run creation because a run listed by the analysis report
    is created before the map is read, and this is a map fact.
    """
    for entry in mapping.get("name_table_usage", []):
        sites = entry.get("indexed_sites", 0)
        if not sites:
            continue
        con.execute(
            "UPDATE exe_name_table SET indexed_sites=? WHERE image_id=? AND base_rva=? AND layout=?",
            (sites, image_id, parse_rva(entry["table_base_rva"]),
             "RECORD" if entry.get("record_layout") else "STRIDE"),
        )


def load_indexed_arrays(con: sqlite3.Connection, image_id: int, mapping: dict) -> int:
    """Stores the arrays the code walks with a scaled index, and their fields."""
    imported = 0
    for entry in mapping.get("indexed_arrays", []):
        base = parse_rva(entry["base_rva"])
        con.execute(
            """INSERT INTO exe_indexed_array(
                   image_id, base_rva, element_bytes, sites, referencing_functions,
                   base_measured)
               VALUES(?,?,?,?,?,?)
               ON CONFLICT(image_id, base_rva, element_bytes) DO UPDATE SET
                   sites=excluded.sites,
                   referencing_functions=excluded.referencing_functions,
                   base_measured=excluded.base_measured""",
            (image_id, base, entry["element_bytes"], entry.get("sites", 0),
             entry.get("referencing_functions", 0),
             1 if entry.get("base_measured", True) else 0),
        )
        row = con.execute(
            "SELECT id FROM exe_indexed_array"
            " WHERE image_id=? AND base_rva=? AND element_bytes=?",
            (image_id, base, entry["element_bytes"]),
        ).fetchone()
        array_id = int(row[0])
        for offset in entry.get("field_offsets", []):
            # A field offset outside the element would mean the element size is
            # wrong, and the reports already drop those; refuse one here too
            # rather than store a layout that contradicts itself.
            if offset >= entry["element_bytes"]:
                raise SystemExit(
                    f"field offset {offset} is outside the {entry['element_bytes']}-byte"
                    f" element of the array at {entry['base_rva']}"
                )
            con.execute(
                "INSERT OR IGNORE INTO exe_indexed_array_field(array_id, field_offset)"
                " VALUES(?,?)",
                (array_id, offset),
            )
        imported += 1
    return imported


def load_class_fields(con: sqlite3.Connection, image_id: int, mapping: dict,
                      classes: dict) -> int:
    """Stores the class layout read out of constructor stores."""
    imported = 0
    for entry in mapping.get("class_field_layout", []):
        con.execute(
            """INSERT OR IGNORE INTO exe_class_field(
                   image_id, class_id, member_class_id, field_offset, member_vtable_rva,
                   site_rva, embedded_member, offset_confirmed_by_rtti)
               VALUES(?,?,?,?,?,?,?,?)""",
            (
                image_id,
                classes.get(entry["class"]),
                classes.get(entry["member_class"]),
                entry["offset"],
                parse_rva(entry.get("member_vtable_rva", 0)),
                parse_rva(entry["site_rva"]),
                1 if entry.get("embedded_member") else 0,
                1 if entry.get("offset_confirmed_by_rtti") else 0,
            ),
        )
        imported += 1
    return imported


def load_resolved_dispatches(con: sqlite3.Connection, image_id: int, mapping: dict,
                             functions: dict, classes: dict) -> int:
    """Stores virtual calls resolved to a class, a slot and a target."""
    imported = 0
    for entry in mapping.get("resolved_dispatches", []):
        con.execute(
            """INSERT OR IGNORE INTO exe_resolved_dispatch(
                   image_id, site_rva, caller_id, target_id, class_id, displacement, slot,
                   receiver_field_offset)
               VALUES(?,?,?,?,?,?,?,?)""",
            (
                image_id,
                parse_rva(entry["site_rva"]),
                functions.get(parse_rva(entry["caller_rva"])),
                functions.get(parse_rva(entry["target_rva"])),
                classes.get(entry["class"]),
                entry["displacement"],
                entry["slot"],
                entry.get("receiver_field_offset", 0),
            ),
        )
        imported += 1
    return imported


def load_classes(con: sqlite3.Connection, image_id: int, mapping: dict) -> dict:
    ids: dict[str, int] = {}
    for entry in mapping.get("class_coverage", []):
        name = entry["class"]
        con.execute(
            """INSERT INTO exe_class(image_id, display_name, vtable_slots,
                                     slots_bound_to_functions, distinct_functions, install_sites)
               VALUES(?,?,?,?,?,?)
               ON CONFLICT(image_id, display_name) DO UPDATE SET
                   vtable_slots=excluded.vtable_slots,
                   slots_bound_to_functions=excluded.slots_bound_to_functions,
                   distinct_functions=excluded.distinct_functions,
                   install_sites=excluded.install_sites""",
            (
                image_id,
                name,
                entry.get("vtable_slots", 0),
                entry.get("slots_bound_to_functions", 0),
                entry.get("distinct_functions", 0),
                entry.get("install_sites", 0),
            ),
        )
        row = con.execute(
            "SELECT id FROM exe_class WHERE image_id=? AND display_name=?", (image_id, name)
        ).fetchone()
        ids[name] = int(row[0])
    return ids


def class_id_for(con: sqlite3.Connection, image_id: int, classes: dict, name: str) -> int:
    if name in classes:
        return classes[name]
    con.execute(
        "INSERT OR IGNORE INTO exe_class(image_id, display_name) VALUES(?,?)", (image_id, name)
    )
    row = con.execute(
        "SELECT id FROM exe_class WHERE image_id=? AND display_name=?", (image_id, name)
    ).fetchone()
    classes[name] = int(row[0])
    return classes[name]


def load_functions(
    con: sqlite3.Connection, image_id: int, mapping: dict, classes: dict, tables: dict
) -> int:
    imported = 0
    for entry in mapping.get("functions", []):
        begin = parse_rva(entry["begin_rva"])
        frame = entry.get("frame", {})
        con.execute(
            """INSERT INTO exe_function(
                   image_id, begin_rva, end_rva, size_bytes, instruction_count, walk_complete,
                   caller_count, callee_count, export_name, reachable_from_entry,
                   reachable_from_export, prolog_size, stack_allocation, pushed_registers,
                   frame_register, exception_handler)
               VALUES(?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?)
               ON CONFLICT(image_id, begin_rva) DO UPDATE SET
                   caller_count=excluded.caller_count,
                   callee_count=excluded.callee_count""",
            (
                image_id,
                begin,
                parse_rva(entry["end_rva"]),
                entry.get("size", 0),
                entry.get("instructions", 0),
                0 if entry.get("walk_complete") is False else 1,
                entry.get("callers", 0),
                entry.get("callees", 0),
                entry.get("export_name"),
                1 if entry.get("reachable_from_entry_point") else 0,
                1 if entry.get("reachable_from_export") else 0,
                frame.get("prolog_size"),
                frame.get("stack_allocation"),
                frame.get("pushed_registers"),
                frame.get("frame_register"),
                1 if frame.get("exception_handler") else 0,
            ),
        )
        row = con.execute(
            "SELECT id FROM exe_function WHERE image_id=? AND begin_rva=?", (image_id, begin)
        ).fetchone()
        function_id = int(row[0])
        imported += 1

        for binding in entry.get("virtual_bindings", []):
            con.execute(
                """INSERT OR IGNORE INTO exe_vtable_binding(
                       function_id, class_id, vtable_index, slot, subobject_offset)
                   VALUES(?,?,?,?,?)""",
                (
                    function_id,
                    class_id_for(con, image_id, classes, binding["class"]),
                    binding["vtable_index"],
                    binding["slot"],
                    binding.get("subobject_offset", 0),
                ),
            )

        for install in entry.get("installs_vtables", []):
            con.execute(
                """INSERT OR IGNORE INTO exe_vtable_install(
                       function_id, class_id, vtable_index, vtable_rva)
                   VALUES(?,?,?,?)""",
                (
                    function_id,
                    class_id_for(con, image_id, classes, install["class"]),
                    install["vtable_index"],
                    parse_rva(install["vtable_rva"]),
                ),
            )

        for call in entry.get("imports_called", []):
            con.execute(
                "INSERT OR IGNORE INTO exe_import_call(function_id, module, symbol) VALUES(?,?,?)",
                (function_id, call["module"], call["function"]),
            )

        for displacement in entry.get("dispatch_displacements", []):
            con.execute(
                """INSERT OR IGNORE INTO exe_dispatch_site(function_id, displacement, slot)
                   VALUES(?,?,?)""",
                (function_id, displacement, displacement // 8),
            )

        for reference in entry.get("name_tables", []):
            base = parse_rva(reference["table_base_rva"])
            layout = "RECORD" if reference.get("record_layout") else "STRIDE"
            table_id = tables.get((base, layout))
            if table_id is None:
                # Every run a reference can name is registered before this
                # point, from one report or the other. A miss here would be a
                # reference to a run neither report describes, which is not a
                # row this importer is entitled to invent.
                continue
            con.execute(
                """INSERT OR IGNORE INTO exe_table_reference(
                       function_id, table_id, offset_in_table, constant_index,
                       element_index, field_index, offset_in_element)
                   VALUES(?,?,?,?,?,?,?)""",
                (
                    function_id,
                    table_id,
                    reference.get("offset_in_table", 0),
                    0 if reference.get("constant_index") is False else 1,
                    reference.get("element_index"),
                    reference.get("field_index"),
                    reference.get("offset_in_element", 0),
                ),
            )

    return imported


def main() -> int:
    ap = argparse.ArgumentParser()
    ap.add_argument("--analysis", type=Path, required=True, help="analyze-exe report JSON")
    ap.add_argument("--map", type=Path, required=True, help="map-functions report JSON")
    ap.add_argument("--db", type=Path, default=DEFAULT_DB)
    ap.add_argument("--schema", type=Path, default=DEFAULT_SCHEMA)
    args = ap.parse_args()

    analysis = json.loads(args.analysis.read_text(encoding="utf-8"))
    mapping = json.loads(args.map.read_text(encoding="utf-8"))

    if analysis["artifact"]["sha256"] != mapping["artifact"]["sha256"]:
        raise SystemExit("analysis and map reports describe different artifacts")

    args.db.parent.mkdir(parents=True, exist_ok=True)
    con = sqlite3.connect(args.db)
    con.executescript(args.schema.read_text(encoding="utf-8"))

    image_id = load_image(con, analysis)
    tables = load_name_tables(con, image_id, analysis)
    load_referenced_tables(con, image_id, mapping, tables)
    apply_indexed_sites(con, image_id, mapping)
    load_indexed_arrays(con, image_id, mapping)
    classes = load_classes(con, image_id, mapping)
    functions = load_functions(con, image_id, mapping, classes, tables)

    # Dispatches name functions by address, so they are stored once every
    # function exists.
    function_ids = {
        int(row[0]): int(row[1])
        for row in con.execute(
            "SELECT begin_rva, id FROM exe_function WHERE image_id=?", (image_id,)
        )
    }
    load_class_fields(con, image_id, mapping, classes)
    load_resolved_dispatches(con, image_id, mapping, function_ids, classes)
    con.commit()

    counts = {
        name: con.execute(f"SELECT COUNT(*) FROM {name}").fetchone()[0]
        for name in (
            "exe_function",
            "exe_class",
            "exe_vtable_binding",
            "exe_vtable_install",
            "exe_import_call",
            "exe_dispatch_site",
            "exe_name_table",
            "exe_table_reference",
            "exe_indexed_array",
            "exe_indexed_array_field",
            "exe_resolved_dispatch",
            "exe_class_field",
        )
    }
    con.close()

    print(f"database: {args.db}")
    print(f"functions imported: {functions}")
    for name, count in counts.items():
        print(f"  {name:<22} {count}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
