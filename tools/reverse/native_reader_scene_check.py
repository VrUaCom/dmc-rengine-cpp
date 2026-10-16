#!/usr/bin/env python3
"""Run the installed DMC Rengine Native Reader against stage SCM/PTX resources.

The Windows CLI currently cannot open paths containing Cyrillic characters.  The
tool therefore creates an ASCII directory junction and passes only the junction
paths to the native reader.  The original tree is never modified.

The report is intentionally an evidence report, not a renderer claim:
``inspect-workspace`` verifies the native structural reader and its Info model;
the CLI does not expose the GUI's texture overlay or low-resolution viewport.
"""

from __future__ import annotations

import argparse
import json
import os
import re
import shutil
import subprocess
import sys
import time
from concurrent.futures import ThreadPoolExecutor, as_completed
from pathlib import Path
from typing import Any


STAGE_RE = re.compile(r"^st\d{3}", re.IGNORECASE)


def parse_args() -> argparse.Namespace:
    p = argparse.ArgumentParser(description=__doc__)
    p.add_argument("--source", required=True, type=Path, help="Extracted - copia root")
    p.add_argument("--cli", required=True, type=Path, help="dmc-rengine.exe")
    p.add_argument("--alias", required=True, type=Path, help="ASCII directory junction")
    p.add_argument("--output", required=True, type=Path, help="JSON report path")
    p.add_argument("--workers", type=int, default=4)
    p.add_argument("--timeout", type=float, default=90.0)
    p.add_argument("--sample-scm", type=int, default=2,
                   help="Detailed inspect-scm samples per stage (0 disables)")
    p.add_argument("--full-scm", action="store_true",
                   help="Run inspect-scm on every scene (larger/slower report)")
    p.add_argument("--low-res", action="store_true",
                   help="Record that low-resolution review was requested")
    return p.parse_args()


def ensure_alias(source: Path, alias: Path) -> None:
    source = source.resolve()
    # Keep the junction path lexical; resolve() follows it to the Unicode
    # target and defeats the native reader's ASCII-path workaround.
    alias = Path(os.path.abspath(alias))
    if alias.exists():
        # A previous run may have left the junction.  Refuse to use a real
        # directory so a typo cannot redirect the native reader.
        if not alias.is_dir():
            raise RuntimeError(f"alias exists but is not a directory: {alias}")
        try:
            target = Path(os.readlink(alias))
        except OSError:
            target = None
        if target is not None and target.resolve() == source:
            return
        # Windows junctions do not always support readlink through Python;
        # GetFinalPathNameByHandle is overkill.  A marker file lets us verify
        # aliases created by this tool without deleting user directories.
        marker = alias / ".native_reader_alias_target"
        if marker.exists() and marker.read_text(encoding="utf-8-sig").strip() == str(source):
            return
        raise RuntimeError(f"refusing to reuse unrelated alias directory: {alias}")

    alias.parent.mkdir(parents=True, exist_ok=True)
    # mklink is the most reliable way to create a directory junction on
    # Windows without requiring Developer Mode.
    proc = subprocess.run(
        ["cmd.exe", "/d", "/c", "mklink", "/J", str(alias), str(source)],
        capture_output=True,
        text=True,
        encoding="utf-8",
        errors="replace",
    )
    if proc.returncode != 0 or not alias.is_dir():
        raise RuntimeError(f"cannot create alias {alias}: {proc.stdout}{proc.stderr}")
    (alias / ".native_reader_alias_target").write_text(str(source), encoding="utf-8")


