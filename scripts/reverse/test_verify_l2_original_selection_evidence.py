#!/usr/bin/env python3
from __future__ import annotations

import hashlib
import importlib.util
import json
from pathlib import Path
import tempfile

SCRIPT = Path(__file__).with_name("verify_l2_original_selection_evidence.py")
SPEC = importlib.util.spec_from_file_location("original_selection_evidence", SCRIPT)
assert SPEC is not None and SPEC.loader is not None
verify = importlib.util.module_from_spec(SPEC)
SPEC.loader.exec_module(verify)


def write_json(path: Path, value: dict[str, object]) -> bytes:
    raw = (json.dumps(value, indent=2) + "\n").encode("utf-8")
    path.write_bytes(raw)
    return raw


def mapping_child(rva: int, digit: str) -> dict[str, object]:
    module_base = 0x7FF600000000
    window_sha = digit * 64
    return {
        "schema": "dmc-rengine.exe-process-window.v1",
        "artifact_sha256": verify.PROTECTED_SHA256,
        "artifact_size": verify.PROTECTED_SIZE,
        "image_path": "C:/Games/DMC3/dmc3.exe",
        "preferred_image_base": f"0x{verify.RUNTIME_MAPPING.PREFERRED_IMAGE_BASE:X}",
        "pid": 4242,
        "module_base": f"0x{module_base:X}",
        "rva": f"0x{rva:X}",
        "runtime_va": f"0x{module_base + rva:X}",
        "size": verify.RUNTIME_MAPPING.WINDOW_SIZE,
        "section": ".text",
        "window_sha256": window_sha,
        "expected_window_artifact_sha256": verify.RUNTIME_MAPPING.CANONICAL_ANALYSIS_SHA256,
        "expected_window_sha256": window_sha,
        "matches_expected_window": True,
    }


def selection_receipt(mapping_sha: str) -> dict[str, object]:
    candidate = "GDataX360.afs/st001.pac"
    key = "gdatax360.afs\\st001.pac"
    return {
        "schema": "dmc-rengine.gdspaces-l2-original-selection.v1",
        "evidence_class": "original-process-observation",
        "executable_sha256": verify.PROTECTED_SHA256,
        "executable_size": verify.PROTECTED_SIZE,
        "runtime_mapping_packet_sha256": mapping_sha,
        "observer_id": "dmc-rengine-l2-observer",
        "observer_version": "synthetic-contract-test",
        "observer_build_sha256": "c" * 64,
        "trace_complete": True,
        "dropped_event_count": 0,
        "pid": 4242,
        "module_base": "0x7FF600000000",
        "flags": 1,
        "request": "scr\\st001.pac",
        "basename": "st001.pac",
        "first_missing_archive_volume": 2,
        "archives": [
            {"volume_index": 0, "filename": "DMC3-0.nbz", "sha256": "a" * 64, "size": 1000},
            {"volume_index": 1, "filename": "DMC3-1.nbz", "sha256": "b" * 64, "size": 1100},
        ],
        "probes": [
            {
                "sequence_index": 0,
                "lookup_attempt_index": 0,
                "provider": "archive",
                "candidate": candidate,
                "provider_key": key,
                "archive_volume_index": 1,
                "outcome": "selected",
            }
        ],
        "selected": {
            "provider": "archive",
            "lookup_attempt_index": 0,
            "candidate": candidate,
            "provider_key": key,
            "archive_volume_index": 1,
            "archive_member_path": "GDataX360.afs/ST001.PAC",
            "physical_relative_path": "",
        },
    }


def rejected(
    mapping_path: Path,
    selection_path: Path,
    child_paths: list[Path],
    contains: str,
) -> None:
    try:
        verify.build_bound_packet(mapping_path, selection_path, child_paths)
    except ValueError as exc:
        assert contains in str(exc), (contains, str(exc))
    else:
        raise AssertionError(f"expected rejection containing {contains!r}")


