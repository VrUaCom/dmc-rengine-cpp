#!/usr/bin/env python3
"""Artifact-bind one sanitized GDSpaces L2 selected-identity content candidate.

This validator cannot create trusted original-process evidence. It accepts only the
strict candidate emitted by `normalize_l2_original_selection_candidate.py`, rebuilds
R2B mapping from child process-window receipts, requires the same Windows process
creation identity on R2B and R3 content, hashes the exact observer and numbered NBZ
artifacts, validates the recovered clean-path resolver order, and emits a bounded
non-promotable candidate packet.
"""

from __future__ import annotations

import argparse
import hashlib
import importlib.util
import json
import os
from pathlib import Path
import sys
from typing import Any

PROTECTED_SHA256 = "81c7e61983564113b5105e931d9f185accc14e44ae147d27f720c2d50935c7d6"
PROTECTED_SIZE = 6_567_320
SELECTION_SCHEMA = "dmc-rengine.gdspaces-l2-original-selection-candidate.v2"
SELECTION_EVIDENCE_CLASS = "original-process-observation-candidate"
BOUND_SCHEMA = "dmc-rengine.gdspaces-l2-original-selection-bound.v2"
PREFIXES = (
    "GDataX360.afs/",
    "GData.afs/",
    "Video/",
    "afs/sound/",
    "SAVEDATA/",
    "",
)

SELECTION_KEYS = {
    "schema",
    "evidence_class",
    "promotion_eligible",
    "trusted_capture_bound",
    "legacy_schema_normalized",
    "executable_sha256",
    "executable_size",
    "runtime_mapping_packet_sha256",
    "observer_id",
    "observer_version",
    "observer_build_sha256",
    "trace_complete",
    "dropped_event_count",
    "pid",
    "process_creation_filetime",
    "module_base",
    "flags",
    "request",
    "basename",
    "first_missing_archive_volume",
    "archives",
    "probes",
    "selected",
    "proves",
    "does_not_prove",
}
ARCHIVE_KEYS = {"volume_index", "filename", "sha256", "size"}
PROBE_KEYS = {
    "sequence_index",
    "lookup_attempt_index",
    "provider",
    "candidate",
    "provider_key",
    "archive_volume_index",
    "outcome",
}
SELECTED_KEYS = {
    "provider",
    "lookup_attempt_index",
    "candidate",
    "provider_key",
    "archive_volume_index",
    "archive_member_path",
    "physical_relative_path",
}


def _load_runtime_mapping_verifier() -> Any:
    script = Path(__file__).with_name("verify_l2_runtime_mapping_packet.py")
    spec = importlib.util.spec_from_file_location("l2_runtime_mapping_verifier", script)
    if spec is None or spec.loader is None:
        raise RuntimeError("could not load canonical L2 runtime mapping verifier")
    module = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(module)
    return module


RUNTIME_MAPPING = _load_runtime_mapping_verifier()


def _sha256_bytes(data: bytes) -> str:
    return hashlib.sha256(data).hexdigest()


def _sha256_file(path: Path) -> tuple[str, int]:
    digest = hashlib.sha256()
    size = 0
    try:
        with path.open("rb") as stream:
            while True:
                chunk = stream.read(1024 * 1024)
                if not chunk:
                    break
                digest.update(chunk)
                size += len(chunk)
    except OSError as exc:
        raise ValueError(f"could not read artifact {path}: {exc}") from exc
    return digest.hexdigest(), size


def _contains_forbidden_key(value: Any, key: str) -> bool:
    if isinstance(value, dict):
        if key in value:
            return True
        return any(_contains_forbidden_key(child, key) for child in value.values())
    if isinstance(value, list):
        return any(_contains_forbidden_key(child, key) for child in value)
    return False


def _exact_keys(value: Any, allowed: set[str], context: str) -> dict[str, Any]:
    if not isinstance(value, dict):
        raise ValueError(f"{context} must be an object")
    extra = set(value) - allowed
    missing = allowed - set(value)
    if extra:
        raise ValueError(f"{context} contains unsupported field(s): {', '.join(sorted(extra))}")
    if missing:
        raise ValueError(f"{context} is missing field(s): {', '.join(sorted(missing))}")
    return value


