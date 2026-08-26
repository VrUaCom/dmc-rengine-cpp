#!/usr/bin/env python3
"""Bind a validated L2 runtime mapping packet to one original selection receipt.

The tool is metadata-only. It does not acquire process/resource bytes and cannot
create original-process evidence from synthetic inputs; it only rejects or binds
already captured evidence files.
"""

from __future__ import annotations

import argparse
import hashlib
import json
import os
from pathlib import Path
import sys
from typing import Any

PROTECTED_SHA256 = "81c7e61983564113b5105e931d9f185accc14e44ae147d27f720c2d50935c7d6"
PROTECTED_SIZE = 6_567_320
PREFIXES = ("GDataX360.afs/", "GData.afs/", "Video/", "afs/sound/", "SAVEDATA/", "")
PHYSICAL_MAPPING_IDS = {
    "type0_mount_registration",
    "type0_mount_resolve",
    "type0_final_open",
}


def _sha256_bytes(data: bytes) -> str:
    return hashlib.sha256(data).hexdigest()


def _read_json(path: Path) -> tuple[dict[str, Any], bytes]:
    try:
        raw = path.read_bytes()
        value = json.loads(raw.decode("utf-8"))
    except (OSError, UnicodeDecodeError, json.JSONDecodeError) as exc:
        raise ValueError(f"could not read JSON {path}: {exc}") from exc
    if not isinstance(value, dict):
        raise ValueError(f"{path} must contain one JSON object")
    if "bytes_hex" in value:
        raise ValueError(f"{path} contains raw bytes_hex")
    return value, raw


def _sha(value: Any, field: str) -> str:
    if not isinstance(value, str) or len(value) != 64 or value != value.lower():
        raise ValueError(f"{field} must be canonical lowercase SHA-256")
    if any(ch not in "0123456789abcdef" for ch in value):
        raise ValueError(f"{field} must be canonical lowercase SHA-256")
    return value


def _hex_u64(value: Any, field: str) -> int:
    if not isinstance(value, str) or not value.startswith("0x"):
        raise ValueError(f"{field} must be 0x-prefixed hexadecimal")
    try:
        parsed = int(value, 16)
    except ValueError as exc:
        raise ValueError(f"{field} is invalid hexadecimal") from exc
    if parsed < 0 or parsed > 0xFFFFFFFFFFFFFFFF:
        raise ValueError(f"{field} is outside uint64 range")
    return parsed


def _normalize(path: str, flags: int) -> str:
    if "\0" in path:
        raise ValueError("path contains embedded NUL")
    begin, end = 0, len(path)
    if flags & 0x04:
        while begin < end and path[begin] in "/\\":
            begin += 1
    if flags & 0x08:
        while end > begin and path[end - 1] in "/\\":
            end -= 1
    lower = bool(flags & 0x02) and not bool(flags & 0x01)
    upper = bool(flags & 0x01)
    out: list[str] = []
    previous_separator = False
    for ch in path[begin:end]:
        if upper and "a" <= ch <= "z":
            ch = chr(ord(ch) - 32)
        elif lower and "A" <= ch <= "Z":
            ch = chr(ord(ch) + 32)
        if ch in "/\\":
            if not previous_separator:
                out.append("\\")
            previous_separator = True
        else:
            out.append(ch)
            previous_separator = False
    return "".join(out)


def _basename(request: str) -> str:
    if not request or "\0" in request:
        return ""
    pos = max(request.rfind("/"), request.rfind("\\"))
    return request if pos < 0 else request[pos + 1 :]


def _validate_mapping(mapping: dict[str, Any]) -> tuple[int, int]:
    if mapping.get("schema") != "dmc-rengine.gdspaces-l2-runtime-mapping.v1":
        raise ValueError("mapping packet schema mismatch")
    if mapping.get("status") != "bounded_match":
        raise ValueError("mapping packet is not a bounded_match")
    if _sha(mapping.get("protected_artifact_sha256"), "mapping protected SHA") != PROTECTED_SHA256:
        raise ValueError("mapping packet is not bound to protected DMC3 executable")
    if mapping.get("protected_artifact_size") != PROTECTED_SIZE:
        raise ValueError("mapping protected artifact size mismatch")
    pid = mapping.get("pid")
    if not isinstance(pid, int) or pid <= 0:
        raise ValueError("mapping pid is invalid")
    module_base = _hex_u64(mapping.get("module_base"), "mapping module_base")
    anchors = mapping.get("anchors")
    if not isinstance(anchors, list) or mapping.get("anchor_count") != len(anchors) or len(anchors) < 3:
        raise ValueError("mapping anchor census is invalid")
    ids = {item.get("id") for item in anchors if isinstance(item, dict)}
    if "OpenGameResource" not in ids or len(ids & PHYSICAL_MAPPING_IDS) < 2:
        raise ValueError("mapping lacks required OpenGameResource + physical anchor breadth")
    return pid, module_base


