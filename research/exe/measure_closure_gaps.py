#!/usr/bin/env python3
"""Measure what the function map's widest closure leaves out.

``map-functions`` flags a function ``outside_every_closure`` when neither direct
calls nor virtual dispatch can reach it from the entry point. Two routes the file
records are not followed by that closure:

* **funclets** - catch and unwind handlers named in C++ ``FuncInfo`` and SEH scope
  tables. The runtime enters them while their parent function is on the stack,
  so a funclet of a reachable parent is reachable;
* **taken addresses** - ``lea r64,[rip+disp32]`` inside a reachable function whose
  target is a function start. The address can be called by whatever receives it
  (in this image, mostly MSVC's array constructor and destructor iterators).

This script re-closes the reachable set over those two edge kinds as well as the
report's call edges and prints how much of the flagged population each absorbs.
It reads a local copy of the executable and a map report produced with ``--all``;
neither is committed.

    dmc-rengine map-functions dmc3.exe --all --out map.json
    python3 research/exe/measure_closure_gaps.py dmc3.exe map.json
"""

from __future__ import annotations

import argparse
import bisect
import collections
import hashlib
import json
import struct
import sys
from typing import Callable, Iterable

EXPECTED_SHA256 = (
    "e454272ed0fb0247fcbcf300e5d55d7a3e96d50b89b9ffaff81bb978dcbdd082"
)
FUNCINFO_MAGICS = (0x19930520, 0x19930521, 0x19930522)


def lea_targets(code: bytes, base_rva: int) -> list[tuple[int, int]]:
    """Every ``lea r64,[rip+disp32]`` in ``code`` as (instruction rva, target rva).

    Matches REX.W or REX.WR, opcode 0x8D, and a ModRM of mod 00 with r/m 101.
    This is a byte-pattern match, not a disassembly, so a caller must filter the
    targets against something that makes a coincidence improbable - here, exact
    function starts, which a control run shows no random address ever hits.
    """

    found = []
    for i in range(len(code) - 6):
        if code[i] in (0x48, 0x4C) and code[i + 1] == 0x8D and (code[i + 2] & 0xC7) == 0x05:
            displacement = struct.unpack_from("<i", code, i + 3)[0]
            found.append((base_rva + i, base_rva + i + 7 + displacement))
    return found


def funcinfo_targets(read_u32: Callable[[int], int], funcinfo: int) -> set[int]:
    """Unwind actions, catch handlers and IP-map entries named by one FuncInfo.

    Returns an empty set when the magic number is not one the compiler emits,
    so a misidentified handler-data pointer contributes nothing.
    """

    def read_i32(rva: int) -> int:
        value = read_u32(rva)
        return value - (1 << 32) if value & 0x80000000 else value

    if read_u32(funcinfo) not in FUNCINFO_MAGICS:
        return set()
    targets: set[int] = set()
    max_state = read_i32(funcinfo + 4)
    unwind_map = read_u32(funcinfo + 8)
    try_count = read_u32(funcinfo + 12)
    try_map = read_u32(funcinfo + 16)
    ip_count = read_u32(funcinfo + 20)
    ip_map = read_u32(funcinfo + 24)
    for state in range(max(max_state, 0)):
        action = read_u32(unwind_map + 8 * state + 4)
        if action:
            targets.add(action)
    for block in range(try_count):
        entry = try_map + 20 * block
        catches = read_i32(entry + 12)
        handlers = read_u32(entry + 16)
        for index in range(max(catches, 0)):
            handler = read_u32(handlers + 20 * index + 12)
            if handler:
                targets.add(handler)
    for index in range(ip_count):
        targets.add(read_u32(ip_map + 8 * index))
    return targets


def close_over(seed: Iterable[int], *edge_maps: dict[int, set[int]]) -> set[int]:
    """Least fixpoint of ``seed`` under the union of the given edge maps."""

    reached = set(seed)
    pending = list(reached)
    while pending:
        node = pending.pop()
        for edges in edge_maps:
            for successor in edges.get(node, ()):
                if successor not in reached:
                    reached.add(successor)
                    pending.append(successor)
    return reached


