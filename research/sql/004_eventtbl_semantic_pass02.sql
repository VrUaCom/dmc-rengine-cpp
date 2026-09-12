PRAGMA foreign_keys = ON;

BEGIN;

INSERT OR REPLACE INTO evidence_claim(
    subject_type, subject_key, claim, evidence_status,
    source_kind, source_locator, notes
) VALUES
(
    'eventtbl_opcode', '0x4D',
    'Observed 492 times in 16 byte-available runtime slots; arity is 1. Argument 0 covers every integer 0..20, with 21 distinct values.',
    'CORPUS_CONFIRMED', 'eventtbl_runtime_family',
    'docs/research/dmc3-eventtbl-semantic-pass02-2026-09-12.md#opcode-0x4d',
    'Exact semantic unresolved.'
),
(
    'eventtbl_opcode', '0x4D',
    'Opcode 0x4F immediately follows 0x4D in 308/492 observations. Alternating 0x4D(arg)->0x4F sequences recur throughout 16 files.',
    'CORPUS_CONFIRMED', 'eventtbl_runtime_family',
    'docs/research/dmc3-eventtbl-semantic-pass02-2026-09-12.md#opcode-0x4d--0x4f-pair',
    'Pair relationship is structural/correlation evidence only.'
),
(
    'eventtbl_opcode', '0x4F',
    'Observed 410 times in 16 byte-available runtime slots; arity is 0. It is immediately preceded by 0x4D in 308 cases and followed by 0x4D in 156 cases.',
    'CORPUS_CONFIRMED', 'eventtbl_runtime_family',
    'docs/research/dmc3-eventtbl-semantic-pass02-2026-09-12.md#opcode-0x4d--0x4f-pair',
    'Do not name it commit/trigger/end without handler evidence.'
),
(
    'eventtbl_opcode', '0x4D_0x4F_pair',
    'The corpus supports a strong selector-or-index plus zero-arity companion pattern for 0x4D/0x4F. Exact ownership and runtime operation remain unknown.',
    'SEMANTIC_CANDIDATE', 'eventtbl_runtime_family',
    'docs/research/dmc3-eventtbl-semantic-pass02-2026-09-12.md#opcode-0x4d--0x4f-pair',
    'Neutral candidate only; not exposed as gameplay semantics.'
),
(
    'eventtbl_opcode', '0x36',
    'Observed 406 times in 15 byte-available runtime slots; arity is 2. Arg1 is a three-value domain 0/1/2 (335/62/9). Arg0 has 61 observed values including clustered decimal-like values 100..503, sparse special values, and one 0xFFFFFFFF sentinel.',
    'CORPUS_CONFIRMED', 'eventtbl_runtime_family',
    'docs/research/dmc3-eventtbl-semantic-pass02-2026-09-12.md#opcode-0x36',
    'Exact semantic unresolved.'
),
(
    'eventtbl_opcode', '0x36',
    '0x36 repeats immediately after itself 123 times and is followed by 0x47 72 times. In EventTbl21 repeated sequences map 0x36(100,0)->0x47(0), 0x36(101,0)->0x47(1), 0x36(102,0)->0x47(2).',
    'CORPUS_CONFIRMED', 'eventtbl_runtime_family',
    'docs/research/dmc3-eventtbl-semantic-pass02-2026-09-12.md#opcode-0x36',
    'Identifier/ordinal relationship candidate; no gameplay name assigned.'
),
(
    'eventtbl_opcode', '0x36',
    'The first argument behaves like an identifier domain more than an unconstrained scalar, while the second behaves like a small mode/variant enum.',
    'SEMANTIC_CANDIDATE', 'eventtbl_runtime_family',
    'docs/research/dmc3-eventtbl-semantic-pass02-2026-09-12.md#opcode-0x36',
    'Do not relabel arg0 as room/stage/entity ID without EXE evidence.'
),
(
    'eventtbl_opcode', '0x49',
    'Observed 280 times in 16 byte-available runtime slots; arity is 2. Arg0 domain is 0/1/2 (218/38/24), arg1 domain is boolean 0/1 (103/177).',
    'CORPUS_CONFIRMED', 'eventtbl_runtime_family',
    'docs/research/dmc3-eventtbl-semantic-pass02-2026-09-12.md#opcode-0x49',
    'Exact semantic unresolved.'
),
(
    'eventtbl_opcode', '0x49',
    'Immediate predecessor is 0x31 in 84 observations and 0x2C in 68; immediate successor is 0x4D in 102 and 0x72 in 51. This places 0x49 inside the same dense control cluster as pass-01 opcodes and the 0x4D/0x4F pair.',
    'SEMANTIC_CANDIDATE', 'eventtbl_runtime_family',
    'docs/research/dmc3-eventtbl-semantic-pass02-2026-09-12.md#opcode-0x49',
    'Small state-selector/control role candidate only.'
);

COMMIT;
