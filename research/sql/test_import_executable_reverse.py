#!/usr/bin/env python3
"""Guardrails for the executable-reverse importer.

The database is an index over the reports, so the properties worth pinning are
the ones that would let it drift from them: a reference must not be dropped, a
table must not be invented, and a corrected extent must arrive labelled as
corrected rather than as something the scan read straight off the bytes.
"""

from __future__ import annotations

import importlib.util
import json
from pathlib import Path
import sqlite3
import tempfile
import unittest

MODULE_PATH = Path(__file__).with_name("import_executable_reverse.py")
SPEC = importlib.util.spec_from_file_location("import_executable_reverse", MODULE_PATH)
assert SPEC is not None and SPEC.loader is not None
importer = importlib.util.module_from_spec(SPEC)
SPEC.loader.exec_module(importer)

SHA = "e454272ed0fb0247fcbcf300e5d55d7a3e96d50b89b9ffaff81bb978dcbdd082"
ARTIFACT = {"sha256": SHA, "size": 6356432}


def make_analysis() -> dict:
    return {
        "schema": "dmc-rengine.executable-analysis.v1",
        "artifact": dict(ARTIFACT),
        "image": {"image_base": "0x140000000", "entry_point_rva": "0x1000", "machine": "x86-64"},
        "rtti": {
            "class_graph": [
                {
                    "display_name": "CCameraRail",
                    "bases": [
                        {"display_name": "CCameraRail", "member_displacement": 0},
                        {"display_name": "IActor", "member_displacement": 0},
                    ],
                }
            ]
        },
        "name_tables": {
            "candidate_names": 3,
            "runs": 3,
            "entries_in_runs": 35,
            "trimmed_runs": 1,
            "absorbed_elements": 6,
            "record_runs": 0,
            "records": [],
            "largest": [
                {
                    "base_rva": "0x506f68",
                    "stride": 16,
                    "entries": 16,
                    "longest_name": 15,
                    "span_bytes": 256,
                    "pure_name_array": True,
                    "absorbed_elements": 6,
                    "sample_name": "FogColor",
                },
                {
                    # A record's block of equal-width fields, cut back to the
                    # block after running on into the next record.
                    "base_rva": "0x4ed6a0",
                    "stride": 40,
                    "entries": 7,
                    "longest_name": 36,
                    "span_bytes": 280,
                    "pure_name_array": True,
                    "interior_of_record_rva": "0x4eccb8",
                    "interior_field_index": 2,
                    "overrun_elements": 1,
                    "sample_name": "message\\japanese\\m01_s00_msg_jpn.txt",
                },
                {
                    "base_rva": "0x520c64",
                    "stride": 20,
                    "entries": 12,
                    "longest_name": 9,
                    "span_bytes": 240,
                    "pure_name_array": False,
                    "records_with_payload": 12,
                    "text_payload_records": 0,
                    "first_payload_offset": 12,
                    "payload_offset_consistent": True,
                    "uniform_records": True,
                    "sample_name": "rate",
                },
            ],
        },
    }


