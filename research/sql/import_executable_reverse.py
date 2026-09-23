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


def verify_summary_identities(mapping: dict) -> None:
    """Refuses a map report whose own counters contradict each other.

    Several counters partition a total, and several are subsets of another. A
    census that does not add up to its own total has a bug in it, and the point
    of checking here is that it is checked against the real image's numbers on
    every import rather than only against a fixture. The checks that found
    something were the ones nobody had written: two counters with similar names,
    printed side by side, counting different things.
    """
    s = mapping.get("summary")
    if not s:
        return

    def has(*keys: str) -> bool:
        return all(k in s for k in keys)

    partitions = [
        ("functions", ["reachable_through_dispatch",
                       "reached_only_through_funclets_or_taken_addresses",
                       "outside_every_closure"]),
        ("vtable_slots_classified",
         ["vtable_slots_pure_virtual", "vtable_slots_empty_body", "vtable_slots_implemented"]),
        ("indexed_accesses", ["image_base_indexed_accesses", "held_base_accesses"]),
        ("held_base_accesses",
         ["consistent_array_accesses", "string_scan_accesses", "inconsistent_array_accesses",
          "accesses_on_a_conflicted_base"]),
        ("image_base_groups_spanning_elements",
         ["image_base_groups_reads_in_several_sections", "image_base_groups_undecided"]),
        ("dispatch_sites",
         ["dispatch_sites_on_this", "dispatch_sites_on_an_argument",
          "dispatch_sites_with_an_unnamed_receiver",
          "dispatch_sites_on_a_fixed_or_taken_address"]),
    ]
    for total, parts in partitions:
        if not has(total, *parts):
            continue
        summed = sum(s[p] for p in parts)
        if summed != s[total]:
            raise SystemExit(
                f"summary contradicts itself: {total}={s[total]} but "
                + " + ".join(f"{p}={s[p]}" for p in parts)
                + f" = {summed}"
            )

    subsets = [
        ("startup_path_dispatch_sites_in_tail_position", "startup_path_dispatch_sites"),
        ("startup_path_dispatch_sites", "dispatch_sites"),
        ("startup_path_functions", "functions"),
        ("dispatch_sites_resolved", "dispatch_sites_in_a_bound_function"),
        ("dispatch_sites_in_a_bound_function", "dispatch_sites_on_this"),
        ("stores_of_a_vtable", "stores_into_this"),
        ("constructors_identified", "stores_of_a_vtable"),
        ("field_offsets_confirmed_by_rtti", "field_layout_entries"),
        ("image_base_groups_spanning_elements", "image_base_groups_with_several_reads"),
        ("size_floors_above_a_vtable_pointer", "class_size_floors"),
        ("walks_complete", "functions"),
    ]
    for smaller, larger in subsets:
        if not has(smaller, larger):
            continue
        if s[smaller] > s[larger]:
            raise SystemExit(
                f"summary contradicts itself: {smaller}={s[smaller]} exceeds {larger}={s[larger]}"
            )

    # Summary against detail. No arithmetic identity catches a counter that
    # measures something other than its name — 68 of one population sits happily
    # inside 99 of another and inside the census above it. What catches it is
    # adding the per-function numbers up and comparing, which needs the report to
    # carry them.
    functions = mapping.get("functions")
    if functions and "dispatch_sites" in s:
        detail = sum(f.get("dispatch_sites", 0) for f in functions)
        if detail != s["dispatch_sites"]:
            raise SystemExit(
                f"summary says dispatch_sites={s['dispatch_sites']} but the functions it "
                f"carries add up to {detail}"
            )
    if functions and "startup_path_dispatch_sites" in s:
        detail = sum(f.get("dispatch_sites", 0) for f in functions
                     if f.get("depth_from_entry") is not None)
        if detail != s["startup_path_dispatch_sites"]:
            raise SystemExit(
                f"summary says startup_path_dispatch_sites={s['startup_path_dispatch_sites']} "
                f"but the functions on the path add up to {detail}"
            )

    # The startup path is the direct-call closure, computed twice by different
    # code. They agreeing is one checking the other.
    if has("startup_path_functions", "reachable_from_entry_point"):
        if s["startup_path_functions"] != s["reachable_from_entry_point"]:
            raise SystemExit(
                "summary contradicts itself: the depth closure reaches "
                f"{s['startup_path_functions']} functions but the reachability flag marks "
                f"{s['reachable_from_entry_point']}"
            )


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
            """INSERT INTO exe_name_table(
                   image_id, base_rva, layout, element_bytes, entries, longest_name,
                   pure_name_array, records_with_payload, text_payload_records,
                   payload_offset_consistent, content_period, sample_name,
                   absorbed_elements, interior_of_record_rva, interior_field_index,
                   overrun_elements, extent_status)
               VALUES(?,?, 'STRIDE', ?,?,?,?,?,?,?,?,?,?,?,?,?,?)
               ON CONFLICT(image_id, base_rva, layout) DO NOTHING""",
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
            """INSERT INTO exe_name_table(
                   image_id, base_rva, layout, element_bytes, entries,
                   fields_per_record, sample_name, extent_status)
               VALUES(?,?, 'RECORD', ?,?,?,?, 'STRUCTURAL_CONFIRMED')
               ON CONFLICT(image_id, base_rva, layout) DO NOTHING""",
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
            """INSERT INTO exe_name_table(
                   image_id, base_rva, layout, element_bytes, entries,
                   indexed_sites, extent_status, known_from)
               VALUES(?,?,?,?,?,?, 'EXTENT_UNCLASSIFIED', 'MAP')
               ON CONFLICT(image_id, base_rva, layout) DO NOTHING""",
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