def _read_json(path: Path) -> tuple[dict[str, Any], bytes]:
    try:
        raw = path.read_bytes()
        value = json.loads(raw.decode("utf-8"))
    except (OSError, UnicodeDecodeError, json.JSONDecodeError) as exc:
        raise ValueError(f"could not read JSON {path}: {exc}") from exc
    if not isinstance(value, dict):
        raise ValueError(f"{path} must contain one JSON object")
    if _contains_forbidden_key(value, "bytes_hex"):
        raise ValueError(f"{path} contains forbidden raw bytes_hex")
    return value, raw


def _sha(value: Any, field: str) -> str:
    if not isinstance(value, str) or len(value) != 64 or value != value.lower():
        raise ValueError(f"{field} must be canonical lowercase SHA-256")
    if any(ch not in "0123456789abcdef" for ch in value):
        raise ValueError(f"{field} must be canonical lowercase SHA-256")
    return value


def _u64(value: Any, field: str, *, nonzero: bool = False) -> int:
    if isinstance(value, bool) or not isinstance(value, int):
        raise ValueError(f"{field} must be an unsigned 64-bit integer")
    if value < 0 or value > 0xFFFFFFFFFFFFFFFF:
        raise ValueError(f"{field} is outside uint64 range")
    if nonzero and value == 0:
        raise ValueError(f"{field} must be non-zero")
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
    upper = bool(flags & 0x01)
    lower = bool(flags & 0x02) and not upper
    output: list[str] = []
    previous_separator = False
    for ch in path[begin:end]:
        if upper and "a" <= ch <= "z":
            ch = chr(ord(ch) - 32)
        elif lower and "A" <= ch <= "Z":
            ch = chr(ord(ch) + 32)
        if ch in "/\\":
            if not previous_separator:
                output.append("\\")
            previous_separator = True
        else:
            output.append(ch)
            previous_separator = False
    return "".join(output)


def _basename(request: str) -> str:
    if not request or "\0" in request:
        return ""
    position = max(request.rfind("/"), request.rfind("\\"))
    return request if position < 0 else request[position + 1 :]


def _reconstruct_mapping(
    child_paths: list[Path],
) -> tuple[dict[str, Any], list[dict[str, str]]]:
    if len(child_paths) < RUNTIME_MAPPING.MIN_ANCHORS:
        raise ValueError(
            f"at least {RUNTIME_MAPPING.MIN_ANCHORS} mapping child receipts are required"
        )
    child_receipts: list[dict[str, str]] = []
    for path in child_paths:
        payload, raw = _read_json(path)
        if payload.get("schema") != "dmc-rengine.exe-process-window.v2":
            raise ValueError(
                f"mapping child {path} has unsupported schema; v2 process identity is required"
            )
        rva = payload.get("rva")
        if not isinstance(rva, str):
            raise ValueError(f"mapping child {path} has invalid RVA")
        child_receipts.append({"rva": rva, "receipt_sha256": _sha256_bytes(raw)})

    rebuilt = RUNTIME_MAPPING.build_packet(child_paths)
    child_receipts.sort(key=lambda item: _hex_u64(item["rva"], "mapping child rva"))
    return rebuilt, child_receipts


def _mapping_session(mapping: dict[str, Any]) -> tuple[int, int, int]:
    if mapping.get("schema") != "dmc-rengine.gdspaces-l2-runtime-mapping.v2":
        raise ValueError("mapping packet schema mismatch")
    if mapping.get("status") != "bounded_match":
        raise ValueError("mapping packet is not bounded_match")
    if mapping.get("protected_artifact_sha256") != PROTECTED_SHA256:
        raise ValueError("mapping protected artifact SHA mismatch")
    if mapping.get("protected_artifact_size") != PROTECTED_SIZE:
        raise ValueError("mapping protected artifact size mismatch")
    pid = mapping.get("pid")
    if not isinstance(pid, int) or isinstance(pid, bool) or pid <= 0:
        raise ValueError("mapping pid is invalid")
    process_creation_filetime = _u64(
        mapping.get("process_creation_filetime"),
        "mapping process_creation_filetime",
        nonzero=True,
    )
    return (
        pid,
        process_creation_filetime,
        _hex_u64(mapping.get("module_base"), "mapping module_base"),
    )


