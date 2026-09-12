# DMC3 EventTbl semantic reverse — pass 03

Date: 2026-09-12

## Scope

The research unit remains the complete 22-slot runtime family `EventTbl00.bin` through `EventTbl21.bin`.

- Byte-backed evidence: `00..09`, `13..21`.
- Missing byte payloads: `10..12`; these remain explicit runtime slots and contribute no fabricated commands.
- This pass follows the control cluster recovered in passes 01–02 and investigates `0x4C`, `0x58`, `0x47`, `0x72`, `0x37`, and `0x65`.

No opcode is assigned a product-facing label such as Cutscene, Camera, Room, Gameplay Start, or Door without executable-handler evidence.

## Opcode `0x4C`

- Arity: **0**.
- Count: **276**.
- Coverage: **16** byte-available files.

Immediate predecessors:

```text
0x4F 100
0x6F  71
0x55  26
0x56  24
0x58  21
```

Immediate successors:

```text
0x3D 71
0x65 33
0x46 27
0x2C 21
0x0E 19
0x49 19
0x31 18
```

Strong motifs include:

```text
4D 4F 4C 46 7B
4D 4F 4C 3D 0E
4D 4F 4C 65 3D
6E 6F 4C 0E 08
```

The placement strongly connects `0x4C` to the `0x4D/0x4F` pair and to boolean/control-bearing commands. A generic boundary/transition companion role is a candidate only; `stop`, `end`, `return`, and similar names remain unsupported.

## Opcode `0x58`

- Arity: **1**.
- Count: **261**.
- Coverage: **18** byte-available files.
- Argument 0: 31 distinct values, range `1..400`.

Dominant values:

```text
60  = 104
30  = 53
10  = 12
120 = 11
5   = 8
200 = 8
1   = 7
130 = 6
```

Frequent predecessors: `0x72` (52), `0x4F` (50), `0x4D` (40), `0x31` (27), `0x46` (20).

Frequent successors: `0x72` (44), `0x37` (29), `0x49` (25), `0x4C` (21), `0x36` (18).

The repeated motif

```text
58 72 58 72 58
```

occurs frequently. Values such as 30/60/120 make a duration-like interpretation plausible, but the corpus alone does not establish frames, ticks, milliseconds, seconds, or a `wait` operation. The field remains a raw scalar in the canonical model.

## Opcode `0x47`

- Arity: **1**.
- Count: **131**.
- Coverage: **16** byte-available files.
- Argument 0: range `0..8`, dominated by `0` (64), `1` (25), `2` (18), `4` (9).

`0x36` immediately precedes `0x47` **72** times. A repeated clean pattern in slot 21 is:

```text
36(100,0) 47(0)
36(101,0) 47(1)
36(102,0) 47(2)
```

Runs such as

```text
70 36 47 36 47
47 36 47 36 47
```

also occur. This strongly supports an ordinal/index companion relationship with `0x36`. The namespace indexed by these values is still unknown.

## Opcode `0x72`

- Arity: **4**.
- Count: **148**.
- Coverage: **14** byte-available files.

Argument domains:

```text
arg0: {0,1}              0=101, 1=47
arg1: 9 values           22,111,157,158,159,160,161,162,163
arg2: 6 values           0,40,41,42,43,44
arg3: 0 in 148/148 observations
```

The fourth argument is therefore `RESERVED_OBSERVED_ZERO` for this byte corpus, not proven reserved by executable evidence.

`0x58` precedes `0x72` 44 times and follows it 52 times. `0x49` precedes it 51 times. Common motifs include:

```text
72 58 72 58 72
2C 49 72 4D 6E
31 49 72 4D 6E
```

This command clearly belongs to the same control subsystem as `0x49`, `0x58`, and `0x4D`, but no exact semantic field names are promoted.

## Opcode `0x37`

- Arity: **2**.
- Count: **198**.
- Coverage: **15** byte-available files.

Argument 0 has 43 values and strongly overlaps the identifier-like namespace observed for `0x36`:

```text
100 = 56
200 = 18
101 = 16
102 = 12
300 = 9
888 = 9
1000/1001/1002 = 6 each
500 = 5
```

Argument 1 is mostly a small ordinal/mode value: 0=130, 1=39, 2=15, 3=7, with sparse larger values.

Neighbourhoods connect it to `0x4D/0x4F`, `0x58`, `0x49`, `0x46`, `0x38`, and `0x72`. The overlapping first-argument domain is strong evidence that `0x36` and `0x37` act on a related identifier namespace. The corpus does not identify that namespace as room, stage, entity, camera, enemy, or another game object.

## Opcode `0x65`

- Arity: **2**.
- Count: **173**.
- Coverage: **15** byte-available files.

Argument domains:

```text
arg0: 24 values in 0..38
arg1: boolean, 1=129 / 0=44
```

Frequent predecessors: `0x4C` (33), `0x2C` (30), `0x65` (30), `0x66` (18), `0x31` (17).

Frequent successors: `0x3D` (33), `0x65` (30), `0x31` (25), `0x0E` (20), `0x36` (14).

Repeated motifs include:

```text
31 66 65 0E 0D
4F 4C 65 3D 0E
10 2C 65 65 31
```

The small-index + boolean shape and its control-cluster placement support a state/flag-like role as a semantic candidate. No concrete flag name is assigned.

## Emerging control graph

Across passes 01–03 the corpus increasingly supports one connected subsystem rather than unrelated commands. Common paths include variants of:

```text
2C -> 3D -> 31 -> 49 -> 4D -> 4F -> 4C -> 3D/65/46
                         \-> 72 <-> 58
36 <-> 47
36/37 share an identifier-like first-argument namespace
```

This structure is suitable for graph rendering in Native Reader even before full semantic closure, provided unknown nodes retain raw opcode, arguments, offsets, and evidence status.

## Executable-evidence boundary

Targeted searches of the connected reverse documentation did not produce handler-level mappings for the six commands in this pass. The canonical executable action-dispatch anchor remains `0x1401A6510`; exact semantics require recovering each case target and its data accesses/calls.

Accordingly this pass records only corpus-confirmed domains/relationships and narrow semantic candidates. Nothing here promotes a Cutscene/Camera/Gameplay/Room label.

## Reproducibility

```bash
python research/sql/analyze_eventtbl_semantics.py \
  --opcode 0x4C --opcode 0x58 --opcode 0x47 \
  --opcode 0x72 --opcode 0x37 --opcode 0x65 \
  --output research-private/eventtbl-semantic-pass03.json
```

Committed evidence rows:

```text
research/sql/005_eventtbl_semantic_pass03.sql
```

## Next step

The next highest-leverage work is handler recovery rather than more blind naming. Corpus work should continue on the directly connected `0x46`, `0x6E`, `0x6F`, `0x38`, `0x71`, and `0x66` cluster while executable evidence is sought for the action dispatcher.
