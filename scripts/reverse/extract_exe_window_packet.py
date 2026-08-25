#!/usr/bin/env python3
"""Batch exact EXE byte-window acquisition through the canonical DMC Rengine CLI.

This script is orchestration only. It never maps PE addresses or reads executable
bytes itself. Every requested window is delegated to `dmc-rengine
extract-exe-window`, so the existing SHA-gated GDSpaces/PE/VA authority remains
canonical.
"""

from __future__ import annotations

import argparse
import hashlib
import json
from pathlib import Path
import shutil
import subprocess
import sys
from typing import Any

MAX_WINDOW_SIZE = 0x10000


def parse_u64(value: Any, field: str) -> int:
    if isinstance(value, int):
        number = value
    elif isinstance(value, str):
        try:
            number = int(value, 0)
        except ValueError as exc:
            raise ValueError(f"{field}: expected decimal or 0x-prefixed integer") from exc
    else:
        raise ValueError(f"{field}: expected integer or string")
    if number < 0 or number > 0xFFFFFFFFFFFFFFFF:
        raise ValueError(f"{field}: outside uint64 range")
    return number


def canonical_sha256(value: Any, field: str) -> str:
    if not isinstance(value, str) or len(value) != 64:
        raise ValueError(f"{field}: expected exactly 64 hexadecimal characters")
    lowered = value.lower()
    if any(ch not in "0123456789abcdef" for ch in lowered):
        raise ValueError(f"{field}: expected exactly 64 hexadecimal characters")
    return lowered


def load_plan(path: Path) -> dict[str, Any]:
    try:
        data = json.loads(path.read_text(encoding="utf-8"))
    except (OSError, json.JSONDecodeError) as exc:
        raise ValueError(f"cannot read plan {path}: {exc}") from exc
    if not isinstance(data, dict):
        raise ValueError("plan root must be an object")
    if data.get("schema") != "dmc-rengine.exe-window-packet-plan.v1":
        raise ValueError("unsupported or missing plan schema")
    plan_id = data.get("id")
    if not isinstance(plan_id, str) or not plan_id.strip():
        raise ValueError("plan id must be a non-empty string")
    canonical_sha256(data.get("artifact_sha256"), "artifact_sha256")
    artifact_size = parse_u64(data.get("artifact_size"), "artifact_size")
    if artifact_size == 0:
        raise ValueError("artifact_size must be non-zero")
    windows = data.get("windows")
    if not isinstance(windows, list) or not windows:
        raise ValueError("windows must be a non-empty array")

    seen: set[str] = set()
    for index, window in enumerate(windows):
        prefix = f"windows[{index}]"
        if not isinstance(window, dict):
            raise ValueError(f"{prefix}: expected object")
        window_id = window.get("id")
        if not isinstance(window_id, str) or not window_id.strip():
            raise ValueError(f"{prefix}.id: expected non-empty string")
        if window_id in seen:
            raise ValueError(f"{prefix}.id: duplicate id {window_id!r}")
        if any(ch not in "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789-_." for ch in window_id):
            raise ValueError(f"{prefix}.id: contains unsafe filename characters")
        seen.add(window_id)
        va = parse_u64(window.get("va"), f"{prefix}.va")
        size = parse_u64(window.get("size"), f"{prefix}.size")
        if va == 0:
            raise ValueError(f"{prefix}.va: zero VA is not accepted")
        if size == 0 or size > MAX_WINDOW_SIZE:
            raise ValueError(f"{prefix}.size: must be in 1..0x{MAX_WINDOW_SIZE:x}")
        mode = window.get("mode")
        if mode not in ("probe", "known-body"):
            raise ValueError(f"{prefix}.mode: expected 'probe' or 'known-body'")
        if mode == "known-body":
            canonical_sha256(window.get("body_sha256"), f"{prefix}.body_sha256")
    return data


def stable_json_bytes(value: Any) -> bytes:
    return (json.dumps(value, indent=2, sort_keys=True) + "\n").encode("utf-8")


def write_new(path: Path, payload: bytes) -> None:
    path.parent.mkdir(parents=True, exist_ok=True)
    with path.open("xb") as handle:
        handle.write(payload)


def validate_plan_only(plan_path: Path) -> int:
    try:
        plan = load_plan(plan_path)
    except ValueError as exc:
        print(f"plan error: {exc}", file=sys.stderr)
        return 2
    summary = {
        "schema": "dmc-rengine.exe-window-packet-plan-validation.v1",
        "status": "valid",
        "plan_id": plan["id"],
        "artifact_sha256": plan["artifact_sha256"].lower(),
        "artifact_size": parse_u64(plan["artifact_size"], "artifact_size"),
        "window_count": len(plan["windows"]),
        "probe_count": sum(1 for window in plan["windows"] if window["mode"] == "probe"),
        "known_body_count": sum(1 for window in plan["windows"] if window["mode"] == "known-body"),
        "semantic_claim": False,
    }
    sys.stdout.buffer.write(stable_json_bytes(summary))
    return 0