class Image:
    def __init__(self, path: str) -> None:
        with open(path, "rb") as handle:
            self.data = handle.read()
        self.sha256 = hashlib.sha256(self.data).hexdigest()
        pe = struct.unpack_from("<I", self.data, 0x3C)[0]
        count = struct.unpack_from("<H", self.data, pe + 6)[0]
        optional = struct.unpack_from("<H", self.data, pe + 20)[0]
        table = pe + 24 + optional
        self.sections = {}
        for index in range(count):
            entry = table + 40 * index
            name = self.data[entry : entry + 8].rstrip(b"\0").decode("latin1")
            vsize, va, rsize, rptr = struct.unpack_from("<IIII", self.data, entry + 8)
            self.sections[name] = (va, vsize, rptr, rsize)

    def offset(self, rva: int) -> int:
        for va, vsize, rptr, rsize in self.sections.values():
            if va <= rva < va + max(vsize, rsize) and rva - va < rsize:
                return rva - va + rptr
        raise ValueError("rva 0x%X is not file-backed" % rva)

    def u32(self, rva: int) -> int:
        return struct.unpack_from("<I", self.data, self.offset(rva))[0]

    def safe_u32(self, rva: int) -> int:
        """Like u32, but 0 for an address with no bytes behind it.

        Handler data is not always an address - the stack-cookie handler's is an
        offset such as 0x28 - so the FuncInfo probe must be able to ask and be
        told nothing is there.
        """

        try:
            return self.u32(rva)
        except (ValueError, struct.error):
            return 0

    def import_names(self) -> dict[int, str]:
        """IAT slot rva -> imported function name."""

        pe = struct.unpack_from("<I", self.data, 0x3C)[0]
        directory = pe + 24 + 112
        import_rva = struct.unpack_from("<I", self.data, directory + 8)[0]
        names = {}
        entry = 0
        while True:
            lookup, _, _, name, thunk = struct.unpack_from(
                "<IIIII", self.data, self.offset(import_rva) + 20 * entry
            )
            if not (lookup or name or thunk):
                return names
            table = lookup or thunk
            slot = 0
            while True:
                value = struct.unpack_from("<Q", self.data, self.offset(table) + 8 * slot)[0]
                if value == 0:
                    break
                if not value & (1 << 63):
                    hint = self.offset(value & 0x7FFFFFFF) + 2
                    names[thunk + 8 * slot] = self.data[hint : self.data.index(b"\0", hint)].decode()
                slot += 1
            entry += 1

    def jump_thunk_target(self, rva: int):
        """If rva holds ``jmp [rip+disp32]``, the IAT slot it jumps through."""

        at = self.offset(rva)
        if self.data[at : at + 2] == b"\xff\x25":
            return rva + 6 + struct.unpack_from("<i", self.data, at + 2)[0]
        return None


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("image")
    parser.add_argument("map_report")
    parser.add_argument("--allow-any-image", action="store_true")
    args = parser.parse_args()

    image = Image(args.image)
    if not args.allow_any_image and image.sha256 != EXPECTED_SHA256:
        print("sha256 %s is not the canonical research target" % image.sha256, file=sys.stderr)
        return 2
    report = json.load(open(args.map_report))
    if report.get("functions_emitted") != report["summary"]["functions"]:
        print("the map report must be produced with --all", file=sys.stderr)
        return 2

    def as_int(value):
        return int(value, 16) if isinstance(value, str) else value

    functions = sorted(report["functions"], key=lambda f: as_int(f["begin_rva"]))
    begins = [as_int(f["begin_rva"]) for f in functions]
    by_begin = {as_int(f["begin_rva"]): f for f in functions}

    def owner(rva: int):
        index = bisect.bisect_right(begins, rva) - 1
        if index >= 0 and rva < as_int(functions[index]["end_rva"]):
            return begins[index]
        return None

    flagged = {b for b, f in by_begin.items() if f.get("outside_every_closure")}
    reached = set(by_begin) - flagged
    calls = {b: {as_int(c) for c in f.get("calls", ())} for b, f in by_begin.items()}

    iat = image.import_names()
    funclets: dict[int, set[int]] = collections.defaultdict(set)
    pdata_va, pdata_size, _, _ = image.sections[".pdata"]
    handler_kinds = collections.Counter()
    for index in range(pdata_size // 12):
        begin, _, unwind = (image.u32(pdata_va + 12 * index + 4 * k) for k in range(3))
        if not unwind:
            continue
        head = image.data[image.offset(unwind)]
        codes = image.data[image.offset(unwind) + 2]
        flags = head >> 3
        if flags & 4 or not flags & 3:
            continue
        handler_rva = unwind + 4 + 2 * ((codes + 1) & ~1)
        handler = image.u32(handler_rva)
        data = handler_rva + 4
        parent = owner(begin)
        if parent is None:
            continue
        slot = image.jump_thunk_target(handler)
        imported = iat.get(slot) if slot is not None else None
        # A FuncInfo is recognised by its magic number wherever the handler data
        # points; that covers __CxxFrameHandler3 and the cookie-checking variant
        # that forwards to it, without naming either by address.
        targets = funcinfo_targets(image.safe_u32, image.safe_u32(data))
        if targets:
            handler_kinds["c++ FuncInfo"] += 1
        elif imported == "__C_specific_handler":
            handler_kinds["scope table"] += 1
            for entry in range(image.u32(data)):
                for field in (8, 12):
                    value = image.u32(data + 4 + 16 * entry + field)
                    if value > 1:
                        targets.add(value)
        else:
            handler_kinds["no funclets"] += 1
        for target in targets:
            child = owner(target)
            if child is not None and child != parent:
                funclets[parent].add(child)

    text_va, _, text_ptr, text_size = image.sections[".text"]
    taken: dict[int, set[int]] = collections.defaultdict(set)
    code = image.data[text_ptr : text_ptr + text_size]
    for site, target in lea_targets(code, text_va):
        if target in by_begin:
            container = owner(site)
            if container is not None:
                taken[container].add(target)

    summary = {
        "flagged_outside_every_closure": len(flagged),
        "tool_closure": len(reached),
        "tool_closure_reclosed_over_calls": len(close_over(reached, calls)),
    }
    for label, maps in (
        ("absorbed_by_funclets", (calls, funclets)),
        ("absorbed_by_taken_addresses", (calls, taken)),
        ("absorbed_by_both", (calls, funclets, taken)),
    ):
        summary[label] = len(close_over(reached, *maps) & flagged)
    remaining = flagged - close_over(reached, calls, funclets, taken)
    summary["remaining_outside"] = len(remaining)
    summary["remaining_outside_bytes"] = sum(by_begin[r]["size"] for r in remaining)
    summary["handler_data_kinds"] = dict(handler_kinds)
    json.dump(summary, sys.stdout, indent=2)
    sys.stdout.write("\n")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
