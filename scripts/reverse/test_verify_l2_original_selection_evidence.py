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


def sha(data: bytes) -> str:
    return hashlib.sha256(data).hexdigest()


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


def selection_receipt(
    mapping_sha: str,
    observer_sha: str,
    archive0_sha: str,
    archive0_size: int,
    archive1_sha: str,
    archive1_size: int,
) -> dict[str, object]:
    candidate = "GDataX360.afs/st001.pac"
    key = "gdatax360.afs\\st001.pac"
    return {
        "schema": verify.SELECTION_SCHEMA,
        "evidence_class": verify.SELECTION_EVIDENCE_CLASS,
        "executable_sha256": verify.PROTECTED_SHA256,
        "executable_size": verify.PROTECTED_SIZE,
        "runtime_mapping_packet_sha256": mapping_sha,
        "observer_id": "dmc-rengine-l2-observer",
        "observer_version": "synthetic-contract-test",
        "observer_build_sha256": observer_sha,
        "trace_complete": True,
        "dropped_event_count": 0,
        "pid": 4242,
        "module_base": "0x7FF600000000",
        "flags": 1,
        "request": "scr\\st001.pac",
        "basename": "st001.pac",
        "first_missing_archive_volume": 2,
        "archives": [
            {
                "volume_index": 0,
                "filename": "DMC3-0.nbz",
                "sha256": archive0_sha,
                "size": archive0_size,
            },
            {
                "volume_index": 1,
                "filename": "DMC3-1.nbz",
                "sha256": archive1_sha,
                "size": archive1_size,
            },
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
    observer_path: Path,
    archives: dict[int, Path],
    contains: str,
) -> None:
    try:
        verify.build_bound_packet(
            mapping_path,
            selection_path,
            child_paths,
            observer_path,
            archives,
        )
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
        mapping_sha = sha(mapping_raw)

        observer_path = root / "observer.bin"
        observer_bytes = b"observer-build-v1"
        observer_path.write_bytes(observer_bytes)

        archive0 = root / "DMC3-0.nbz"
        archive1 = root / "DMC3-1.nbz"
        archive0_bytes = b"archive-zero"
        archive1_bytes = b"archive-one"
        archive0.write_bytes(archive0_bytes)
        archive1.write_bytes(archive1_bytes)
        archives = {0: archive0, 1: archive1}

        valid_selection = selection_receipt(
            mapping_sha,
            sha(observer_bytes),
            sha(archive0_bytes),
            len(archive0_bytes),
            sha(archive1_bytes),
            len(archive1_bytes),
        )
        selection_path = root / "selection.json"
        write_json(selection_path, valid_selection)

        packet = verify.build_bound_packet(
            mapping_path,
            selection_path,
            child_paths,
            observer_path,
            archives,
        )
        assert packet["status"] == "bound_candidate"
        assert packet["promotion_eligible"] is False
        assert packet["trusted_capture_bound"] is False
        assert packet["runtime_mapping_packet_sha256"] == mapping_sha
        assert len(packet["runtime_mapping_child_receipts"]) == 3
        assert packet["observer"]["sha256"] == sha(observer_bytes)
        assert len(packet["archives"]) == 2
        assert packet["selected"]["archive_volume_index"] == 1

        wrong_hash = dict(valid_selection)
        wrong_hash["runtime_mapping_packet_sha256"] = "0" * 64
        wrong_hash_path = root / "wrong-hash.json"
        write_json(wrong_hash_path, wrong_hash)
        rejected(
            mapping_path,
            wrong_hash_path,
            child_paths,
            observer_path,
            archives,
            "does not hash-bind",
        )

        wrong_observer = dict(valid_selection)
        wrong_observer["observer_build_sha256"] = "f" * 64
        wrong_observer_path = root / "wrong-observer.json"
        write_json(wrong_observer_path, wrong_observer)
        rejected(
            mapping_path,
            wrong_observer_path,
            child_paths,
            observer_path,
            archives,
            "observer artifact SHA",
        )

        tampered_archive = root / "tampered-DMC3-1.nbz"
        tampered_archive.write_bytes(b"tampered")
        rejected(
            mapping_path,
            selection_path,
            child_paths,
            observer_path,
            {0: archive0, 1: tampered_archive},
            "does not match claimed SHA/size",
        )

        rejected(
            mapping_path,
            selection_path,
            child_paths,
            observer_path,
            {1: archive1},
            "does not exactly match selection volume set",
        )

        backend_failure = json.loads(json.dumps(valid_selection))
        backend_failure["probes"][0]["outcome"] = "provider_failure"
        backend_failure_path = root / "backend-failure.json"
        write_json(backend_failure_path, backend_failure)
        rejected(
            mapping_path,
            backend_failure_path,
            child_paths,
            observer_path,
            archives,
            "provider/backend failure is fail-closed",
        )

        incomplete = dict(valid_selection)
        incomplete["trace_complete"] = False
        incomplete_path = root / "incomplete.json"
        write_json(incomplete_path, incomplete)
        rejected(
            mapping_path,
            incomplete_path,
            child_paths,
            observer_path,
            archives,
            "not marked complete",
        )

        nested_raw = json.loads(json.dumps(valid_selection))
        nested_raw["selected"]["debug"] = {"bytes_hex": "00"}
        nested_raw_path = root / "nested-raw.json"
        write_json(nested_raw_path, nested_raw)
        rejected(
            mapping_path,
            nested_raw_path,
            child_paths,
            observer_path,
            archives,
            "forbidden raw bytes_hex",
        )

        forged_mapping = json.loads(json.dumps(mapping_value))
        forged_mapping["anchors"][0]["window_sha256"] = "f" * 64
        forged_mapping_path = root / "forged-mapping.json"
        forged_mapping_raw = write_json(forged_mapping_path, forged_mapping)
        forged_selection = dict(valid_selection)
        forged_selection["runtime_mapping_packet_sha256"] = sha(forged_mapping_raw)
        forged_selection_path = root / "forged-selection.json"
        write_json(forged_selection_path, forged_selection)
        rejected(
            forged_mapping_path,
            forged_selection_path,
            child_paths,
            observer_path,
            archives,
            "does not exactly match reconstruction",
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
