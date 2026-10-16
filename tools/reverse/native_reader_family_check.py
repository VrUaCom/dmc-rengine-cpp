#!/usr/bin/env python3
"""Run Native Reader workspace Info over unresolved stage file families.

The input must be an ASCII path (the installed Windows CLI cannot open the
original Cyrillic source path). The report keeps one compact row per file and
aggregates parser/maturity/diagnostic outcomes for later review.
"""

from __future__ import annotations

import argparse
import json
import subprocess
import time
from collections import Counter
from concurrent.futures import ThreadPoolExecutor, as_completed
from pathlib import Path


def inspect(cli: Path, path: Path, timeout: float) -> dict[str, object]:
    started = time.perf_counter()
    try:
        proc = subprocess.run(
            [str(cli), "inspect-workspace", str(path), "--stage", "--menu"],
            capture_output=True,
            text=True,
            encoding="utf-8",
            errors="replace",
            timeout=timeout,
        )
    except subprocess.TimeoutExpired:
        return {"ok": False, "error": "timeout", "path": str(path), "ms": round((time.perf_counter() - started) * 1000)}
    raw = proc.stdout.strip()
    first = min((i for i in (raw.find("{"), raw.find("[")) if i >= 0), default=-1)
    last = max(raw.rfind("}"), raw.rfind("]"))
    if first < 0 or last < first:
        return {"ok": False, "error": "invalid-json", "exit_code": proc.returncode, "path": str(path), "ms": round((time.perf_counter() - started) * 1000)}
    try:
        obj = json.loads(raw[first:last + 1])
    except json.JSONDecodeError:
        return {"ok": False, "error": "invalid-json", "exit_code": proc.returncode, "path": str(path), "ms": round((time.perf_counter() - started) * 1000)}
    integration = obj.get("format_integration") or {}
    diagnostics = obj.get("diagnostics") or []
    resource = obj.get("resource") or {}
    return {
        "ok": proc.returncode == 0,
        "path": str(path),
        "format": resource.get("format"),
        "parser_id": integration.get("parser_id", ""),
        "maturity": integration.get("maturity", "") if integration else "",
        "diagnostic_codes": sorted({d.get("code", "") for d in diagnostics if isinstance(d, dict)}),
        "size": resource.get("size"),
        "ms": round((time.perf_counter() - started) * 1000),
    }


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("--source", required=True, type=Path, help="ASCII junction containing top-level st* directories")
    parser.add_argument("--cli", required=True, type=Path)
    parser.add_argument("--output", required=True, type=Path)
    parser.add_argument("--workers", type=int, default=8)
    parser.add_argument("--timeout", type=float, default=30.0)
    parser.add_argument("--extensions", nargs="+", default=[".ukn", ".bd", ".so", ".mod"])
    args = parser.parse_args()

    files = sorted(
        p for d in args.source.iterdir() if d.is_dir() and d.name.lower().startswith("st")
        for p in d.rglob("*") if p.is_file() and p.suffix.lower() in {e.lower() for e in args.extensions}
    )
    rows: list[dict[str, object]] = []
    with ThreadPoolExecutor(max_workers=max(1, args.workers)) as pool:
        futures = {pool.submit(inspect, args.cli, path, args.timeout): path for path in files}
        for future in as_completed(futures):
            rows.append(future.result())
    rows.sort(key=lambda row: row["path"])
    summary = Counter()
    for row in rows:
        key = (str(row.get("format") or "<none>"), str(row.get("parser_id") or "<none>"), str(row.get("maturity") or "<none>"), str(row.get("ok")))
        summary["|".join(key)] += 1
    report = {
        "schema_version": 1,
        "source": str(args.source),
        "cli": str(args.cli),
        "extensions": [e.lower() for e in args.extensions],
        "files": len(rows),
        "ok": sum(1 for row in rows if row.get("ok")),
        "failed": sum(1 for row in rows if not row.get("ok")),
        "summary": dict(sorted(summary.items())),
        "rows": rows,
    }
    args.output.parent.mkdir(parents=True, exist_ok=True)
    args.output.write_text(json.dumps(report, indent=2, ensure_ascii=False) + "\n", encoding="utf-8")
    print(json.dumps({k: report[k] for k in ("files", "ok", "failed", "summary")}, ensure_ascii=False))
    return 0 if not report["failed"] else 2


if __name__ == "__main__":
    raise SystemExit(main())