def run_native(cli: Path, command: str, file: Path, timeout: float) -> dict[str, Any]:
    started = time.perf_counter()
    try:
        proc = subprocess.run(
            [str(cli), command, str(file), "--stage", "--menu"]
            if command == "inspect-workspace"
            else [str(cli), command, str(file), "--json"],
            capture_output=True,
            text=True,
            encoding="utf-8",
            errors="replace",
            timeout=timeout,
        )
    except subprocess.TimeoutExpired as exc:
        return {"ok": False, "error": "timeout", "duration_ms": round((time.perf_counter() - started) * 1000),
                "stdout_tail": (exc.stdout or "")[-2000:] if isinstance(exc.stdout, str) else "",
                "stderr_tail": (exc.stderr or "")[-2000:] if isinstance(exc.stderr, str) else ""}
    raw = proc.stdout.strip()
    parsed: Any = None
    parse_error = None
    if raw:
        # Native Reader emits JSON on stdout.  Keep this tolerant of a future
        # informational prefix/suffix while still rejecting malformed payloads.
        first = min((i for i in (raw.find("{"), raw.find("[")) if i >= 0), default=-1)
        last = max(raw.rfind("}"), raw.rfind("]"))
        if first >= 0 and last >= first:
            try:
                parsed = json.loads(raw[first : last + 1])
            except json.JSONDecodeError as exc:
                parse_error = str(exc)
    return {
        "ok": proc.returncode == 0 and parsed is not None,
        "exit_code": proc.returncode,
        "duration_ms": round((time.perf_counter() - started) * 1000),
        "json": parsed,
        "parse_error": parse_error,
        "stdout_tail": raw[-2000:] if parsed is None else None,
        "stderr_tail": proc.stderr.strip()[-2000:] or None,
    }


def summarize_workspace(result: dict[str, Any]) -> dict[str, Any]:
    obj = result.get("json") if result.get("ok") else None
    if not isinstance(obj, dict):
        return {k: result.get(k) for k in ("ok", "exit_code", "duration_ms", "parse_error", "stdout_tail", "stderr_tail")}
    doc = obj.get("binary_document") or {}
    integration = obj.get("format_integration") or {}
    diagnostics = obj.get("diagnostics") or []
    return {
        "ok": True,
        "schema_version": obj.get("schema_version"),
        "resource": obj.get("resource"),
        "workspace_status": obj.get("workspace_status"),
        "context": obj.get("context"),
        "parser_id": integration.get("parser_id"),
        "maturity": integration.get("maturity"),
        "write_policy": integration.get("write_policy"),
        "stage_category": integration.get("stage_category"),
        "binary_document": {
            k: doc.get(k)
            for k in ("byte_size", "coverage_bytes", "region_count", "field_count",
                      "ownership_count", "annotation_count", "unknown_range_count",
                      "region_conflict_count", "ownership_conflict_count")
        },
        "diagnostics": [
            {"severity": d.get("severity"), "code": d.get("code"), "message": d.get("message")}
            for d in diagnostics if isinstance(d, dict)
        ],
        "duration_ms": result.get("duration_ms"),
    }


def summarize_scm(result: dict[str, Any]) -> dict[str, Any]:
    obj = result.get("json") if result.get("ok") else None
    if not isinstance(obj, dict):
        return {k: result.get(k) for k in ("ok", "exit_code", "duration_ms", "parse_error", "stdout_tail", "stderr_tail")}
    return {
        "ok": True,
        "byte_size": obj.get("byte_size"),
        "coverage_bytes": obj.get("coverage_bytes"),
        "region_count": len(obj.get("regions") or []),
        "field_count": len(obj.get("fields") or []),
        "annotation_count": len(obj.get("annotations") or []),
        "unknown_range_count": len(obj.get("unknown_ranges") or []),
        "region_conflict_count": len(obj.get("region_conflicts") or []),
        "ownership_conflict_count": len(obj.get("ownership_conflicts") or []),
        "duration_ms": result.get("duration_ms"),
    }


def collect_stage_dirs(source: Path) -> list[Path]:
    return sorted(
        (p for p in source.iterdir() if p.is_dir() and STAGE_RE.match(p.name)),
        key=lambda p: p.name.lower(),
    )