def load_call_edges(con: sqlite3.Connection, mapping: dict, functions: dict) -> int:
    """Stores the direct call edges between functions in the inventory."""
    imported = 0
    rows = []
    for entry in mapping.get("functions", []):
        caller = functions.get(parse_rva(entry["begin_rva"]))
        if caller is None:
            continue
        for target in entry.get("calls", []):
            callee = functions.get(parse_rva(target))
            if callee is None or callee == caller:
                # A function calling itself says nothing about who reaches what,
                # and the schema promises the table holds no such row.
                continue
            rows.append((caller, callee))
            imported += 1
    con.executemany(
        "INSERT OR IGNORE INTO exe_call_edge(caller_id, callee_id) VALUES(?,?)", rows
    )
    return imported


def load_constant_arguments(con: sqlite3.Connection, image_id: int, mapping: dict,
                            functions: dict) -> int:
    """Stores which functions take a constant first argument, and which values."""
    imported = 0
    for entry in mapping.get("constant_argument_callees", []):
        rva = parse_rva(entry["callee_rva"])
        con.execute(
            """INSERT INTO exe_constant_argument_callee(
                   image_id, callee_id, callee_rva, call_sites, distinct_arguments)
               VALUES(?,?,?,?,?)
               ON CONFLICT(image_id, callee_rva) DO UPDATE SET
                   call_sites=excluded.call_sites,
                   distinct_arguments=excluded.distinct_arguments""",
            (image_id, functions.get(rva), rva, entry.get("call_sites", 0),
             entry.get("distinct_arguments", 0)),
        )
        row = con.execute(
            "SELECT id FROM exe_constant_argument_callee WHERE image_id=? AND callee_rva=?",
            (image_id, rva),
        ).fetchone()
        for argument in entry.get("arguments", []):
            con.execute(
                "INSERT OR IGNORE INTO exe_constant_argument(callee_id, argument) VALUES(?,?)",
                (int(row[0]), argument),
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


def load_global_blocks(con: sqlite3.Connection, image_id: int, mapping: dict,
                       classes: dict) -> int:
    """Stores fixed addresses the code hands to a call as its first argument."""
    imported = 0
    for entry in mapping.get("global_state_blocks", []):
        gap = entry["bytes_to_next_block"]
        reach = entry["field_reach"]
        expected = 1 if gap > 0 and reach > gap else 0
        if int(bool(entry["reach_runs_past_the_next_block"])) != expected:
            raise SystemExit(
                f"block at {entry['base_rva']} flags an overrun its own numbers do not show"
            )
        owner = entry.get("constructed_class") or None
        con.execute(
            """INSERT INTO exe_global_block(
                   image_id, base_rva, section, call_sites, distinct_callees, distinct_callers,
                   field_reach, bytes_to_next_block, reach_runs_past_the_next_block,
                   constructed_class_id)
               VALUES(?,?,?,?,?,?,?,?,?,?)
               ON CONFLICT(image_id, base_rva) DO NOTHING""",
            (
                image_id,
                parse_rva(entry["base_rva"]),
                entry.get("section") or None,
                entry["call_sites"],
                entry["distinct_callees"],
                entry["distinct_callers"],
                reach,
                gap,
                expected,
                class_id_for(con, image_id, classes, owner) if owner else None,
            ),
        )
        imported += 1
    return imported


def load_class_size_floors(con: sqlite3.Connection, image_id: int, mapping: dict,
                           classes: dict) -> int:
    """Stores a floor on each class's object size, never a size."""
    imported = 0
    for entry in mapping.get("class_size_floors", []):
        floor = entry["floor_bytes"]
        if floor < entry["floor_from_bases"] or floor < entry["floor_from_field_access"]:
            raise SystemExit(
                f"size floor for {entry['class']} is below one of its own sources"
            )
        deepest = entry.get("deepest_base")
        con.execute(
            """INSERT INTO exe_class_size_floor(
                   image_id, class_id, floor_bytes, floor_from_bases, deepest_base_id,
                   floor_from_field_access, functions_speaking, functions_reaching_half)
               VALUES(?,?,?,?,?,?,?,?)
               ON CONFLICT(image_id, class_id) DO NOTHING""",
            (
                image_id,
                class_id_for(con, image_id, classes, entry["class"]),
                floor,
                entry["floor_from_bases"],
                class_id_for(con, image_id, classes, deepest) if deepest else None,
                entry["floor_from_field_access"],
                entry["functions_speaking"],
                entry["functions_reaching_half"],
            ),
        )
        imported += 1
    return imported


def load_function_pointer_runs(con: sqlite3.Connection, image_id: int, mapping: dict) -> int:
    """Stores runs of function addresses in data that are not a located vtable."""
    imported = 0
    for entry in mapping.get("function_pointer_runs", []):
        if entry["entries_reaching_nothing_else"] > entry["entries"]:
            raise SystemExit(
                f"run at {entry['base_rva']} claims more unreachable entries than entries"
            )
        con.execute(
            # Scoped to the uniqueness conflict on purpose: `INSERT OR IGNORE`
            # would also swallow a CHECK failure, turning a malformed row into a
            # silently missing one.
            """INSERT INTO exe_function_pointer_run(
                   image_id, base_rva, entries, entries_reaching_nothing_else,
                   referencing_functions)
               VALUES(?,?,?,?,?)
               ON CONFLICT(image_id, base_rva) DO NOTHING""",
            (
                image_id,
                parse_rva(entry["base_rva"]),
                entry["entries"],
                entry["entries_reaching_nothing_else"],
                entry["referencing_functions"],
            ),
        )
        imported += 1
    return imported


def load_base_slot_overrides(con: sqlite3.Connection, image_id: int, mapping: dict,
                             classes: dict) -> int:
    """Stores, per base class and slot, what its inheritors put in that slot.

    Layout and linkage only: counts of classes and of distinct targets, plus the
    address the base itself names. No slot is given a meaning here, and no body
    is described beyond the three-way split the report already carries.
    """
    imported = 0
    for entry in mapping.get("base_slot_overrides", []):
        derived = entry["derived_classes"]
        empty = entry["empty_bodies"]
        pure = entry["pure_virtual"]
        # The schema states these as CHECKs; refusing here names the offending
        # row instead of failing on an opaque constraint.
        if empty + pure > derived or entry["distinct_implementations"] > derived:
            raise SystemExit(
                f"slot census for {entry['base_class']} slot {entry['slot']} "
                "counts more outcomes than classes"
            )
        con.execute(
            # Scoped to the uniqueness conflict, so the table's CHECKs still
            # raise rather than dropping the row.
            """INSERT INTO exe_base_slot_override(
                   image_id, base_id, slot, base_kind, base_target_rva, derived_classes,
                   keep_base_target, empty_bodies, pure_virtual, distinct_implementations)
               VALUES(?,?,?,?,?,?,?,?,?,?)
               ON CONFLICT(image_id, base_id, slot) DO NOTHING""",
            (
                image_id,
                class_id_for(con, image_id, classes, entry["base_class"]),
                entry["slot"],
                entry["base_kind"],
                parse_rva(entry["base_target_rva"]),
                derived,
                entry["keep_base_target"],
                empty,
                pure,
                entry["distinct_implementations"],
            ),
        )
        imported += 1
    return imported


def load_class_bases(con: sqlite3.Connection, image_id: int, analysis: dict,
                     classes: dict) -> int:
    """Stores the hierarchy the compiler declared, from the analysis report.

    A class the graph names as a base but that carries no vtable of its own has
    no row in `exe_class`, which is built from vtable coverage; such a base is
    created here so the hierarchy is not silently truncated.
    """
    imported = 0
    image_classes = analysis.get("rtti", {}).get("class_graph", [])

    def class_id(name: str) -> int:
        existing = classes.get(name)
        if existing is not None:
            return existing
        con.execute(
            "INSERT OR IGNORE INTO exe_class(image_id, display_name) VALUES(?,?)",
            (image_id, name),
        )
        row = con.execute(
            "SELECT id FROM exe_class WHERE image_id=? AND display_name=?", (image_id, name)
        ).fetchone()
        classes[name] = int(row[0])
        return classes[name]

    for entry in image_classes:
        name = entry["display_name"]
        for base in entry.get("bases", []):
            base_name = base["display_name"]
            if base_name == name:
                continue
            con.execute(
                """INSERT OR IGNORE INTO exe_class_base(class_id, base_id, member_displacement)
                   VALUES(?,?,?)""",
                (class_id(name), class_id(base_name), base.get("member_displacement", 0)),
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
                   reachable_from_export, outside_every_closure,
                   reached_only_through_recorded_edges, depth_from_entry, prolog_size,
                   stack_allocation, pushed_registers, frame_register, exception_handler)
               VALUES(?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?)
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
                1 if entry.get("outside_every_closure") else 0,
                1 if entry.get("reached_only_through_funclets_or_taken_addresses") else 0,
                entry.get("depth_from_entry"),
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
                """INSERT OR IGNORE INTO exe_dispatch_offset(function_id, displacement, slot)
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

    verify_summary_identities(mapping)

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
    load_class_bases(con, image_id, analysis, classes)
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
    load_call_edges(con, mapping, function_ids)
    load_constant_arguments(con, image_id, mapping, function_ids)
    load_resolved_dispatches(con, image_id, mapping, function_ids, classes)
    load_base_slot_overrides(con, image_id, mapping, classes)
    load_function_pointer_runs(con, image_id, mapping)
    load_class_size_floors(con, image_id, mapping, classes)
    load_global_blocks(con, image_id, mapping, classes)
    con.commit()

    counts = {
        name: con.execute(f"SELECT COUNT(*) FROM {name}").fetchone()[0]
        for name in (
            "exe_function",
            "exe_class",
            "exe_class_base",
            "exe_vtable_binding",
            "exe_vtable_install",
            "exe_call_edge",
            "exe_import_call",
            "exe_dispatch_offset",
            "exe_name_table",
            "exe_table_reference",
            "exe_indexed_array",
            "exe_indexed_array_field",
            "exe_resolved_dispatch",
            "exe_class_field",
            "exe_constant_argument_callee",
            "exe_constant_argument",
            "exe_base_slot_override",
            "exe_function_pointer_run",
            "exe_class_size_floor",
            "exe_global_block",
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
