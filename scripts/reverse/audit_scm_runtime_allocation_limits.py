#!/usr/bin/env python3
"""Audit canonical dmc3.exe SCM runtime allocation limits.

Validates exact instruction signatures in the canonical executable and derives
the fixed block-pool capacities used by the SCM allocation route. No game bytes
are emitted.
"""
import argparse
import hashlib
import json
import struct
from pathlib import Path

CANONICAL_SHA256 = "e454272ed0fb0247fcbcf300e5d55d7a3e96d50b89b9ffaff81bb978dcbdd082"


def pe_map(data: bytes):
    if data[:2] != b"MZ":
        raise ValueError("not a PE image")
    pe = struct.unpack_from("<I", data, 0x3C)[0]
    if data[pe:pe + 4] != b"PE\0\0":
        raise ValueError("bad PE signature")
    coff = pe + 4
    section_count = struct.unpack_from("<H", data, coff + 2)[0]
    optional_size = struct.unpack_from("<H", data, coff + 16)[0]
    optional = coff + 20
    if struct.unpack_from("<H", data, optional)[0] != 0x20B:
        raise ValueError("expected PE32+")
    image_base = struct.unpack_from("<Q", data, optional + 24)[0]
    section_table = optional + optional_size
    sections = []
    for index in range(section_count):
        offset = section_table + index * 40
        name = data[offset:offset + 8].split(b"\0", 1)[0].decode("ascii", "replace")
        virtual_size, virtual_address, raw_size, raw_offset = struct.unpack_from(
            "<IIII", data, offset + 8
        )
        sections.append(
            (name, virtual_address, max(virtual_size, raw_size), raw_offset)
        )
    return image_base, sections


def va_bytes(data, image_base, sections, va, size):
    rva = va - image_base
    for _, section_va, section_size, raw_offset in sections:
        if section_va <= rva and rva + size <= section_va + section_size:
            offset = raw_offset + (rva - section_va)
            return data[offset:offset + size]
    raise ValueError(f"VA 0x{va:X} is not mapped")


def expect(data, image_base, sections, va, expected_hex):
    expected = bytes.fromhex(expected_hex)
    actual = va_bytes(data, image_base, sections, va, len(expected))
    if actual != expected:
        raise AssertionError(
            f"0x{va:X}: {actual.hex()} != expected {expected.hex()}"
        )


def pool(span_bytes, block_bytes, alignment_bytes):
    # 0x140337780 stores one occupancy byte per data block before the aligned
    # data region. The canonical slot-count formula is:
    # floor((span - alignment) / (block_size + 1)).
    slot_count = (span_bytes - alignment_bytes) // (block_bytes + 1)
    usable = slot_count * block_bytes
    return {
        "spanBytes": span_bytes,
        "blockBytes": block_bytes,
        "alignmentBytes": alignment_bytes,
        "slotCount": slot_count,
        "usableDataBytes": usable,
        "metadataAndRemainderBytes": span_bytes - usable,
    }


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("exe", type=Path)
    parser.add_argument("--json", type=Path)
    args = parser.parse_args()

    data = args.exe.read_bytes()
    digest = hashlib.sha256(data).hexdigest()
    if digest != CANONICAL_SHA256:
        raise SystemExit(f"wrong SHA-256: {digest}")

    image_base, sections = pe_map(data)
    if image_base != 0x140000000:
        raise SystemExit(f"wrong image base: 0x{image_base:X}")

    signatures = {
        0x140030194: "b900004010",        # top-level 0x10400000 request
        0x1402C60ED: "41b800000010",      # zero 0x10000000 master span
        0x1402C6113: "41b800000010",      # initialize 0x10000000 master arena
        0x1402C6064: "ba00005000",        # allocate 5 MiB subpool
        0x1402C6078: "c744242004000000",  # alignment exponent 4
        0x1402C6080: "41b900005000",      # 5 MiB pool span
        0x1402C608D: "41b800040000",      # 1024-byte blocks
        0x1402C6098: "ba00004000",        # allocate 4 MiB subpool
        0x1402C60AC: "c744242006000000",  # alignment exponent 6
        0x1402C60B4: "41b900004000",      # 4 MiB pool span
        0x1402C60C1: "41b800020000",      # 512-byte blocks
        0x1402C61A2: "83fbfe7505",        # selector -2 test
        0x1400899FB: "41b8feffffff",      # SCM requests selector -2
        0x140089A07: "e844c72300",        # -> 0x1402C6150
        0x1402FDE82: "0fb70083c03b",      # u16 vertex count + 59
        0x1402FDE89: "b93c000000",        # divide by 60
        0x1402FDE92: "4869c080000000",    # *0x80 per chunk per pass
        0x140305318: "0fb70039453c",       # normalizer u16 loop bound
        0x1403053CC: "668908",             # generated u16 index write
    }
    for va, signature in signatures.items():
        expect(data, image_base, sections, va, signature)

    pool_64m = pool(0x4000000, 0x800, 1 << 6)
    pool_5m = pool(0x500000, 0x400, 1 << 4)
    pool_4m = pool(0x400000, 0x200, 1 << 6)

    result = {
        "schema": "dmc-rengine.scm-runtime-allocation-audit.v1",
        "canonicalExecutable": {
            "sha256": digest,
            "imageBase": "0x140000000",
        },
        "topLevelRuntimeBlock": {"bytes": 0x10400000, "mib": 260},
        "masterArena": {"bytes": 0x10000000, "mib": 256},
        "subpools": {
            "index0": pool_64m,
            "index1": pool_5m,
            "index2": pool_4m,
        },
        "scmAllocation": {
            "sizePlanner":
                "0x1402FD8D0 -> common 0x1402FD9C0 + SCM 0x1402FDD10 -> align16",
            "callsite": "0x1400899FB..0x140089A12",
            "selector": -2,
            "normalRoute": "index1 / 5 MiB pool",
            "overrideRoute":
                "byte[0x140CA8AB1] == 1 forces index2 / 4 MiB pool",
            "requestRounding":
                "ceil(requestBytes / selectedPool.blockBytes) contiguous bitmap slots",
            "normalEmptyPoolMaxRequestBytes": pool_5m["usableDataBytes"],
            "overrideEmptyPoolMaxRequestBytes": pool_4m["usableDataBytes"],
        },
        "scmSpecificFormula":
            "0xA0 + 0x740*objects + 0x220*meshes + "
            "0x100*sum(ceil(meshVertexCount/60)) + 2*auxSize",
        "commonPlannerFormula":
            "0xB0 + 0xE0*sceneNodes + 0x40*textureCount + "
            "(flags!=0 ? 0x20 + 0x50*textureCount*firstTextureLevelCount + "
            "((flags&2)!=0 ? 0x30*textureCount : 0) : 0)",
        "normalizer": {
            "vertexCountType": "u16 zero-extended to 32-bit loop bound",
            "generatedIndexType": "u16",
            "explicitLowerVertexCapFound": False,
        },
        "status": "EXE_CONFIRMED_BOUNDED_RUNTIME_ALLOCATION_PATH",
        "claimBoundary":
            "Pool occupancy/fragmentation and the live route override can lower "
            "available contiguous capacity. The auxiliary sizing helper may also "
            "reject a resource before pool allocation.",
    }

    output = json.dumps(result, indent=2) + "\n"
    if args.json:
        args.json.write_text(output, encoding="utf-8")
    else:
        print(output, end="")


if __name__ == "__main__":
    main()
