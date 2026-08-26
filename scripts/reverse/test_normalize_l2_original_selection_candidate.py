#!/usr/bin/env python3
from __future__ import annotations

import importlib.util
from pathlib import Path

SCRIPT = Path(__file__).with_name("normalize_l2_original_selection_candidate.py")
SPEC = importlib.util.spec_from_file_location("selection_candidate_normalizer", SCRIPT)
assert SPEC is not None and SPEC.loader is not None
normalizer = importlib.util.module_from_spec(SPEC)
SPEC.loader.exec_module(normalizer)


def legacy() -> dict[str, object]:
    return {
        "schema": normalizer.LEGACY_SCHEMA,
        "evidence_class": normalizer.LEGACY_EVIDENCE_CLASS,
        "executable_sha256": "1" * 64,
        "executable_size": 1,
        "runtime_mapping_packet_sha256": "2" * 64,
        "observer_id": "observer",
        "observer_version": "v1",
        "observer_build_sha256": "3" * 64,
        "trace_complete": True,
        "dropped_event_count": 0,
        "pid": 1,
        "module_base": "0x1",
        "flags": 1,
        "request": "scr\\st001.pac",
        "basename": "st001.pac",
        "first_missing_archive_volume": 0,
        "archives": [],
        "probes": [
            {
                "sequence_index": 0,
                "lookup_attempt_index": 6,
                "provider": "physical",
                "candidate": "GDataX360.afs/st001.pac",
                "provider_key": "GDataX360.afs\\st001.pac",
                "archive_volume_index": None,
                "outcome": "selected",
            }
        ],
        "selected": {
            "provider": "physical",
            "lookup_attempt_index": 6,
            "candidate": "GDataX360.afs/st001.pac",
            "provider_key": "GDataX360.afs\\st001.pac",
            "archive_volume_index": None,
            "archive_member_path": "",
            "physical_relative_path": "GDataX360.afs/st001.pac",
        },
        "proves": [
            "original-process-provider-traversal-prefix",
            "original-process-selected-resource-identity",
        ],
        "does_not_prove": ["retail-archive-collision-freedom"],
    }


def rejected(value: dict[str, object], contains: str) -> None:
    try:
        normalizer.normalize_candidate(value)
    except ValueError as exc:
        assert contains in str(exc), (contains, str(exc))
    else:
        raise AssertionError(f"expected rejection containing {contains!r}")


def main() -> None:
    value = normalizer.normalize_candidate(legacy())
    assert value["schema"] == normalizer.CANDIDATE_SCHEMA
    assert value["evidence_class"] == normalizer.CANDIDATE_EVIDENCE_CLASS
    assert value["promotion_eligible"] is False
    assert value["trusted_capture_bound"] is False
    assert value["legacy_schema_normalized"] is True
    assert "original-process-selected-resource-identity" not in value["proves"]
    assert "trusted-observer-execution-or-trace-origin" in value["does_not_prove"]
    assert "original-process-selected-provider-identity" in value["does_not_prove"]
    assert set(value["selected"]) == normalizer.SELECTED_KEYS
    assert set(value["probes"][0]) == normalizer.PROBE_KEYS

    wrong_schema = legacy()
    wrong_schema["schema"] = "other"
    rejected(wrong_schema, "schema mismatch")

    wrong_class = legacy()
    wrong_class["evidence_class"] = "other"
    rejected(wrong_class, "evidence class mismatch")

    forged_trust = legacy()
    forged_trust["promotion_eligible"] = True
    rejected(forged_trust, "unsupported field(s): promotion_eligible")

    forged_top_level = legacy()
    forged_top_level["trusted_origin"] = True
    rejected(forged_top_level, "unsupported field(s): trusted_origin")

    nested_extra = legacy()
    selected = dict(nested_extra["selected"])
    selected["trusted_origin"] = True
    nested_extra["selected"] = selected
    rejected(nested_extra, "legacy selected contains unsupported field(s): trusted_origin")

    nested_raw = legacy()
    selected = dict(nested_raw["selected"])
    selected["debug"] = {"bytes_hex": "00"}
    nested_raw["selected"] = selected
    rejected(nested_raw, "forbidden raw bytes_hex")

    missing = legacy()
    del missing["observer_build_sha256"]
    rejected(missing, "missing required field observer_build_sha256")

    print("selection candidate normalizer guardrails: ok")


if __name__ == "__main__":
    main()
