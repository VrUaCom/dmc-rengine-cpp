PRAGMA foreign_keys = ON;

BEGIN;

-- Pass 01 intentionally records corpus facts separately from semantic hypotheses.
-- None of 0x2C/0x31/0x48/0x3D is promoted to a gameplay semantic name here.

INSERT OR REPLACE INTO evidence_claim(
    subject_type, subject_key, claim, evidence_status,
    source_kind, source_locator, notes
) VALUES
(
    'eventtbl_opcode', '0x2C',
    'Observed 976 times across all 19 currently byte-available EventTbl runtime slots; arity is 1 in every observation. Argument 0 has 43 observed values in range 0..420. The most frequent value is 0 (481/976).',
    'CORPUS_CONFIRMED', 'eventtbl_runtime_family',
    'docs/research/dmc3-eventtbl-semantic-pass01-2026-09-12.md#opcode-0x2c',
    'Semantic name intentionally unresolved.'
),
(
    'eventtbl_opcode', '0x2C',
    'Immediate successor distribution is strongly structured: 0x3D follows 303 times, 0x48 181 times, 0x8A 81 times, 0x49 68 times and 0x66 55 times. A repeated 0x48,0x48,0x2C,0x48,0x48 motif occurs 160 times.',
    'CORPUS_CONFIRMED', 'eventtbl_runtime_family',
    'docs/research/dmc3-eventtbl-semantic-pass01-2026-09-12.md#opcode-0x2c',
    'This is structural/correlation evidence only.'
),
(
    'eventtbl_opcode', '0x31',
    'Observed 898 times across all 19 currently byte-available EventTbl runtime slots; arity is 0. Opcode 0x3D is its immediate predecessor 498 times.',
    'CORPUS_CONFIRMED', 'eventtbl_runtime_family',
    'docs/research/dmc3-eventtbl-semantic-pass01-2026-09-12.md#opcode-0x31',
    'Semantic name intentionally unresolved.'
),
(
    'eventtbl_opcode', '0x31',
    'The strongest repeated neighbourhoods place 0x31 after 0x3D and before a new condition/action sequence or structural close. This supports a control-boundary role as a semantic candidate but does not identify the exact runtime operation.',
    'SEMANTIC_CANDIDATE', 'eventtbl_runtime_family',
    'docs/research/dmc3-eventtbl-semantic-pass01-2026-09-12.md#opcode-0x31',
    'Do not expose as END/COMMIT/ELSE without EXE handler evidence.'
),
(
    'eventtbl_opcode', '0x48',
    'Observed 777 times in 10 byte-available EventTbl slots; 740/777 observations are in EventTbl21.bin. Arity is 4. Arg0 is 0..3, arg1 is 0..1, arg2 has 40 values in 11..449, arg3 is 0..3.',
    'CORPUS_CONFIRMED', 'eventtbl_runtime_family',
    'docs/research/dmc3-eventtbl-semantic-pass01-2026-09-12.md#opcode-0x48',
    'Semantic name intentionally unresolved.'
),
(
    'eventtbl_opcode', '0x48',
    'The concentration in runtime slot 21 and long repeated 0x48 sequences strongly correlate this opcode with a special-mode/table-like control domain. Exact field ownership and gameplay meaning remain unconfirmed.',
    'SEMANTIC_CANDIDATE', 'eventtbl_runtime_family',
    'docs/research/dmc3-eventtbl-semantic-pass01-2026-09-12.md#opcode-0x48',
    'Do not label as Bloody Palace floor/room/stage command until executable evidence binds the fields.'
),
(
    'eventtbl_opcode', '0x3D',
    'Observed 770 times in 18 byte-available EventTbl slots; arity is 1 and argument 0 is strictly boolean in the supplied corpus: value 1 occurs 427 times and value 0 occurs 343 times.',
    'CORPUS_CONFIRMED', 'eventtbl_runtime_family',
    'docs/research/dmc3-eventtbl-semantic-pass01-2026-09-12.md#opcode-0x3d',
    'Semantic name intentionally unresolved.'
),
(
    'eventtbl_opcode', '0x3D',
    'When arg0=0, the immediate successor is 0x31 in 333/343 cases. When arg0=1, successor is 0x0E in 259 cases and 0x31 in 165 cases. This is strong evidence for a boolean-bearing control operation, but not enough to name the boolean.',
    'SEMANTIC_CANDIDATE', 'eventtbl_runtime_family',
    'docs/research/dmc3-eventtbl-semantic-pass01-2026-09-12.md#opcode-0x3d',
    'Do not label TRUE/FALSE, success, branch or enable without EXE evidence.'
);

COMMIT;
