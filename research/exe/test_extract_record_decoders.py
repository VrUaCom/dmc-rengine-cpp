#!/usr/bin/env python3
"""Tests for the record decoder extractor.

These synthesise the instruction shapes rather than reading the executable, so
they run anywhere. The fail-closed cases matter more than the happy path: the
whole value of the extractor over a hand-typed table is that it stops rather
than silently skipping an instruction it does not understand, because a skipped
instruction moves every field after it.
"""

from __future__ import annotations

import sys
import unittest

sys.path.insert(0, __file__.rsplit("/", 1)[0])

from extract_record_decoders import Unrecognised, decode_fields  # noqa: E402


def movzx_from_r8(source: int) -> bytes:
    """movzx ecx, byte ptr [r8 + source]"""
    return bytes([0x41, 0x0F, 0xB6, 0x48, source])


def shl_ecx_8() -> bytes:
    return bytes([0xC1, 0xE1, 0x08])


def or_ecx_eax() -> bytes:
    return bytes([0x0B, 0xC8])


def store32(destination: int) -> bytes:
    """mov [rdx + destination], ecx"""
    return bytes([0x89, 0x4A, destination])


def store16(destination: int) -> bytes:
    """mov [rdx + destination], cx"""
    return bytes([0x66, 0x89, 0x4A, destination])


class DecodeFieldsTest(unittest.TestCase):
    def test_a_four_byte_field_is_its_lowest_source_and_its_store(self) -> None:
        code = (
            movzx_from_r8(0x02)
            + movzx_from_r8(0x03)
            + shl_ecx_8()
            + or_ecx_eax()
            + movzx_from_r8(0x01)
            + movzx_from_r8(0x00)
            + store32(0x10)
        )
        self.assertEqual(
            decode_fields(code, 0x1000),
            [{"record_offset": 0, "width": 4, "struct_offset": 0x10}],
        )

    def test_two_fields_do_not_leak_source_bytes_into_each_other(self) -> None:
        code = (
            movzx_from_r8(0x04)
            + movzx_from_r8(0x05)
            + store16(0x04)
            + movzx_from_r8(0x06)
            + movzx_from_r8(0x07)
            + store16(0x06)
        )
        self.assertEqual(
            decode_fields(code, 0x1000),
            [
                {"record_offset": 4, "width": 2, "struct_offset": 4},
                {"record_offset": 6, "width": 2, "struct_offset": 6},
            ],
        )

    def test_a_store_wider_than_its_sources_is_refused(self) -> None:
        code = movzx_from_r8(0x00) + movzx_from_r8(0x01) + store32(0x00)
        with self.assertRaises(Unrecognised) as raised:
            decode_fields(code, 0x1000)
        self.assertIn("into a 4-byte store", str(raised.exception))

    def test_a_store_narrower_than_its_sources_is_refused(self) -> None:
        code = (
            movzx_from_r8(0x00)
            + movzx_from_r8(0x01)
            + movzx_from_r8(0x02)
            + movzx_from_r8(0x03)
            + store16(0x00)
        )
        with self.assertRaises(Unrecognised) as raised:
            decode_fields(code, 0x1000)
        self.assertIn("into a 2-byte store", str(raised.exception))

    def test_a_discontiguous_field_is_refused(self) -> None:
        code = movzx_from_r8(0x00) + movzx_from_r8(0x02) + store16(0x00)
        with self.assertRaises(Unrecognised):
            decode_fields(code, 0x1000)

    def test_an_unknown_opcode_stops_rather_than_being_skipped(self) -> None:
        # Skipping the two unknown bytes would leave a perfectly well formed
        # two-byte field behind, so a decoder that skipped would return a
        # plausible answer instead of failing. That is the case to pin.
        code = (
            movzx_from_r8(0x00)
            + movzx_from_r8(0x01)
            + bytes([0xF3, 0xA4])
            + store16(0x00)
        )
        with self.assertRaises(Unrecognised) as raised:
            decode_fields(code, 0x1000)
        self.assertIn("unrecognised opcode 0xF3", str(raised.exception))

    def test_source_bytes_with_no_store_are_refused(self) -> None:
        code = movzx_from_r8(0x00) + movzx_from_r8(0x01) + store16(0x00)
        code += movzx_from_r8(0x02)
        with self.assertRaises(Unrecognised):
            decode_fields(code, 0x1000)

    def test_a_store_through_a_base_other_than_rdx_is_refused(self) -> None:
        # Four source bytes and a four-byte store, so the field itself is
        # well formed and the destination register is the only thing wrong.
        # mov [rbx + 0x10], ecx.
        sources = b"".join(movzx_from_r8(index) for index in range(4))
        code = sources + bytes([0x89, 0x4B, 0x10])
        with self.assertRaises(Unrecognised) as raised:
            decode_fields(code, 0x1000)
        self.assertIn("unexpected base", str(raised.exception))

    def test_a_store_through_r10_is_refused_although_the_modrm_matches(self) -> None:
        # REX.B makes 0x89 0x4A 0x10 mean [r10 + 0x10], not [rdx + 0x10]: the
        # same ModRM byte, a different register, and the only thing telling
        # them apart is the prefix.
        sources = b"".join(movzx_from_r8(index) for index in range(4))
        code = sources + bytes([0x41, 0x89, 0x4A, 0x10])
        with self.assertRaises(Unrecognised) as raised:
            decode_fields(code, 0x1000)
        self.assertIn("unexpected base", str(raised.exception))

    def test_an_indexed_source_is_refused_for_being_indexed(self) -> None:
        # movzx ecx, byte ptr [r8 + rax*1 + 0x00] -- a SIB the shape never
        # uses. Without the SIB test this still fails, but for the wrong
        # reason: the SIB byte gets eaten as a displacement and the byte after
        # it as an opcode. Asserting the message is what makes this a test of
        # the SIB guard rather than of the confusion it prevents.
        code = bytes([0x41, 0x0F, 0xB6, 0x4C, 0x00, 0x00]) + store16(0x00)
        with self.assertRaises(Unrecognised) as raised:
            decode_fields(code, 0x1000)
        self.assertIn("indexed source", str(raised.exception))

    def test_an_indexed_destination_is_refused_for_being_indexed(self) -> None:
        sources = b"".join(movzx_from_r8(index) for index in range(4))
        code = sources + bytes([0x89, 0x4C, 0x02, 0x10])
        with self.assertRaises(Unrecognised) as raised:
            decode_fields(code, 0x1000)
        self.assertIn("indexed destination", str(raised.exception))


if __name__ == "__main__":
    unittest.main(verbosity=2)
