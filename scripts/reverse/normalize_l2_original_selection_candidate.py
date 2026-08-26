#!/usr/bin/env python3
"""Normalize the legacy C++ L2 selection JSON into a non-promotable candidate.

The existing C++ serializer predates the trusted-origin review correction and uses
legacy labels that sound stronger than the evidence actually is. This adapter is
intentionally narrow: it accepts only that exact legacy schema/evidence pair,
rejects unknown evidence-surface fields, and builds a sanitized content candidate.
It does not validate runtime origin and cannot make the candidate promotion eligible.
"""

from __future__ import annotations

import argparse
import json
import os
from pathlib import Path
import sys
from typing import Any

LEGACY_SCHEMA = "dmc-rengine.gdspaces-l2-original-selection.v1"
LEGACY_EVIDENCE_CLASS = "original-process-observation"
CANDIDATE_SCHEMA = "dmc-rengine.gdspaces-l2-original-selection-candidate.v1"
CANDIDATE_EVIDENCE_CLASS = "original-process-observation-candidate"

TOP_LEVEL_KEYS = {
    "schema",
    "evidence_class",
    "executable_sha256",
    "executable_size",
    "runtime_mapping_packet_sha256",
    "observer_id",
    "observer_version",
    "observer_build_sha256",
    "trace_complete",
    "dropped_event_count",
    "pid",
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
        raise ValueError(f"legacy {context} must be an object")
    extra = set(value) - allowed
    if extra:
        raise ValueError(
            f"legacy {context} contains unsupported field(s): {', '.join(sorted(extra))}"
        )
    return value


def _sanitize_archives(value: Any) -> list[dict[str, Any]]:
    if not isinstance(value, list):
        raise ValueError("legacy archives must be an array")
    sanitized: list[dict[str, Any]] = []
    for index, item in enumerate(value):
        entry = _exact_keys(item, ARCHIVE_KEYS, f"archive[{index}]")
        sanitized.append({key: entry[key] for key in ARCHIVE_KEYS if key in entry})
    return sanitized


def _sanitize_probes(value: Any) -> list[dict[str, Any]]:
    if not isinstance(value, list):
        raise ValueError("legacy probes must be an array")
    sanitized: list[dict[str, Any]] = []
    for index, item in enumerate(value):
        entry = _exact_keys(item, PROBE_KEYS, f"probe[{index}]")
        sanitized.append({key: entry[key] for key in PROBE_KEYS if key in entry})
    return sanitized


def _sanitize_selected(value: Any) -> dict[str, Any]:
    entry = _exact_keys(value, SELECTED_KEYS, "selected")
    return {key: entry[key] for key in SELECTED_KEYS if key in entry}


def normalize_candidate(value: dict[str, Any]) -> dict[str, Any]:
    _exact_keys(value, TOP_LEVEL_KEYS, "selection")
    if value.get("schema") != LEGACY_SCHEMA:
        raise ValueError("legacy selection schema mismatch")
    if value.get("evidence_class") != LEGACY_EVIDENCE_CLASS:
        raise ValueError("legacy selection evidence class mismatch")
    if _contains_forbidden_key(value, "bytes_hex"):
        raise ValueError("legacy selection contains forbidden raw bytes_hex")

    required = (
        "executable_sha256",
        "executable_size",
        "runtime_mapping_packet_sha256",
        "observer_id",
        "observer_version",
        "observer_build_sha256",
        "trace_complete",
        "dropped_event_count",
        "pid",
        "module_base",
        "flags",
        "request",
        "basename",
        "first_missing_archive_volume",
        "archives",
        "probes",
        "selected",
    )
    for field in required:
        if field not in value:
            raise ValueError(f"legacy selection missing required field {field}")

    normalized: dict[str, Any] = {
        "schema": CANDIDATE_SCHEMA,
        "evidence_class": CANDIDATE_EVIDENCE_CLASS,
        "promotion_eligible": False,
        "trusted_capture_bound": False,
        "legacy_schema_normalized": True,
        "executable_sha256": value["executable_sha256"],
        "executable_size": value["executable_size"],
        "runtime_mapping_packet_sha256": value["runtime_mapping_packet_sha256"],
        "observer_id": value["observer_id"],
        "observer_version": value["observer_version"],
        "observer_build_sha256": value["observer_build_sha256"],
        "trace_complete": value["trace_complete"],
        "dropped_event_count": value["dropped_event_count"],
        "pid": value["pid"],
        "module_base": value["module_base"],
        "flags": value["flags"],
        "request": value["request"],
        "basename": value["basename"],
        "first_missing_archive_volume": value["first_missing_archive_volume"],
        "archives": _sanitize_archives(value["archives"]),
        "probes": _sanitize_probes(value["probes"]),
        "selected": _sanitize_selected(value["selected"]),
        "proves": ["self-authored-selection-content-has-candidate-shape-only"],
    }

    old_nonclaims = value.get("does_not_prove")
    nonclaims: list[str] = []
    if isinstance(old_nonclaims, list):
        nonclaims.extend(item for item in old_nonclaims if isinstance(item, str))
    for item in (
        "trusted-observer-execution-or-trace-origin",
        "original-process-selected-provider-identity",
        "promotion-eligibility",
    ):
        if item not in nonclaims:
            nonclaims.append(item)
    normalized["does_not_prove"] = nonclaims
    return normalized


def _read_json(path: Path) -> dict[str, Any]:
    try:
        value = json.loads(path.read_text(encoding="utf-8"))
    except (OSError, UnicodeDecodeError, json.JSONDecodeError) as exc:
        raise ValueError(f"could not read legacy selection JSON: {exc}") from exc
    if not isinstance(value, dict):
        raise ValueError("legacy selection must contain one JSON object")
    return value


def _write_no_replace(path: Path, value: dict[str, Any]) -> None:
    encoded = (json.dumps(value, indent=2) + "\n").encode("utf-8")
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
    parser.add_argument("--input", required=True, type=Path)
    parser.add_argument("--output", required=True, type=Path)
    args = parser.parse_args()

    try:
        normalized = normalize_candidate(_read_json(args.input))
        _write_no_replace(args.output, normalized)
    except (OSError, ValueError, TypeError) as exc:
        print(f"selection candidate normalization rejected: {exc}", file=sys.stderr)
        return 2

    print(json.dumps(normalized, indent=2))
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
