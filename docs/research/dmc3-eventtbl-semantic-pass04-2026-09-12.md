# DMC3 EventTbl semantic reverse — pass 04

Date: 2026-09-12

## Scope

The unit remains the complete runtime family `EventTbl00.bin` through `EventTbl21.bin` (22 slots). Byte-backed corpus evidence currently exists for 19 slots; `10..12` remain explicit `MISSING_BYTES` slots and contribute no invented command data.

Pass 04 follows the control chains recovered in passes 01–03 and investigates `0x46`, `0x6E`, `0x6F`, `0x38`, `0x71`, and `0x66`.

## Opcode `0x46`

- Arity: **1**.
- Count: **75**.
- Coverage: **12** byte-available files.
- Arg0 domain: `{0,1,2,4}`; `0` occurs 47 times.

Strong predecessors are `0x37` (32) and `0x4C` (27). Strong successors are `0x7B` (22), `0x58` (20), `0x4F` (10), and `0x3D` (9).

Representative motifs:

```text
4F 4C 46 7B 3D
4D 37 46 58 37
58 37 46 58 37
58 37 46 4F 4C
```

The command is clearly part of the same compact control subsystem but remains semantically unresolved.

## Opcode `0x6E` / `0x6F`

### `0x6E`

- Arity: **1**.
- Count: **97**.
- Coverage: **12** byte-available files.
- Arg0 has **55 distinct code-like values**, range `10003..13057`.

Common examples include `13040`, `13041`, `13042`, `10701`, `10150`, `10151`, `10158`, `10250`, `10256`, and similar clustered values.

`0x6F` immediately follows `0x6E` in **54** cases.

### `0x6F`

- Arity: **0**.
- Count: **81**.
- Coverage: **12** byte-available files.
- Preceded by `0x6E` in **54** cases.
- Followed by `0x4C` in **71** cases.

A dominant chain is therefore:

```text
4D/72/49 -> 6E(code) -> 6F -> 4C -> control continuation
```

This supports a code-bearing command plus zero-arity companion relationship. The numeric namespace is not identified as sound, effect, cutscene, resource, stage, or any other domain without handler evidence.

## Opcode `0x38`

- Arity: **3**.
- Count: **193**.
- Coverage: **14** byte-available files.

Argument domains:

- arg0: 34 identifier-like values, dominated by `100`, `101`, `200`, `600`, `103`, `110`, `650`, `111`, `112`, `102`, `300`;
- arg1: `0..5`, dominated by `1` (111) and `2` (66);
- arg2: boolean (`0`=119, `1`=74).

`0x38` repeats after itself 61 times and is followed by `0x71` 41 times. Its first-argument domain overlaps strongly with `0x36` and `0x37`, supporting one related identifier-operation family:

```text
0x36(identifier, small_mode)
0x37(identifier, mode/value)
0x38(identifier, small_mode, bool)
```

The identifier namespace itself remains unresolved.

## Opcode `0x71` and timeline marker `0x57`

This is the strongest semantic-correlation result in pass 04.

`0x71`:

- Arity: **6**.
- Count: **151**.
- Coverage: **11** byte-available files.
- Immediate predecessor `0x57`: **70** times.
- Immediate successor `0x57`: **67** times.

Repeated data contains explicit alternating marker/payload sequences. Examples:

```text
EventTbl06 / EventTbl07:
57(0)
71(0,28,37,3330,10,2500)
57(15)
71(0,28,37,3330,10,2500)
57(30)
71(0,28,37,3330,10,2500)
57(45)
...
```

and:

```text
EventTbl03:
57(0)
71(0,30,0,3380,448,2500)
57(50)
71(...)
57(100)
71(...)
57(150)
71(...)
57(200)
```

A second example shows the payload itself changing across marker positions:

```text
57(540)
71(0,163,40,2650,650,4400)
57(570)
71(0,164,40,2650,650,4600)
```

This materially strengthens the existing interpretation of `0x57` as a **timeline/position marker candidate**.

For `0x71`, args 3..5 form large numeric triplets. Interpreting the raw u32 values as signed integers across the corpus gives:

```text
arg3:  115 .. 6036
arg4:    0 .. 5610
arg5: -1870 .. 4900
```

The signed negative value plus the scale and triplet shape support a **spatial/vector-like payload candidate**. Combined with the alternating `0x57` markers, `0x71` is promoted only to the narrow semantic candidate **timeline-key payload with a possible spatial/vector subpayload**.

This is *not* sufficient to label it Camera Position, Camera Target, Actor Position, or Cutscene Keyframe. Those labels require executable-handler evidence.

## Opcode `0x66`

- Arity: **2**.
- Count: **117**.
- Coverage: **11** byte-available files.

Argument domains:

```text
arg0: {0,1,4,5,6}
arg1: 18 values in 0..42
```

Strong predecessors: `0x2C` (55), `0x31` (20), `0x4C` (11).

Strong successors: `0x31` (34), `0x65` (18), `0x2C` (14), `0x3D` (12), `0x15` (10).

Representative motifs:

```text
48 31 66 65 0E
15 2C 66 31 15
66 2C 66 2C 66
31 3E 66 3D 0E
```

This supports another compact state/control operation in the pass-01 cluster. Exact state ownership is unresolved.

## Updated structural picture

The first four semantic passes now expose a reproducible connected graph:

```text
2C -> 3D -> 31 -> 49 -> 4D -> 4F -> 4C -> ...
                         |             |
                         |             +-> 46 / 65 / 3D / 2C
                         +-> 72 <-> 58
                         +-> 6E -> 6F -> 4C

36 <-> 47
36 / 37 / 38 share an identifier-like numeric namespace

57(marker) <-> 71(six-field timeline payload candidate)
```

This is enough to improve an EventFlow graph structurally without pretending unknown commands are understood. Product-facing nodes should still display raw opcode + arguments + evidence for all candidate-only semantics.

## EXE evidence boundary

Targeted searches of the connected reverse documentation did not expose dispatcher-case handlers for these exact opcodes. The canonical action dispatcher anchor remains `0x1401A6510`. Handler-level closure requires the canonical executable or a preserved disassembly that resolves case targets and downstream calls/field writes.

No candidate in this report is promoted to `EXE_CONFIRMED`.

## Reproducibility

```bash
python research/sql/analyze_eventtbl_semantics.py \
  --opcode 0x46 --opcode 0x6E --opcode 0x6F \
  --opcode 0x38 --opcode 0x71 --opcode 0x66 \
  --output research-private/eventtbl-semantic-pass04.json
```

Evidence migration:

```text
research/sql/006_eventtbl_semantic_pass04.sql
```

## Next reverse frontier

The corpus is now sufficiently structured that the highest-value next step is dispatcher handler recovery. In parallel, `0x70`, `0x79`, `0x7B`, `0x55`, `0x56`, and `0x3E` are the next connected commands worth profiling because they surround the timeline/vector and identifier-operation families.
