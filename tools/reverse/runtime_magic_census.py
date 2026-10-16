#!/usr/bin/env python3
"""Census of object-header prefixes used by the DMC3 EXE dispatcher.

This is a corpus check, not a semantic parser.  It records the first three
bytes for SCM/EFM/SEF resources so the static EXE dispatch table can be checked
against the extracted files without copying game binaries into the repository.
"""

from __future__ import annotations

import argparse
import json
from collections import Counter
from pathlib import Path


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("--source", required=True, type=Path)
    parser.add_argument("--output", required=True, type=Path)
    args = parser.parse_args()

    result: dict[str, object] = {"schema_version": 1, "source": str(args.source), "families": {}}
    for suffix in (".scm", ".efm", ".sef"):
        files = sorted(args.source.rglob(f"*{suffix}"))
        prefixes: Counter[str] = Counter()
        short = 0
        for path in files:
            head = path.read_bytes()[:3]
            if len(head) < 3:
                short += 1
            else:
                prefixes[head.decode("ascii", errors="replace")] += 1
        result["families"][suffix[1:].upper()] = {
            "files": len(files),
            "prefix_counts": dict(sorted(prefixes.items())),
            "short_files": short,
        }

    args.output.parent.mkdir(parents=True, exist_ok=True)
    args.output.write_text(json.dumps(result, indent=2, ensure_ascii=False) + "\n", encoding="utf-8")
    print(json.dumps(result, ensure_ascii=False))
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
