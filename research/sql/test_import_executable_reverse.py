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
        "name_tables": {
            "candidate_names": 3,
            "runs": 2,
            "entries_in_runs": 28,
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
        "class_coverage": [],
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
    classes = importer.load_classes(con, image_id, mapping)
    importer.load_functions(con, image_id, mapping, classes, tables)
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
        expected = sum(len(f["name_tables"]) for f in mapping["functions"])
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
