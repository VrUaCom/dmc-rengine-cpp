#!/usr/bin/env python3
"""Guardrails for the evidence-figure checker.

The check exists because four records went stale without anyone noticing, so
what matters is that it notices: a figure the report contradicts must fail, and
a figure in a record a correction supersedes must not, because a correction's
whole point is that the old numbers no longer hold.
"""

from __future__ import annotations

import importlib.util
import json
from pathlib import Path
import tempfile
import unittest

MODULE_PATH = Path(__file__).with_name("check_evidence_figures.py")
SPEC = importlib.util.spec_from_file_location("check_evidence_figures", MODULE_PATH)
assert SPEC is not None and SPEC.loader is not None
checker = importlib.util.module_from_spec(SPEC)
SPEC.loader.exec_module(checker)


def packet(records: list[dict]) -> dict:
    return {"schema_version": 1, "id": "p", "title": "t", "project": "x",
            "artifacts": [], "records": records}


def record(rid: str, figures: dict, supersedes: list[str] | None = None) -> dict:
    return {"id": rid, "claim_id": "c", "title": "t", "summary": "s",
            "confidence": "confirmed", "tags": [], "supersedes": supersedes or [],
            "locations": [], "figures": figures}


class CheckerTests(unittest.TestCase):
    def setUp(self) -> None:
        self._dir = tempfile.TemporaryDirectory()
        self.dir = Path(self._dir.name)

    def tearDown(self) -> None:
        self._dir.cleanup()

    def write(self, records: list[dict], summary: dict) -> tuple[Path, Path]:
        (self.dir / "a.evidence.json").write_text(json.dumps(packet(records)))
        report = self.dir / "map.json"
        report.write_text(json.dumps({"summary": summary}))
        return self.dir, report

    def test_a_figure_the_report_agrees_with_passes(self) -> None:
        d, r = self.write([record("one", {"dispatch_sites": 11434})],
                          {"dispatch_sites": 11434})
        self.assertEqual(checker.check(d, r), 0)

    def test_a_figure_the_report_contradicts_fails(self) -> None:
        d, r = self.write([record("one", {"dispatch_sites": 68})],
                          {"dispatch_sites": 99})
        self.assertEqual(checker.check(d, r), 1)

    def test_a_superseded_record_is_not_checked(self) -> None:
        # The old record's figures are exactly what the correction says are
        # wrong; checking them would report the correction as the failure.
        d, r = self.write(
            [record("old", {"dispatch_sites": 68}),
             record("new", {"dispatch_sites": 99}, supersedes=["old"])],
            {"dispatch_sites": 99})
        self.assertEqual(checker.check(d, r), 0)

    def test_a_correction_in_another_packet_supersedes_too(self) -> None:
        # Corrections routinely land in a later packet than the record they
        # correct. Honouring supersession only within one packet reported the
        # correction's targets as stale the moment the counter moved.
        (self.dir / "a.evidence.json").write_text(
            json.dumps(packet([record("old", {"dispatch_sites": 68})])))
        (self.dir / "b.evidence.json").write_text(
            json.dumps(packet([record("new", {"dispatch_sites": 99}, supersedes=["old"])])))
        report = self.dir / "map.json"
        report.write_text(json.dumps({"summary": {"dispatch_sites": 99}}))
        self.assertEqual(checker.check(self.dir, report), 0)

    def test_a_figure_naming_no_counter_fails(self) -> None:
        # A binding to a counter that does not exist checks nothing, and a check
        # that silently checks nothing is worse than no check.
        d, r = self.write([record("one", {"dispatch_sights": 99})],
                          {"dispatch_sites": 99})
        self.assertEqual(checker.check(d, r), 1)

    def test_records_without_figures_are_simply_not_checked(self) -> None:
        d, r = self.write([{"id": "one", "claim_id": "c", "title": "t", "summary": "s",
                            "confidence": "confirmed", "tags": [], "supersedes": [],
                            "locations": []}],
                          {"dispatch_sites": 99})
        self.assertEqual(checker.check(d, r), 0)


if __name__ == "__main__":
    unittest.main()
