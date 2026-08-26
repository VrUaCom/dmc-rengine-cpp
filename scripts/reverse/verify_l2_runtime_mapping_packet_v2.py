#!/usr/bin/env python3
"""Validate process-instance-bound GDSpaces L2 runtime mapping receipts.

V2 is the real-promotion mapping surface. It accepts only process-window v2
children carrying one OS-derived Windows process creation FILETIME and derives
approved canonical anchor hashes directly from the exact canonical analysis EXE.
Legacy v1 receipts and self-declared expected-window hashes are not promotion
authority.
"""

from __future__ import annotations

import argparse
import hashlib
import json
import os
from pathlib import Path, PureWindowsPath
import struct
import sys
from typing import Any

CANONICAL_ANALYSIS_SHA256 = (
    "e454272ed0fb0247fcbcf300e5d55d7a3e96d50b89b9ffaff81bb978dcbdd082"
)
CANONICAL_ANALYSIS_SIZE = 6_356_432
PROTECTED_DISTRIBUTION_SHA256 = (
    "81c7e61983564113b5105e931d9f185accc14e44ae147d27f720c2d50935c7d6"
)
PROTECTED_DISTRIBUTION_SIZE = 6_567_320
PREFERRED_IMAGE_BASE = 0x140000000
WINDOW_SIZE = 0x40

# These are bounded canonical-analysis RVAs only. V2 requires enough anchors to
# cover physical/archive bootstrap mounting, generic provider resolution,
# archive selection and physical final-open edges needed by the later trusted
# R3 trace.
ANCHORS = {
    0x0002FCA0: "OpenGameResource",
    0x00326D20: "type0_mount_registration",
    0x00326DA0: "type1_archive_mount_registration",
    0x00327430: "ResourceMountResolve",
    0x00327800: "type0_final_open",
    0x00328160: "archive_normalized_lookup",
    0x00328290: "archive_wrapper_open",
}
REQUIRED_ANCHORS = {
    0x0002FCA0,
    0x00326D20,
    0x00326DA0,
    0x00327430,
    0x00327800,
    0x00328160,
    0x00328290,
}
MIN_ANCHORS = len(REQUIRED_ANCHORS)


def _u16(data: bytes, offset: int, field: str) -> int:
    if offset < 0 or offset + 2 > len(data):
        raise ValueError(f"canonical EXE is truncated while reading {field}")
    return struct.unpack_from("<H", data, offset)[0]


def _u32(data: bytes, offset: int, field: str) -> int:
    if offset < 0 or offset + 4 > len(data):
        raise ValueError(f"canonical EXE is truncated while reading {field}")
    return struct.unpack_from("<I", data, offset)[0]


def _u64(data: bytes, offset: int, field: str) -> int:
    if offset < 0 or offset + 8 > len(data):
        raise ValueError(f"canonical EXE is truncated while reading {field}")
    return struct.unpack_from("<Q", data, offset)[0]


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


def _require_process_creation_filetime(value: Any, field: str) -> int:
    if not isinstance(value, int) or isinstance(value, bool):
        raise ValueError(f"{field} must be an unsigned 64-bit integer")
    if value <= 0 or value > 0xFFFFFFFFFFFFFFFF:
        raise ValueError(f"{field} must be a non-zero unsigned 64-bit integer")
    return value


