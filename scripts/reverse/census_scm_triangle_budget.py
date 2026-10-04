#!/usr/bin/env python3
"""Census canonical DMC3 SCM triangle workload from retail/hash-bound inputs.

Reconstructs the same topology index stream as formats/scm_topology.cpp and
counts only non-degenerate triangles. Proprietary resource bytes are never
written to the repository; output is hashes/metadata only.
"""
import argparse
import hashlib
import json
import struct
import zipfile
from pathlib import Path

BREAK_BIT = 0x02
HEADER = 0x40
OBJECT = 0x40
MESH = 0x50
DYNAMIC_VERTEX_RING_BYTES = 0x3C0000


def u16(data, offset):
    return struct.unpack_from("<H", data, offset)[0]


def u64(data, offset):
    return struct.unpack_from("<Q", data, offset)[0]


def sha256_bytes(data):
    return hashlib.sha256(data).hexdigest()


def sha256_file(path):
    h = hashlib.sha256()
    with path.open("rb") as f:
        for chunk in iter(lambda: f.read(1024 * 1024), b""):
            h.update(chunk)
    return h.hexdigest()


def generate_indices(flags):
    indices = []
    previous = -1
    inside_run = False
    for index, flag in enumerate(flags):
        if index <= 1 or (flag & BREAK_BIT):
            inside_run = False
            continue
        if not inside_run:
            inside_run = True
            if previous >= 0:
                indices.extend((previous, index - 2))
            indices.extend((index - 2, index - 1))
        indices.append(index)
        previous = index
    return indices


def nondegenerate_strip_triangles(indices):
    total = 0
    for index in range(2, len(indices)):
        a, b, c = indices[index - 2], indices[index - 1], indices[index]
        if a != b and a != c and b != c:
            total += 1
    return total


def parse_scm(name, data):
    if len(data) < HEADER or data[:4] != b"SCM ":
        raise ValueError(f"{name}: not SCM")

    object_count = data[0x10]
    rows = []
    vertices = 0
    triangles = 0
    meshes = 0

    for object_index in range(object_count):
        object_offset = HEADER + object_index * OBJECT
        if object_offset + OBJECT > len(data):
            raise ValueError(f"{name}: object table OOB")

        mesh_count = data[object_offset]
        mesh_offset = u64(data, object_offset + 8)

        for mesh_index in range(mesh_count):
            offset = mesh_offset + mesh_index * MESH
            if offset + MESH > len(data):
                raise ValueError(f"{name}: mesh table OOB")

            vertex_count = u16(data, offset)
            color_offset = u64(data, offset + 0x38)
            if color_offset + vertex_count * 4 > len(data):
                raise ValueError(f"{name}: topology stream OOB")

            flags = data[color_offset + 3:color_offset + vertex_count * 4:4]
            indices = generate_indices(flags)
            triangle_count = nondegenerate_strip_triangles(indices)

            rows.append({
                "object": object_index,
                "mesh": mesh_index,
                "vertices": vertex_count,
                "triangles": triangle_count,
            })
            vertices += vertex_count
            triangles += triangle_count
            meshes += 1

    return {
        "name": name,
        "sha256": sha256_bytes(data),
        "bytes": len(data),
        "objects": object_count,
        "meshes": meshes,
        "vertices": vertices,
        "triangles": triangles,
        "maxMeshTriangles": max((row["triangles"] for row in rows), default=0),
        "textureSlots": data[0x12],
        "sceneNodes": data[0x11],
        "meshRows": rows,
    }


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("archive", type=Path)
    parser.add_argument("--extra-scm", action="append", type=Path, default=[])
    parser.add_argument("--json", type=Path)
    args = parser.parse_args()

    documents = []
    seen = set()
    extras = []

    with zipfile.ZipFile(args.archive) as archive:
        for name in archive.namelist():
            if not name.lower().endswith(".scm"):
                continue
            data = archive.read(name)
            digest = sha256_bytes(data)
            if digest in seen:
                continue
            seen.add(digest)
            documents.append(parse_scm(name, data))

    for path in args.extra_scm:
        data = path.read_bytes()
        digest = sha256_bytes(data)
        extras.append({
            "name": path.name,
            "sha256": digest,
            "bytes": len(data),
        })
        if digest in seen:
            continue
        seen.add(digest)
        documents.append(parse_scm(path.name, data))

    max_file = max(documents, key=lambda item: item["triangles"])
    max_mesh = max(
        (
            dict(file=document["name"], **row)
            for document in documents
            for row in document["meshRows"]
        ),
        key=lambda item: item["triangles"],
    )

    result = {
        "schema": "dmc-rengine.scm-triangle-budget-census.v1",
        "archive": {
            "name": args.archive.name,
            "sha256": sha256_file(args.archive),
            "bytes": args.archive.stat().st_size,
        },
        "extras": extras,
        "uniqueScm": len(documents),
        "vertices": sum(item["vertices"] for item in documents),
        "triangles": sum(item["triangles"] for item in documents),
        "meshes": sum(item["meshes"] for item in documents),
        "maxFile": {
            key: max_file[key]
            for key in (
                "name", "sha256", "bytes", "objects",
                "meshes", "vertices", "triangles"
            )
        },
        "maxMesh": max_mesh,
        "emptyRingProjection": [
            {
                "stride": stride,
                "bytesPerTriangle": 3 * stride,
                "maxFileBytes": max_file["triangles"] * 3 * stride,
                "maxFileFraction":
                    (max_file["triangles"] * 3 * stride)
                    / DYNAMIC_VERTEX_RING_BYTES,
            }
            for stride in (20, 28, 36)
        ],
        "files": [
            {
                key: document[key]
                for key in (
                    "name", "sha256", "bytes", "objects", "meshes",
                    "vertices", "triangles", "maxMeshTriangles",
                    "textureSlots", "sceneNodes"
                )
            }
            for document in documents
        ],
    }

    text = json.dumps(result, indent=2) + "\n"
    if args.json:
        args.json.write_text(text, encoding="utf-8")
    else:
        print(text, end="")


if __name__ == "__main__":
    main()
