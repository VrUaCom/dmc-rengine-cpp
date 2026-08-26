#!/usr/bin/env python3
from __future__ import annotations

import importlib.util
import json
from pathlib import Path
import tempfile

SCRIPT = Path(__file__).with_name("verify_l2_runtime_mapping_packet.py")
SPEC = importlib.util.spec_from_file_location("runtime_mapping_packet", SCRIPT)
assert SPEC is not None and SPEC.loader is not None
mapping = importlib.util.module_from_spec(SPEC)
SPEC.loader.exec_module(mapping)


def make_receipt(rva: int, digit: str, *, pid: int = 4242) -> dict[str, object]:
    module_base = 0x7FF600000000
    window_sha = digit * 64
    return {
        "schema": "dmc-rengine.exe-process-window.v1",
        "artifact_sha256": mapping.PROTECTED_DISTRIBUTION_SHA256,
        "artifact_size": mapping.PROTECTED_DISTRIBUTION_SIZE,
        "image_path": "C:/Users/LocalUser/Games/DMC3/dmc3.exe",
        "preferred_image_base": f"0x{mapping.PREFERRED_IMAGE_BASE:X}",
        "pid": pid,
        "module_base": f"0x{module_base:X}",
        "rva": f"0x{rva:X}",
        "runtime_va": f"0x{module_base + rva:X}",
        "size": mapping.WINDOW_SIZE,
        "section": ".text",
        "window_sha256": window_sha,
        "expected_window_artifact_sha256": mapping.CANONICAL_ANALYSIS_SHA256,
        "expected_window_sha256": window_sha,
        "matches_expected_window": True,
    }


def write_receipt(root: Path, name: str, payload: dict[str, object]) -> Path:
    path = root / name
    path.write_text(json.dumps(payload) + "\n", encoding="utf-8")
    return path


def expect_rejected(paths: list[Path], contains: str) -> None:
    try:
        mapping.build_packet(paths)
    except ValueError as exc:
        assert contains in str(exc), (contains, str(exc))
    else:
        raise AssertionError(f"expected mapping packet rejection containing: {contains}")


def main() -> None:
    with tempfile.TemporaryDirectory() as temp_dir:
        root = Path(temp_dir)
        open_game = write_receipt(
            root,
            "open-game.json",
            make_receipt(0x0002FCA0, "1"),
        )
        registration = write_receipt(
            root,
            "registration.json",
            make_receipt(0x00326D20, "2"),
        )
        resolve = write_receipt(
            root,
            "resolve.json",
            make_receipt(0x00327430, "3"),
        )

        packet = mapping.build_packet([open_game, registration, resolve])
        assert packet["schema"] == "dmc-rengine.gdspaces-l2-runtime-mapping.v1"
        assert packet["status"] == "bounded_match"
        assert packet["anchor_count"] == 3
        assert packet["canonical_analysis_artifact_sha256"] == (
            mapping.CANONICAL_ANALYSIS_SHA256
        )
        assert packet["image_name"] == "dmc3.exe"
        assert "image_path" not in packet
        assert "LocalUser" not in json.dumps(packet)
        assert "original-process-selected-provider-identity" in packet["does_not_prove"]

        expect_rejected(
            [open_game, registration, registration],
            "duplicate mapping anchor",
        )

        wrong_pid_payload = make_receipt(0x00327430, "3", pid=9999)
        wrong_pid = write_receipt(root, "wrong-pid.json", wrong_pid_payload)
        expect_rejected(
            [open_game, registration, wrong_pid],
            "one process/module session",
        )

        wrong_authority_payload = make_receipt(0x00327430, "3")
        wrong_authority_payload["expected_window_artifact_sha256"] = "f" * 64
        wrong_authority = write_receipt(
            root, "wrong-authority.json", wrong_authority_payload
        )
        expect_rejected(
            [open_game, registration, wrong_authority],
            "canonical analysis artifact",
        )

        mismatch_payload = make_receipt(0x00327430, "3")
        mismatch_payload["matches_expected_window"] = False
        mismatch = write_receipt(root, "mismatch.json", mismatch_payload)
        expect_rejected(
            [open_game, registration, mismatch],
            "exact canonical window match",
        )

        raw_payload = make_receipt(0x00327430, "3")
        raw_payload["bytes_hex"] = "00" * mapping.WINDOW_SIZE
        raw = write_receipt(root, "raw.json", raw_payload)
        expect_rejected(
            [open_game, registration, raw],
            "metadata-only receipts",
        )

        missing_open_game = write_receipt(
            root,
            "final-open.json",
            make_receipt(0x00327800, "4"),
        )
        expect_rejected(
            [registration, resolve, missing_open_game],
            "OpenGameResource anchor",
        )

        output = root / "mapping.packet.json"
        mapping._write_no_replace(output, packet)
        first_bytes = output.read_bytes()
        try:
            mapping._write_no_replace(output, packet)
        except FileExistsError:
            pass
        else:
            raise AssertionError("no-replace writer overwrote an existing packet")
        assert output.read_bytes() == first_bytes

    print("runtime mapping packet guardrails: ok")


if __name__ == "__main__":
    main()
