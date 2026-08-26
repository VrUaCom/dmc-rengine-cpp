#!/usr/bin/env python3
"""Validate bounded GDSpaces L2 protected-runtime address mapping receipts.

This tool never reads proprietary process bytes. It consumes metadata-only child
receipts produced by `dmc-rengine capture-exe-process-window` and promotes a
bounded mapping packet only when several independent resolver anchors from one
protected process match canonical-analysis window hashes.
"""

from __future__ import annotations

import argparse
import json
import os
from pathlib import Path
import sys
from typing import Any

CANONICAL_ANALYSIS_SHA256 = (
    "e454272ed0fb0247fcbcf300e5d55d7a3e96d50b89b9ffaff81bb978dcbdd082"
)
PROTECTED_DISTRIBUTION_SHA256 = (
    "81c7e61983564113b5105e931d9f185accc14e44ae147d27f720c2d50935c7d6"
)
PROTECTED_DISTRIBUTION_SIZE = 6_567_320
PREFERRED_IMAGE_BASE = 0x140000000
WINDOW_SIZE = 0x40

ANCHORS = {
    0x0002FCA0: "OpenGameResource",
    0x00326D20: "type0_mount_registration",
    0x00327430: "type0_mount_resolve",
    0x00327800: "type0_final_open",
}
PHYSICAL_ANCHORS = {
    0x00326D20,
    0x00327430,
    0x00327800,
}
MIN_ANCHORS = 3


def _parse_hex_u64(value: Any, field: str) -> int:
    if not isinstance(value, str) or not value.startswith("0x"):
        raise ValueError(f"{field} must be a 0x-prefixed hexadecimal string")
    try:
        parsed = int(value, 16)
    except ValueError as exc:
        raise ValueError(f"{field} is not valid hexadecimal") from exc
    if parsed < 0 or parsed > 0xFFFFFFFFFFFFFFFF:
        raise ValueError(f"{field} is outside uint64 range")
    return parsed


def _require_sha256(value: Any, field: str) -> str:
    if not isinstance(value, str) or len(value) != 64:
        raise ValueError(f"{field} must be a 64-character SHA-256")
    lowered = value.lower()
    if any(ch not in "0123456789abcdef" for ch in lowered):
        raise ValueError(f"{field} must be hexadecimal")
    if value != lowered:
        raise ValueError(f"{field} must use canonical lowercase hex")
    return value


def _load_receipt(path: Path) -> dict[str, Any]:
    try:
        payload = json.loads(path.read_text(encoding="utf-8"))
    except (OSError, json.JSONDecodeError) as exc:
        raise ValueError(f"could not read JSON receipt {path}: {exc}") from exc
    if not isinstance(payload, dict):
        raise ValueError(f"receipt {path} must contain one JSON object")
    if payload.get("schema") != "dmc-rengine.exe-process-window.v1":
        raise ValueError(f"receipt {path} has unsupported schema")
    if "bytes_hex" in payload:
        raise ValueError(
            f"receipt {path} contains raw bytes_hex; mapping packets accept metadata-only receipts"
        )
    return payload


def _validate_child(path: Path, payload: dict[str, Any]) -> dict[str, Any]:
    artifact_sha = _require_sha256(payload.get("artifact_sha256"), "artifact_sha256")
    if artifact_sha != PROTECTED_DISTRIBUTION_SHA256:
        raise ValueError(f"receipt {path} is not bound to the protected distribution SHA")
    if payload.get("artifact_size") != PROTECTED_DISTRIBUTION_SIZE:
        raise ValueError(f"receipt {path} has wrong protected distribution size")

    expected_artifact_sha = _require_sha256(
        payload.get("expected_window_artifact_sha256"),
        "expected_window_artifact_sha256",
    )
    if expected_artifact_sha != CANONICAL_ANALYSIS_SHA256:
        raise ValueError(f"receipt {path} is not bound to the canonical analysis artifact")

    window_sha = _require_sha256(payload.get("window_sha256"), "window_sha256")
    expected_window_sha = _require_sha256(
        payload.get("expected_window_sha256"), "expected_window_sha256"
    )
    if payload.get("matches_expected_window") is not True or window_sha != expected_window_sha:
        raise ValueError(f"receipt {path} does not prove an exact canonical window match")

    if not isinstance(payload.get("pid"), int) or payload["pid"] <= 0:
        raise ValueError(f"receipt {path} has invalid pid")
    if not isinstance(payload.get("artifact_size"), int):
        raise ValueError(f"receipt {path} has invalid artifact_size")
    if not isinstance(payload.get("size"), int) or payload["size"] != WINDOW_SIZE:
        raise ValueError(f"receipt {path} must use exact mapping window size 0x40")
    if payload.get("section") != ".text":
        raise ValueError(f"receipt {path} must map an L2 .text anchor")
    image_path = payload.get("image_path")
    if not isinstance(image_path, str) or not image_path:
        raise ValueError(f"receipt {path} has invalid image_path")

    preferred_image_base = _parse_hex_u64(
        payload.get("preferred_image_base"), "preferred_image_base"
    )
    if preferred_image_base != PREFERRED_IMAGE_BASE:
        raise ValueError(f"receipt {path} has unexpected preferred image base")

    module_base = _parse_hex_u64(payload.get("module_base"), "module_base")
    rva = _parse_hex_u64(payload.get("rva"), "rva")
    runtime_va = _parse_hex_u64(payload.get("runtime_va"), "runtime_va")
    if rva not in ANCHORS:
        raise ValueError(f"receipt {path} RVA 0x{rva:X} is not an approved L2 mapping anchor")
    if module_base + rva != runtime_va:
        raise ValueError(f"receipt {path} has inconsistent module_base/RVA/runtime_va")

    return {
        "id": ANCHORS[rva],
        "rva": rva,
        "runtime_va": runtime_va,
        "size": payload["size"],
        "window_sha256": window_sha,
        "pid": payload["pid"],
        "module_base": module_base,
        "image_path": image_path,
    }