def _canonical_anchor_hashes(canonical_exe: Path) -> dict[int, str]:
    try:
        data = canonical_exe.read_bytes()
    except OSError as exc:
        raise ValueError(f"could not read canonical EXE {canonical_exe}: {exc}") from exc

    if len(data) != CANONICAL_ANALYSIS_SIZE:
        raise ValueError(
            f"canonical EXE size mismatch: expected {CANONICAL_ANALYSIS_SIZE}, got {len(data)}"
        )
    actual_sha = hashlib.sha256(data).hexdigest()
    if actual_sha != CANONICAL_ANALYSIS_SHA256:
        raise ValueError(
            "canonical EXE SHA-256 mismatch; mapping v2 requires the exact analysis artifact"
        )

    if data[:2] != b"MZ":
        raise ValueError("canonical EXE does not have an MZ header")
    pe_offset = _u32(data, 0x3C, "e_lfanew")
    if pe_offset + 24 > len(data) or data[pe_offset : pe_offset + 4] != b"PE\0\0":
        raise ValueError("canonical EXE does not have a valid PE signature")

    section_count = _u16(data, pe_offset + 6, "NumberOfSections")
    optional_size = _u16(data, pe_offset + 20, "SizeOfOptionalHeader")
    optional_offset = pe_offset + 24
    if optional_offset + optional_size > len(data):
        raise ValueError("canonical EXE optional header is truncated")
    if _u16(data, optional_offset, "OptionalHeader.Magic") != 0x20B:
        raise ValueError("canonical EXE is not PE32+")
    if _u64(data, optional_offset + 24, "ImageBase") != PREFERRED_IMAGE_BASE:
        raise ValueError("canonical EXE has unexpected preferred image base")

    section_table = optional_offset + optional_size
    sections: list[tuple[str, int, int, int, int]] = []
    for index in range(section_count):
        offset = section_table + index * 40
        if offset + 40 > len(data):
            raise ValueError("canonical EXE section table is truncated")
        name = data[offset : offset + 8].split(b"\0", 1)[0].decode("ascii", errors="strict")
        virtual_size = _u32(data, offset + 8, "VirtualSize")
        virtual_address = _u32(data, offset + 12, "VirtualAddress")
        raw_size = _u32(data, offset + 16, "SizeOfRawData")
        raw_pointer = _u32(data, offset + 20, "PointerToRawData")
        sections.append((name, virtual_address, virtual_size, raw_size, raw_pointer))

    hashes: dict[int, str] = {}
    for rva in ANCHORS:
        match: tuple[str, int, int, int, int] | None = None
        for section in sections:
            name, virtual_address, virtual_size, raw_size, raw_pointer = section
            mapped_size = max(virtual_size, raw_size)
            if (
                rva >= virtual_address
                and rva + WINDOW_SIZE <= virtual_address + mapped_size
            ):
                match = section
                break
        if match is None:
            raise ValueError(f"canonical anchor RVA 0x{rva:X} is not mapped by a PE section")

        name, virtual_address, _, raw_size, raw_pointer = match
        if name != ".text":
            raise ValueError(f"canonical anchor RVA 0x{rva:X} is not in .text")
        delta = rva - virtual_address
        if delta + WINDOW_SIZE > raw_size:
            raise ValueError(f"canonical anchor RVA 0x{rva:X} has no full raw-file window")
        file_offset = raw_pointer + delta
        if file_offset + WINDOW_SIZE > len(data):
            raise ValueError(f"canonical anchor RVA 0x{rva:X} exceeds the artifact")
        hashes[rva] = hashlib.sha256(
            data[file_offset : file_offset + WINDOW_SIZE]
        ).hexdigest()

    return hashes


def _load_receipt(path: Path) -> dict[str, Any]:
    try:
        payload = json.loads(path.read_text(encoding="utf-8"))
    except (OSError, json.JSONDecodeError) as exc:
        raise ValueError(f"could not read JSON receipt {path}: {exc}") from exc
    if not isinstance(payload, dict):
        raise ValueError(f"receipt {path} must contain one JSON object")
    if payload.get("schema") != "dmc-rengine.exe-process-window.v2":
        raise ValueError(
            f"receipt {path} must use process-window v2; legacy v1 is not promotion authority"
        )
    if "bytes_hex" in payload:
        raise ValueError(
            f"receipt {path} contains raw bytes_hex; mapping packets accept metadata-only receipts"
        )
    return payload


def _validate_child(
    path: Path,
    payload: dict[str, Any],
    canonical_hashes: dict[int, str],
) -> dict[str, Any]:
    artifact_sha = _require_sha256(payload.get("artifact_sha256"), "artifact_sha256")
    if artifact_sha != PROTECTED_DISTRIBUTION_SHA256:
        raise ValueError(f"receipt {path} is not bound to the protected distribution SHA")
    if payload.get("artifact_size") != PROTECTED_DISTRIBUTION_SIZE:
        raise ValueError(f"receipt {path} has wrong protected distribution size")

    if not isinstance(payload.get("pid"), int) or isinstance(payload.get("pid"), bool) or payload["pid"] <= 0:
        raise ValueError(f"receipt {path} has invalid pid")
    process_creation_filetime = _require_process_creation_filetime(
        payload.get("process_creation_filetime"), "process_creation_filetime"
    )
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

    live_window_sha = _require_sha256(payload.get("window_sha256"), "window_sha256")
    canonical_window_sha = canonical_hashes[rva]
    if live_window_sha != canonical_window_sha:
        raise ValueError(
            f"receipt {path} live window does not match the independently derived canonical window"
        )

    # Expectation fields are diagnostic only in v2. If present, they must agree
    # with the independently derived canonical artifact/window, but they never
    # establish authority by themselves.
    has_expected_artifact = "expected_window_artifact_sha256" in payload
    has_expected_window = "expected_window_sha256" in payload
    has_expected_match = "matches_expected_window" in payload
    if has_expected_artifact or has_expected_window or has_expected_match:
        if not (has_expected_artifact and has_expected_window and has_expected_match):
            raise ValueError(f"receipt {path} has a partial diagnostic expectation tuple")
        expected_artifact_sha = _require_sha256(
            payload.get("expected_window_artifact_sha256"),
            "expected_window_artifact_sha256",
        )
        expected_window_sha = _require_sha256(
            payload.get("expected_window_sha256"), "expected_window_sha256"
        )
        if (
            expected_artifact_sha != CANONICAL_ANALYSIS_SHA256
            or expected_window_sha != canonical_window_sha
            or payload.get("matches_expected_window") is not True
        ):
            raise ValueError(
                f"receipt {path} diagnostic expectation does not match independently derived canonical evidence"
            )

    return {
        "id": ANCHORS[rva],
        "rva": rva,
        "runtime_va": runtime_va,
        "size": payload["size"],
        "window_sha256": live_window_sha,
        "canonical_window_sha256": canonical_window_sha,
        "pid": payload["pid"],
        "process_creation_filetime": process_creation_filetime,
        "module_base": module_base,
        "image_path": image_path,
    }


