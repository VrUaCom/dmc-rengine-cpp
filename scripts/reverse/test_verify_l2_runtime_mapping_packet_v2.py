#!/usr/bin/env python3
from __future__ import annotations

import hashlib
import importlib.util
import json
from pathlib import Path
import struct
import tempfile

SCRIPT = Path(__file__).with_name("verify_l2_runtime_mapping_packet_v2.py")
SPEC = importlib.util.spec_from_file_location("runtime_mapping_packet_v2", SCRIPT)
assert SPEC is not None and SPEC.loader is not None
mapping = importlib.util.module_from_spec(SPEC)
SPEC.loader.exec_module(mapping)

CREATION = 133_800_000_000_000_000
TEXT_RVA = 0x1000
RAW_POINTER = 0x200


def make_canonical_exe(root: Path) -> tuple[Path, dict[int, str]]:
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


def make_receipt(
    rva: int,
    window_sha: str,
    *,
    pid: int = 4242,
    creation: int = CREATION,
    schema: str = "dmc-rengine.exe-process-window.v2",
    include_expectation: bool = False,
) -> dict[str, object]:
    module_base = 0x7FF600000000
    payload: dict[str, object] = {
        "schema": schema,
        "artifact_sha256": mapping.PROTECTED_DISTRIBUTION_SHA256,
        "artifact_size": mapping.PROTECTED_DISTRIBUTION_SIZE,
        "image_path": "C:/Users/LocalUser/Games/DMC3/dmc3.exe",
        "preferred_image_base": f"0x{mapping.PREFERRED_IMAGE_BASE:X}",
        "pid": pid,
        "process_creation_filetime": creation,
        "module_base": f"0x{module_base:X}",
        "rva": f"0x{rva:X}",
        "runtime_va": f"0x{module_base + rva:X}",
        "size": mapping.WINDOW_SIZE,
        "section": ".text",
        "window_sha256": window_sha,
    }
    if include_expectation:
        payload.update(
            {
                "expected_window_artifact_sha256": mapping.CANONICAL_ANALYSIS_SHA256,
                "expected_window_sha256": window_sha,
                "matches_expected_window": True,
            }
        )
    return payload


def write_receipt(root: Path, name: str, payload: dict[str, object]) -> Path:
    path = root / name
    path.write_text(json.dumps(payload) + "\n", encoding="utf-8")
    return path


def expect_rejected(
    paths: list[Path], canonical_exe: Path, contains: str
) -> None:
    try:
        mapping.build_packet(paths, canonical_exe)
    except ValueError as exc:
        assert contains in str(exc), (contains, str(exc))
    else:
        raise AssertionError(f"expected mapping packet rejection containing: {contains}")


def required_receipts(
    root: Path, hashes: dict[int, str]
) -> tuple[list[Path], dict[int, Path]]:
    paths: list[Path] = []
    by_rva: dict[int, Path] = {}
    for rva in sorted(mapping.REQUIRED_ANCHORS):
        path = write_receipt(
            root,
            f"anchor-{rva:X}.json",
            make_receipt(rva, hashes[rva]),
        )
        paths.append(path)
        by_rva[rva] = path
    return paths, by_rva


def replace_path(paths: list[Path], old: Path, new: Path) -> list[Path]:
    return [new if path == old else path for path in paths]


