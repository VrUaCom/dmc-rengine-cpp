#!/usr/bin/env python3
"""Import the canonical EventTbl research census into the local SQLite DB.

Uses only Python's standard library. The database is intentionally local and
regenerable; Git tracks schema + evidence sources instead of a mutable binary
.sqlite file.
"""

from __future__ import annotations

import argparse
import json
import sqlite3
from pathlib import Path

ROOT = Path(__file__).resolve().parents[2]
DEFAULT_SCHEMA = ROOT / "research" / "sql" / "001_schema.sql"
DEFAULT_CENSUS = ROOT / "docs" / "research" / "dmc3-eventtbl-opcode-census-2026-09-12.json"
DEFAULT_DB = ROOT / "research-private" / "dmc_rengine_research.sqlite3"


def parse_int(value):
    if value is None:
        return None
    if isinstance(value, int):
        return value
    return int(value, 0)


def ensure_parent(path: Path) -> None:
    path.parent.mkdir(parents=True, exist_ok=True)


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("--db", type=Path, default=DEFAULT_DB)
    parser.add_argument("--schema", type=Path, default=DEFAULT_SCHEMA)
    parser.add_argument("--census", type=Path, default=DEFAULT_CENSUS)
    args = parser.parse_args()

    ensure_parent(args.db)
    census = json.loads(args.census.read_text(encoding="utf-8"))

    con = sqlite3.connect(args.db)
    con.execute("PRAGMA foreign_keys = ON")
    con.executescript(args.schema.read_text(encoding="utf-8"))

    with con:
        fmt = census["format"]
        con.execute(
            """
            INSERT INTO format_family(family, magic_text, stock_extension, description)
            VALUES(?, ?, ?, ?)
            ON CONFLICT(family) DO UPDATE SET
                magic_text=excluded.magic_text,
                stock_extension=excluded.stock_extension,
                description=excluded.description
            """,
            (
                fmt["family"],
                fmt["magic"],
                fmt["stock_extension"],
                "DMC3 EventTbl bytecode/event-control format",
            ),
        )
        format_id = con.execute(
            "SELECT id FROM format_family WHERE family=?", (fmt["family"],)
        ).fetchone()[0]

        sources = [
            (
                "archive_corpus",
                "GData.afs EventTbl00-09 owner-supplied corpus",
                "OWNER_SUPPLIED_COMPLETE_GDATA_EVENTTBL_SUBSET",
                "Complete owner-supplied GData.afs EventTbl subset currently identified as 00..09.",
            ),
            (
                "additional_corpus",
                "Additional EventTbl13-21 owner-supplied samples",
                "OWNER_SUPPLIED_ADDITIONAL",
                "Useful EventTbl corpus evidence; source is intentionally not relabelled as GData.afs.",
            ),
            (
                "executable",
                "canonical dmc3.exe",
                "EXE_CONFIRMED",
                "Canonical DMC3 HD executable authority for loader/dispatcher semantics.",
            ),
        ]
        for kind, name, provenance, notes in sources:
            con.execute(
                """
                INSERT INTO source_artifact(source_kind, name, provenance, notes)
                VALUES(?, ?, ?, ?)
                ON CONFLICT(source_kind, name, sha256) DO NOTHING
                """,
                (kind, name, provenance, notes),
            )

        source_ids = {
            row[0]: row[1]
            for row in con.execute("SELECT name, id FROM source_artifact").fetchall()
        }

        scope = census["scope"]
        meta = {
            "eventtbl_census_schema": census["schema"],
            "eventtbl_census_date": census["date"],
            "eventtbl_files_analyzed": str(scope["files_analyzed"]),
            "eventtbl_commands": str(scope["commands"]),
            "eventtbl_distinct_observed_opcodes": str(scope["distinct_observed_opcodes"]),
            "eventtbl_max_opcode": scope["max_opcode"],
            "eventtbl_max_argument_count": str(scope["max_argument_count"]),
        }
        for key, value in meta.items():
            con.execute(
                "INSERT INTO research_meta(key,value) VALUES(?,?) "
                "ON CONFLICT(key) DO UPDATE SET value=excluded.value",
                (key, value),
            )

        # Full opcode census. Unknown semantics stay PRESERVED_UNDECODED until
        # promoted by corpus or executable evidence.
        columns = census["opcode_census_columns"]
        if columns != ["opcode", "argument_count", "command_count", "file_count"]:
            raise RuntimeError(f"unexpected opcode census columns: {columns}")

        for opcode_text, argc, command_count, file_count in census["opcode_census"]:
            opcode = parse_int(opcode_text)
            con.execute(
                """
                INSERT INTO evt_opcode(
                    opcode, observed_argument_count, observed_command_count,
                    observed_file_count, observed_in_supplied_corpus,
                    semantic_name, semantic_class, evidence_status)
                VALUES(?, ?, ?, ?, 1, NULL, 'unknown', 'PRESERVED_UNDECODED')
                ON CONFLICT(opcode) DO UPDATE SET
                    observed_argument_count=excluded.observed_argument_count,
                    observed_command_count=excluded.observed_command_count,
                    observed_file_count=excluded.observed_file_count,
                    observed_in_supplied_corpus=1
                """,
                (opcode, argc, command_count, file_count),
            )
            con.execute(
                """
                INSERT INTO evt_opcode_census(opcode, corpus_scope, command_count, file_count)
                VALUES(?, 'all_supplied_2026-09-12', ?, ?)
                ON CONFLICT(opcode, corpus_scope) DO UPDATE SET
                    command_count=excluded.command_count,
                    file_count=excluded.file_count
                """,
                (opcode, command_count, file_count),
            )

        # Promote only semantics explicitly present in the machine-readable registry.
        for item in census["semantic_registry"]:
            opcode = parse_int(item["opcode"])
            observed = 1 if item.get("observed_in_supplied_corpus", True) else 0
            con.execute(
                """
                INSERT INTO evt_opcode(
                    opcode, observed_in_supplied_corpus, semantic_name,
                    semantic_class, evidence_status)
                VALUES(?, ?, ?, ?, ?)
                ON CONFLICT(opcode) DO UPDATE SET
                    observed_in_supplied_corpus=excluded.observed_in_supplied_corpus,
                    semantic_name=excluded.semantic_name,
                    semantic_class=excluded.semantic_class,
                    evidence_status=excluded.evidence_status
                """,
                (opcode, observed, item["name"], item["class"], item["evidence"]),
            )

        for pair in census["structural_pairs"]:
            con.execute(
                """
                INSERT INTO evt_structural_pair(
                    begin_opcode, end_opcode, begin_argument_count,
                    end_argument_count, evidence_status)
                VALUES(?, ?, ?, ?, ?)
                ON CONFLICT(begin_opcode, end_opcode) DO UPDATE SET
                    begin_argument_count=excluded.begin_argument_count,
                    end_argument_count=excluded.end_argument_count,
                    evidence_status=excluded.evidence_status
                """,
                (
                    parse_int(pair["begin"]),
                    parse_int(pair["end"]),
                    pair["begin_arity"],
                    pair["end_arity"],
                    pair["evidence"],
                ),
            )

        def import_files(items, corpus_group, source_name):
            source_id = source_ids[source_name]
            for item in items:
                idx = item["index"]
                name = f"EventTbl{idx:02d}.bin"
                terminal = parse_int(item.get("terminal"))
                con.execute(
                    """
                    INSERT INTO resource_file(
                        format_id, source_artifact_id, stock_name, stock_extension,
                        corpus_group, corpus_index, size_bytes, sha256, magic_text,
                        revision, stream_count, terminal_offset, command_count,
                        evidence_status)
                    VALUES(?, ?, ?, '.bin', ?, ?, ?, ?, 'EVT\\0', 1, ?, ?, ?, 'STRUCTURAL_CONFIRMED')
                    ON CONFLICT(corpus_group, stock_name, sha256) DO UPDATE SET
                        size_bytes=excluded.size_bytes,
                        stream_count=excluded.stream_count,
                        terminal_offset=excluded.terminal_offset,
                        command_count=excluded.command_count,
                        evidence_status=excluded.evidence_status
                    """,
                    (
                        format_id,
                        source_id,
                        name,
                        corpus_group,
                        idx,
                        item.get("size"),
                        item.get("sha256"),
                        item.get("streams"),
                        terminal,
                        item.get("commands"),
                    ),
                )
                file_id = con.execute(
                    "SELECT id FROM resource_file WHERE corpus_group=? AND stock_name=? AND sha256=?",
                    (corpus_group, name, item.get("sha256")),
                ).fetchone()[0]
                for ordinal, root in enumerate(item.get("roots", [])):
                    root_offset = parse_int(root)
                    con.execute(
                        """
                        INSERT INTO evt_stream(file_id, stream_ordinal, root_offset, evidence_status)
                        VALUES(?, ?, ?, 'STRUCTURAL_CONFIRMED')
                        ON CONFLICT(file_id, stream_ordinal) DO UPDATE SET
                            root_offset=excluded.root_offset,
                            evidence_status=excluded.evidence_status
                        """,
                        (file_id, ordinal, root_offset),
                    )

        import_files(
            census["gdata_files"],
            "gdata_00_09",
            "GData.afs EventTbl00-09 owner-supplied corpus",
        )
        import_files(
            census["additional_files"],
            "additional_13_21",
            "Additional EventTbl13-21 owner-supplied samples",
        )

        claims = [
            (
                "format",
                "EventTbl",
                "Content family is identified by EVT\\0 magic; stock files retain generic .bin extension.",
                "EXE_AND_CORPUS_CONFIRMED",
                "docs/research/dmc3-eventtbl-corpus-reverse-2026-09-12.md",
            ),
            (
                "format",
                "EventTbl.command_descriptor",
                "bits0..7=opcode, bits8..15=u32 argument count, bits16..31=0 in supplied corpus.",
                "STRUCTURAL_CONFIRMED",
                "docs/research/dmc3-eventtbl-opcode-census-2026-09-12.json",
            ),
            (
                "format",
                "EventTbl.streams",
                "First stream root is implicit at 0x20; additional roots are stored after terminal opcode 0x20.",
                "STRUCTURAL_CONFIRMED",
                "docs/research/dmc3-eventtbl-corpus-reverse-2026-09-12.md",
            ),
        ]
        for subject_type, subject_key, claim, status, locator in claims:
            con.execute(
                """
                INSERT INTO evidence_claim(
                    subject_type, subject_key, claim, evidence_status,
                    source_kind, source_locator)
                VALUES(?, ?, ?, ?, 'repository', ?)
                ON CONFLICT(subject_type, subject_key, claim, source_locator) DO UPDATE SET
                    evidence_status=excluded.evidence_status
                """,
                (subject_type, subject_key, claim, status, locator),
            )

    # Basic integrity checks catch drift between the JSON source and SQL import.
    expected_files = census["scope"]["files_analyzed"]
    imported_files = con.execute(
        "SELECT COUNT(*) FROM resource_file WHERE format_id=?", (format_id,)
    ).fetchone()[0]
    if imported_files != expected_files:
        raise RuntimeError(f"file count mismatch: expected {expected_files}, got {imported_files}")

    imported_opcodes = con.execute(
        "SELECT COUNT(*) FROM evt_opcode WHERE observed_in_supplied_corpus=1"
    ).fetchone()[0]
    expected_opcodes = census["scope"]["distinct_observed_opcodes"]
    if imported_opcodes != expected_opcodes:
        raise RuntimeError(
            f"opcode count mismatch: expected {expected_opcodes}, got {imported_opcodes}"
        )

    con.close()
    print(args.db)
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
