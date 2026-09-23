#!/usr/bin/env python3
"""Checks the figures evidence records declare against a fresh map report.

A summary is prose and its numbers age silently: a later fix moves a count and
the sentence claiming it stays as it was. This reverse produced four such stale
records before the check existed, all found by hand.

Only records that declare a `figures` map are checked, and only against
counters the report carries. A record superseded by another is skipped, because
a correction's whole point is that the old figures no longer hold.
"""

from __future__ import annotations

import argparse
import json
from pathlib import Path
import sys


def check(evidence_dir: Path, report: Path) -> int:
    summary = json.loads(report.read_text(encoding="utf-8")).get("summary", {})
    if not summary:
        print(f"{report}: no summary to check against", file=sys.stderr)
        return 2

    checked = 0
    stale: list[str] = []
    unknown: list[str] = []
    packets = [
        json.loads(path.read_text(encoding="utf-8"))
        for path in sorted(evidence_dir.glob("*.evidence.json"))
    ]
    # A correction may live in a different packet from the record it corrects,
    # so what is superseded is decided across all of them, not per packet.
    superseded = {
        target
        for packet in packets
        for record in packet.get("records", [])
        for target in record.get("supersedes", [])
    }
    for packet in packets:
        for record in packet.get("records", []):
            if record["id"] in superseded:
                continue
            for counter, claimed in record.get("figures", {}).items():
                if counter not in summary:
                    unknown.append(f"{record['id']}: no counter named {counter}")
                    continue
                checked += 1
                if summary[counter] != claimed:
                    stale.append(
                        f"{record['id']} claims {counter}={claimed}, "
                        f"the report says {summary[counter]}"
                    )

    for line in unknown:
        print(f"unknown: {line}", file=sys.stderr)
    for line in stale:
        print(f"stale:   {line}", file=sys.stderr)
    print(f"{checked} declared figure(s) checked, {len(stale)} stale, {len(unknown)} unknown")
    return 1 if (stale or unknown) else 0


def main() -> int:
    ap = argparse.ArgumentParser()
    ap.add_argument("--evidence", type=Path, default=Path("evidence/executable"))
    ap.add_argument("--map", type=Path, required=True, help="map-functions report JSON")
    args = ap.parse_args()
    return check(args.evidence, args.map)


if __name__ == "__main__":
    raise SystemExit(main())