def _validate_archives(selection: dict[str, Any]) -> dict[int, dict[str, Any]]:
    first_missing = selection.get("first_missing_archive_volume")
    if (
        not isinstance(first_missing, int)
        or isinstance(first_missing, bool)
        or first_missing < 0
        or first_missing > 0x7FFFFFFF
    ):
        raise ValueError("first_missing_archive_volume is invalid")
    archives = selection.get("archives")
    if not isinstance(archives, list) or len(archives) != first_missing:
        raise ValueError("archive identity census is not contiguous")

    claims: dict[int, dict[str, Any]] = {}
    for position, raw in enumerate(archives):
        item = _exact_keys(raw, ARCHIVE_KEYS, f"archive[{position}]")
        index = item["volume_index"]
        if (
            not isinstance(index, int)
            or isinstance(index, bool)
            or index < 0
            or index >= first_missing
            or index in claims
        ):
            raise ValueError("archive volume index is invalid/duplicate")
        if item["filename"] != f"DMC3-{index}.nbz":
            raise ValueError("archive filename does not match runtime volume identity")
        _sha(item["sha256"], f"archive {index} SHA")
        size = item["size"]
        if not isinstance(size, int) or isinstance(size, bool) or size <= 0:
            raise ValueError("archive size is invalid")
        claims[index] = item

    if set(claims) != set(range(first_missing)):
        raise ValueError("archive volume identity set has a gap")
    return claims


def _validate_candidate(
    selection: dict[str, Any],
    mapping_sha: str,
    mapping_pid: int,
    mapping_process_creation_filetime: int,
    mapping_base: int,
) -> dict[int, dict[str, Any]]:
    _exact_keys(selection, SELECTION_KEYS, "selection candidate")
    if selection["schema"] != SELECTION_SCHEMA:
        raise ValueError("selection receipt schema mismatch")
    if selection["evidence_class"] != SELECTION_EVIDENCE_CLASS:
        raise ValueError("selection evidence class mismatch")
    if selection["promotion_eligible"] is not False:
        raise ValueError("selection candidate may not predeclare promotion eligibility")
    if selection["trusted_capture_bound"] is not False:
        raise ValueError("selection candidate may not predeclare trusted capture")
    if selection["legacy_schema_normalized"] is not True:
        raise ValueError("selection candidate lacks legacy-normalizer provenance marker")

    if _sha(selection["executable_sha256"], "selection executable SHA") != PROTECTED_SHA256:
        raise ValueError("selection is not bound to protected DMC3 executable")
    if selection["executable_size"] != PROTECTED_SIZE:
        raise ValueError("selection executable size mismatch")
    if _sha(selection["runtime_mapping_packet_sha256"], "mapping packet SHA") != mapping_sha:
        raise ValueError("selection does not hash-bind the supplied mapping packet")
    if selection["pid"] != mapping_pid:
        raise ValueError("selection and mapping pid differ")
    selection_creation = _u64(
        selection["process_creation_filetime"],
        "selection process_creation_filetime",
        nonzero=True,
    )
    if selection_creation != mapping_process_creation_filetime:
        raise ValueError("selection and mapping process creation identities differ")
    if _hex_u64(selection["module_base"], "selection module_base") != mapping_base:
        raise ValueError("selection and mapping module base differ")
    if selection["flags"] != 1:
        raise ValueError("selection must use recovered direct-call flags=1 mode")

    for field in ("observer_id", "observer_version"):
        value = selection[field]
        if not isinstance(value, str) or not value or len(value) > 128 or "\0" in value:
            raise ValueError(f"selection {field} is invalid")
    _sha(selection["observer_build_sha256"], "observer build SHA")
    if selection["trace_complete"] is not True:
        raise ValueError("selection trace is not marked complete")
    dropped = selection["dropped_event_count"]
    if not isinstance(dropped, int) or isinstance(dropped, bool) or dropped != 0:
        raise ValueError("selection trace reports dropped events")

    request = selection["request"]
    basename = selection["basename"]
    if (
        not isinstance(request, str)
        or not isinstance(basename, str)
        or not basename
        or _basename(request) != basename
    ):
        raise ValueError("selection request/basename binding is invalid")
    if len(PREFIXES[0] + basename) >= 0x400:
        raise ValueError("selection request violates recovered 0x400 first-candidate bound")

    archive_claims = _validate_archives(selection)
    first_missing = selection["first_missing_archive_volume"]

    probes_raw = selection["probes"]
    if not isinstance(probes_raw, list) or not probes_raw:
        raise ValueError("selection probes are missing")
    selected = _exact_keys(selection["selected"], SELECTED_KEYS, "selected identity")

    expected: list[tuple[int, str, str, int | None]] = []
    for attempt in range(12):
        provider = "archive" if attempt < 6 else "physical"
        candidate = PREFIXES[attempt % 6] + basename
        if provider == "archive":
            for volume in range(first_missing - 1, -1, -1):
                expected.append((attempt, provider, candidate, volume))
        else:
            expected.append((attempt, provider, candidate, None))

    if len(probes_raw) > len(expected):
        raise ValueError("selection contains more probes than recovered policy permits")

    selected_seen = False
    for sequence, raw in enumerate(probes_raw):
        probe = _exact_keys(raw, PROBE_KEYS, f"probe[{sequence}]")
        attempt, provider, candidate, volume = expected[sequence]
        if probe["sequence_index"] != sequence or probe["lookup_attempt_index"] != attempt:
            raise ValueError("probe sequence/attempt order mismatch")
        if probe["provider"] != provider or probe["candidate"] != candidate:
            raise ValueError("probe provider/candidate order mismatch")
        if probe["archive_volume_index"] != volume:
            raise ValueError("probe archive volume precedence mismatch")
        expected_key = _normalize(candidate, 0x0E if provider == "archive" else 0x0C)
        if probe["provider_key"] != expected_key:
            raise ValueError("probe provider key mismatch")
        outcome = probe["outcome"]
        if outcome not in ("miss", "selected"):
            raise ValueError(
                "v2 selection candidate supports only clean miss/selected outcomes; "
                "provider/backend failure is fail-closed"
            )
        if outcome == "selected":
            if sequence != len(probes_raw) - 1:
                raise ValueError("selected probe must terminate trace")
            selected_seen = True

    if not selected_seen:
        raise ValueError("trace has no selected probe")

    terminal = probes_raw[-1]
    for key in (
        "provider",
        "lookup_attempt_index",
        "candidate",
        "provider_key",
        "archive_volume_index",
    ):
        if selected[key] != terminal[key]:
            raise ValueError(f"selected identity disagrees with terminal probe field {key}")

    selected_provider = selected["provider"]
    selected_key = selected["provider_key"]
    if not isinstance(selected_key, str) or not selected_key:
        raise ValueError("selected provider key is invalid")
    if selected_provider == "archive":
        member = selected["archive_member_path"]
        if not isinstance(member, str) or not member or _normalize(member, 0x0E) != selected_key:
            raise ValueError("selected archive member identity does not match provider key")
        if selected["physical_relative_path"] != "":
            raise ValueError("archive selection carries physical identity")
    elif selected_provider == "physical":
        relative = selected["physical_relative_path"]
        if (
            not isinstance(relative, str)
            or not relative
            or relative.startswith(("/", "\\"))
            or (len(relative) >= 2 and relative[1] == ":")
        ):
            raise ValueError("physical identity must be mounted-root-relative")
        if _normalize(relative, 0x0C) != selected_key:
            raise ValueError("selected physical identity does not match provider key")
        if selected["archive_member_path"] != "":
            raise ValueError("physical selection carries archive member identity")
    else:
        raise ValueError("selected provider is invalid")

    return archive_claims