def _validate_selection(selection: dict[str, Any], mapping_sha: str, mapping_pid: int, mapping_base: int) -> None:
    if selection.get("schema") != "dmc-rengine.gdspaces-l2-original-selection.v1":
        raise ValueError("selection receipt schema mismatch")
    if selection.get("evidence_class") != "original-process-observation":
        raise ValueError("selection evidence class mismatch")
    if _sha(selection.get("executable_sha256"), "selection executable SHA") != PROTECTED_SHA256:
        raise ValueError("selection is not bound to protected DMC3 executable")
    if selection.get("executable_size") != PROTECTED_SIZE:
        raise ValueError("selection executable size mismatch")
    if _sha(selection.get("runtime_mapping_packet_sha256"), "mapping packet SHA") != mapping_sha:
        raise ValueError("selection does not hash-bind the supplied mapping packet")
    if selection.get("pid") != mapping_pid:
        raise ValueError("selection and mapping pid differ")
    if _hex_u64(selection.get("module_base"), "selection module_base") != mapping_base:
        raise ValueError("selection and mapping module base differ")
    if selection.get("flags") != 1:
        raise ValueError("selection must use recovered direct-call flags=1 mode")
    for field in ("observer_id", "observer_version"):
        value = selection.get(field)
        if not isinstance(value, str) or not value or len(value) > 128 or "\0" in value:
            raise ValueError(f"selection {field} is invalid")

    request = selection.get("request")
    basename = selection.get("basename")
    if not isinstance(request, str) or not isinstance(basename, str) or _basename(request) != basename or not basename:
        raise ValueError("selection request/basename binding is invalid")
    if len(PREFIXES[0] + basename) >= 0x400:
        raise ValueError("selection request violates recovered 0x400 first-candidate bound")

    first_missing = selection.get("first_missing_archive_volume")
    archives = selection.get("archives")
    if not isinstance(first_missing, int) or first_missing < 0 or first_missing > 0x7FFFFFFF:
        raise ValueError("first_missing_archive_volume is invalid")
    if not isinstance(archives, list) or len(archives) != first_missing:
        raise ValueError("archive identity census is not contiguous")
    seen: set[int] = set()
    for item in archives:
        if not isinstance(item, dict):
            raise ValueError("archive identity entry is invalid")
        index = item.get("volume_index")
        if not isinstance(index, int) or index < 0 or index >= first_missing or index in seen:
            raise ValueError("archive volume index is invalid/duplicate")
        seen.add(index)
        if item.get("filename") != f"DMC3-{index}.nbz":
            raise ValueError("archive filename does not match runtime volume identity")
        _sha(item.get("sha256"), "archive SHA")
        if not isinstance(item.get("size"), int) or item["size"] <= 0:
            raise ValueError("archive size is invalid")
    if seen != set(range(first_missing)):
        raise ValueError("archive volume identity set has a gap")

    probes = selection.get("probes")
    selected = selection.get("selected")
    if not isinstance(probes, list) or not probes or not isinstance(selected, dict):
        raise ValueError("selection probes/selected identity are missing")

    expected: list[tuple[int, str, str, int | None]] = []
    for attempt in range(12):
        provider = "archive" if attempt < 6 else "physical"
        prefix = PREFIXES[attempt % 6]
        candidate = prefix + basename
        if provider == "archive":
            for volume in range(first_missing - 1, -1, -1):
                expected.append((attempt, provider, candidate, volume))
        else:
            expected.append((attempt, provider, candidate, None))

    if len(probes) > len(expected):
        raise ValueError("selection contains more probes than recovered policy permits")
    selected_seen = False
    for sequence, probe in enumerate(probes):
        if not isinstance(probe, dict):
            raise ValueError("probe is not an object")
        attempt, provider, candidate, volume = expected[sequence]
        if probe.get("sequence_index") != sequence or probe.get("lookup_attempt_index") != attempt:
            raise ValueError("probe sequence/attempt order mismatch")
        if probe.get("provider") != provider or probe.get("candidate") != candidate:
            raise ValueError("probe provider/candidate order mismatch")
        if probe.get("archive_volume_index") != volume:
            raise ValueError("probe archive volume precedence mismatch")
        expected_key = _normalize(candidate, 0x0E if provider == "archive" else 0x0C)
        if probe.get("provider_key") != expected_key:
            raise ValueError("probe provider key mismatch")
        outcome = probe.get("outcome")
        if outcome not in ("miss", "selected"):
            raise ValueError("probe outcome is invalid")
        if outcome == "selected":
            if sequence != len(probes) - 1:
                raise ValueError("selected probe must terminate trace")
            selected_seen = True

    if not selected_seen:
        raise ValueError("trace has no selected probe")
    last = probes[-1]
    for key in ("provider", "lookup_attempt_index", "candidate", "provider_key", "archive_volume_index"):
        if selected.get(key) != last.get(key):
            raise ValueError(f"selected identity disagrees with terminal probe field {key}")
    if selected["provider"] == "archive":
        member = selected.get("archive_member_path")
        if not isinstance(member, str) or not member or _normalize(member, 0x0E) != selected["provider_key"]:
            raise ValueError("selected archive member identity does not match provider key")
        if selected.get("physical_relative_path") != "":
            raise ValueError("archive selection carries physical identity")
    else:
        relative = selected.get("physical_relative_path")
        if not isinstance(relative, str) or not relative or relative.startswith(("/", "\\")) or (len(relative) >= 2 and relative[1] == ":"):
            raise ValueError("physical identity must be mounted-root-relative")
        if _normalize(relative, 0x0C) != selected["provider_key"]:
            raise ValueError("selected physical identity does not match provider key")
        if selected.get("archive_member_path") != "":
            raise ValueError("physical selection carries archive member identity")


