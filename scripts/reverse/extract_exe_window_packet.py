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
PLAN_SCHEMA = "dmc-rengine.exe-window-packet-plan.v1"
VALIDATION_SCHEMA = "dmc-rengine.exe-window-packet-plan-validation.v1"
CHILD_RECEIPT_SCHEMA = "dmc-rengine.exe-byte-window.v1"
PACKET_RECEIPT_SCHEMA = "dmc-rengine.exe-window-packet-receipt.v1"


def parse_u64(value: Any, field: str) -> int:
    if isinstance(value, bool):
        raise ValueError(f"{field}: expected integer or string")
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


def canonical_lower_hex(value: Any, expected_bytes: int, field: str) -> str:
    if not isinstance(value, str) or len(value) != expected_bytes * 2:
        raise ValueError(
            f"{field}: expected exactly {expected_bytes * 2} lowercase hexadecimal characters"
        )
    if any(ch not in "0123456789abcdef" for ch in value):
        raise ValueError(f"{field}: expected lowercase hexadecimal bytes")
    return value


def _validate_plan(data: Any) -> dict[str, Any]:
    if not isinstance(data, dict):
        raise ValueError("plan root must be an object")
    if data.get("schema") != PLAN_SCHEMA:
        raise ValueError("unsupported or missing plan schema")
    plan_id = data.get("id")
    if not isinstance(plan_id, str) or not plan_id.strip():
        raise ValueError("plan id must be a non-empty string")
    canonical_sha256(data.get("artifact_sha256"), "artifact_sha256")
    artifact_size = parse_u64(data.get("artifact_size"), "artifact_size")
    if artifact_size == 0:
        raise ValueError("artifact_size must be non-zero")
    authority_role = data.get("authority_role")
    if not isinstance(authority_role, str) or not authority_role.strip():
        raise ValueError("authority_role must be a non-empty string")
    window_size_policy = data.get("window_size_policy")
    if not isinstance(window_size_policy, str) or not window_size_policy.strip():
        raise ValueError("window_size_policy must be a non-empty string")
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
        if any(
            ch not in "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789-_."
            for ch in window_id
        ):
            raise ValueError(f"{prefix}.id: contains unsafe filename characters")
        seen.add(window_id)
        va = parse_u64(window.get("va"), f"{prefix}.va")
        size = parse_u64(window.get("size"), f"{prefix}.size")
        if va == 0:
            raise ValueError(f"{prefix}.va: zero VA is not accepted")
        if size == 0 or size > MAX_WINDOW_SIZE:
            raise ValueError(
                f"{prefix}.size: must be in 1..0x{MAX_WINDOW_SIZE:x}"
            )
        mode = window.get("mode")
        if mode not in ("probe", "known-body"):
            raise ValueError(f"{prefix}.mode: expected 'probe' or 'known-body'")
        if mode == "known-body":
            canonical_sha256(window.get("body_sha256"), f"{prefix}.body_sha256")
        issues = window.get("issues")
        if not isinstance(issues, list) or any(
            isinstance(issue, bool) or not isinstance(issue, int) or issue <= 0
            for issue in issues
        ):
            raise ValueError(
                f"{prefix}.issues: expected an array of positive issue numbers"
            )
        purpose = window.get("purpose")
        if not isinstance(purpose, str) or not purpose.strip():
            raise ValueError(f"{prefix}.purpose: expected non-empty string")
    return data


def load_plan(path: Path) -> tuple[dict[str, Any], bytes]:
    try:
        raw = path.read_bytes()
    except OSError as exc:
        raise ValueError(f"cannot read plan {path}: {exc}") from exc
    try:
        text = raw.decode("utf-8")
    except UnicodeDecodeError as exc:
        raise ValueError(f"cannot decode plan {path} as UTF-8: {exc}") from exc
    try:
        data = json.loads(text)
    except json.JSONDecodeError as exc:
        raise ValueError(f"cannot parse plan {path}: {exc}") from exc
    return _validate_plan(data), raw


def stable_json_bytes(value: Any) -> bytes:
    return (json.dumps(value, indent=2, sort_keys=True) + "\n").encode("utf-8")


