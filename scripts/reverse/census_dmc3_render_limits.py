#!/usr/bin/env python3
"""SHA-bound DMC3 DDS/SCM corpus census for rendering/resource-limit research."""

from __future__ import annotations

import argparse
import hashlib
import json
import struct
import zipfile
from collections import Counter
from pathlib import Path


def u8(data: bytes, offset: int) -> int:
    return data[offset]


def u16(data: bytes, offset: int) -> int:
    return struct.unpack_from("<H", data, offset)[0]


def u32(data: bytes, offset: int) -> int:
    return struct.unpack_from("<I", data, offset)[0]


def u64(data: bytes, offset: int) -> int:
    return struct.unpack_from("<Q", data, offset)[0]


def digest(data: bytes) -> str:
    return hashlib.sha256(data).hexdigest()


def scan_dds(zf: zipfile.ZipFile) -> dict:
    rows = []
    unique = {}
    for name in zf.namelist():
        if not name.lower().endswith(".dds"):
            continue
        data = zf.read(name)
        if len(data) < 128 or data[:4] != b"DDS ":
            continue
        row = {
            "path": name,
            "size": len(data),
            "width": u32(data, 16),
            "height": u32(data, 12),
            "mip_count": u32(data, 28),
            "fourcc": data[84:88].decode("latin1"),
            "sha256": digest(data),
        }
        rows.append(row)
        unique.setdefault(row["sha256"], row)

    values = list(unique.values())
    dims = Counter((r["width"], r["height"], r["fourcc"]) for r in values)
    formats = Counter(r["fourcc"] for r in values)

    def max_by(fn):
        return max(values, key=fn) if values else None

    return {
        "paths": len(rows),
        "unique_sha256": len(values),
        "compression_counts": dict(sorted(formats.items())),
        "dimension_counts": [
            {"width": w, "height": h, "fourcc": f, "count": count}
            for (w, h, f), count in sorted(dims.items())
        ],
        "max_width": max_by(lambda r: r["width"]),
        "max_height": max_by(lambda r: r["height"]),
        "max_area": max_by(lambda r: r["width"] * r["height"]),
    }


def mesh_stats(data: bytes, mesh_offset: int) -> tuple[int, int]:
    vertex_count = u16(data, mesh_offset)
    color_flags = u64(data, mesh_offset + 0x38)
    end = color_flags + vertex_count * 4
    if end > len(data):
        raise ValueError("SCM color/topology stream exceeds payload")
    triangles = sum(
        1
        for index in range(vertex_count)
        if index >= 2 and (data[color_flags + index * 4 + 3] & 0x02) == 0
    )
    return vertex_count, triangles


def scan_scm(zf: zipfile.ZipFile) -> dict:
    path_count = 0
    unique = {}

    for name in zf.namelist():
        if not name.lower().endswith(".scm"):
            continue
        path_count += 1
        data = zf.read(name)
        sha = digest(data)
        if sha in unique or len(data) < 0x40 or data[:4] != b"SCM ":
            continue

        objects = u8(data, 0x10)
        nodes = u8(data, 0x11)
        texture_slots = u8(data, 0x12)
        total_meshes = total_vertices = total_triangles = 0
        max_mesh_vertices = max_mesh_triangles = 0
        max_object_vertices = max_object_triangles = 0

        for oi in range(objects):
            oo = 0x40 + oi * 0x40
            if oo + 0x40 > len(data):
                raise ValueError(f"{name}: object table exceeds payload")
            mesh_count = u8(data, oo)
            object_vertices = u16(data, oo + 0x02)
            mesh_table = u64(data, oo + 0x08)
            total_meshes += mesh_count
            total_vertices += object_vertices
            max_object_vertices = max(max_object_vertices, object_vertices)

            child_vertices = object_triangles = 0
            for mi in range(mesh_count):
                mo = mesh_table + mi * 0x50
                if mo + 0x50 > len(data):
                    raise ValueError(f"{name}: mesh table exceeds payload")
                vertices, triangles = mesh_stats(data, mo)
                child_vertices += vertices
                object_triangles += triangles
                max_mesh_vertices = max(max_mesh_vertices, vertices)
                max_mesh_triangles = max(max_mesh_triangles, triangles)

            if child_vertices != object_vertices:
                raise ValueError(
                    f"{name}: child vertex sum {child_vertices} != object total {object_vertices}"
                )
            total_triangles += object_triangles
            max_object_triangles = max(max_object_triangles, object_triangles)

        unique[sha] = {
            "path": name,
            "sha256": sha,
            "size": len(data),
            "objects": objects,
            "scene_nodes": nodes,
            "texture_slots": texture_slots,
            "meshes": total_meshes,
            "vertices": total_vertices,
            "triangles": total_triangles,
            "max_mesh_vertices": max_mesh_vertices,
            "max_mesh_triangles": max_mesh_triangles,
            "max_object_vertices": max_object_vertices,
            "max_object_triangles": max_object_triangles,
        }

    values = list(unique.values())

    def winner(field: str):
        return max(values, key=lambda r: r[field]) if values else None

    fields = (
        "size",
        "objects",
        "scene_nodes",
        "texture_slots",
        "meshes",
        "vertices",
        "triangles",
        "max_mesh_vertices",
        "max_mesh_triangles",
        "max_object_vertices",
        "max_object_triangles",
    )
    return {
        "paths": path_count,
        "unique_sha256": len(values),
        "maxima": {field: winner(field) for field in fields},
    }


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("archive", type=Path)
    parser.add_argument("--json", type=Path)
    args = parser.parse_args()

    raw = args.archive.read_bytes()
    with zipfile.ZipFile(args.archive) as zf:
        report = {
            "schema": "dmc-rengine.dmc3-resource-limit-census.v1",
            "source": {
                "name": args.archive.name,
                "size": len(raw),
                "sha256": digest(raw),
            },
            "dds": scan_dds(zf),
            "scm": scan_scm(zf),
            "claim_boundary": (
                "Corpus maxima only. Serialized field widths and executable "
                "allocation/draw limits require separate evidence."
            ),
        }

    encoded = json.dumps(report, indent=2, sort_keys=True) + "\n"
    if args.json:
        args.json.write_text(encoded, encoding="utf-8")
    else:
        print(encoded, end="")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
