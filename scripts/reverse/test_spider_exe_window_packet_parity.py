#!/usr/bin/env python3
"""Compare the real Python and native metadata packet paths on a synthetic PE.

No game bytes or executable authority claims are used. Receipt formatting may
differ; each recorded digest must match its own exact bytes before semantic
comparison. This test intentionally keeps Python as the migration oracle.
"""

from __future__ import annotations

import argparse
import copy
import hashlib
import json
from pathlib import Path
import struct
import subprocess
import sys
import tempfile
import unittest

sys.dont_write_bytecode = True
import extract_exe_window_packet as legacy

ROOT = Path(__file__).resolve().parents[2]
CLI: Path


def synthetic_pe() -> bytes:
    data = bytearray(0x600)
    data[:2] = b"MZ"
    struct.pack_into("<I", data, 0x3C, 0x80)
    data[0x80:0x84] = b"PE\0\0"
    struct.pack_into("<HH", data, 0x84, 0x8664, 1)
    struct.pack_into("<H", data, 0x94, 0xF0)
    optional = 0x98
    struct.pack_into("<H", data, optional, 0x20B)
    struct.pack_into("<I", data, optional + 16, 0x1000)
    struct.pack_into("<Q", data, optional + 24, 0x140000000)
    struct.pack_into("<II", data, optional + 56, 0x2000, 0x200)
    section = optional + 0xF0
    data[section:section + 5] = b".text"
    struct.pack_into("<IIII", data, section + 8, 0x400, 0x1000, 0x400, 0x200)
    for i in range(0x200, len(data)):
        data[i] = (i * 7 + 3) & 255
    return bytes(data)


