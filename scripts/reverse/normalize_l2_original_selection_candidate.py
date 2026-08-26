#!/usr/bin/env python3
"""Normalize the legacy C++ L2 selection JSON into a non-promotable candidate.

The existing C++ serializer predates the trusted-origin review correction and uses
legacy labels that sound stronger than the evidence actually is. This adapter is
intentionally narrow: it accepts only that exact legacy schema/evidence pair and
rewrites the public claim surface into an explicit content candidate. It does not
validate runtime origin and cannot make the candidate promotion eligible.
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


def _contains_forbidden_key(value: Any, key: str) -> bool:
    if isinstance(value, dict):
        if key in value:
            return True
        return any(_contains_forbidden_key(child, key) for child in value.values())
    if isinstance(value, list):
        return any(_contains_forbidden_key(child, key) for child in value)
    return False


def normalize_candidate(value: dict[str, Any]) -> dict[str, Any]:
    if value.get("schema") != LEGACY_SCHEMA:
        raise ValueError("legacy selection schema mismatch")
    if value.get("evidence_class") != LEGACY_EVIDENCE_CLASS:
        raise ValueError("legacy selection evidence class mismatch")
    if _contains_forbidden_key(value, "bytes_hex"):
        raise ValueError("legacy selection contains forbidden raw bytes_hex")
    if "promotion_eligible" in value or "trusted_capture_bound" in value:
        raise ValueError("legacy self-authored selection may not predeclare promotion/trust")

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

    normalized = dict(value)
    normalized["schema"] = CANDIDATE_SCHEMA
    normalized["evidence_class"] = CANDIDATE_EVIDENCE_CLASS
    normalized["promotion_eligible"] = False
    normalized["trusted_capture_bound"] = False
    normalized["legacy_schema_normalized"] = True
    normalized["proves"] = [
        "self-authored-selection-content-has-candidate-shape-only",
    ]

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
