#!/usr/bin/env python3
"""Recover a byte-at-a-time record decoder's field map from the image.

The archive reader in the DMC3 HD executable decodes each fixed-size record
with a function that has exactly one shape per field: one ``movzx`` per source
byte, shifts and ors to assemble it low byte first, and one store into the
output struct. That shape is machine-readable, so the field map in
``docs/reverse/dmc3-zip-directory-2026-09-18.md`` does not have to be typed by
hand and can be regenerated instead.

This decodes only the four instruction forms those functions use. It is not a
disassembler and it is deliberately fail-closed: an unrecognised byte inside a
requested range aborts with the offset, rather than being skipped, because a
skipped instruction would silently move a field.

The executable is not part of this repository. Pass a local copy:

    python3 research/exe/extract_record_decoders.py /path/to/dmc3.exe

Expected SHA-256 of the canonical research target:
e454272ed0fb0247fcbcf300e5d55d7a3e96d50b89b9ffaff81bb978dcbdd082
"""

from __future__ import annotations

import argparse
import hashlib
import json
import struct
import sys

EXPECTED_SHA256 = (
    "e454272ed0fb0247fcbcf300e5d55d7a3e96d50b89b9ffaff81bb978dcbdd082"
)

# The decoders this recovers, as (name, first rva, one past the last rva).
DECODERS = (
    ("central_directory_file_header", 0x327E40, 0x32801C),
    ("local_file_header", 0x328020, 0x32815C),
)


class Image:
    """Just enough PE to turn an RVA into a file offset."""

    def __init__(self, path: str) -> None:
        with open(path, "rb") as handle:
            self.data = handle.read()
        self.sha256 = hashlib.sha256(self.data).hexdigest()
        data = self.data
        pe = struct.unpack_from("<I", data, 0x3C)[0]
        if data[pe : pe + 4] != b"PE\0\0":
            raise ValueError("not a PE image")
        section_count = struct.unpack_from("<H", data, pe + 6)[0]
        optional_size = struct.unpack_from("<H", data, pe + 20)[0]
        table = pe + 24 + optional_size
        self.sections = []
        for index in range(section_count):
            entry = table + index * 40
            name = data[entry : entry + 8].rstrip(b"\0").decode("latin1")
            virtual_size, virtual_address, raw_size, raw_pointer = struct.unpack_from(
                "<IIII", data, entry + 8
            )
            self.sections.append(
                (name, virtual_address, virtual_size, raw_pointer, raw_size)
            )

    def offset(self, rva: int) -> int:
        for _name, virtual_address, virtual_size, raw_pointer, raw_size in self.sections:
            span = max(virtual_size, raw_size)
            if virtual_address <= rva < virtual_address + span:
                offset = rva - virtual_address + raw_pointer
                if offset < raw_pointer + raw_size:
                    return offset
        raise ValueError("rva 0x%06X is not backed by raw bytes" % rva)

    def read(self, rva: int, count: int) -> bytes:
        start = self.offset(rva)
        return self.data[start : start + count]


class Unrecognised(Exception):
    pass


def _modrm(byte: int) -> tuple[int, int, int]:
    return byte >> 6, (byte >> 3) & 0x07, byte & 0x07


