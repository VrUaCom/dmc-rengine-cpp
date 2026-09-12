PRAGMA foreign_keys = ON;

BEGIN;

CREATE TABLE IF NOT EXISTS evt_runtime_slot (
    runtime_index INTEGER PRIMARY KEY CHECK(runtime_index BETWEEN 0 AND 21),
    stock_name TEXT NOT NULL UNIQUE,
    runtime_path TEXT NOT NULL UNIQUE,
    resource_file_id INTEGER REFERENCES resource_file(id) ON DELETE SET NULL,
    bytes_status TEXT NOT NULL CHECK(bytes_status IN ('AVAILABLE_PARSED','MISSING_BYTES','INVALID_BYTES')),
    semantic_role TEXT,
    role_evidence_status TEXT NOT NULL DEFAULT 'PRESERVED_UNDECODED',
    notes TEXT
);

CREATE INDEX IF NOT EXISTS idx_evt_runtime_slot_file
    ON evt_runtime_slot(resource_file_id);

CREATE VIEW IF NOT EXISTS v_evt_runtime_coverage AS
SELECT
    s.runtime_index,
    s.stock_name,
    s.runtime_path,
    s.bytes_status,
    s.semantic_role,
    s.role_evidence_status,
    f.size_bytes,
    f.sha256,
    f.revision,
    f.stream_count,
    f.command_count,
    CASE WHEN f.terminal_offset IS NULL THEN NULL ELSE printf('0x%X', f.terminal_offset) END AS terminal_offset_hex
FROM evt_runtime_slot AS s
LEFT JOIN resource_file AS f ON f.id = s.resource_file_id
ORDER BY s.runtime_index;

CREATE VIEW IF NOT EXISTS v_evt_runtime_missing AS
SELECT runtime_index, stock_name, runtime_path, bytes_status
FROM evt_runtime_slot
WHERE bytes_status <> 'AVAILABLE_PARSED'
ORDER BY runtime_index;

COMMIT;
