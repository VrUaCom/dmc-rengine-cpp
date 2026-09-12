# DMC3 EventTbl semantic reverse — pass 01

Date: 2026-09-12

## Scope

This pass treats `EventTbl00.bin` through `EventTbl21.bin` as one 22-slot runtime family.

- Runtime slots modelled: **22** (`00..21`).
- Byte-available slots: **19** (`00..09`, `13..21`).
- Byte-missing slots: **10, 11, 12**. They remain first-class runtime slots and contribute no invented corpus data.
- Parsed command instances from available bytes: **12,012**.
- Parsed raw u32 arguments: **16,927**.
- Distinct observed opcodes: **121**.

The pass investigates the four highest-frequency unresolved opcodes selected from the corrected full-family census: `0x2C`, `0x31`, `0x48`, and `0x3D`.

## Method

For every command instance the normalized SQLite corpus was queried for:

- argument count and argument-value domains;
- per-file distribution across the runtime family;
- immediate predecessor and successor opcode distributions;
- repeated five-command neighbourhood motifs;
- structural-scope placement where already recovered.

Corpus correlations are kept separate from gameplay semantics. No opcode is promoted to an authoritative gameplay name without executable-handler evidence.

## Opcode `0x2C`

Status: **PRESERVED_UNDECODED** semantic; corpus properties **CORPUS_CONFIRMED**.

- Arity: **1** in every observation.
- Count: **976** commands.
- Coverage: **19/19 byte-available files**.
- Argument 0: **43 distinct values**, range `0..420`.
- Most frequent argument values: `0` = 481, `1` = 171, `2` = 99, `3` = 26, `4` = 19, `5` = 14.

Strong immediate successors:

- `0x3D`: 303
- `0x48`: 181
- `0x8A`: 81
- `0x49`: 68
- `0x66`: 55

Strong immediate predecessors include `0x48` (168), `0x10` (121), `0x0F` (111), `0x17` (74), and `0x15` (68).

Repeated motifs include:

```text
48 48 2C 48 48   x160
31 0F 2C 3D 31   x58
0D 0F 2C 3D 31   x53
31 1C 2C 3D 31   x43
31 17 2C 3D 31   x42
```

Every observed `0x2C` instance falls within already recovered `0x07`/`0x0D` structural scopes in the current corpus walk. This is structural context, not a semantic name.

## Opcode `0x31`

Status: exact semantic **PRESERVED_UNDECODED**; a generic control-boundary role is a **SEMANTIC_CANDIDATE** only.

- Arity: **0**.
- Count: **898**.
- Coverage: **19/19 byte-available files**.
- Immediate predecessor `0x3D`: **498** instances.

Frequent successors include `0x0E` (95), `0x49` (84), `0x2A` (68), `0x0F` (58), `0x10` (53), and another `0x31` (53).

Repeated motifs include:

```text
2D 3D 31 0F 2C   x54
2C 3D 31 1C 2C   x43
2D 3D 31 17 2C   x41
2E 48 31 2A 2E   x40
2C 3D 31 58 49   x21
```

The placement supports a control-boundary/separator hypothesis, but names such as `END`, `ELSE`, `COMMIT`, or `RETURN` are explicitly rejected until a handler proves one.

## Opcode `0x48`

Status: exact semantic **PRESERVED_UNDECODED**; special-mode/table-like correlation **SEMANTIC_CANDIDATE**.

- Arity: **4**.
- Count: **777**.
- Coverage: **10** byte-available files.
- `EventTbl21.bin`: **740/777** instances.

Argument domains:

- arg0: `0..3` (`0`=366, `2`=203, `1`=198, `3`=10)
- arg1: `0..1` (`0`=774, `1`=3)
- arg2: 40 observed values, range `11..449`
- arg3: `0..3` (`0`=725, `2`=30, `3`=12, `1`=10)

Neighbourhood structure is highly repetitive:

```text
2C 48 48 48 2C   x160
48 48 48 2C 48   x160
48 2F 48 2F 48   x136
2A 2E 48 31 2A   x40
```

Examples outside slot 21 include:

```text
EventTbl03 @ 0x4C8: [0,0,11,1]
EventTbl05 @ 0x0D4: [1,0,0x91,1]
EventTbl05 @ 0x504: [2,0,0x91,0]
EventTbl06:          [1,0,0x7B,3], [2,0,0x7B,3], [3,0,0x7B,3]
```

The extreme slot-21 concentration and array-like repetition are strong correlations, but this pass does **not** label the command as a Bloody Palace floor, room, stage, wave, or reward command without executable-handler evidence.

## Opcode `0x3D`

Status: boolean-bearing control operation **SEMANTIC_CANDIDATE**; exact boolean meaning **PRESERVED_UNDECODED**.

- Arity: **1**.
- Count: **770**.
- Coverage: **18** byte-available files.
- Argument 0 is strictly boolean in the supplied corpus:
  - `1`: **427**
  - `0`: **343**

Immediate predecessor `0x2C` occurs 303 times and `0x2D` 173 times. Immediate successor `0x31` occurs 498 times and `0x0E` 259 times.

Conditioned on the raw boolean:

- arg0=`0` -> next `0x31` in **333/343** cases;
- arg0=`1` -> next `0x0E` in **259** cases and next `0x31` in **165** cases.

Representative motifs:

```text
0F 2C 3D 31 1C
10 2D 3D 31 0F
17 2C 3D 31 49
4F 4C 3D 0E 08
```

This is strong evidence that the argument controls a binary state/flow property. It is not evidence for the labels `true/false`, `success/failure`, `enable/disable`, or branch polarity.

## Executable-evidence check

The project already binds EventTbl runtime infrastructure to the canonical executable, including the main action dispatcher at `0x1401A6510` and condition dispatcher at `0x1401A8320`. Existing repository/Drive evidence does not currently contain a handler-level mapping for `0x2C`, `0x31`, `0x48`, or `0x3D` sufficient to promote their exact semantics in this pass.

Therefore all four retain evidence-aware neutral names in the canonical API until their dispatcher cases and handler data accesses are recovered from the canonical `dmc3.exe`.

## Reproducibility

The normalized database is built with:

```bash
python research/sql/import_eventtbl_runtime_family.py --eventtbl-dir research-private/eventtbl
```

The corpus analysis can be regenerated with:

```bash
python research/sql/analyze_eventtbl_semantics.py \
  --opcode 0x2C --opcode 0x31 --opcode 0x48 --opcode 0x3D \
  --output research-private/eventtbl-semantic-pass01.json
```

Evidence claims are committed in:

```text
research/sql/003_eventtbl_semantic_pass01.sql
```

## Next reverse cluster

The next high-value unresolved cluster is `0x4D`, `0x4F`, `0x36`, and `0x49`. These commands are heavily connected to the pass-01 neighbourhoods and will be analysed using the same full `00..21` runtime-slot model before any semantic promotion.
