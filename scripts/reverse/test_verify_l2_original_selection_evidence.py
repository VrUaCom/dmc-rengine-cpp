#!/usr/bin/env python3
from __future__ import annotations

import hashlib
import importlib.util
import json
from pathlib import Path
import struct
import tempfile

SCRIPT = Path(__file__).with_name("verify_l2_original_selection_evidence.py")
SPEC = importlib.util.spec_from_file_location("original_selection_evidence", SCRIPT)
assert SPEC is not None and SPEC.loader is not None
verify = importlib.util.module_from_spec(SPEC)
SPEC.loader.exec_module(verify)

PROCESS_CREATION_FILETIME = 133_801_234_567_890_123
TEXT_RVA = 0x1000
RAW_POINTER = 0x200


def write_json(path: Path, value: dict[str, object]) -> bytes:
    raw = (json.dumps(value, indent=2) + "\n").encode("utf-8")
    path.write_bytes(raw)
    return raw


def sha(data: bytes) -> str:
    return hashlib.sha256(data).hexdigest()


def make_canonical_exe(root: Path) -> tuple[Path, dict[int, str]]:
    mapping = verify.RUNTIME_MAPPING
    raw_size = 0x330000
    data = bytearray(RAW_POINTER + raw_size)
    data[0:2] = b"MZ"
    struct.pack_into("<I", data, 0x3C, 0x80)
    pe = 0x80
    data[pe : pe + 4] = b"PE\0\0"
    struct.pack_into("<H", data, pe + 4, 0x8664)
    struct.pack_into("<H", data, pe + 6, 1)
    struct.pack_into("<H", data, pe + 20, 0xF0)

    optional = pe + 24
    struct.pack_into("<H", data, optional, 0x20B)
    struct.pack_into("<Q", data, optional + 24, mapping.PREFERRED_IMAGE_BASE)
    struct.pack_into("<I", data, optional + 56, 0x340000)
    struct.pack_into("<I", data, optional + 60, RAW_POINTER)

    section = optional + 0xF0
    data[section : section + 8] = b".text\0\0\0"
    struct.pack_into("<I", data, section + 8, raw_size)
    struct.pack_into("<I", data, section + 12, TEXT_RVA)
    struct.pack_into("<I", data, section + 16, raw_size)
    struct.pack_into("<I", data, section + 20, RAW_POINTER)

    hashes: dict[int, str] = {}
    for index, rva in enumerate(sorted(mapping.ANCHORS), start=1):
        value = bytes([index]) * mapping.WINDOW_SIZE
        offset = RAW_POINTER + (rva - TEXT_RVA)
        data[offset : offset + mapping.WINDOW_SIZE] = value
        hashes[rva] = hashlib.sha256(value).hexdigest()

    path = root / "canonical.exe"
    path.write_bytes(data)
    mapping.CANONICAL_ANALYSIS_SIZE = len(data)
    mapping.CANONICAL_ANALYSIS_SHA256 = hashlib.sha256(data).hexdigest()
    return path, hashes


def mapping_child(rva: int, window_sha: str) -> dict[str, object]:
    mapping = verify.RUNTIME_MAPPING
    module_base = 0x7FF600000000
    return {
        "schema": "dmc-rengine.exe-process-window.v2",
        "artifact_sha256": verify.PROTECTED_SHA256,
        "artifact_size": verify.PROTECTED_SIZE,
        "image_path": "C:/Games/DMC3/dmc3.exe",
        "preferred_image_base": f"0x{mapping.PREFERRED_IMAGE_BASE:X}",
        "pid": 4242,
        "process_creation_filetime": PROCESS_CREATION_FILETIME,
        "module_base": f"0x{module_base:X}",
        "rva": f"0x{rva:X}",
        "runtime_va": f"0x{module_base + rva:X}",
        "size": mapping.WINDOW_SIZE,
        "section": ".text",
        "window_sha256": window_sha,
    }