def decode_fields(code: bytes, begin_rva: int) -> list[dict]:
    """Walk one decoder and return its fields in emission order.

    Source bytes accumulate until a store into the output struct, at which
    point the accumulated set is that field: its record offset is the lowest
    source displacement seen and its width is how many were seen.
    """

    fields: list[dict] = []
    pending: set[int] = set()
    position = 0
    limit = len(code)

    while position < limit:
        start = position
        rex = 0
        operand16 = False

        while position < limit and code[position] == 0x66:
            operand16 = True
            position += 1
        if position < limit and 0x40 <= code[position] <= 0x4F:
            rex = code[position]
            position += 1

        if position >= limit:
            raise Unrecognised("truncated at 0x%06X" % (begin_rva + start))

        opcode = code[position]

        # movzx r32, byte ptr [base + disp]
        if opcode == 0x0F and position + 1 < limit and code[position + 1] == 0xB6:
            position += 2
            mod, _reg, rm = _modrm(code[position])
            position += 1
            if rm == 0x04:  # a SIB byte would mean an indexed source
                raise Unrecognised(
                    "indexed source at 0x%06X" % (begin_rva + start)
                )
            if mod == 0x00:
                displacement = 0
            elif mod == 0x01:
                displacement = code[position]
                position += 1
            elif mod == 0x02:
                displacement = struct.unpack_from("<i", code, position)[0]
                position += 4
            else:
                raise Unrecognised(
                    "register source at 0x%06X" % (begin_rva + start)
                )
            pending.add(displacement)
            continue

        # mov [rdx + disp], r8/r16/r32  -- the store that ends a field
        if opcode in (0x88, 0x89):
            position += 1
            mod, reg, rm = _modrm(code[position])
            position += 1
            if rm == 0x04:
                raise Unrecognised(
                    "indexed destination at 0x%06X" % (begin_rva + start)
                )
            if mod == 0x00:
                displacement = 0
            elif mod == 0x01:
                displacement = code[position]
                position += 1
            elif mod == 0x02:
                displacement = struct.unpack_from("<i", code, position)[0]
                position += 4
            else:
                raise Unrecognised(
                    "register destination at 0x%06X" % (begin_rva + start)
                )
            if rm != 0x02 or (rex & 0x01):
                raise Unrecognised(
                    "store to an unexpected base at 0x%06X" % (begin_rva + start)
                )
            if not pending:
                raise Unrecognised(
                    "store with no source bytes at 0x%06X" % (begin_rva + start)
                )
            width = 1 if opcode == 0x88 else (2 if operand16 else (8 if rex & 0x08 else 4))
            low = min(pending)
            # One test, not two: a set that equals range(low, low + width) has
            # exactly width members, so this rejects a short or long assembly
            # as well as a gapped one, and a separate count check would be
            # dead code that looked like a guard.
            if pending != set(range(low, low + width)):
                raise Unrecognised(
                    "field at 0x%06X assembles {%s} into a %d-byte store at %d"
                    % (
                        begin_rva + start,
                        ", ".join(str(value) for value in sorted(pending)),
                        width,
                        displacement,
                    )
                )
            fields.append(
                {
                    "record_offset": low,
                    "width": width,
                    "struct_offset": displacement,
                }
            )
            pending.clear()
            continue

        # The assembling and bookkeeping instructions, which carry no offsets
        # this needs but must be recognised so nothing is silently skipped.
        if opcode in (0xC1, 0xC0):  # shl/shr r, imm8
            position += 1
            _mod, _reg, rm = _modrm(code[position])
            position += 1
            if rm == 0x04:
                raise Unrecognised("indexed shift at 0x%06X" % (begin_rva + start))
            position += 1  # the immediate
            continue
        if opcode in (0x0B, 0x33, 0x8B, 0x3B, 0x2B, 0x03, 0x09):
            position += 1
            mod, _reg, rm = _modrm(code[position])
            position += 1
            if rm == 0x04:
                position += 1
            if mod == 0x01:
                position += 1
            elif mod == 0x02:
                position += 4
            continue
        if opcode == 0x81:  # cmp/and/or r, imm32 -- the signature test
            position += 1
            mod, _reg, rm = _modrm(code[position])
            position += 1
            if rm == 0x04:
                position += 1
            if mod == 0x01:
                position += 1
            elif mod == 0x02:
                position += 4
            position += 4
            continue
        if opcode == 0x0F and position + 1 < limit and code[position + 1] == 0x94:
            position += 2  # sete
            _mod, _reg, rm = _modrm(code[position])
            position += 1
            if rm == 0x04:
                position += 1
            continue
        if opcode == 0xC3:  # ret
            position += 1
            continue

        raise Unrecognised(
            "unrecognised opcode 0x%02X at 0x%06X" % (opcode, begin_rva + start)
        )

    if pending:
        raise Unrecognised("source bytes with no store at the end of the range")
    return fields


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("image", help="path to a local copy of the executable")
    parser.add_argument(
        "--allow-any-image",
        action="store_true",
        help="do not require the canonical SHA-256",
    )
    arguments = parser.parse_args()

    image = Image(arguments.image)
    if not arguments.allow_any_image and image.sha256 != EXPECTED_SHA256:
        print(
            "sha256 %s is not the canonical research target" % image.sha256,
            file=sys.stderr,
        )
        return 2

    report = {"image_sha256": image.sha256, "decoders": []}
    for name, begin, end in DECODERS:
        fields = decode_fields(image.read(begin, end - begin), begin)
        consumed = max(field["record_offset"] + field["width"] for field in fields)
        report["decoders"].append(
            {
                "name": name,
                "begin_rva": begin,
                "end_rva": end,
                "record_bytes_consumed": consumed,
                "struct_bytes_written": max(
                    field["struct_offset"] + field["width"] for field in fields
                ),
                "fields": fields,
            }
        )
    json.dump(report, sys.stdout, indent=2)
    sys.stdout.write("\n")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