def make_map() -> dict:
    return {
        "schema": "dmc-rengine.function-map.v1",
        "artifact": dict(ARTIFACT),
        "class_coverage": [
            {
                "class": "CCameraRail",
                "vtable_slots": 4,
                "slots_bound_to_functions": 4,
                "distinct_functions": 4,
                "install_sites": 1,
            }
        ],
        "constant_argument_callees": [
            {
                "callee_rva": "0x2e7ca0",
                "call_sites": 180,
                "distinct_arguments": 7,
                "arguments": [0, 1, 2, 3, 8, 16, 64],
            }
        ],
        "class_field_layout": [
            {
                "class": "CCameraRail",
                "offset": 96,
                "member_class": "CCameraRail",
                "member_vtable_rva": "0x2180",
                "site_rva": "0x23f7f7",
                "embedded_member": False,
                "offset_confirmed_by_rtti": True,
            }
        ],
        "base_slot_overrides": [
            {
                # A slot nearly every inheritor implements for itself.
                "base_class": "IActor",
                "slot": 0,
                "base_kind": "implemented",
                "base_target_rva": "0x4c9b0",
                "derived_classes": 10,
                "keep_base_target": 0,
                "empty_bodies": 0,
                "pure_virtual": 0,
                "distinct_implementations": 10,
            },
            {
                # One the base declares and almost nobody defines.
                "base_class": "IActor",
                "slot": 1,
                "base_kind": "pure-virtual",
                "base_target_rva": "0x346bf0",
                "derived_classes": 10,
                "keep_base_target": 0,
                "empty_bodies": 8,
                "pure_virtual": 0,
                "distinct_implementations": 2,
            },
        ],
        "resolved_dispatches": [
            {
                "site_rva": "0x58cc3",
                "caller_rva": "0x2d7210",
                "displacement": 24,
                "slot": 3,
                "class": "CCameraRail",
                "receiver_field_offset": 96,
                "target_rva": "0x2d7210",
            }
        ],
        "indexed_arrays": [
            {
                "base_rva": "0x5d08a0",
                "element_bytes": 20,
                "sites": 4,
                "referencing_functions": 1,
                "field_offsets": [4, 8, 12, 16],
            },
            {
                "base_rva": "0x597150",
                "element_bytes": 8,
                "sites": 2,
                "referencing_functions": 1,
                "field_offsets": [0, 4],
            },
        ],
        "name_table_usage": [
            {
                "table_base_rva": "0x506f68",
                "record_layout": False,
                "element_bytes": 16,
                "entries": 16,
                "referencing_functions": 1,
                "base_references": 1,
                "elements_named_by_constant": 2,
            },
            # The trimmed tail, reached by code but absent from the capped
            # analysis list.
            {
                "table_base_rva": "0x507068",
                "record_layout": False,
                "element_bytes": 8,
                "entries": 11,
                "referencing_functions": 1,
                "base_references": 1,
                "elements_named_by_constant": 1,
            },
        ],
        "functions": [
            {
                "begin_rva": "0x2d7210",
                "end_rva": "0x2d7300",
                "size": 240,
                "instructions": 60,
                "walk_complete": True,
                "callers": 2,
                "callees": 3,
                "calls": ["0x2d7210"],
                "name_tables": [
                    {
                        "table_base_rva": "0x506f68",
                        "record_layout": False,
                        "element_bytes": 16,
                        "entries": 16,
                        "offset_in_table": 0,
                        "element_index": 0,
                    },
                    {
                        "table_base_rva": "0x506f68",
                        "record_layout": False,
                        "element_bytes": 16,
                        "entries": 16,
                        "offset_in_table": 16,
                        "element_index": 1,
                    },
                    {
                        "table_base_rva": "0x507068",
                        "record_layout": False,
                        "element_bytes": 8,
                        "entries": 11,
                        "offset_in_table": 8,
                        "element_index": 1,
                    },
                ],
            },
            {
                # Nothing calls it, it is in no vtable, and the loosest
                # dispatch bound does not reach it.
                "begin_rva": "0x31b2d0",
                "end_rva": "0x31e0b0",
                "size": 11744,
                "instructions": 2400,
                "walk_complete": True,
                "callers": 1,
                "callees": 0,
                "outside_every_closure": True,
            },
        ],
        "class_size_floors": [
            {
                "class": "CCameraRail",
                "floor_bytes": 4096,
                "floor_from_bases": 104,
                "deepest_base": "IActor",
                "floor_from_field_access": 4096,
                "functions_speaking": 9,
                "functions_reaching_half": 4,
            },
            {
                # Only its own vtable pointer, and nothing reaches into it.
                "class": "CQuiet",
                "floor_bytes": 8,
                "floor_from_bases": 8,
                "deepest_base": "CQuiet",
                "floor_from_field_access": 0,
                "functions_speaking": 0,
                "functions_reaching_half": 0,
            },
        ],
        "function_pointer_runs": [
            {
                "base_rva": "0x35a3f0",
                "entries": 12,
                "entries_reaching_nothing_else": 12,
                "referencing_functions": 0,
            }
        ],
        "functions_emitted": 1,
    }