class PacketParity(unittest.TestCase):
    def setUp(self) -> None:
        self.temp = tempfile.TemporaryDirectory(prefix="spider-parity-")
        self.addCleanup(self.temp.cleanup)
        self.root = Path(self.temp.name)
        self.exe = self.root / "synthetic.exe"
        self.data = synthetic_pe()
        self.exe.write_bytes(self.data)
        self.sha = hashlib.sha256(self.data).hexdigest()
        self.plan = {
            "schema": legacy.PLAN_SCHEMA,
            "id": 'synthetic "packet"\nline',
            "artifact_sha256": self.sha.upper(),
            "artifact_size": len(self.data),
            "authority_role": "synthetic-test-only",
            "window_size_policy": "test coverage only",
            "windows": [
                {"id": "probe", "va": "0x140001000", "size": 16,
                 "mode": "probe", "issues": [88], "purpose": "probe\tmetadata"},
                {"id": "known", "va": "0x140001020", "size": "0x10",
                 "mode": "known-body", "issues": [], "purpose": "synthetic known body",
                 "body_sha256": hashlib.sha256(self.data[0x220:0x230]).hexdigest()},
            ],
        }
        self.plan_path = self.root / "plan.json"
        self.write_plan()

    def write_plan(self) -> None:
        # Include CRLF/trailing whitespace to exercise byte-authoritative hashing.
        self.plan_path.write_bytes(
            (json.dumps(self.plan, indent=2).replace("\n", "\r\n") + "\r\n \t").encode())

    def run_path(self, native: bool, output: Path | None = None,
                 *, expected: str | None = None, extra: tuple[str, ...] = ()):
        command = ([str(CLI), "extract-exe-window-packet"] if native else
                   [sys.executable, str(ROOT / "scripts/reverse/extract_exe_window_packet.py")])
        command += ["--plan", str(self.plan_path)]
        if output is None:
            command += ["--validate-plan-only"]
        else:
            command += ["--exe", str(self.exe), "--expected-sha256", expected or self.sha,
                        "--output", str(output)]
            if not native:
                command += ["--dmc-rengine", str(CLI)]
        return subprocess.run(command + list(extra), capture_output=True, text=True,
                              encoding="utf-8", timeout=30, check=False)

    def assert_ok(self, result) -> None:
        self.assertEqual(result.returncode, 0, result.stdout + result.stderr)

    def checked_packet(self, output: Path):
        raw = self.plan_path.read_bytes()
        self.assertEqual((output / "packet.plan.json").read_bytes(), raw)
        manifest = json.loads((output / "packet.receipt.json").read_bytes())
        self.assertEqual(manifest["plan_sha256"], hashlib.sha256(raw).hexdigest())
        self.assertFalse(manifest["raw_bytes_included"])
        self.assertFalse(manifest["semantic_claim"])
        children = []
        for window in manifest["windows"]:
            receipt_bytes = (output / window["receipt"]).read_bytes()
            self.assertEqual(window.pop("receipt_sha256"), hashlib.sha256(receipt_bytes).hexdigest())
            child = json.loads(receipt_bytes)
            legacy.validate_child_receipt(
                child, expected_sha=self.sha, expected_artifact_size=len(self.data),
                expected_va=int(window["va"], 0), expected_size=window["size"], include_hex=False)
            children.append(child)
        self.assertEqual(len(list(output.iterdir())), 2 + len(children))
        return manifest, children

    def test_metadata_packet_and_exact_plan_parity(self) -> None:
        packets = []
        for native in (False, True):
            output = self.root / str(native)
            self.assert_ok(self.run_path(native, output, expected=self.sha.upper()))
            packets.append(self.checked_packet(output))
        self.assertEqual(*packets)

    def test_validation_summary_parity(self) -> None:
        for path in [self.plan_path,
                     ROOT / "data/reverse/dmc3-gdspaces-blocked-window-plan.v1.json",
                     ROOT / "data/reverse/dmc3-materialization-completion-boundary-plan.v1.json"]:
            self.plan_path = path
            results = [self.run_path(native) for native in (False, True)]
            for result in results:
                self.assert_ok(result)
            self.assertEqual(json.loads(results[0].stdout), json.loads(results[1].stdout))

    def test_plan_rejections_before_exe_access(self) -> None:
        original = copy.deepcopy(self.plan)
        invalid_plans = []
        for field, value in [("artifact_size", True), ("artifact_size", 0),
                             ("id", "   "), ("windows", []), ("artifact_size", 2**64)]:
            modified = copy.deepcopy(original)
            modified[field] = value
            invalid_plans.append(modified)
        for field, value in [("id", "../escape"), ("va", True), ("size", 65537),
                             ("issues", [True]), ("purpose", "   ")]:
            modified = copy.deepcopy(original)
            modified["windows"][0][field] = value
            invalid_plans.append(modified)
        duplicate = copy.deepcopy(original)
        duplicate["windows"][1]["id"] = duplicate["windows"][0]["id"]
        invalid_plans.append(duplicate)
        self.exe.unlink()
        for self.plan in invalid_plans:
            self.write_plan()
            for native in (False, True):
                with self.subTest(native=native, plan=self.plan):
                    output = self.root / str(native)
                    self.assertEqual(self.run_path(native, output).returncode, 2)
                    self.assertFalse(output.exists())

    def test_authority_and_window_failures_leave_no_packet(self) -> None:
        original = copy.deepcopy(self.plan)
        for failure in ("expected", "artifact", "size", "body", "mapping"):
            self.plan = copy.deepcopy(original)
            self.exe.write_bytes(self.data)
            expected = self.sha
            if failure == "expected": expected = "b" * 64
            elif failure == "artifact": self.exe.write_bytes(self.data + b"x")
            elif failure == "size": self.plan["artifact_size"] += 1
            elif failure == "body": self.plan["windows"][1]["body_sha256"] = "b" * 64
            else: self.plan["windows"][1]["va"] = "0x140009000"
            self.write_plan()
            for native in (False, True):
                with self.subTest(failure=failure, native=native):
                    output = self.root / str(native)
                    result = self.run_path(native, output, expected=expected)
                    self.assertNotEqual(result.returncode, 0)
                    if failure == "expected": self.assertEqual(result.returncode, 3)
                    if failure == "body": self.assertEqual(result.returncode, 6)
                    self.assertFalse(output.exists())

    def test_existing_outputs_are_preserved(self) -> None:
        for native in (False, True):
            output = self.root / str(native)
            self.assert_ok(self.run_path(native, output))
            before = {p.name: p.read_bytes() for p in output.iterdir()}
            self.assertNotEqual(self.run_path(native, output).returncode, 0)
            self.assertEqual(before, {p.name: p.read_bytes() for p in output.iterdir()})

    def test_native_rejects_unsupported_hex_and_bad_options(self) -> None:
        for extra in [("--hex",), ("--plan", str(self.plan_path)),
                      ("--output",), ("--unexpected",)]:
            with self.subTest(extra=extra):
                output = self.root / "rejected"
                self.assertEqual(self.run_path(True, output, extra=extra).returncode, 2)
                self.assertFalse(output.exists())


if __name__ == "__main__":
    parser = argparse.ArgumentParser()
    parser.add_argument("--dmc-rengine", required=True, type=Path)
    args, remaining = parser.parse_known_args()
    CLI = args.dmc_rengine.resolve()
    unittest.main(argv=[sys.argv[0], *remaining])
