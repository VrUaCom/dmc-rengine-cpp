PRAGMA foreign_keys = ON;
PRAGMA journal_mode = WAL;

BEGIN;

CREATE TABLE IF NOT EXISTS research_meta (
    key TEXT PRIMARY KEY,
    value TEXT NOT NULL
);

CREATE TABLE IF NOT EXISTS format_family (
    id INTEGER PRIMARY KEY,
    family TEXT NOT NULL UNIQUE,
    magic_text TEXT,
    stock_extension TEXT,
    description TEXT
);

CREATE TABLE IF NOT EXISTS source_artifact (
    id INTEGER PRIMARY KEY,
    source_kind TEXT NOT NULL,
    name TEXT NOT NULL,
    sha256 TEXT,
    provenance TEXT NOT NULL,
    notes TEXT,
    UNIQUE(source_kind, name, sha256)
);

CREATE TABLE IF NOT EXISTS resource_file (
    id INTEGER PRIMARY KEY,
    format_id INTEGER NOT NULL REFERENCES format_family(id),
    source_artifact_id INTEGER REFERENCES source_artifact(id),
    stock_name TEXT NOT NULL,
    stock_extension TEXT,
    corpus_group TEXT NOT NULL,
    corpus_index INTEGER,
    size_bytes INTEGER,
    sha256 TEXT,
    magic_text TEXT,
    revision INTEGER,
    stream_count INTEGER,
    terminal_offset INTEGER,
    command_count INTEGER,
    evidence_status TEXT NOT NULL DEFAULT 'PRESERVED_UNDECODED',
    notes TEXT,
    UNIQUE(corpus_group, stock_name, sha256)
);

CREATE INDEX IF NOT EXISTS idx_resource_file_format ON resource_file(format_id);
CREATE INDEX IF NOT EXISTS idx_resource_file_corpus ON resource_file(corpus_group, corpus_index);
CREATE INDEX IF NOT EXISTS idx_resource_file_sha256 ON resource_file(sha256);

CREATE TABLE IF NOT EXISTS evt_stream (
    id INTEGER PRIMARY KEY,
    file_id INTEGER NOT NULL REFERENCES resource_file(id) ON DELETE CASCADE,
    stream_ordinal INTEGER NOT NULL,
    root_offset INTEGER NOT NULL,
    entry_opcode INTEGER,
    entry_argument_count INTEGER,
    evidence_status TEXT NOT NULL DEFAULT 'STRUCTURAL_CONFIRMED',
    UNIQUE(file_id, stream_ordinal)
);

CREATE TABLE IF NOT EXISTS evt_opcode (
    opcode INTEGER PRIMARY KEY CHECK(opcode BETWEEN 0 AND 255),
    observed_argument_count INTEGER,
    observed_command_count INTEGER NOT NULL DEFAULT 0,
    observed_file_count INTEGER NOT NULL DEFAULT 0,
    observed_in_supplied_corpus INTEGER NOT NULL DEFAULT 1 CHECK(observed_in_supplied_corpus IN (0,1)),
    semantic_name TEXT,
    semantic_class TEXT NOT NULL DEFAULT 'unknown',
    evidence_status TEXT NOT NULL DEFAULT 'PRESERVED_UNDECODED',
    notes TEXT
);

CREATE TABLE IF NOT EXISTS evt_opcode_census (
    opcode INTEGER NOT NULL REFERENCES evt_opcode(opcode),
    corpus_scope TEXT NOT NULL,
    command_count INTEGER NOT NULL,
    file_count INTEGER NOT NULL,
    PRIMARY KEY(opcode, corpus_scope)
);

CREATE TABLE IF NOT EXISTS evt_structural_pair (
    begin_opcode INTEGER NOT NULL REFERENCES evt_opcode(opcode),
    end_opcode INTEGER NOT NULL REFERENCES evt_opcode(opcode),
    begin_argument_count INTEGER NOT NULL,
    end_argument_count INTEGER NOT NULL,
    evidence_status TEXT NOT NULL,
    notes TEXT,
    PRIMARY KEY(begin_opcode, end_opcode)
);

-- The parser/exporter can populate these two tables with every decoded command.
-- They are intentionally normalized so raw arguments remain queryable even while
-- their semantics are still PRESERVED_UNDECODED.
CREATE TABLE IF NOT EXISTS evt_command (
    id INTEGER PRIMARY KEY,
    file_id INTEGER NOT NULL REFERENCES resource_file(id) ON DELETE CASCADE,
    stream_ordinal INTEGER,
    sequence_index INTEGER NOT NULL,
    file_offset INTEGER NOT NULL,
    raw_descriptor INTEGER NOT NULL,
    opcode INTEGER NOT NULL REFERENCES evt_opcode(opcode),
    argument_count INTEGER NOT NULL,
    evidence_status TEXT NOT NULL DEFAULT 'STRUCTURAL_CONFIRMED',
    UNIQUE(file_id, sequence_index),
    UNIQUE(file_id, file_offset)
);

CREATE INDEX IF NOT EXISTS idx_evt_command_opcode ON evt_command(opcode);
CREATE INDEX IF NOT EXISTS idx_evt_command_file_stream ON evt_command(file_id, stream_ordinal, sequence_index);

CREATE TABLE IF NOT EXISTS evt_argument (
    command_id INTEGER NOT NULL REFERENCES evt_command(id) ON DELETE CASCADE,
    argument_index INTEGER NOT NULL,
    value_u32 INTEGER NOT NULL,
    semantic_name TEXT,
    evidence_status TEXT NOT NULL DEFAULT 'PRESERVED_UNDECODED',
    PRIMARY KEY(command_id, argument_index)
);

CREATE TABLE IF NOT EXISTS evidence_claim (
    id INTEGER PRIMARY KEY,
    subject_type TEXT NOT NULL,
    subject_key TEXT NOT NULL,
    claim TEXT NOT NULL,
    evidence_status TEXT NOT NULL,
    source_kind TEXT,
    source_locator TEXT,
    created_utc TEXT NOT NULL DEFAULT CURRENT_TIMESTAMP,
    notes TEXT,
    UNIQUE(subject_type, subject_key, claim, source_locator)
);

CREATE VIEW IF NOT EXISTS v_evt_opcode_status AS
SELECT
    printf('0x%02X', opcode) AS opcode_hex,
    observed_argument_count AS argc,
    observed_command_count AS commands,
    observed_file_count AS files,
    COALESCE(semantic_name, 'unknown') AS name,
    semantic_class,
    evidence_status,
    observed_in_supplied_corpus
FROM evt_opcode
ORDER BY opcode;

CREATE VIEW IF NOT EXISTS v_evt_file_summary AS
SELECT
    f.stock_name,
    f.corpus_group,
    f.corpus_index,
    f.size_bytes,
    f.sha256,
    f.revision,
    f.stream_count,
    printf('0x%X', f.terminal_offset) AS terminal_offset_hex,
    f.command_count,
    f.evidence_status
FROM resource_file AS f
JOIN format_family AS ff ON ff.id = f.format_id
WHERE ff.family = 'EventTbl'
ORDER BY f.corpus_group, f.corpus_index;

INSERT OR IGNORE INTO research_meta(key, value) VALUES
    ('schema', 'dmc-rengine.research.sqlite.v1'),
    ('created', '2026-09-12'),
    ('authority_rule', 'preserve evidence status; never promote semantics without corpus or EXE evidence');

COMMIT;