def build_packet(receipt_paths: list[Path]) -> dict[str, Any]:
    if len(receipt_paths) < MIN_ANCHORS:
        raise ValueError(f"at least {MIN_ANCHORS} child receipts are required")

    anchors: list[dict[str, Any]] = []
    seen_rvas: set[int] = set()
    common_pid: int | None = None
    common_module_base: int | None = None
    common_image_path: str | None = None

    for path in receipt_paths:
        child = _validate_child(path, _load_receipt(path))
        rva = child["rva"]
        if rva in seen_rvas:
            raise ValueError(f"duplicate mapping anchor RVA 0x{rva:X}")
        seen_rvas.add(rva)

        if common_pid is None:
            common_pid = child["pid"]
            common_module_base = child["module_base"]
            common_image_path = child["image_path"]
        elif (
            child["pid"] != common_pid
            or child["module_base"] != common_module_base
            or child["image_path"] != common_image_path
        ):
            raise ValueError("all mapping receipts must come from one process/module session")

        anchors.append(child)

    if 0x0002FCA0 not in seen_rvas:
        raise ValueError("mapping packet requires the OpenGameResource anchor RVA 0x2FCA0")
    if len(seen_rvas & PHYSICAL_ANCHORS) < 2:
        raise ValueError("mapping packet requires at least two independent type-0 physical anchors")

    anchors.sort(key=lambda item: item["rva"])
    return {
        "schema": "dmc-rengine.gdspaces-l2-runtime-mapping.v1",
        "status": "bounded_match",
        "scope": "approved-l2-rva-anchors-only",
        "protected_artifact_sha256": PROTECTED_DISTRIBUTION_SHA256,
        "protected_artifact_size": PROTECTED_DISTRIBUTION_SIZE,
        "canonical_analysis_artifact_sha256": CANONICAL_ANALYSIS_SHA256,
        "preferred_image_base": f"0x{PREFERRED_IMAGE_BASE:X}",
        "pid": common_pid,
        "module_base": f"0x{common_module_base:X}",
        "image_path": common_image_path,
        "anchor_count": len(anchors),
        "anchors": [
            {
                "id": anchor["id"],
                "rva": f"0x{anchor['rva']:X}",
                "runtime_va": f"0x{anchor['runtime_va']:X}",
                "size": anchor["size"],
                "window_sha256": anchor["window_sha256"],
            }
            for anchor in anchors
        ],
        "proves": [
            "exact-live-byte-match-at-listed-l2-ranges",
            "bounded-protected-runtime-rva-mapping-for-listed-anchors",
        ],
        "does_not_prove": [
            "global-build-byte-equivalence",
            "unlisted-function-address-equivalence",
            "retail-nbz-collision-freedom",
            "original-process-selected-provider-identity",
            "layer-1-or-layer-3-completion",
        ],
    }


def _write_no_replace(path: Path, packet: dict[str, Any]) -> None:
    encoded = (json.dumps(packet, indent=2, sort_keys=False) + "\n").encode("utf-8")
    flags = os.O_WRONLY | os.O_CREAT | os.O_EXCL
    descriptor: int | None = None
    created_by_this_call = False
    try:
        descriptor = os.open(path, flags, 0o600)
        created_by_this_call = True
        with os.fdopen(descriptor, "wb", closefd=True) as stream:
            descriptor = None
            stream.write(encoded)
            stream.flush()
    except Exception:
        if descriptor is not None:
            os.close(descriptor)
        if created_by_this_call:
            try:
                path.unlink()
            except OSError:
                pass
        raise


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("--receipt", action="append", required=True, type=Path)
    parser.add_argument("--output", required=True, type=Path)
    args = parser.parse_args()

    try:
        packet = build_packet(args.receipt)
        _write_no_replace(args.output, packet)
    except (OSError, ValueError) as exc:
        print(f"runtime mapping packet rejected: {exc}", file=sys.stderr)
        return 2

    print(json.dumps(packet, indent=2))
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