def selection_candidate(
    mapping_sha: str,
    observer_sha: str,
    archive0_sha: str,
    archive0_size: int,
    archive2_sha: str,
    archive2_size: int,
) -> dict[str, object]:
    # Discovery found 0,1,2 before the first absent volume 3, but volume 1 is
    # deliberately absent from the mounted set to model a discovered archive
    # whose type-1 mount initialization failed. Resolver traversal must therefore
    # see 2 -> 0 and must not synthesize a volume-1 lookup miss.
    candidate = "GDataX360.afs/st001.pac"
    key = "gdatax360.afs\\st001.pac"
    return {
        "schema": verify.SELECTION_SCHEMA,
        "evidence_class": verify.SELECTION_EVIDENCE_CLASS,
        "promotion_eligible": False,
        "trusted_capture_bound": False,
        "legacy_schema_normalized": True,
        "executable_sha256": verify.PROTECTED_SHA256,
        "executable_size": verify.PROTECTED_SIZE,
        "runtime_mapping_packet_sha256": mapping_sha,
        "observer_id": "dmc-rengine-l2-observer",
        "observer_version": "synthetic-contract-test-v2",
        "observer_build_sha256": observer_sha,
        "trace_complete": True,
        "dropped_event_count": 0,
        "pid": 4242,
        "process_creation_filetime": PROCESS_CREATION_FILETIME,
        "module_base": "0x7FF600000000",
        "flags": 1,
        "request": "scr\\st001.pac",
        "basename": "st001.pac",
        "first_missing_archive_volume": 3,
        "archives": [
            {
                "volume_index": 0,
                "filename": "DMC3-0.nbz",
                "sha256": archive0_sha,
                "size": archive0_size,
            },
            {
                "volume_index": 2,
                "filename": "DMC3-2.nbz",
                "sha256": archive2_sha,
                "size": archive2_size,
            },
        ],
        "probes": [
            {
                "sequence_index": 0,
                "lookup_attempt_index": 0,
                "provider": "archive",
                "candidate": candidate,
                "provider_key": key,
                "archive_volume_index": 2,
                "outcome": "selected",
            }
        ],
        "selected": {
            "provider": "archive",
            "lookup_attempt_index": 0,
            "candidate": candidate,
            "provider_key": key,
            "archive_volume_index": 2,
            "archive_member_path": "GDataX360.afs/ST001.PAC",
            "physical_relative_path": "",
        },
        "proves": ["self-authored-selection-content-has-candidate-shape-only"],
        "does_not_prove": [
            "trusted-observer-execution-or-trace-origin",
            "original-process-selected-provider-identity",
            "promotion-eligibility",
        ],
    }