def build(analysis: dict, mapping: dict, directory: Path) -> sqlite3.Connection:
    con = sqlite3.connect(directory / "test.sqlite3")
    con.executescript(importer.DEFAULT_SCHEMA.read_text(encoding="utf-8"))
    image_id = importer.load_image(con, analysis)
    tables = importer.load_name_tables(con, image_id, analysis)
    importer.load_referenced_tables(con, image_id, mapping, tables)
    importer.load_indexed_arrays(con, image_id, mapping)
    classes = importer.load_classes(con, image_id, mapping)
    importer.load_functions(con, image_id, mapping, classes, tables)
    function_ids = {
        int(row[0]): int(row[1])
        for row in con.execute(
            "SELECT begin_rva, id FROM exe_function WHERE image_id=?", (image_id,)
        )
    }
    importer.load_class_bases(con, image_id, analysis, classes)
    importer.load_class_fields(con, image_id, mapping, classes)
    importer.load_call_edges(con, mapping, function_ids)
    importer.load_constant_arguments(con, image_id, mapping, function_ids)
    importer.load_resolved_dispatches(con, image_id, mapping, function_ids, classes)
    importer.load_base_slot_overrides(con, image_id, mapping, classes)
    importer.load_function_pointer_runs(con, image_id, mapping)
    importer.load_class_size_floors(con, image_id, mapping, classes)
    con.commit()
    return con