def main() -> int:
    args = parse_args()
    source = args.source.resolve()
    cli = args.cli.resolve()
    # Do not call Path.resolve() here: on Windows it follows an existing
    # junction and turns the ASCII alias back into the Cyrillic target.
    alias = Path(os.path.abspath(args.alias))
    if not source.is_dir():
        raise SystemExit(f"source is not a directory: {source}")
    if not cli.is_file():
        raise SystemExit(f"native reader is not a file: {cli}")
    ensure_alias(source, alias)

    stage_dirs = collect_stage_dirs(source)
    entries: list[tuple[str, Path, Path]] = []
    for stage_dir in stage_dirs:
        for scm in sorted(stage_dir.rglob("*.scm")):
            rel = scm.relative_to(source)
            entries.append((stage_dir.name, scm, alias / rel))

    ptx_entries: list[dict[str, Any]] = []
    for ptx in sorted(source.rglob("*.ptx")):
        rel = ptx.relative_to(source)
        native = run_native(cli, "inspect-workspace", alias / rel, args.timeout)
        ptx_entries.append({
            "path": str(rel),
            "size": ptx.stat().st_size,
            "native": summarize_workspace(native),
        })

    def inspect_one(item: tuple[str, Path, Path]) -> dict[str, Any]:
        stage, original, native_path = item
        result = run_native(cli, "inspect-workspace", native_path, args.timeout)
        row: dict[str, Any] = {
            "stage": stage,
            "path": str(original.relative_to(source)),
            "size": original.stat().st_size,
            "native": summarize_workspace(result),
            "ptx_overlay": {
                "available": bool(ptx_entries),
                "bundles": [p["path"] for p in ptx_entries],
                "mode": "structural-association-only",
            },
        }
        return row

    rows: list[dict[str, Any]] = []
    with ThreadPoolExecutor(max_workers=max(1, args.workers)) as pool:
        futures = [pool.submit(inspect_one, item) for item in entries]
        for future in as_completed(futures):
            rows.append(future.result())
    rows.sort(key=lambda r: (r["stage"].lower(), r["path"].lower()))

    # A small detailed SCM sample preserves the native reader's full region and
    # field evidence without making the default report hundreds of megabytes.
    sample_paths: list[tuple[str, Path]] = []
    for stage in sorted({r["stage"] for r in rows}, key=str.lower):
        stage_rows = [r for r in rows if r["stage"] == stage]
        for row in stage_rows[: max(0, args.sample_scm)]:
            sample_paths.append((stage, source / row["path"]))
    if args.full_scm:
        sample_paths = [(r["stage"], source / r["path"]) for r in rows]

    detailed: list[dict[str, Any]] = []
    for stage, original in sample_paths:
        native_path = alias / original.relative_to(source)
        result = run_native(cli, "inspect-scm", native_path, args.timeout)
        detailed.append({"stage": stage, "path": str(original.relative_to(source)), "native": summarize_scm(result)})

    native_ok = sum(1 for r in rows if r["native"].get("ok"))
    native_failed = len(rows) - native_ok
    diagnostics = sum(len(r["native"].get("diagnostics", [])) for r in rows if r["native"].get("ok"))
    report = {
        "schema_version": 1,
        "tool": "native_reader_scene_check.py",
        "created_utc": time.strftime("%Y-%m-%dT%H:%M:%SZ", time.gmtime()),
        "source": str(source),
        "native_reader": str(cli),
        "alias": str(alias),
        "scope": {"stage_directories": len(stage_dirs), "scm_files": len(rows), "ptx_files": len(ptx_entries)},
        "native_workspace_summary": {
            "scm_ok": native_ok,
            "scm_failed": native_failed,
            "diagnostic_count": diagnostics,
            "detailed_scm_count": len(detailed),
        },
        "review_request": {
            "info": True,
            "low_resolution_requested": bool(args.low_res),
            "low_resolution_cli_support": False,
            "texture_overlay_cli_support": False,
            "note": "Native Reader CLI exposes structural Info for SCM/PTX. GUI viewport overlay and low-resolution rendering are not exposed by this CLI build.",
        },
        "ptx": ptx_entries,
        "scenes": rows,
        "detailed_scm_samples": detailed,
    }
    args.output.parent.mkdir(parents=True, exist_ok=True)
    args.output.write_text(json.dumps(report, ensure_ascii=False, indent=2), encoding="utf-8")
    print(json.dumps({
        "output": str(args.output),
        "stage_directories": len(stage_dirs),
        "scm_files": len(rows),
        "native_ok": native_ok,
        "native_failed": native_failed,
        "ptx_files": len(ptx_entries),
        "detailed_scm": len(detailed),
    }, ensure_ascii=False))
    return 0 if native_failed == 0 else 2


if __name__ == "__main__":
    try:
        raise SystemExit(main())
    except KeyboardInterrupt:
        raise SystemExit(130)