def rejected(
    mapping_path: Path,
    selection_path: Path,
    child_paths: list[Path],
    canonical_exe: Path,
    observer_path: Path,
    archives: dict[int, Path],
    contains: str,
) -> None:
    try:
        verify.build_bound_packet(
            mapping_path,
            selection_path,
            child_paths,
            canonical_exe,
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
        canonical_exe, canonical_hashes = make_canonical_exe(root)

        child_paths: list[Path] = []
        child_by_rva: dict[int, Path] = {}
        for rva in sorted(verify.RUNTIME_MAPPING.REQUIRED_ANCHORS):
            path = root / f"anchor-{rva:X}.json"
            write_json(path, mapping_child(rva, canonical_hashes[rva]))
            child_paths.append(path)
            child_by_rva[rva] = path

        mapping_value = verify.RUNTIME_MAPPING.build_packet(child_paths, canonical_exe)
        mapping_path = root / "mapping.json"
        mapping_raw = write_json(mapping_path, mapping_value)
        mapping_sha = sha(mapping_raw)

        observer_path = root / "observer.bin"
        observer_bytes = b"observer-build-v2"
        observer_path.write_bytes(observer_bytes)

        archive0 = root / "DMC3-0.nbz"
        archive2 = root / "DMC3-2.nbz"
        archive0_bytes = b"archive-zero"
        archive2_bytes = b"archive-two"
        archive0.write_bytes(archive0_bytes)
        archive2.write_bytes(archive2_bytes)
        archives = {0: archive0, 2: archive2}

        valid_selection = selection_candidate(
            mapping_sha,
            sha(observer_bytes),
            sha(archive0_bytes),
            len(archive0_bytes),
            sha(archive2_bytes),
            len(archive2_bytes),
        )
        selection_path = root / "selection.json"
        write_json(selection_path, valid_selection)

        packet = verify.build_bound_packet(
            mapping_path,
            selection_path,
            child_paths,
            canonical_exe,
            observer_path,
            archives,
        )
        assert packet["schema"] == verify.BOUND_SCHEMA
        assert packet["status"] == "bound_candidate"
        assert packet["promotion_eligible"] is False
        assert packet["trusted_capture_bound"] is False
        assert packet["runtime_mapping_packet_sha256"] == mapping_sha
        assert packet["process_creation_filetime"] == PROCESS_CREATION_FILETIME
        assert len(packet["runtime_mapping_child_receipts"]) == len(
            verify.RUNTIME_MAPPING.REQUIRED_ANCHORS
        )
        assert packet["canonical_window_authority"] == (
            "derived-directly-from-exact-canonical-exe-by-validator"
        )
        assert packet["observer"]["sha256"] == sha(observer_bytes)
        assert packet["first_missing_archive_volume"] == 3
        assert packet["mounted_archive_volume_indices"] == [0, 2]
        assert len(packet["archives"]) == 2
        assert packet["selected"]["archive_volume_index"] == 2
        assert (
            "mapping-and-selection-candidate-share-one-process-instance-identity"
            in packet["proves"]
        )

        # A discovered-but-failed volume is not a provider probe. Inserting
        # volume 1 between the successful mounted topology must fail closed.
        fabricated_mount = json.loads(json.dumps(valid_selection))
        fabricated_mount["probes"][0]["archive_volume_index"] = 1
        fabricated_mount["selected"]["archive_volume_index"] = 1
        fabricated_mount_path = root / "fabricated-mount.json"
        write_json(fabricated_mount_path, fabricated_mount)
        rejected(
            mapping_path,
            fabricated_mount_path,
            child_paths,
            canonical_exe,
            observer_path,
            archives,
            "archive mount precedence mismatch",
        )

        unsorted_mounts = json.loads(json.dumps(valid_selection))
        unsorted_mounts["archives"] = list(reversed(unsorted_mounts["archives"]))
        unsorted_mounts_path = root / "unsorted-mounts.json"
        write_json(unsorted_mounts_path, unsorted_mounts)
        rejected(
            mapping_path,
            unsorted_mounts_path,
            child_paths,
            canonical_exe,
            observer_path,
            archives,
            "sorted by ascending volume index",
        )

        wrong_process_instance = json.loads(json.dumps(valid_selection))
        wrong_process_instance["process_creation_filetime"] = PROCESS_CREATION_FILETIME + 1
        wrong_process_instance_path = root / "wrong-process-instance.json"
        write_json(wrong_process_instance_path, wrong_process_instance)
        rejected(
            mapping_path,
            wrong_process_instance_path,
            child_paths,
            canonical_exe,
            observer_path,
            archives,
            "process creation identities differ",
        )

        wrong_hash = dict(valid_selection)
        wrong_hash["runtime_mapping_packet_sha256"] = "0" * 64
        wrong_hash_path = root / "wrong-hash.json"
        write_json(wrong_hash_path, wrong_hash)
        rejected(
            mapping_path, wrong_hash_path, child_paths, canonical_exe, observer_path,
            archives, "does not hash-bind",
        )

        forged_promotion = dict(valid_selection)
        forged_promotion["promotion_eligible"] = True
        forged_promotion_path = root / "forged-promotion.json"
        write_json(forged_promotion_path, forged_promotion)
        rejected(
            mapping_path, forged_promotion_path, child_paths, canonical_exe,
            observer_path, archives, "may not predeclare promotion eligibility",
        )

        forged_trust = dict(valid_selection)
        forged_trust["trusted_capture_bound"] = True
        forged_trust_path = root / "forged-trust.json"
        write_json(forged_trust_path, forged_trust)
        rejected(
            mapping_path, forged_trust_path, child_paths, canonical_exe, observer_path,
            archives, "may not predeclare trusted capture",
        )

        wrong_observer = dict(valid_selection)
        wrong_observer["observer_build_sha256"] = "f" * 64
        wrong_observer_path = root / "wrong-observer.json"
        write_json(wrong_observer_path, wrong_observer)
        rejected(
            mapping_path, wrong_observer_path, child_paths, canonical_exe, observer_path,
            archives, "observer artifact SHA",
        )

        tampered_archive = root / "tampered-DMC3-2.nbz"
        tampered_archive.write_bytes(b"tampered")
        rejected(
            mapping_path,
            selection_path,
            child_paths,
            canonical_exe,
            observer_path,
            {0: archive0, 2: tampered_archive},
            "does not match claimed SHA/size",
        )

        rejected(
            mapping_path, selection_path, child_paths, canonical_exe, observer_path,
            {2: archive2}, "does not exactly match the successfully mounted selection volume set",
        )

        backend_failure = json.loads(json.dumps(valid_selection))
        backend_failure["probes"][0]["outcome"] = "provider_failure"
        backend_failure_path = root / "backend-failure.json"
        write_json(backend_failure_path, backend_failure)
        rejected(
            mapping_path, backend_failure_path, child_paths, canonical_exe, observer_path,
            archives, "provider/backend failure is fail-closed",
        )

        incomplete = dict(valid_selection)
        incomplete["trace_complete"] = False
        incomplete_path = root / "incomplete.json"
        write_json(incomplete_path, incomplete)
        rejected(
            mapping_path, incomplete_path, child_paths, canonical_exe, observer_path,
            archives, "not marked complete",
        )

        nested_raw = json.loads(json.dumps(valid_selection))
        nested_raw["selected"]["debug"] = {"bytes_hex": "00"}
        nested_raw_path = root / "nested-raw.json"
        write_json(nested_raw_path, nested_raw)
        rejected(
            mapping_path, nested_raw_path, child_paths, canonical_exe, observer_path,
            archives, "forbidden raw bytes_hex",
        )

        # R3 binder itself must inherit the R2B authority-laundering defense.
        target_rva = 0x00327430
        target_path = child_by_rva[target_rva]
        forged_child = mapping_child(target_rva, "f" * 64)
        forged_child["expected_window_artifact_sha256"] = (
            verify.RUNTIME_MAPPING.CANONICAL_ANALYSIS_SHA256
        )
        forged_child["expected_window_sha256"] = "f" * 64
        forged_child["matches_expected_window"] = True
        forged_child_path = root / "forged-child.json"
        write_json(forged_child_path, forged_child)
        forged_children = [
            forged_child_path if path == target_path else path for path in child_paths
        ]
        rejected(
            mapping_path, selection_path, forged_children, canonical_exe, observer_path,
            archives, "independently derived canonical window",
        )

        tampered_canonical = root / "tampered-canonical.exe"
        tampered_bytes = bytearray(canonical_exe.read_bytes())
        tampered_bytes[-1] ^= 0x01
        tampered_canonical.write_bytes(tampered_bytes)
        rejected(
            mapping_path, selection_path, child_paths, tampered_canonical, observer_path,
            archives, "canonical EXE SHA-256 mismatch",
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
            canonical_exe,
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

    print("original selection v2 canonical-authority/mounted-topology guardrails: ok")


if __name__ == "__main__":
    main()
