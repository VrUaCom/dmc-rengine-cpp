#!/usr/bin/env python3
from __future__ import annotations

import importlib.util
import json
from pathlib import Path
import tempfile

SCRIPT = Path(__file__).with_name("verify_l2_runtime_mapping_packet_v2.py")
SPEC = importlib.util.spec_from_file_location("runtime_mapping_packet_v2", SCRIPT)
assert SPEC is not None and SPEC.loader is not None
mapping = importlib.util.module_from_spec(SPEC)
SPEC.loader.exec_module(mapping)

CREATION = 133_800_000_000_000_000


def make_receipt(
    rva: int,
    digit: str,
    *,
    pid: int = 4242,
    creation: int = CREATION,
    schema: str = "dmc-rengine.exe-process-window.v2",
) -> dict[str, object]:
    module_base = 0x7FF600000000
    window_sha = digit * 64
    return {
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
            root, "open-game.json", make_receipt(0x0002FCA0, "1")
        )
        registration = write_receipt(
            root, "registration.json", make_receipt(0x00326D20, "2")
        )
        resolve = write_receipt(
            root, "resolve.json", make_receipt(0x00327430, "3")
        )

        packet = mapping.build_packet([open_game, registration, resolve])
        assert packet["schema"] == "dmc-rengine.gdspaces-l2-runtime-mapping.v2"
        assert packet["status"] == "bounded_process_instance_match"
        assert packet["anchor_count"] == 3
        assert packet["pid"] == 4242
        assert packet["process_creation_filetime"] == CREATION
        assert packet["image_name"] == "dmc3.exe"
        assert "image_path" not in packet
        assert "LocalUser" not in json.dumps(packet)
        assert "all-listed-anchors-belong-to-one-os-identified-process-instance" in (
            packet["proves"]
        )

        legacy = write_receipt(
            root,
            "legacy-v1.json",
            make_receipt(0x00327430, "3", schema="dmc-rengine.exe-process-window.v1"),
        )
        expect_rejected(
            [open_game, registration, legacy],
            "legacy v1 is not promotion authority",
        )

        wrong_creation = write_receipt(
            root,
            "wrong-creation.json",
            make_receipt(0x00327430, "3", creation=CREATION + 1),
        )
        expect_rejected(
            [open_game, registration, wrong_creation],
            "one exact process instance/module session",
        )

        zero_creation_payload = make_receipt(0x00327430, "3")
        zero_creation_payload["process_creation_filetime"] = 0
        zero_creation = write_receipt(
            root, "zero-creation.json", zero_creation_payload
        )
        expect_rejected(
            [open_game, registration, zero_creation],
            "non-zero unsigned 64-bit integer",
        )

        wrong_pid = write_receipt(
            root, "wrong-pid.json", make_receipt(0x00327430, "3", pid=9999)
        )
        expect_rejected(
            [open_game, registration, wrong_pid],
            "one exact process instance/module session",
        )

        duplicate = write_receipt(
            root, "duplicate.json", make_receipt(0x00326D20, "4")
        )
        expect_rejected(
            [open_game, registration, duplicate],
            "duplicate mapping anchor",
        )

        raw_payload = make_receipt(0x00327430, "3")
        raw_payload["bytes_hex"] = "00" * mapping.WINDOW_SIZE
        raw = write_receipt(root, "raw.json", raw_payload)
        expect_rejected(
            [open_game, registration, raw],
            "metadata-only receipts",
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

    print("runtime mapping v2 process-instance guardrails: ok")


if __name__ == "__main__":
    main()