def main() -> None:
    with tempfile.TemporaryDirectory() as tmp:
        root = Path(tmp)

        child_paths: list[Path] = []
        for name, rva, digit in (
            ("open-game.json", 0x0002FCA0, "1"),
            ("resolve.json", 0x00327430, "2"),
            ("final-open.json", 0x00327800, "3"),
        ):
            path = root / name
            write_json(path, mapping_child(rva, digit))
            child_paths.append(path)

        mapping_value = verify.RUNTIME_MAPPING.build_packet(child_paths)
        mapping_path = root / "mapping.json"
        mapping_raw = write_json(mapping_path, mapping_value)
        mapping_sha = hashlib.sha256(mapping_raw).hexdigest()

        selection_path = root / "selection.json"
        valid_selection = selection_receipt(mapping_sha)
        write_json(selection_path, valid_selection)

        packet = verify.build_bound_packet(
            mapping_path, selection_path, child_paths
        )
        assert packet["status"] == "bound"
        assert packet["runtime_mapping_packet_sha256"] == mapping_sha
        assert len(packet["runtime_mapping_child_receipts"]) == 3
        assert packet["observer_build_sha256"] == "c" * 64
        assert packet["trace_complete"] is True
        assert packet["dropped_event_count"] == 0
        assert packet["request"] == "scr\\st001.pac"
        assert packet["selected"]["archive_volume_index"] == 1

        wrong_hash = selection_receipt("0" * 64)
        wrong_hash_path = root / "wrong-hash.json"
        write_json(wrong_hash_path, wrong_hash)
        rejected(mapping_path, wrong_hash_path, child_paths, "does not hash-bind")

        wrong_pid = selection_receipt(mapping_sha)
        wrong_pid["pid"] = 9999
        wrong_pid_path = root / "wrong-pid.json"
        write_json(wrong_pid_path, wrong_pid)
        rejected(mapping_path, wrong_pid_path, child_paths, "pid differ")

        skipped_volume = selection_receipt(mapping_sha)
        skipped_volume["probes"][0]["archive_volume_index"] = 0
        skipped_path = root / "skipped.json"
        write_json(skipped_path, skipped_volume)
        rejected(mapping_path, skipped_path, child_paths, "volume precedence")

        incomplete = selection_receipt(mapping_sha)
        incomplete["trace_complete"] = False
        incomplete_path = root / "incomplete.json"
        write_json(incomplete_path, incomplete)
        rejected(mapping_path, incomplete_path, child_paths, "not marked complete")

        dropped = selection_receipt(mapping_sha)
        dropped["dropped_event_count"] = 1
        dropped_path = root / "dropped.json"
        write_json(dropped_path, dropped)
        rejected(mapping_path, dropped_path, child_paths, "dropped events")

        bad_observer = selection_receipt(mapping_sha)
        bad_observer["observer_build_sha256"] = "not-a-sha"
        bad_observer_path = root / "bad-observer.json"
        write_json(bad_observer_path, bad_observer)
        rejected(mapping_path, bad_observer_path, child_paths, "observer build SHA")

        malformed_selected = selection_receipt(mapping_sha)
        malformed_selected["selected"] = {}
        malformed_selected_path = root / "malformed-selected.json"
        write_json(malformed_selected_path, malformed_selected)
        rejected(
            mapping_path,
            malformed_selected_path,
            child_paths,
            "terminal probe field",
        )

        nested_raw = selection_receipt(mapping_sha)
        nested_raw["selected"]["debug"] = {"bytes_hex": "00"}
        nested_raw_path = root / "nested-raw.json"
        write_json(nested_raw_path, nested_raw)
        rejected(
            mapping_path,
            nested_raw_path,
            child_paths,
            "forbidden raw bytes_hex",
        )

        # A structurally plausible mapping packet cannot be hand-forged: it must
        # exactly reconstruct from the supplied process-window child receipts.
        forged_mapping = dict(mapping_value)
        forged_anchors = [dict(anchor) for anchor in mapping_value["anchors"]]
        forged_anchors[0]["window_sha256"] = "f" * 64
        forged_mapping["anchors"] = forged_anchors
        forged_mapping_path = root / "forged-mapping.json"
        forged_mapping_raw = write_json(forged_mapping_path, forged_mapping)
        forged_selection = selection_receipt(
            hashlib.sha256(forged_mapping_raw).hexdigest()
        )
        forged_selection_path = root / "forged-selection.json"
        write_json(forged_selection_path, forged_selection)
        rejected(
            forged_mapping_path,
            forged_selection_path,
            child_paths,
            "does not exactly match reconstruction",
        )

        nested_mapping_raw = dict(mapping_value)
        nested_anchors = [dict(anchor) for anchor in mapping_value["anchors"]]
        nested_anchors[0]["debug"] = {"bytes_hex": "00"}
        nested_mapping_raw["anchors"] = nested_anchors
        nested_mapping_path = root / "nested-mapping-raw.json"
        nested_mapping_bytes = write_json(nested_mapping_path, nested_mapping_raw)
        nested_mapping_selection = selection_receipt(
            hashlib.sha256(nested_mapping_bytes).hexdigest()
        )
        nested_mapping_selection_path = root / "nested-mapping-selection.json"
        write_json(nested_mapping_selection_path, nested_mapping_selection)
        rejected(
            nested_mapping_path,
            nested_mapping_selection_path,
            child_paths,
            "forbidden raw bytes_hex",
        )

        nested_child = dict(mapping_child(0x00327800, "3"))
        nested_child["debug"] = {"bytes_hex": "00"}
        nested_child_path = root / "nested-child.json"
        write_json(nested_child_path, nested_child)
        bad_children = [child_paths[0], child_paths[1], nested_child_path]
        rejected(
            mapping_path,
            selection_path,
            bad_children,
            "forbidden raw bytes_hex",
        )

        output = root / "bound.json"
        verify._write_no_replace(output, packet)
        first = output.read_bytes()
        try:
            verify._write_no_replace(output, packet)
        except FileExistsError:
            pass
        else:
            raise AssertionError("no-replace writer overwrote an existing packet")
        assert output.read_bytes() == first

    print("original selection evidence guardrails: ok")


if __name__ == "__main__":
    main()
