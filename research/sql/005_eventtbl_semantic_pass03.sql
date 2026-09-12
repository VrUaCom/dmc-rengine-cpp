PRAGMA foreign_keys = ON;

BEGIN;

INSERT OR REPLACE INTO evidence_claim(
    subject_type, subject_key, claim, evidence_status,
    source_kind, source_locator, notes
) VALUES
(
    'eventtbl_opcode','0x4C',
    'Observed 276 times in 16 byte-available runtime slots; arity is 0. Immediate predecessor is 0x4F in 100 cases and 0x6F in 71; immediate successor is 0x3D in 71, 0x65 in 33 and 0x46 in 27.',
    'CORPUS_CONFIRMED','eventtbl_runtime_family',
    'docs/research/dmc3-eventtbl-semantic-pass03-2026-09-12.md#opcode-0x4c',
    'Exact semantic unresolved.'
),
(
    'eventtbl_opcode','0x4C',
    '0x4C repeatedly appears after the 0x4D/0x4F pair and before boolean/control-bearing operations, supporting a boundary/transition companion role only as a semantic candidate.',
    'SEMANTIC_CANDIDATE','eventtbl_runtime_family',
    'docs/research/dmc3-eventtbl-semantic-pass03-2026-09-12.md#opcode-0x4c',
    'Do not expose as stop/end/return without EXE evidence.'
),
(
    'eventtbl_opcode','0x58',
    'Observed 261 times in 18 byte-available runtime slots; arity is 1. Arg0 has 31 values in range 1..400; dominant values are 60 (104), 30 (53), 10 (12), 120 (11), 5 (8), 200 (8), 1 (7).',
    'CORPUS_CONFIRMED','eventtbl_runtime_family',
    'docs/research/dmc3-eventtbl-semantic-pass03-2026-09-12.md#opcode-0x58',
    'Exact unit and semantic unresolved.'
),
(
    'eventtbl_opcode','0x58',
    'The scalar distribution and repeated 0x58/0x72 alternation are compatible with a duration/timing-like quantity, but frames/ticks/time/wait are not established.',
    'SEMANTIC_CANDIDATE','eventtbl_runtime_family',
    'docs/research/dmc3-eventtbl-semantic-pass03-2026-09-12.md#opcode-0x58',
    'Keep raw scalar in product UI until handler evidence resolves the unit.'
),
(
    'eventtbl_opcode','0x47',
    'Observed 131 times in 16 byte-available runtime slots; arity is 1. Arg0 spans 0..8 with 0/1/2 dominant. 0x36 is the immediate predecessor in 72 cases and 0x36 is the immediate successor in 25.',
    'CORPUS_CONFIRMED','eventtbl_runtime_family',
    'docs/research/dmc3-eventtbl-semantic-pass03-2026-09-12.md#opcode-0x47',
    'Exact semantic unresolved.'
),
(
    'eventtbl_opcode','0x47',
    'Repeated 0x36(identifier,mode)->0x47(smallOrdinal) sequences support an ordinal/index companion relationship between 0x36 and 0x47.',
    'SEMANTIC_CANDIDATE','eventtbl_runtime_family',
    'docs/research/dmc3-eventtbl-semantic-pass03-2026-09-12.md#opcode-0x47',
    'Identifier namespace remains unknown.'
),
(
    'eventtbl_opcode','0x72',
    'Observed 148 times in 14 byte-available runtime slots; arity is 4. Arg0 is 0/1; arg1 has nine values (22, 111, 157..163); arg2 has six values (0,40..44); arg3 is zero in all 148 observations.',
    'CORPUS_CONFIRMED','eventtbl_runtime_family',
    'docs/research/dmc3-eventtbl-semantic-pass03-2026-09-12.md#opcode-0x72',
    'Arg3 is RESERVED_OBSERVED_ZERO within this corpus, not proven reserved by EXE.'
),
(
    'eventtbl_opcode','0x72',
    '0x72 and 0x58 form a strongly repeated alternating neighbourhood: 0x58 precedes 0x72 44 times and follows it 52 times.',
    'SEMANTIC_CANDIDATE','eventtbl_runtime_family',
    'docs/research/dmc3-eventtbl-semantic-pass03-2026-09-12.md#opcode-0x72',
    'Exact relation and field ownership unresolved.'
),
(
    'eventtbl_opcode','0x37',
    'Observed 198 times in 15 byte-available runtime slots; arity is 2. Arg0 has 43 identifier-like values dominated by 100/101/102/200/300 and overlaps the numeric namespace seen in 0x36. Arg1 is usually 0/1/2 but has sparse larger values.',
    'CORPUS_CONFIRMED','eventtbl_runtime_family',
    'docs/research/dmc3-eventtbl-semantic-pass03-2026-09-12.md#opcode-0x37',
    'Exact semantic unresolved.'
),
(
    'eventtbl_opcode','0x37',
    'The overlapping first-argument domain strongly suggests 0x36 and 0x37 operate on a related identifier namespace, without proving what that namespace represents.',
    'SEMANTIC_CANDIDATE','eventtbl_runtime_family',
    'docs/research/dmc3-eventtbl-semantic-pass03-2026-09-12.md#opcode-0x37',
    'Do not name room/stage/entity/camera without handler evidence.'
),
(
    'eventtbl_opcode','0x65',
    'Observed 173 times in 15 byte-available runtime slots; arity is 2. Arg0 has 24 values in range 0..38; arg1 is boolean (1=129,0=44).',
    'CORPUS_CONFIRMED','eventtbl_runtime_family',
    'docs/research/dmc3-eventtbl-semantic-pass03-2026-09-12.md#opcode-0x65',
    'Exact semantic unresolved.'
),
(
    'eventtbl_opcode','0x65',
    'Small-index plus boolean arguments and frequent adjacency to 0x4C, 0x2C and 0x3D support a state/flag-like operation only as a semantic candidate.',
    'SEMANTIC_CANDIDATE','eventtbl_runtime_family',
    'docs/research/dmc3-eventtbl-semantic-pass03-2026-09-12.md#opcode-0x65',
    'No public flag name assigned.'
);

COMMIT;