def _bind_observer(selection: dict[str, Any], observer_path: Path) -> dict[str, Any]:
    actual_sha, actual_size = _sha256_file(observer_path)
    if actual_sha != selection["observer_build_sha256"]:
        raise ValueError("observer artifact SHA does not match selection receipt")
    return {"sha256": actual_sha, "size": actual_size}


def _bind_archives(
    claims: dict[int, dict[str, Any]], artifacts: dict[int, Path]
) -> list[dict[str, Any]]:
    if set(artifacts) != set(claims):
        raise ValueError("archive artifact set does not exactly match selection volume set")
    bound: list[dict[str, Any]] = []
    for index in sorted(claims):
        claim = claims[index]
        actual_sha, actual_size = _sha256_file(artifacts[index])
        if actual_sha != claim["sha256"] or actual_size != claim["size"]:
            raise ValueError(f"archive artifact {index} does not match claimed SHA/size")
        bound.append(
            {
                "volume_index": index,
                "filename": claim["filename"],
                "sha256": actual_sha,
                "size": actual_size,
            }
        )
    return bound


def build_bound_packet(
    mapping_path: Path,
    selection_path: Path,
    mapping_child_paths: list[Path],
    observer_artifact_path: Path,
    archive_artifacts: dict[int, Path],
) -> dict[str, Any]:
    mapping, mapping_raw = _read_json(mapping_path)
    selection, selection_raw = _read_json(selection_path)

    rebuilt_mapping, child_receipts = _reconstruct_mapping(mapping_child_paths)
    if mapping != rebuilt_mapping:
        raise ValueError(
            "supplied mapping packet does not exactly match reconstruction from mapping child receipts"
        )

    mapping_sha = _sha256_bytes(mapping_raw)
    mapping_pid, mapping_creation, mapping_base = _mapping_session(mapping)
    archive_claims = _validate_candidate(
        selection,
        mapping_sha,
        mapping_pid,
        mapping_creation,
        mapping_base,
    )
    observer = _bind_observer(selection, observer_artifact_path)
    bound_archives = _bind_archives(archive_claims, archive_artifacts)

    return {
        "schema": BOUND_SCHEMA,
        "status": "bound_candidate",
        "evidence_class": SELECTION_EVIDENCE_CLASS,
        "promotion_eligible": False,
        "trusted_capture_bound": False,
        "protected_artifact_sha256": PROTECTED_SHA256,
        "protected_artifact_size": PROTECTED_SIZE,
        "runtime_mapping_packet_sha256": mapping_sha,
        "runtime_mapping_child_receipts": child_receipts,
        "selection_candidate_sha256": _sha256_bytes(selection_raw),
        "observer": {
            "id": selection["observer_id"],
            "version": selection["observer_version"],
            "sha256": observer["sha256"],
            "size": observer["size"],
        },
        "archives": bound_archives,
        "pid": mapping_pid,
        "process_creation_filetime": mapping_creation,
        "module_base": f"0x{mapping_base:X}",
        "trace_complete": True,
        "dropped_event_count": 0,
        "request": selection["request"],
        "selected": dict(selection["selected"]),
        "proves": [
            "selection-candidate-structure-matches-recovered-clean-path-policy",
            "mapping-reconstructs-from-supplied-process-window-receipts",
            "mapping-and-selection-candidate-share-one-process-instance-identity",
            "candidate-hash-binds-exact-mapping-file",
            "observer-artifact-matches-declared-build-sha",
            "numbered-archive-artifacts-match-declared-sha-and-size",
        ],
        "does_not_prove": [
            "trusted-observer-execution-or-trace-origin",
            "original-process-selected-provider-identity",
            "retail-archive-collision-freedom",
            "global-build-equivalence",
            "layer-1-or-layer-3-completion",
        ],
    }


