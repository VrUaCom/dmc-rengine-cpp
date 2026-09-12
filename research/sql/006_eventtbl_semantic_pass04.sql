PRAGMA foreign_keys = ON;

BEGIN;

INSERT OR REPLACE INTO evidence_claim(
    subject_type, subject_key, claim, evidence_status,
    source_kind, source_locator, notes
) VALUES
(
    'eventtbl_opcode','0x46',
    'Observed 75 times in 12 byte-available runtime slots; arity is 1. Arg0 domain is {0,1,2,4}, dominated by 0 (47). It is preceded by 0x37 in 32 cases and 0x4C in 27.',
    'CORPUS_CONFIRMED','eventtbl_runtime_family',
    'docs/research/dmc3-eventtbl-semantic-pass04-2026-09-12.md#opcode-0x46',
    'Exact semantic unresolved.'
),
(
    'eventtbl_opcode','0x6E',
    'Observed 97 times in 12 byte-available runtime slots; arity is 1. Arg0 has 55 code-like values in range 10003..13057. 0x6F immediately follows in 54 cases.',
    'CORPUS_CONFIRMED','eventtbl_runtime_family',
    'docs/research/dmc3-eventtbl-semantic-pass04-2026-09-12.md#opcode-0x6e--0x6f',
    'Exact code namespace unresolved.'
),
(
    'eventtbl_opcode','0x6F',
    'Observed 81 times in 12 byte-available runtime slots; arity is 0. It is preceded by 0x6E in 54 cases and followed by 0x4C in 71 cases.',
    'CORPUS_CONFIRMED','eventtbl_runtime_family',
    'docs/research/dmc3-eventtbl-semantic-pass04-2026-09-12.md#opcode-0x6e--0x6f',
    'Exact semantic unresolved.'
),
(
    'eventtbl_opcode','0x6E_0x6F_pair',
    'The repeated 0x6E(code)->0x6F->0x4C chain supports a code-bearing command plus zero-arity companion relationship.',
    'SEMANTIC_CANDIDATE','eventtbl_runtime_family',
    'docs/research/dmc3-eventtbl-semantic-pass04-2026-09-12.md#opcode-0x6e--0x6f',
    'Do not name sound/effect/cutscene resource without EXE evidence.'
),
(
    'eventtbl_opcode','0x38',
    'Observed 193 times in 14 byte-available runtime slots; arity is 3. Arg0 has 34 values dominated by 100/101/200/600/103/110 and overlaps the identifier-like domains of 0x36/0x37; arg1 is 0..5 dominated by 1/2; arg2 is boolean.',
    'CORPUS_CONFIRMED','eventtbl_runtime_family',
    'docs/research/dmc3-eventtbl-semantic-pass04-2026-09-12.md#opcode-0x38',
    'Exact semantic unresolved.'
),
(
    'eventtbl_opcode','0x38',
    'Shared identifier-like first-argument values support an operation-family relationship among 0x36, 0x37 and 0x38.',
    'SEMANTIC_CANDIDATE','eventtbl_runtime_family',
    'docs/research/dmc3-eventtbl-semantic-pass04-2026-09-12.md#opcode-0x38',
    'Underlying identifier namespace remains unknown.'
),
(
    'eventtbl_opcode','0x71',
    'Observed 151 times in 11 byte-available runtime slots; arity is 6. 0x57 immediately precedes it 70 times and follows it 67 times. Repeated sequences include 0x57(0)->0x71(...)->0x57(15)->0x71(...)->0x57(30) and 0,50,100,150,200 marker progressions.',
    'CORPUS_CONFIRMED','eventtbl_runtime_family',
    'docs/research/dmc3-eventtbl-semantic-pass04-2026-09-12.md#opcode-0x71-and-timeline-marker-0x57',
    'This materially strengthens the existing 0x57 timeline-position candidate.'
),
(
    'eventtbl_opcode','0x71',
    'Args3..5 form large numeric triplets; interpreted as signed u32 in the supplied corpus their ranges are 115..6036, 0..5610, and -1870..4900. Together with alternating 0x57 markers this supports a timeline-key payload and spatial/vector-like subpayload candidate.',
    'SEMANTIC_CANDIDATE','eventtbl_runtime_family',
    'docs/research/dmc3-eventtbl-semantic-pass04-2026-09-12.md#opcode-0x71-and-timeline-marker-0x57',
    'Do not label camera position/target without handler evidence.'
),
(
    'eventtbl_opcode','0x66',
    'Observed 117 times in 11 byte-available runtime slots; arity is 2. Arg0 has five values {0,1,4,5,6}; arg1 has 18 values in 0..42. It is preceded by 0x2C in 55 cases and followed by 0x31 in 34, 0x65 in 18 and 0x2C in 14.',
    'CORPUS_CONFIRMED','eventtbl_runtime_family',
    'docs/research/dmc3-eventtbl-semantic-pass04-2026-09-12.md#opcode-0x66',
    'Exact semantic unresolved.'
),
(
    'eventtbl_opcode','0x66',
    'Its small category/index domains and repeated placement inside 0x2C/0x31/0x65 control sequences support another compact state/control operation candidate.',
    'SEMANTIC_CANDIDATE','eventtbl_runtime_family',
    'docs/research/dmc3-eventtbl-semantic-pass04-2026-09-12.md#opcode-0x66',
    'No public semantic name assigned.'
);

COMMIT;