def write_new(path: Path, payload: bytes) -> None:
    path.parent.mkdir(parents=True, exist_ok=True)
    with path.open("xb") as handle:
        handle.write(payload)


def validate_child_receipt(
    receipt: Any,
    *,
    expected_sha: str,
    expected_artifact_size: int,
    expected_va: int,
    expected_size: int,
    include_hex: bool,
) -> dict[str, Any]:
    if not isinstance(receipt, dict):
        raise ValueError("receipt root must be an object")
    if receipt.get("schema") != CHILD_RECEIPT_SCHEMA:
        raise ValueError("receipt schema mismatch")

    receipt_sha = canonical_sha256(
        receipt.get("artifact_sha256"), "receipt.artifact_sha256"
    )
    receipt_window_sha = canonical_sha256(
        receipt.get("window_sha256"), "receipt.window_sha256"
    )
    receipt_artifact_size = parse_u64(
        receipt.get("artifact_size"), "receipt.artifact_size"
    )
    receipt_image_base = parse_u64(receipt.get("image_base"), "receipt.image_base")
    receipt_va = parse_u64(receipt.get("va"), "receipt.va")
    receipt_rva = parse_u64(receipt.get("rva"), "receipt.rva")
    receipt_file_offset = parse_u64(
        receipt.get("file_offset"), "receipt.file_offset"
    )
    receipt_size = parse_u64(receipt.get("size"), "receipt.size")
    receipt_section = receipt.get("section")
    if not isinstance(receipt_section, str) or not receipt_section:
        raise ValueError("receipt.section must be a non-empty string")

    if receipt_sha != expected_sha:
        raise ValueError("receipt artifact SHA mismatch")
    if receipt_artifact_size != expected_artifact_size:
        raise ValueError("receipt artifact size mismatch")
    if receipt_va != expected_va:
        raise ValueError("receipt VA mismatch")
    if receipt_size != expected_size:
        raise ValueError("receipt size mismatch")
    if receipt_va < receipt_image_base or receipt_rva != receipt_va - receipt_image_base:
        raise ValueError("receipt RVA/image-base relationship mismatch")
    if (
        receipt_file_offset > receipt_artifact_size
        or receipt_size > receipt_artifact_size - receipt_file_offset
    ):
        raise ValueError("receipt file range exceeds artifact size")

    if include_hex:
        bytes_hex = canonical_lower_hex(
            receipt.get("bytes_hex"), receipt_size, "receipt.bytes_hex"
        )
        try:
            raw_bytes = bytes.fromhex(bytes_hex)
        except ValueError as exc:  # defensive; canonical_lower_hex constrains input
            raise ValueError("receipt.bytes_hex is invalid") from exc
        if hashlib.sha256(raw_bytes).hexdigest() != receipt_window_sha:
            raise ValueError("receipt raw bytes do not match window SHA")
    elif "bytes_hex" in receipt:
        raise ValueError("receipt unexpectedly contains raw bytes without --hex")

    return {
        "schema": CHILD_RECEIPT_SCHEMA,
        "artifact_sha256": receipt_sha,
        "artifact_size": receipt_artifact_size,
        "image_base": receipt_image_base,
        "va": receipt_va,
        "rva": receipt_rva,
        "file_offset": receipt_file_offset,
        "size": receipt_size,
        "section": receipt_section,
        "window_sha256": receipt_window_sha,
    }


