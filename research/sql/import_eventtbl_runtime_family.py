#!/usr/bin/env python3
"""Build the EventTbl research database from the full runtime slot family 00..21.

The runtime family is always 22 slots: EventTbl00.bin through EventTbl21.bin.
Every slot is recorded even when its byte payload is not currently available.
For every available valid EVT payload, this importer stores every stream, command,
and raw u32 argument in SQLite. It can optionally merge the canonical semantic
registry/census JSON without promoting unknown semantics on its own.
"""

from __future__ import annotations

import argparse
import hashlib
import json
import sqlite3
import struct
from dataclasses import dataclass
from pathlib import Path

ROOT = Path(__file__).resolve().parents[2]
DEFAULT_SCHEMA = ROOT / "research" / "sql" / "001_schema.sql"
DEFAULT_RUNTIME_SCHEMA = ROOT / "research" / "sql" / "002_eventtbl_runtime_family.sql"
DEFAULT_CENSUS = ROOT / "docs" / "research" / "dmc3-eventtbl-opcode-census-2026-09-12.json"
DEFAULT_DB = ROOT / "research-private" / "dmc_rengine_research.sqlite3"
DEFAULT_EVENTTBL_DIR = ROOT / "research-private" / "eventtbl"

MAGIC = b"EVT\0"
HEADER_SIZE = 0x20
TERMINAL_OPCODE = 0x20
RUNTIME_INDICES = range(22)


@dataclass(frozen=True)
class Command:
    offset: int
    raw_descriptor: int
    opcode: int
    argc: int
    args: tuple[int, ...]


@dataclass(frozen=True)
class ParsedEventTbl:
    revision: int
    stream_count: int
    terminal_offset: int
    stream_roots: tuple[int, ...]
    commands: tuple[Command, ...]


def u32(data: bytes, offset: int) -> int:
    if offset < 0 or offset + 4 > len(data):
        raise ValueError(f"u32 out of bounds at 0x{offset:X}")
    return struct.unpack_from("<I", data, offset)[0]


def parse_evt(data: bytes) -> ParsedEventTbl:
    if len(data) < HEADER_SIZE:
        raise ValueError("payload shorter than 0x20-byte EVT header")
    if data[:4] != MAGIC:
        raise ValueError("missing EVT\\0 magic")

    packed = u32(data, 0x04)
    revision = packed & 0xFFFF
    stream_count = (packed >> 16) & 0xFFFF
    terminal = u32(data, 0x08)
    if stream_count == 0:
        raise ValueError("stream_count is zero")
    if terminal < HEADER_SIZE or terminal + 4 > len(data) or terminal % 4:
        raise ValueError(f"invalid terminal offset 0x{terminal:X}")

    commands: list[Command] = []
    cursor = HEADER_SIZE
    while True:
        descriptor = u32(data, cursor)
        if descriptor & 0xFFFF0000:
            raise ValueError(f"command upper descriptor bits non-zero at 0x{cursor:X}")
        opcode = descriptor & 0xFF
        argc = (descriptor >> 8) & 0xFF
        size = 4 + argc * 4
        if cursor + size > len(data):
            raise ValueError(f"command escapes file at 0x{cursor:X}")
        if cursor < terminal and cursor + size > terminal:
            raise ValueError(f"command crosses terminal boundary at 0x{cursor:X}")
        args = tuple(u32(data, cursor + 4 + i * 4) for i in range(argc))
        commands.append(Command(cursor, descriptor, opcode, argc, args))
        if cursor == terminal:
            break
        cursor += size
        if cursor > terminal:
            raise ValueError("command walk skipped terminal offset")

    if commands[-1].opcode != TERMINAL_OPCODE or commands[-1].argc != 0:
        raise ValueError("terminal offset does not contain opcode 0x20/argc0")

    roots = [HEADER_SIZE]
    table = terminal + 4
    for i in range(stream_count - 1):
        root = u32(data, table + i * 4)
        if root < HEADER_SIZE or root >= terminal or root % 4:
            raise ValueError(f"invalid stream root 0x{root:X}")
        if root <= roots[-1]:
            raise ValueError("stream roots are not strictly increasing")
        roots.append(root)

    command_offsets = {c.offset for c in commands}
    for root in roots:
        if root not in command_offsets:
            raise ValueError(f"stream root 0x{root:X} is not a command boundary")

    return ParsedEventTbl(revision, stream_count, terminal, tuple(roots), tuple(commands))