def build_packet(receipt_paths: list[Path], canonical_exe: Path) -> dict[str, Any]:
    if len(receipt_paths) < MIN_ANCHORS:
        raise ValueError(f"at least {MIN_ANCHORS} child receipts are required")

    canonical_hashes = _canonical_anchor_hashes(canonical_exe)
    anchors: list[dict[str, Any]] = []
    seen_rvas: set[int] = set()
    common_pid: int | None = None
    common_process_creation_filetime: int | None = None
    common_module_base: int | None = None
    common_image_path: str | None = None

    for path in receipt_paths:
        child = _validate_child(path, _load_receipt(path), canonical_hashes)
        rva = child["rva"]
        if rva in seen_rvas:
            raise ValueError(f"duplicate mapping anchor RVA 0x{rva:X}")
        seen_rvas.add(rva)

        if common_pid is None:
            common_pid = child["pid"]
            common_process_creation_filetime = child["process_creation_filetime"]
            common_module_base = child["module_base"]
            common_image_path = child["image_path"]
        elif (
            child["pid"] != common_pid
            or child["process_creation_filetime"] != common_process_creation_filetime
            or child["module_base"] != common_module_base
            or child["image_path"] != common_image_path
        ):
            raise ValueError(
                "all mapping receipts must come from one exact process instance/module session"
            )

        anchors.append(child)

    missing_required = REQUIRED_ANCHORS - seen_rvas
    if missing_required:
        formatted = ", ".join(f"0x{rva:X}" for rva in sorted(missing_required))
        raise ValueError(f"mapping packet is missing required R2B/R3 anchor(s): {formatted}")
    if common_image_path is None or common_process_creation_filetime is None:
        raise ValueError("mapping packet has no process-instance identity")

    image_name = PureWindowsPath(common_image_path).name
    if not image_name:
        raise ValueError("mapping packet process image has no basename")

    anchors.sort(key=lambda item: item["rva"])
    return {
        "schema": "dmc-rengine.gdspaces-l2-runtime-mapping.v2",
        "status": "bounded_process_instance_match",
        "scope": "required-r2b-r3-anchors-one-process-instance-only",
        "protected_artifact_sha256": PROTECTED_DISTRIBUTION_SHA256,
        "protected_artifact_size": PROTECTED_DISTRIBUTION_SIZE,
        "canonical_analysis_artifact_sha256": CANONICAL_ANALYSIS_SHA256,
        "canonical_analysis_artifact_size": CANONICAL_ANALYSIS_SIZE,
        "canonical_window_authority": "derived-directly-from-exact-canonical-exe-by-validator",
        "preferred_image_base": f"0x{PREFERRED_IMAGE_BASE:X}",
        "pid": common_pid,
        "process_creation_filetime": common_process_creation_filetime,
        "module_base": f"0x{common_module_base:X}",
        "image_name": image_name,
        "anchor_count": len(anchors),
        "anchors": [
            {
                "id": anchor["id"],
                "rva": f"0x{anchor['rva']:X}",
                "runtime_va": f"0x{anchor['runtime_va']:X}",
                "size": anchor["size"],
                "window_sha256": anchor["window_sha256"],
                "canonical_window_sha256": anchor["canonical_window_sha256"],
            }
            for anchor in anchors
        ],
        "proves": [
            "exact-live-byte-match-at-listed-l2-ranges-against-validator-derived-canonical-windows",
            "bounded-protected-runtime-rva-mapping-for-listed-anchors",
            "all-listed-anchors-belong-to-one-os-identified-process-instance",
        ],
        "does_not_prove": [
            "global-build-byte-equivalence",
            "unlisted-function-address-equivalence",
            "retail-nbz-collision-freedom",
            "all-discovered-numbered-archives-mounted-successfully",
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
    parser.add_argument("--canonical-exe", required=True, type=Path)
    parser.add_argument("--receipt", action="append", required=True, type=Path)
    parser.add_argument("--output", required=True, type=Path)
    args = parser.parse_args()

    try:
        packet = build_packet(args.receipt, args.canonical_exe)
        _write_no_replace(args.output, packet)
    except (OSError, ValueError) as exc:
        print(f"runtime mapping v2 packet rejected: {exc}", file=sys.stderr)
        return 2

    print(json.dumps(packet, indent=2))
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