def build_bound_packet(mapping_path: Path, selection_path: Path) -> dict[str, Any]:
    mapping, mapping_raw = _read_json(mapping_path)
    selection, selection_raw = _read_json(selection_path)
    mapping_sha = _sha256_bytes(mapping_raw)
    mapping_pid, mapping_base = _validate_mapping(mapping)
    _validate_selection(selection, mapping_sha, mapping_pid, mapping_base)
    return {
        "schema": "dmc-rengine.gdspaces-l2-original-selection-bound.v1",
        "status": "bound",
        "evidence_class": "original-process-observation",
        "protected_artifact_sha256": PROTECTED_SHA256,
        "protected_artifact_size": PROTECTED_SIZE,
        "runtime_mapping_packet_sha256": mapping_sha,
        "original_selection_receipt_sha256": _sha256_bytes(selection_raw),
        "pid": mapping_pid,
        "module_base": f"0x{mapping_base:X}",
        "observer_id": selection["observer_id"],
        "observer_version": selection["observer_version"],
        "request": selection["request"],
        "selected": selection["selected"],
        "proves": ["mapping-bound-original-selected-resource-identity"],
        "does_not_prove": [
            "retail-archive-collision-freedom",
            "global-build-equivalence",
            "layer-1-or-layer-3-completion",
        ],
    }


def _write_no_replace(path: Path, packet: dict[str, Any]) -> None:
    data = (json.dumps(packet, indent=2) + "\n").encode("utf-8")
    fd: int | None = None
    created = False
    try:
        fd = os.open(path, os.O_WRONLY | os.O_CREAT | os.O_EXCL, 0o600)
        created = True
        with os.fdopen(fd, "wb", closefd=True) as stream:
            fd = None
            stream.write(data)
            stream.flush()
    except Exception:
        if fd is not None:
            os.close(fd)
        if created:
            try:
                path.unlink()
            except OSError:
                pass
        raise


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("--mapping", required=True, type=Path)
    parser.add_argument("--selection", required=True, type=Path)
    parser.add_argument("--output", required=True, type=Path)
    args = parser.parse_args()
    try:
        packet = build_bound_packet(args.mapping, args.selection)
        _write_no_replace(args.output, packet)
    except (OSError, ValueError) as exc:
        print(f"original selection evidence rejected: {exc}", file=sys.stderr)
        return 2
    print(json.dumps(packet, indent=2))
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