def stream_for_offset(roots: tuple[int, ...], offset: int) -> int:
    ordinal = 0
    for i, root in enumerate(roots):
        if root <= offset:
            ordinal = i
        else:
            break
    return ordinal


def load_census(path: Path | None) -> dict | None:
    if path is None or not path.exists():
        return None
    return json.loads(path.read_text(encoding="utf-8"))


def main() -> int:
    ap = argparse.ArgumentParser()
    ap.add_argument("--db", type=Path, default=DEFAULT_DB)
    ap.add_argument("--eventtbl-dir", type=Path, default=DEFAULT_EVENTTBL_DIR)
    ap.add_argument("--schema", type=Path, default=DEFAULT_SCHEMA)
    ap.add_argument("--runtime-schema", type=Path, default=DEFAULT_RUNTIME_SCHEMA)
    ap.add_argument("--census", type=Path, default=DEFAULT_CENSUS)
    ap.add_argument("--require-all-bytes", action="store_true")
    args = ap.parse_args()

    args.db.parent.mkdir(parents=True, exist_ok=True)
    con = sqlite3.connect(args.db)
    con.execute("PRAGMA foreign_keys = ON")
    con.executescript(args.schema.read_text(encoding="utf-8"))
    con.executescript(args.runtime_schema.read_text(encoding="utf-8"))
    census = load_census(args.census)

    with con:
        con.execute(
            """
            INSERT INTO format_family(family, magic_text, stock_extension, description)
            VALUES('EventTbl','EVT\\0','.bin','DMC3 EventTbl bytecode/event-control format')
            ON CONFLICT(family) DO UPDATE SET
              magic_text=excluded.magic_text,
              stock_extension=excluded.stock_extension,
              description=excluded.description
            """
        )
        format_id = con.execute(
            "SELECT id FROM format_family WHERE family='EventTbl'"
        ).fetchone()[0]

        con.execute(
            """
            INSERT INTO source_artifact(source_kind,name,provenance,notes)
            VALUES('runtime_family','DMC3 EventTbl00-21 runtime family','EXE_CONFIRMED',
                   'Canonical runtime family contains 22 stock slots, indices 0 through 21.')
            ON CONFLICT(source_kind,name,sha256) DO NOTHING
            """
        )
        source_id = con.execute(
            "SELECT id FROM source_artifact WHERE source_kind='runtime_family' AND name='DMC3 EventTbl00-21 runtime family' ORDER BY id LIMIT 1"
        ).fetchone()[0]

        old_file_ids = [r[0] for r in con.execute(
            "SELECT id FROM resource_file WHERE format_id=?", (format_id,)
        )]
        con.execute("DELETE FROM evt_runtime_slot")
        if old_file_ids:
            placeholders = ",".join("?" for _ in old_file_ids)
            con.execute(f"DELETE FROM resource_file WHERE id IN ({placeholders})", old_file_ids)
        con.execute("DELETE FROM evt_opcode_census WHERE corpus_scope='runtime_family_00_21_available_bytes'")
        con.execute("DELETE FROM evt_structural_pair")
        con.execute("DELETE FROM evt_opcode")

        observed_counts: dict[int, int] = {}
        observed_files: dict[int, set[int]] = {}
        observed_arity: dict[int, int] = {}
        parsed_slots = 0
        parsed_commands = 0
        missing: list[int] = []

        for idx in RUNTIME_INDICES:
            name = f"EventTbl{idx:02d}.bin"
            runtime_path = f"eventtbl\\\\{name}"
            path = args.eventtbl_dir / name
            if not path.exists():
                missing.append(idx)
                con.execute(
                    """
                    INSERT INTO evt_runtime_slot(runtime_index,stock_name,runtime_path,bytes_status,notes)
                    VALUES(?,?,?,'MISSING_BYTES','Runtime slot is part of the 00..21 family; byte payload is not present in this local corpus.')
                    """,
                    (idx, name, runtime_path),
                )
                continue

            data = path.read_bytes()
            digest = hashlib.sha256(data).hexdigest()
            try:
                parsed = parse_evt(data)
            except ValueError as exc:
                con.execute(
                    """
                    INSERT INTO evt_runtime_slot(runtime_index,stock_name,runtime_path,bytes_status,notes)
                    VALUES(?,?,?,'INVALID_BYTES',?)
                    """,
                    (idx, name, runtime_path, str(exc)),
                )
                continue

            con.execute(
                """
                INSERT INTO resource_file(
                    format_id,source_artifact_id,stock_name,stock_extension,
                    corpus_group,corpus_index,size_bytes,sha256,magic_text,
                    revision,stream_count,terminal_offset,command_count,evidence_status,notes)
                VALUES(?,?,?,'.bin','runtime_family_00_21',?,?,?,?,?,?,?,?,
                       'STRUCTURAL_CONFIRMED','Parsed directly from supplied/local bytes.')
                """,
                (
                    format_id, source_id, name, idx, len(data), digest, 'EVT\\0',
                    parsed.revision, parsed.stream_count, parsed.terminal_offset,
                    len(parsed.commands),
                ),
            )
            file_id = con.execute("SELECT last_insert_rowid()").fetchone()[0]
            con.execute(
                """
                INSERT INTO evt_runtime_slot(runtime_index,stock_name,runtime_path,resource_file_id,bytes_status)
                VALUES(?,?,?,?,'AVAILABLE_PARSED')
                """,
                (idx, name, runtime_path, file_id),
            )

            for ordinal, root in enumerate(parsed.stream_roots):
                entry = next(c for c in parsed.commands if c.offset == root)
                con.execute(
                    """
                    INSERT INTO evt_stream(
                        file_id,stream_ordinal,root_offset,entry_opcode,
                        entry_argument_count,evidence_status)
                    VALUES(?,?,?,?,?,'STRUCTURAL_CONFIRMED')
                    """,
                    (file_id, ordinal, root, entry.opcode, entry.argc),
                )

            for seq, cmd in enumerate(parsed.commands):
                old_arity = observed_arity.setdefault(cmd.opcode, cmd.argc)
                if old_arity != cmd.argc:
                    raise RuntimeError(
                        f"opcode 0x{cmd.opcode:02X} arity drift: {old_arity} vs {cmd.argc}"
                    )
                observed_counts[cmd.opcode] = observed_counts.get(cmd.opcode, 0) + 1
                observed_files.setdefault(cmd.opcode, set()).add(idx)

                con.execute(
                    """
                    INSERT INTO evt_opcode(
                        opcode,observed_argument_count,observed_command_count,
                        observed_file_count,observed_in_supplied_corpus,
                        semantic_class,evidence_status)
                    VALUES(?,?,0,0,1,'unknown','PRESERVED_UNDECODED')
                    ON CONFLICT(opcode) DO UPDATE SET
                      observed_argument_count=excluded.observed_argument_count,
                      observed_in_supplied_corpus=1
                    """,
                    (cmd.opcode, cmd.argc),
                )
                stream_ordinal = stream_for_offset(parsed.stream_roots, cmd.offset)
                con.execute(
                    """
                    INSERT INTO evt_command(
                        file_id,stream_ordinal,sequence_index,file_offset,
                        raw_descriptor,opcode,argument_count,evidence_status)
                    VALUES(?,?,?,?,?,?,?,'STRUCTURAL_CONFIRMED')
                    """,
                    (file_id, stream_ordinal, seq, cmd.offset, cmd.raw_descriptor, cmd.opcode, cmd.argc),
                )
                command_id = con.execute("SELECT last_insert_rowid()").fetchone()[0]
                for arg_index, value in enumerate(cmd.args):
                    con.execute(
                        """
                        INSERT INTO evt_argument(command_id,argument_index,value_u32,evidence_status)
                        VALUES(?,?,?,'PRESERVED_UNDECODED')
                        """,
                        (command_id, arg_index, value),
                    )

            parsed_slots += 1
            parsed_commands += len(parsed.commands)

        for opcode, count in observed_counts.items():
            file_count = len(observed_files[opcode])
            con.execute(
                """UPDATE evt_opcode SET observed_command_count=?, observed_file_count=? WHERE opcode=?""",
                (count, file_count, opcode),
            )
            con.execute(
                """
                INSERT INTO evt_opcode_census(opcode,corpus_scope,command_count,file_count)
                VALUES(?,'runtime_family_00_21_available_bytes',?,?)
                """,
                (opcode, count, file_count),
            )

        if census:
            for item in census.get("semantic_registry", []):
                opcode = int(item["opcode"], 0)
                con.execute(
                    """
                    INSERT INTO evt_opcode(
                        opcode,observed_in_supplied_corpus,semantic_name,
                        semantic_class,evidence_status)
                    VALUES(?,?,?,?,?)
                    ON CONFLICT(opcode) DO UPDATE SET
                      semantic_name=excluded.semantic_name,
                      semantic_class=excluded.semantic_class,
                      evidence_status=excluded.evidence_status
                    """,
                    (
                        opcode,
                        1 if opcode in observed_counts else 0,
                        item["name"], item["class"], item["evidence"],
                    ),
                )
            for pair in census.get("structural_pairs", []):
                con.execute(
                    """
                    INSERT INTO evt_structural_pair(
                        begin_opcode,end_opcode,begin_argument_count,end_argument_count,
                        evidence_status)
                    VALUES(?,?,?,?,?)
                    """,
                    (
                        int(pair["begin"], 0), int(pair["end"], 0),
                        pair["begin_arity"], pair["end_arity"], pair["evidence"],
                    ),
                )

        meta = {
            "eventtbl_runtime_slot_min": "0",
            "eventtbl_runtime_slot_max": "21",
            "eventtbl_runtime_slot_count": "22",
            "eventtbl_available_parsed_slots": str(parsed_slots),
            "eventtbl_missing_byte_slots": ",".join(f"{i:02d}" for i in missing),
            "eventtbl_instance_commands": str(parsed_commands),
            "eventtbl_instance_arguments": str(con.execute("SELECT COUNT(*) FROM evt_argument").fetchone()[0]),
        }
        for key, value in meta.items():
            con.execute(
                """
                INSERT INTO research_meta(key,value) VALUES(?,?)
                ON CONFLICT(key) DO UPDATE SET value=excluded.value
                """,
                (key, value),
            )

        con.execute(
            """
            INSERT INTO evidence_claim(
                subject_type,subject_key,claim,evidence_status,source_kind,source_locator)
            VALUES('runtime_family','EventTbl00-21',
                   'Canonical EventTbl runtime family is represented as 22 indexed slots 0 through 21.',
                   'EXE_CONFIRMED','executable','eventtbl\\\\EventTbl00.bin .. eventtbl\\\\EventTbl21.bin')
            ON CONFLICT(subject_type,subject_key,claim,source_locator) DO UPDATE SET
              evidence_status=excluded.evidence_status
            """
        )

    if args.require_all_bytes and missing:
        raise SystemExit(
            "missing required EventTbl byte payloads: " + ", ".join(f"{i:02d}" for i in missing)
        )

    slot_count = con.execute("SELECT COUNT(*) FROM evt_runtime_slot").fetchone()[0]
    if slot_count != 22:
        raise RuntimeError(f"runtime slot model incomplete: expected 22, got {slot_count}")
    db_commands = con.execute("SELECT COUNT(*) FROM evt_command").fetchone()[0]
    if db_commands != parsed_commands:
        raise RuntimeError(f"command import mismatch: parsed {parsed_commands}, db {db_commands}")

    print(f"db={args.db}")
    print(f"runtime_slots=22 available_parsed={parsed_slots} missing={missing}")
    print(f"commands={parsed_commands} arguments={con.execute('SELECT COUNT(*) FROM evt_argument').fetchone()[0]}")
    con.close()
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
