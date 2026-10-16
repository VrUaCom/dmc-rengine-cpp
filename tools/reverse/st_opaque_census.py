#!/usr/bin/env python3
"""Classify every file below top-level st* folders by extension, size and prefix.

The output is a bounded inventory for reverse work. It does not infer a format
from a filename; prefixes are reported as ASCII when printable and as hex
otherwise. Game binaries are never copied into the repository.
"""

from __future__ import annotations

import argparse
import json
from collections import Counter, defaultdict
from pathlib import Path


def prefix_label(data: bytes) -> str:
    head = data[:4]
    if not head:
        return "<empty>"
    if all(32 <= byte < 127 for byte in head):
        return head.decode("ascii")
    return head.hex(" ").upper()


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("--source", required=True, type=Path)
    parser.add_argument("--output", required=True, type=Path)
    args = parser.parse_args()

    stage_dirs = sorted(p for p in args.source.iterdir() if p.is_dir() and p.name.lower().startswith("st"))
    rows: dict[str, dict[str, object]] = {}
    for stage_dir in stage_dirs:
        for path in stage_dir.rglob("*"):
            if not path.is_file():
                continue
            suffix = path.suffix.lower() or "<none>"
            item = rows.setdefault(suffix, {"files": 0, "sizes": Counter(), "prefixes": Counter(), "short_lt16": 0})
            item["files"] += 1
            size = path.stat().st_size
            item["sizes"][str(size)] += 1
            if size < 16:
                item["short_lt16"] += 1
            item["prefixes"][prefix_label(path.read_bytes())] += 1

    # JSON cannot serialize Counter; keep counts deterministic and compact.
    for item in rows.values():
        item["sizes"] = dict(sorted(item["sizes"].items(), key=lambda pair: int(pair[0])))
        item["prefixes"] = dict(sorted(item["prefixes"].items(), key=lambda pair: (-pair[1], pair[0])))

    report = {
        "schema_version": 1,
        "source": str(args.source),
        "top_level_st_directories": len(stage_dirs),
        "files": sum(int(item["files"]) for item in rows.values()),
        "families": dict(sorted(rows.items())),
    }
    args.output.parent.mkdir(parents=True, exist_ok=True)
    args.output.write_text(json.dumps(report, indent=2, ensure_ascii=False) + "\n", encoding="utf-8")
    print(json.dumps(report, ensure_ascii=False))
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
