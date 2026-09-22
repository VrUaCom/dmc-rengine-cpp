#!/usr/bin/env python3
"""Tests for the closure-gap measurement. They need no executable."""

from __future__ import annotations

import struct
import sys
import unittest

sys.path.insert(0, __file__.rsplit("/", 1)[0])

from measure_closure_gaps import close_over, funcinfo_targets, lea_targets  # noqa: E402


def lea(rex: int, modrm: int, displacement: int) -> bytes:
    return bytes([rex, 0x8D, modrm]) + struct.pack("<i", displacement)


class LeaTargetsTest(unittest.TestCase):
    def test_rip_relative_lea_resolves_from_the_end_of_the_instruction(self) -> None:
        code = b"\x90" * 3 + lea(0x48, 0x05, 0x100)
        self.assertEqual(lea_targets(code, 0x1000), [(0x1003, 0x1003 + 7 + 0x100)])

    def test_rex_wr_and_other_registers_are_accepted(self) -> None:
        # 4C 8D 0D = lea r9,[rip+disp]; ModRM 0x0D is reg=1 r/m=101.
        code = lea(0x4C, 0x0D, -0x10)
        self.assertEqual(lea_targets(code, 0x2000), [(0x2000, 0x2000 + 7 - 0x10)])

    def test_a_register_based_lea_is_not_an_address_take(self) -> None:
        # ModRM 0x45 is mod=01 r/m=101: [rbp+disp8], not RIP-relative.
        self.assertEqual(lea_targets(lea(0x48, 0x45, 0x100), 0), [])

    def test_a_lea_without_rex_w_is_ignored(self) -> None:
        self.assertEqual(lea_targets(lea(0x40, 0x05, 0x100), 0), [])


class FuncInfoTargetsTest(unittest.TestCase):
    def reader(self, words: dict[int, int]):
        return lambda rva: words.get(rva, 0)

    def funcinfo(self, magic: int = 0x19930522) -> dict[int, int]:
        fi, unwind, tries, handlers, ipmap = 0x100, 0x200, 0x300, 0x400, 0x500
        words = {
            fi: magic, fi + 4: 2, fi + 8: unwind, fi + 12: 1, fi + 16: tries,
            fi + 20: 2, fi + 24: ipmap,
            unwind + 4: 0xA000, unwind + 12: 0,        # second action is null
            tries + 12: 1, tries + 16: handlers,
            handlers + 12: 0xB000,
            ipmap: 0xC000, ipmap + 8: 0xC100,
        }
        return words

    def test_actions_handlers_and_ip_map_are_collected_and_nulls_are_not(self) -> None:
        self.assertEqual(
            funcinfo_targets(self.reader(self.funcinfo()), 0x100),
            {0xA000, 0xB000, 0xC000, 0xC100},
        )

    def test_a_wrong_magic_contributes_nothing(self) -> None:
        self.assertEqual(funcinfo_targets(self.reader(self.funcinfo(0x12345678)), 0x100), set())

    def test_a_negative_state_count_is_not_a_huge_loop(self) -> None:
        words = self.funcinfo()
        words[0x104] = 0xFFFFFFFF
        self.assertNotIn(0xA000, funcinfo_targets(self.reader(words), 0x100))


class CloseOverTest(unittest.TestCase):
    def test_fixpoint_over_several_edge_kinds_is_transitive(self) -> None:
        calls = {1: {2}, 3: {4}}
        funclets = {2: {3}}
        self.assertEqual(close_over({1}, calls, funclets), {1, 2, 3, 4})

    def test_unconnected_nodes_stay_out(self) -> None:
        self.assertEqual(close_over({1}, {1: {2}, 5: {6}}), {1, 2})


if __name__ == "__main__":
    unittest.main(verbosity=1)