def main() -> None:
    with tempfile.TemporaryDirectory() as temp_dir:
        root = Path(temp_dir)
        canonical_exe, hashes = make_canonical_exe(root)
        paths, by_rva = required_receipts(root, hashes)

        packet = mapping.build_packet(paths, canonical_exe)
        assert packet["schema"] == "dmc-rengine.gdspaces-l2-runtime-mapping.v2"
        assert packet["status"] == "bounded_process_instance_match"
        assert packet["anchor_count"] == len(mapping.REQUIRED_ANCHORS)
        assert packet["pid"] == 4242
        assert packet["process_creation_filetime"] == CREATION
        assert packet["image_name"] == "dmc3.exe"
        assert packet["canonical_analysis_artifact_sha256"] == (
            mapping.CANONICAL_ANALYSIS_SHA256
        )
        assert packet["canonical_window_authority"] == (
            "derived-directly-from-exact-canonical-exe-by-validator"
        )
        assert "image_path" not in packet
        assert "LocalUser" not in json.dumps(packet)
        for anchor in packet["anchors"]:
            assert anchor["window_sha256"] == anchor["canonical_window_sha256"]

        target_rva = 0x00327430
        target = by_rva[target_rva]

        legacy = write_receipt(
            root,
            "legacy-v1.json",
            make_receipt(
                target_rva,
                hashes[target_rva],
                schema="dmc-rengine.exe-process-window.v1",
            ),
        )
        expect_rejected(
            replace_path(paths, target, legacy),
            canonical_exe,
            "legacy v1 is not promotion authority",
        )

        wrong_creation = write_receipt(
            root,
            "wrong-creation.json",
            make_receipt(
                target_rva, hashes[target_rva], creation=CREATION + 1
            ),
        )
        expect_rejected(
            replace_path(paths, target, wrong_creation),
            canonical_exe,
            "one exact process instance/module session",
        )

        zero_creation_payload = make_receipt(target_rva, hashes[target_rva])
        zero_creation_payload["process_creation_filetime"] = 0
        zero_creation = write_receipt(root, "zero-creation.json", zero_creation_payload)
        expect_rejected(
            replace_path(paths, target, zero_creation),
            canonical_exe,
            "non-zero unsigned 64-bit integer",
        )

        # Core authority-laundering regression: a live child cannot choose an
        # arbitrary hash and declare that same value as its canonical expected
        # hash. The validator derives the real canonical window independently.
        forged_hash = "f" * 64
        forged = make_receipt(
            target_rva, forged_hash, include_expectation=True
        )
        forged["expected_window_sha256"] = forged_hash
        forged["matches_expected_window"] = True
        forged_path = write_receipt(root, "forged-expected.json", forged)
        expect_rejected(
            replace_path(paths, target, forged_path),
            canonical_exe,
            "independently derived canonical window",
        )

        # Even a correct live window cannot carry a contradictory diagnostic
        # expectation tuple and still pass the v2 gate.
        wrong_diagnostic = make_receipt(
            target_rva, hashes[target_rva], include_expectation=True
        )
        wrong_diagnostic["expected_window_sha256"] = "e" * 64
        wrong_diagnostic_path = write_receipt(
            root, "wrong-diagnostic.json", wrong_diagnostic
        )
        expect_rejected(
            replace_path(paths, target, wrong_diagnostic_path),
            canonical_exe,
            "diagnostic expectation does not match independently derived canonical evidence",
        )

        raw_payload = make_receipt(target_rva, hashes[target_rva])
        raw_payload["bytes_hex"] = "00" * mapping.WINDOW_SIZE
        raw = write_receipt(root, "raw.json", raw_payload)
        expect_rejected(
            replace_path(paths, target, raw),
            canonical_exe,
            "metadata-only receipts",
        )

        missing_required = [path for path in paths if path != target]
        expect_rejected(
            missing_required,
            canonical_exe,
            "at least",
        )

        tampered_canonical = root / "tampered-canonical.exe"
        tampered = bytearray(canonical_exe.read_bytes())
        tampered[-1] ^= 0x01
        tampered_canonical.write_bytes(tampered)
        expect_rejected(
            paths,
            tampered_canonical,
            "canonical EXE SHA-256 mismatch",
        )

        output = root / "mapping-v2.packet.json"
        mapping._write_no_replace(output, packet)
        first_bytes = output.read_bytes()
        try:
            mapping._write_no_replace(output, packet)
        except FileExistsError:
            pass
        else:
            raise AssertionError("no-replace writer overwrote an existing packet")
        assert output.read_bytes() == first_bytes

    print("runtime mapping v2 canonical-artifact/process-instance guardrails: ok")


if __name__ == "__main__":
    main()
