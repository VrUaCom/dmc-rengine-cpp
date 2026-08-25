#!/usr/bin/env python3
from __future__ import annotations

import argparse
import hashlib
import importlib.util
import json
from pathlib import Path
import subprocess
import tempfile
import unittest
from unittest import mock

MODULE_PATH = Path(__file__).with_name("extract_exe_window_packet.py")
SPEC = importlib.util.spec_from_file_location("extract_exe_window_packet", MODULE_PATH)
assert SPEC is not None and SPEC.loader is not None
packet = importlib.util.module_from_spec(SPEC)
SPEC.loader.exec_module(packet)

SHA = "e454272ed0fb0247fcbcf300e5d55d7a3e96d50b89b9ffaff81bb978dcbdd082"


def make_plan(path: Path) -> dict[str, object]:
    data: dict[str, object] = {
        "schema": packet.PLAN_SCHEMA,
        "id": "test-l3-writer-plan",
        "artifact_sha256": SHA,
        "artifact_size": 4096,
        "authority_role": "analysis-reverse",
        "window_size_policy": "Probe coverage only; not a body-boundary assertion.",
        "windows": [
            {
                "id": "writer-probe",
                "va": "0x140001000",
                "size": "0x4",
                "mode": "probe",
                "issues": [88],
                "purpose": "Synthetic guardrail coverage only.",
            }
        ],
    }
    path.write_text(json.dumps(data, indent=2) + "\n", encoding="utf-8")
    return data


def make_receipt(*, include_hex: bool = False) -> dict[str, object]:
    raw = bytes.fromhex("90909090")
    receipt: dict[str, object] = {
        "schema": packet.CHILD_RECEIPT_SCHEMA,
        "artifact_sha256": SHA,
        "artifact_size": 4096,
        "image_base": "0x140000000",
        "va": "0x140001000",
        "rva": "0x1000",
        "file_offset": "0x200",
        "size": 4,
        "section": ".text",
        "window_sha256": hashlib.sha256(raw).hexdigest(),
    }
    if include_hex:
        receipt["bytes_hex"] = raw.hex()
    return receipt


class PacketGuardrailTests(unittest.TestCase):
    def test_plan_hash_covers_exact_plan_bytes(self) -> None:
        with tempfile.TemporaryDirectory() as temp:
            plan_path = Path(temp) / "plan.json"
            make_plan(plan_path)
            _, first_raw = packet.load_plan(plan_path)
            first_sha = hashlib.sha256(first_raw).hexdigest()

            plan_path.write_text(
                plan_path.read_text(encoding="utf-8") + " \n",
                encoding="utf-8",
            )
            _, second_raw = packet.load_plan(plan_path)
            second_sha = hashlib.sha256(second_raw).hexdigest()
            self.assertNotEqual(first_sha, second_sha)

    def test_child_receipt_requires_exact_schema(self) -> None:
        receipt = make_receipt()
        receipt["schema"] = "wrong.schema"
        with self.assertRaisesRegex(ValueError, "schema mismatch"):
            packet.validate_child_receipt(
                receipt,
                expected_sha=SHA,
                expected_artifact_size=4096,
                expected_va=0x140001000,
                expected_size=4,
                include_hex=False,
            )

    def test_child_receipt_rejects_unrequested_raw_bytes(self) -> None:
        receipt = make_receipt(include_hex=True)
        with self.assertRaisesRegex(ValueError, "unexpectedly contains raw bytes"):
            packet.validate_child_receipt(
                receipt,
                expected_sha=SHA,
                expected_artifact_size=4096,
                expected_va=0x140001000,
                expected_size=4,
                include_hex=False,
            )

    def test_child_receipt_validates_requested_raw_bytes_hash(self) -> None:
        receipt = make_receipt(include_hex=True)
        normalized = packet.validate_child_receipt(
            receipt,
            expected_sha=SHA,
            expected_artifact_size=4096,
            expected_va=0x140001000,
            expected_size=4,
            include_hex=True,
        )
        self.assertEqual(normalized["section"], ".text")

        receipt["bytes_hex"] = "91909090"
        with self.assertRaisesRegex(ValueError, "do not match window SHA"):
            packet.validate_child_receipt(
                receipt,
                expected_sha=SHA,
                expected_artifact_size=4096,
                expected_va=0x140001000,
                expected_size=4,
                include_hex=True,
            )

    def test_run_packet_binds_plan_and_child_receipt(self) -> None:
        with tempfile.TemporaryDirectory() as temp:
            root = Path(temp)
            plan_path = root / "plan.json"
            make_plan(plan_path)
            exe_path = root / "dmc3.exe"
            exe_path.write_bytes(b"placeholder")
            output = root / "packet"
            receipt = make_receipt()
            completed = subprocess.CompletedProcess(
                args=["dmc-rengine"],
                returncode=0,
                stdout=json.dumps(receipt),
                stderr="",
            )
            args = argparse.Namespace(
                plan=plan_path,
                dmc_rengine=Path("dmc-rengine"),
                exe=exe_path,
                expected_sha256=SHA,
                output=output,
                hex=False,
            )

            with mock.patch.object(packet.subprocess, "run", return_value=completed):
                self.assertEqual(packet.run_packet(args), 0)

            manifest = json.loads(
                (output / "packet.receipt.json").read_text(encoding="utf-8")
            )
            exact_plan = (output / "packet.plan.json").read_bytes()
            self.assertEqual(
                manifest["plan_sha256"], hashlib.sha256(exact_plan).hexdigest()
            )
            self.assertEqual(manifest["plan_receipt"], "packet.plan.json")
            self.assertEqual(
                manifest["windows"][0]["receipt_schema"], packet.CHILD_RECEIPT_SCHEMA
            )
            self.assertEqual(manifest["windows"][0]["section"], ".text")

    def test_invalid_child_receipt_removes_partial_packet(self) -> None:
        with tempfile.TemporaryDirectory() as temp:
            root = Path(temp)
            plan_path = root / "plan.json"
            make_plan(plan_path)
            exe_path = root / "dmc3.exe"
            exe_path.write_bytes(b"placeholder")
            output = root / "packet"
            receipt = make_receipt()
            receipt["schema"] = "wrong.schema"
            completed = subprocess.CompletedProcess(
                args=["dmc-rengine"],
                returncode=0,
                stdout=json.dumps(receipt),
                stderr="",
            )
            args = argparse.Namespace(
                plan=plan_path,
                dmc_rengine=Path("dmc-rengine"),
                exe=exe_path,
                expected_sha256=SHA,
                output=output,
                hex=False,
            )

            with mock.patch.object(packet.subprocess, "run", return_value=completed):
                self.assertEqual(packet.run_packet(args), 5)
            self.assertFalse(output.exists())


if __name__ == "__main__":
    unittest.main()