def validate_plan_only(plan_path: Path) -> int:
    try:
        plan, plan_bytes = load_plan(plan_path)
    except ValueError as exc:
        print(f"plan error: {exc}", file=sys.stderr)
        return 2
    summary = {
        "schema": VALIDATION_SCHEMA,
        "status": "valid",
        "plan_id": plan["id"],
        "plan_sha256": hashlib.sha256(plan_bytes).hexdigest(),
        "artifact_sha256": plan["artifact_sha256"].lower(),
        "artifact_size": parse_u64(plan["artifact_size"], "artifact_size"),
        "authority_role": plan["authority_role"],
        "window_count": len(plan["windows"]),
        "probe_count": sum(
            1 for window in plan["windows"] if window["mode"] == "probe"
        ),
        "known_body_count": sum(
            1 for window in plan["windows"] if window["mode"] == "known-body"
        ),
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
        print(
            f"missing required acquisition arguments: {', '.join(missing)}",
            file=sys.stderr,
        )
        return 2

    try:
        plan, plan_bytes = load_plan(args.plan)
        plan_sha = hashlib.sha256(plan_bytes).hexdigest()
        expected_sha = canonical_sha256(
            args.expected_sha256, "--expected-sha256"
        )
        expected_artifact_size = parse_u64(
            plan["artifact_size"], "artifact_size"
        )
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
        print(
            f"output already exists; refusing replacement: {args.output}",
            file=sys.stderr,
        )
        return 4
    except OSError as exc:
        print(f"cannot reserve output directory {args.output}: {exc}", file=sys.stderr)
        return 4

    completed: list[dict[str, Any]] = []
    published = False
    try:
        write_new(args.output / "packet.plan.json", plan_bytes)

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
                print(
                    f"cannot execute dmc-rengine acquisition command: {exc}",
                    file=sys.stderr,
                )
                return 5
            if process.stderr:
                sys.stderr.write(f"[{window['id']}] {process.stderr}")
            if process.returncode != 0:
                print(
                    f"window {window['id']} failed with exit code "
                    f"{process.returncode}; packet not published",
                    file=sys.stderr,
                )
                return process.returncode or 5
            try:
                receipt = json.loads(process.stdout)
                normalized = validate_child_receipt(
                    receipt,
                    expected_sha=expected_sha,
                    expected_artifact_size=expected_artifact_size,
                    expected_va=va,
                    expected_size=size,
                    include_hex=bool(args.hex),
                )
            except (json.JSONDecodeError, ValueError) as exc:
                print(
                    f"window {window['id']} emitted an invalid receipt: {exc}",
                    file=sys.stderr,
                )
                return 5

            if window["mode"] == "known-body":
                expected_body = window["body_sha256"].lower()
                if normalized["window_sha256"] != expected_body:
                    print(
                        f"window {window['id']} known-body SHA mismatch",
                        file=sys.stderr,
                    )
                    return 6

            receipt_bytes = stable_json_bytes(receipt)
            receipt_name = f"{window['id']}.receipt.json"
            write_new(args.output / receipt_name, receipt_bytes)
            completed.append(
                {
                    "id": window["id"],
                    "mode": window["mode"],
                    "va": hex(va),
                    "rva": hex(normalized["rva"]),
                    "file_offset": hex(normalized["file_offset"]),
                    "size": size,
                    "section": normalized["section"],
                    "issues": window["issues"],
                    "purpose": window["purpose"],
                    "receipt": receipt_name,
                    "receipt_schema": normalized["schema"],
                    "receipt_sha256": hashlib.sha256(receipt_bytes).hexdigest(),
                    "window_sha256": normalized["window_sha256"],
                }
            )

        manifest = {
            "schema": PACKET_RECEIPT_SCHEMA,
            "status": "acquired",
            "plan_id": plan["id"],
            "plan_schema": plan["schema"],
            "plan_receipt": "packet.plan.json",
            "plan_sha256": plan_sha,
            "artifact_sha256": expected_sha,
            "artifact_size": expected_artifact_size,
            "authority_role": plan["authority_role"],
            "raw_bytes_included": bool(args.hex),
            "semantic_claim": False,
            "windows": completed,
        }
        write_new(
            args.output / "packet.receipt.json", stable_json_bytes(manifest)
        )
        published = True
        print(args.output / "packet.receipt.json")
        return 0
    finally:
        if not published and args.output.exists():
            shutil.rmtree(args.output, ignore_errors=True)


def main() -> int:
    parser = argparse.ArgumentParser(
        description=(
            "Acquire a hash-gated EXE reverse window packet through "
            "dmc-rengine extract-exe-window"
        )
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
        help=(
            "include local-only raw executable bytes in child receipts; "
            "never commit those receipts"
        ),
    )
    args = parser.parse_args()
    if args.validate_plan_only:
        return validate_plan_only(args.plan)
    return run_packet(args)


if __name__ == "__main__":
    raise SystemExit(main())