class ImporterTests(unittest.TestCase):
    def setUp(self) -> None:
        self._directory = tempfile.TemporaryDirectory()
        self.directory = Path(self._directory.name)

    def tearDown(self) -> None:
        self._directory.cleanup()

    def test_a_trimmed_extent_is_labelled_as_corrected(self) -> None:
        con = build(make_analysis(), make_map(), self.directory)
        row = con.execute(
            "SELECT entries, absorbed_elements, extent_status, known_from"
            " FROM exe_name_table WHERE base_rva=?",
            (0x506F68,),
        ).fetchone()
        self.assertEqual(row, (16, 6, "EXTENT_TRIMMED", "ANALYSIS"))

    def test_a_payload_run_is_not_confirmed(self) -> None:
        con = build(make_analysis(), make_map(), self.directory)
        status = con.execute(
            "SELECT extent_status FROM exe_name_table WHERE base_rva=?", (0x520C64,)
        ).fetchone()[0]
        self.assertEqual(status, "SEMANTIC_CANDIDATE")

    def test_indexed_arrays_carry_their_field_layout(self) -> None:
        con = build(make_analysis(), make_map(), self.directory)
        row = con.execute(
            "SELECT element_bytes, sites, fields_observed, field_offsets"
            " FROM v_exe_array_layout WHERE base_rva='0x5d08a0'"
        ).fetchone()
        self.assertEqual(row, (20, 4, 4, "4,8,12,16"))

    def test_a_field_outside_its_element_is_refused(self) -> None:
        # A field offset at or past the element size would mean the element size
        # is wrong, so the layout contradicts itself and is not stored.
        mapping = make_map()
        mapping["indexed_arrays"][0]["field_offsets"] = [4, 8, 20]
        con = sqlite3.connect(self.directory / "bad.sqlite3")
        con.executescript(importer.DEFAULT_SCHEMA.read_text(encoding="utf-8"))
        image_id = importer.load_image(con, make_analysis())
        with self.assertRaises(SystemExit):
            importer.load_indexed_arrays(con, image_id, mapping)

    def test_class_layout_records_the_relation_and_its_confirmation(self) -> None:
        con = build(make_analysis(), make_map(), self.directory)
        row = con.execute(
            "SELECT field_offset, member_class, relation, offset_confirmed_by_rtti"
            " FROM v_exe_class_layout WHERE class_name='CCameraRail'"
        ).fetchone()
        self.assertEqual(row, (96, "CCameraRail", "BASE", 1))

    def test_a_self_call_is_not_an_edge(self) -> None:
        # The fixture's one function lists itself; an edge from a function to
        # itself says nothing about who reaches what, and the map does not
        # produce one. Whatever reaches the importer, the table refuses it.
        con = build(make_analysis(), make_map(), self.directory)
        self.assertEqual(
            con.execute("SELECT COUNT(*) FROM exe_call_edge WHERE caller_id=callee_id")
            .fetchone()[0],
            0,
        )

    def test_a_base_with_no_vtable_of_its_own_is_still_recorded(self) -> None:
        # IActor carries no vtable in the map's class coverage, so it has no row
        # until the hierarchy needs one. Creating it there is what keeps the
        # hierarchy from being silently truncated at exactly the interfaces that
        # make it worth having.
        con = build(make_analysis(), make_map(), self.directory)
        row = con.execute(
            "SELECT base_name, implementors, named_as_interface FROM v_exe_base_reach"
        ).fetchone()
        self.assertEqual(row, ("IActor", 1, 1))

    def test_constant_arguments_are_listed_without_interpretation(self) -> None:
        con = build(make_analysis(), make_map(), self.directory)
        row = con.execute(
            "SELECT call_sites, distinct_arguments, arguments FROM v_exe_constant_argument"
        ).fetchone()
        self.assertEqual(row, (180, 7, "0,1,2,3,8,16,64"))

    def test_a_resolved_dispatch_becomes_a_virtual_call_edge(self) -> None:
        con = build(make_analysis(), make_map(), self.directory)
        row = con.execute(
            "SELECT caller_rva, class_name, receiver_field_offset, slot, target_rva"
            " FROM v_exe_virtual_call_edge"
        ).fetchone()
        self.assertEqual(row, ("0x2d7210", "CCameraRail", 96, 3, "0x2d7210"))

    def test_a_record_interior_is_marked_as_one(self) -> None:
        con = build(make_analysis(), make_map(), self.directory)
        row = con.execute(
            "SELECT entries, interior_of_record_rva, interior_field_index,"
            " overrun_elements, extent_status FROM exe_name_table WHERE base_rva=?",
            (0x4ED6A0,),
        ).fetchone()
        # Interior wins over the pure-array reading: what the run *is* outranks
        # how its extent was arrived at.
        self.assertEqual(row, (7, 0x4ECCB8, 2, 1, "RECORD_INTERIOR"))

    def test_independent_tables_exclude_record_interiors(self) -> None:
        con = build(make_analysis(), make_map(), self.directory)
        bases = {
            row[0]
            for row in con.execute("SELECT base_rva FROM v_exe_independent_table")
        }
        self.assertNotIn("0x4ed6a0", bases)
        self.assertIn("0x506f68", bases)

    def test_a_run_known_only_to_the_map_is_kept_and_marked(self) -> None:
        con = build(make_analysis(), make_map(), self.directory)
        row = con.execute(
            "SELECT element_bytes, entries, extent_status, known_from, pure_name_array"
            " FROM exe_name_table WHERE base_rva=?",
            (0x507068,),
        ).fetchone()
        # Layout from the map, payload measurements left unknown rather than
        # guessed at.
        self.assertEqual(row, (8, 11, "EXTENT_UNCLASSIFIED", "MAP", None))

    def test_no_reference_is_dropped(self) -> None:
        mapping = make_map()
        expected = sum(len(f.get("name_tables", [])) for f in mapping["functions"])
        con = build(make_analysis(), mapping, self.directory)
        stored = con.execute("SELECT COUNT(*) FROM exe_table_reference").fetchone()[0]
        self.assertEqual(stored, expected)

    def test_a_reference_to_an_undescribed_run_is_not_invented(self) -> None:
        mapping = make_map()
        mapping["name_table_usage"] = []
        con = build(make_analysis(), mapping, self.directory)
        # 0x507068 is described by neither report now, so its reference goes and
        # no row is conjured for it.
        self.assertEqual(
            con.execute("SELECT COUNT(*) FROM exe_name_table WHERE base_rva=?",
                        (0x507068,)).fetchone()[0],
            0,
        )
        self.assertEqual(
            con.execute("SELECT COUNT(*) FROM exe_table_reference").fetchone()[0], 2
        )

    def test_coverage_view_reports_the_corrected_extent(self) -> None:
        con = build(make_analysis(), make_map(), self.directory)
        row = con.execute(
            "SELECT entries, elements_named, interior_references, absorbed_elements"
            " FROM v_exe_table_coverage WHERE base_rva='0x506f68'"
        ).fetchone()
        self.assertEqual(row, (16, 2, 0, 6))

    def test_the_override_ratio_is_computed_from_the_counts(self) -> None:
        con = build(make_analysis(), make_map(), self.directory)
        rows = con.execute(
            "SELECT slot, base_kind, derived_classes, distinct_implementations,"
            " override_ratio, implementing_classes"
            " FROM v_exe_slot_override WHERE base_name='IActor' ORDER BY slot"
        ).fetchall()
        self.assertEqual(rows[0], (0, "implemented", 10, 10, 1.0, 10))
        # Eight of the ten leave the slot empty, so two carry the two bodies.
        self.assertEqual(rows[1], (1, "pure-virtual", 10, 2, 0.2, 2))

    def test_a_slot_counting_more_outcomes_than_classes_is_refused(self) -> None:
        # A class puts exactly one thing in a slot, so the outcomes cannot
        # outnumber the classes. A row that says otherwise is a counting error
        # upstream, and storing it would launder the error into a ratio.
        mapping = make_map()
        mapping["base_slot_overrides"][1]["empty_bodies"] = 11
        con = sqlite3.connect(self.directory / "overcount.sqlite3")
        con.executescript(importer.DEFAULT_SCHEMA.read_text(encoding="utf-8"))
        image_id = importer.load_image(con, make_analysis())
        with self.assertRaises(SystemExit):
            importer.load_base_slot_overrides(con, image_id, mapping, {})

    def test_the_interface_shape_view_separates_declared_from_defaulted(self) -> None:
        con = build(make_analysis(), make_map(), self.directory)
        row = con.execute(
            "SELECT inheritors, slots, slots_declared_pure, slots_defaulted_empty,"
            " slots_with_a_body FROM v_exe_base_interface_shape WHERE base_name='IActor'"
        ).fetchone()
        self.assertEqual(row, (10, 2, 1, 0, 1))

    def test_a_base_named_only_by_the_slot_census_is_still_a_class(self) -> None:
        # The census can name a base the map's own class list never mentions.
        # It becomes a class row rather than being dropped, which is what keeps
        # the ratio joinable to the rest of the schema.
        mapping = make_map()
        mapping["base_slot_overrides"][0]["base_class"] = "CUnlisted"
        con = build(make_analysis(), mapping, self.directory)
        self.assertEqual(
            con.execute(
                "SELECT COUNT(*) FROM v_exe_slot_override WHERE base_name='CUnlisted'"
            ).fetchone()[0],
            1,
        )

    def test_reachability_is_reported_as_a_bracket(self) -> None:
        con = build(make_analysis(), make_map(), self.directory)
        row = con.execute(
            "SELECT functions, direct_calls_only, through_any_dispatch, outside_every_closure"
            " FROM v_exe_reachability_bracket"
        ).fetchone()
        # Two functions, neither directly reachable from the entry point in the
        # fixture; one of them the dispatch bound does not reach either.
        self.assertEqual(row, (2, 0, 1, 1))

    def test_only_flagged_functions_are_listed_as_unreachable(self) -> None:
        con = build(make_analysis(), make_map(), self.directory)
        rows = con.execute(
            "SELECT begin_rva, size_bytes, vtable_slots FROM v_exe_unreachable_function"
        ).fetchall()
        self.assertEqual(rows, [("0x31b2d0", 11744, 0)])

    def test_a_run_claiming_more_unreachable_entries_than_entries_is_refused(self) -> None:
        # entries_reaching_nothing_else counts a subset of the run, so it can
        # never exceed it. A report that says otherwise miscounted, and storing
        # it would make the unreachable figure look better than it is.
        mapping = make_map()
        mapping["function_pointer_runs"][0]["entries_reaching_nothing_else"] = 13
        con = sqlite3.connect(self.directory / "run.sqlite3")
        con.executescript(importer.DEFAULT_SCHEMA.read_text(encoding="utf-8"))
        image_id = importer.load_image(con, make_analysis())
        with self.assertRaises(SystemExit):
            importer.load_function_pointer_runs(con, image_id, mapping)

    def test_a_run_shorter_than_three_is_refused_by_the_schema(self) -> None:
        # Two addresses side by side are as likely to be unrelated pointers as
        # a table, which is why the scan has a minimum; the schema states it.
        mapping = make_map()
        mapping["function_pointer_runs"][0]["entries"] = 2
        mapping["function_pointer_runs"][0]["entries_reaching_nothing_else"] = 2
        con = sqlite3.connect(self.directory / "short.sqlite3")
        con.executescript(importer.DEFAULT_SCHEMA.read_text(encoding="utf-8"))
        image_id = importer.load_image(con, make_analysis())
        with self.assertRaises(sqlite3.IntegrityError):
            importer.load_function_pointer_runs(con, image_id, mapping)

    def test_a_size_floor_carries_how_well_supported_it_is(self) -> None:
        con = build(make_analysis(), make_map(), self.directory)
        rows = con.execute(
            "SELECT class_name, floor_bytes, deepest_base, functions_reaching_half, support"
            " FROM v_exe_class_size ORDER BY floor_bytes DESC"
        ).fetchall()
        self.assertEqual(rows[0], ("CCameraRail", 4096, "IActor", 4, "CORROBORATED"))
        # Nothing reaches into it, so only the type information speaks.
        self.assertEqual(rows[1], ("CQuiet", 8, "CQuiet", 0, "BASES_ONLY"))

    def test_a_floor_below_its_own_sources_is_refused(self) -> None:
        # The floor is the larger of the two sources by construction. One below
        # either of them did not come from this measurement, and storing it
        # would understate a class's size — the one direction a floor must
        # never be wrong in.
        mapping = make_map()
        mapping["class_size_floors"][0]["floor_bytes"] = 100
        con = sqlite3.connect(self.directory / "floor.sqlite3")
        con.executescript(importer.DEFAULT_SCHEMA.read_text(encoding="utf-8"))
        image_id = importer.load_image(con, make_analysis())
        with self.assertRaises(SystemExit):
            importer.load_class_size_floors(con, image_id, mapping, {})

    def test_the_importer_refuses_mismatched_reports_end_to_end(self) -> None:
        import subprocess
        import sys

        analysis_path = self.directory / "analysis.json"
        map_path = self.directory / "map.json"
        analysis_path.write_text(json.dumps(make_analysis()), encoding="utf-8")
        mapping = make_map()
        mapping["artifact"] = {"sha256": "0" * 64, "size": 1}
        map_path.write_text(json.dumps(mapping), encoding="utf-8")

        result = subprocess.run(
            [
                sys.executable,
                str(MODULE_PATH),
                "--analysis",
                str(analysis_path),
                "--map",
                str(map_path),
                "--db",
                str(self.directory / "out.sqlite3"),
            ],
            capture_output=True,
            text=True,
            check=False,
        )
        self.assertNotEqual(result.returncode, 0)
        self.assertIn("different artifacts", result.stderr)


if __name__ == "__main__":
    unittest.main()