def _parse_archive_artifacts(values: list[str]) -> dict[int, Path]:
    result: dict[int, Path] = {}
    for value in values:
        if "=" not in value:
            raise ValueError("--archive-artifact must use INDEX=PATH")
        index_text, path_text = value.split("=", 1)
        try:
            index = int(index_text, 10)
        except ValueError as exc:
            raise ValueError("--archive-artifact INDEX must be decimal") from exc
        if index < 0 or index > 0x7FFFFFFF or index in result or not path_text:
            raise ValueError("--archive-artifact index/path is invalid or duplicate")
        result[index] = Path(path_text)
    return result


def _write_no_replace(path: Path, packet: dict[str, Any]) -> None:
    encoded = (json.dumps(packet, indent=2) + "\n").encode("utf-8")
    descriptor: int | None = None
    created = False
    try:
        descriptor = os.open(path, os.O_WRONLY | os.O_CREAT | os.O_EXCL, 0o600)
        created = True
        with os.fdopen(descriptor, "wb", closefd=True) as stream:
            descriptor = None
            stream.write(encoded)
            stream.flush()
    except Exception:
        if descriptor is not None:
            os.close(descriptor)
        if created:
            try:
                path.unlink()
            except OSError:
                pass
        raise


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("--mapping", required=True, type=Path)
    parser.add_argument("--mapping-child", action="append", required=True, type=Path)
    parser.add_argument("--selection", required=True, type=Path)
    parser.add_argument("--observer-artifact", required=True, type=Path)
    parser.add_argument(
        "--archive-artifact",
        action="append",
        default=[],
        metavar="INDEX=PATH",
        help="Exact numbered NBZ artifact; repeat for every mounted volume.",
    )
    parser.add_argument("--output", required=True, type=Path)
    args = parser.parse_args()

    try:
        packet = build_bound_packet(
            args.mapping,
            args.selection,
            args.mapping_child,
            args.observer_artifact,
            _parse_archive_artifacts(args.archive_artifact),
        )
        _write_no_replace(args.output, packet)
    except (OSError, ValueError, TypeError, RuntimeError) as exc:
        print(f"original selection candidate rejected: {exc}", file=sys.stderr)
        return 2

    print(json.dumps(packet, indent=2))
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