def run_packet(args: argparse.Namespace) -> int:
    required = {
        "--dmc-rengine": args.dmc_rengine,
        "--exe": args.exe,
        "--expected-sha256": args.expected_sha256,
        "--output": args.output,
    }
    missing = [name for name, value in required.items() if value is None]
    if missing:
        print(f"missing required acquisition arguments: {', '.join(missing)}", file=sys.stderr)
        return 2

    try:
        plan = load_plan(args.plan)
        expected_sha = canonical_sha256(args.expected_sha256, "--expected-sha256")
        expected_artifact_size = parse_u64(plan["artifact_size"], "artifact_size")
    except ValueError as exc:
        print(f"plan error: {exc}", file=sys.stderr)
        return 2

    if expected_sha != plan["artifact_sha256"].lower():
        print("expected SHA does not match the plan artifact authority", file=sys.stderr)
        return 3
    if not args.exe.is_file():
        print(f"executable does not exist: {args.exe}", file=sys.stderr)
        return 4

    try:
        args.output.parent.mkdir(parents=True, exist_ok=True)
        args.output.mkdir()
    except FileExistsError:
        print(f"output already exists; refusing replacement: {args.output}", file=sys.stderr)
        return 4
    except OSError as exc:
        print(f"cannot reserve output directory {args.output}: {exc}", file=sys.stderr)
        return 4

    completed: list[dict[str, Any]] = []
    published = False
    try:
        for window in plan["windows"]:
            va = parse_u64(window["va"], "va")
            size = parse_u64(window["size"], "size")
            command = [
                str(args.dmc_rengine),
                "extract-exe-window",
                str(args.exe),
                expected_sha,
                hex(va),
                hex(size),
            ]
            if args.hex:
                command.append("--hex")
            try:
                process = subprocess.run(
                    command,
                    text=True,
                    encoding="utf-8",
                    stdout=subprocess.PIPE,
                    stderr=subprocess.PIPE,
                    check=False,
                )
            except OSError as exc:
                print(f"cannot execute dmc-rengine acquisition command: {exc}", file=sys.stderr)
                return 5
            if process.stderr:
                sys.stderr.write(f"[{window['id']}] {process.stderr}")
            if process.returncode != 0:
                print(
                    f"window {window['id']} failed with exit code {process.returncode}; packet not published",
                    file=sys.stderr,
                )
                return process.returncode or 5
            try:
                receipt = json.loads(process.stdout)
            except json.JSONDecodeError as exc:
                print(f"window {window['id']} emitted invalid JSON: {exc}", file=sys.stderr)
                return 5
            if receipt.get("artifact_sha256", "").lower() != expected_sha:
                print(f"window {window['id']} receipt artifact SHA mismatch", file=sys.stderr)
                return 5
            try:
                receipt_artifact_size = parse_u64(receipt.get("artifact_size"), "receipt.artifact_size")
                receipt_va = parse_u64(receipt.get("va"), "receipt.va")
                receipt_size = parse_u64(receipt.get("size"), "receipt.size")
            except ValueError as exc:
                print(f"window {window['id']} receipt metadata is invalid: {exc}", file=sys.stderr)
                return 5
            if receipt_artifact_size != expected_artifact_size:
                print(f"window {window['id']} receipt artifact size mismatch", file=sys.stderr)
                return 5
            if receipt_va != va:
                print(f"window {window['id']} receipt VA mismatch", file=sys.stderr)
                return 5
            if receipt_size != size:
                print(f"window {window['id']} receipt size mismatch", file=sys.stderr)
                return 5
            if window["mode"] == "known-body":
                expected_body = window["body_sha256"].lower()
                if receipt.get("window_sha256", "").lower() != expected_body:
                    print(f"window {window['id']} known-body SHA mismatch", file=sys.stderr)
                    return 6

            receipt_bytes = stable_json_bytes(receipt)
            receipt_name = f"{window['id']}.receipt.json"
            write_new(args.output / receipt_name, receipt_bytes)
            completed.append(
                {
                    "id": window["id"],
                    "mode": window["mode"],
                    "va": hex(va),
                    "size": size,
                    "issues": window.get("issues", []),
                    "purpose": window.get("purpose", ""),
                    "receipt": receipt_name,
                    "receipt_sha256": hashlib.sha256(receipt_bytes).hexdigest(),
                    "window_sha256": receipt["window_sha256"],
                }
            )

        manifest = {
            "schema": "dmc-rengine.exe-window-packet-receipt.v1",
            "status": "acquired",
            "plan_id": plan["id"],
            "plan_schema": plan["schema"],
            "artifact_sha256": expected_sha,
            "artifact_size": expected_artifact_size,
            "raw_bytes_included": bool(args.hex),
            "semantic_claim": False,
            "windows": completed,
        }
        write_new(args.output / "packet.receipt.json", stable_json_bytes(manifest))
        published = True
        print(args.output / "packet.receipt.json")
        return 0
    finally:
        if not published and args.output.exists():
            shutil.rmtree(args.output, ignore_errors=True)


def main() -> int:
    parser = argparse.ArgumentParser(
        description="Acquire a hash-gated EXE reverse window packet through dmc-rengine extract-exe-window"
    )
    parser.add_argument("--plan", type=Path, required=True)
    parser.add_argument("--validate-plan-only", action="store_true")
    parser.add_argument("--dmc-rengine", type=Path)
    parser.add_argument("--exe", type=Path)
    parser.add_argument("--expected-sha256")
    parser.add_argument("--output", type=Path)
    parser.add_argument(
        "--hex",
        action="store_true",
        help="include local-only raw executable bytes in child receipts; never commit those receipts",
    )
    args = parser.parse_args()
    if args.validate_plan_only:
        return validate_plan_only(args.plan)
    return run_packet(args)


if __name__ == "__main__":
    raise SystemExit(main())
