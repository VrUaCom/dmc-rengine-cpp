#!/usr/bin/env python3
"""Corpus-neighbourhood analysis for the full EventTbl00..21 runtime family.

Reads the normalized SQLite database produced by import_eventtbl_runtime_family.py.
The analysis is deliberately semantic-neutral: it reports argument domains,
file coverage and neighbouring opcode distributions without inventing handler
names. Missing runtime slots remain visible through evt_runtime_slot but do not
contribute fake byte evidence.
"""

from __future__ import annotations

import argparse
import json
import sqlite3
from collections import Counter, defaultdict
from pathlib import Path

ROOT = Path(__file__).resolve().parents[2]
DEFAULT_DB = ROOT / "research-private" / "dmc_rengine_research.sqlite3"


def parse_opcode(text: str) -> int:
    value = int(text, 0)
    if not 0 <= value <= 0xFF:
        raise argparse.ArgumentTypeError("opcode must be in 0x00..0xFF")
    return value


def top(counter: Counter[int], limit: int) -> list[dict[str, int | str]]:
    return [
        {"opcode": f"0x{opcode:02X}", "count": count}
        for opcode, count in counter.most_common(limit)
    ]


def main() -> int:
    ap = argparse.ArgumentParser()
    ap.add_argument("--db", type=Path, default=DEFAULT_DB)
    ap.add_argument(
        "--opcode",
        type=parse_opcode,
        action="append",
        required=True,
        help="opcode to analyse; repeat for several opcodes, e.g. --opcode 0x2C",
    )
    ap.add_argument("--top", type=int, default=15)
    ap.add_argument("--output", type=Path)
    args = ap.parse_args()

    con = sqlite3.connect(args.db)
    con.row_factory = sqlite3.Row

    slot_rows = con.execute(
        "SELECT runtime_index, stock_name, bytes_status FROM evt_runtime_slot ORDER BY runtime_index"
    ).fetchall()
    slots = [dict(row) for row in slot_rows]

    results: list[dict] = []
    for opcode in args.opcode:
        commands = con.execute(
            """
            SELECT c.id, c.file_id, c.sequence_index, c.file_offset,
                   f.stock_name, f.corpus_index
            FROM evt_command AS c
            JOIN resource_file AS f ON f.id = c.file_id
            WHERE c.opcode = ?
            ORDER BY f.corpus_index, c.sequence_index
            """,
            (opcode,),
        ).fetchall()

        by_file: Counter[str] = Counter()
        prev_ops: Counter[int] = Counter()
        next_ops: Counter[int] = Counter()
        arg_values: dict[int, list[int]] = defaultdict(list)

        for row in commands:
            by_file[row["stock_name"]] += 1
            prev_row = con.execute(
                "SELECT opcode FROM evt_command WHERE file_id=? AND sequence_index=?",
                (row["file_id"], row["sequence_index"] - 1),
            ).fetchone()
            next_row = con.execute(
                "SELECT opcode FROM evt_command WHERE file_id=? AND sequence_index=?",
                (row["file_id"], row["sequence_index"] + 1),
            ).fetchone()
            if prev_row is not None:
                prev_ops[prev_row[0]] += 1
            if next_row is not None:
                next_ops[next_row[0]] += 1

            for arg in con.execute(
                "SELECT argument_index, value_u32 FROM evt_argument WHERE command_id=? ORDER BY argument_index",
                (row["id"],),
            ):
                arg_values[arg[0]].append(arg[1])

        arg_domains = []
        for index in sorted(arg_values):
            values = arg_values[index]
            counts = Counter(values)
            arg_domains.append(
                {
                    "argument_index": index,
                    "count": len(values),
                    "distinct": len(counts),
                    "minimum_u32": min(values),
                    "maximum_u32": max(values),
                    "top_values": [
                        {"value_u32": value, "value_hex": f"0x{value:X}", "count": count}
                        for value, count in counts.most_common(args.top)
                    ],
                }
            )

        results.append(
            {
                "opcode": f"0x{opcode:02X}",
                "command_count": len(commands),
                "file_count": len(by_file),
                "by_file": [
                    {"file": name, "count": count}
                    for name, count in sorted(by_file.items())
                ],
                "arguments": arg_domains,
                "previous_opcodes": top(prev_ops, args.top),
                "next_opcodes": top(next_ops, args.top),
                "semantic_status": "PRESERVED_UNDECODED",
            }
        )

    payload = {
        "schema": "dmc-rengine.eventtbl-semantic-corpus-analysis.v1",
        "runtime_family": "EventTbl00.bin..EventTbl21.bin",
        "runtime_slots": 22,
        "slot_coverage": slots,
        "analysis_rule": (
            "Corpus statistics are evidence, not semantic names. Missing byte slots do not "
            "contribute inferred commands. Promote semantics only with explicit evidence."
        ),
        "opcodes": results,
    }

    encoded = json.dumps(payload, indent=2, ensure_ascii=False) + "\n"
    if args.output:
        args.output.parent.mkdir(parents=True, exist_ok=True)
        args.output.write_text(encoded, encoding="utf-8")
    else:
        print(encoded, end="")

    con.close()
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
