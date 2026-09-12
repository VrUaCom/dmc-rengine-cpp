# DMC3 EventTbl semantic reverse — pass 02

Date: 2026-09-12

## Scope

The unit of work remains the complete runtime family `EventTbl00.bin` through `EventTbl21.bin`.

- 22 runtime slots are modelled.
- 19 slots currently have bytes and contribute corpus evidence.
- `EventTbl10.bin`, `EventTbl11.bin`, and `EventTbl12.bin` remain explicit `MISSING_BYTES` slots; no command or argument data is inferred for them.

Pass 02 analyses `0x4D`, `0x4F`, `0x36`, and `0x49`, selected because they are high-frequency unresolved commands and connect densely to pass-01 opcodes.

## Opcode `0x4D` / `0x4F` pair

### `0x4D`

- Arity: **1**.
- Count: **492**.
- Coverage: **16** byte-available files.
- Argument 0 has exactly **21 distinct values and covers every integer `0..20`**.

Argument frequency begins:

```text
0=112, 1=75, 2=53, 3=38, 4=35,
6=22, 10=22, 5=21, 7=20, 11=19 ...
```

Immediate predecessors:

```text
0x4F 156
0x49 102
0x37  49
0x72  46
0x57  24
```

Immediate successors:

```text
0x4F 308
0x6E  55
0x58  40
0x57  20
0x37  15
```

### `0x4F`

- Arity: **0**.
- Count: **410**.
- Coverage: **16** byte-available files.
- Preceded by `0x4D` in **308/410** observations.
- Followed by `0x4D` in **156** observations.

Repeated alternating patterns are common:

```text
4D 4F 4D 4F 4D
4F 4D 4F 4D 4F
```

Representative sequences include:

```text
49(0,1) 4D(1) 4F 4D(5) 4F 4D(6) 4F 4D(7)
49(0,1) 4D(0) 4F 4D(2) 4F 58(60)
3E(0,0) 4D(0) 4F 4D(1) 4F 4D(2) 4F
```

### Evidence decision

The corpus strongly supports a paired relationship in which `0x4D` carries a compact index/selector-like argument and `0x4F` is its zero-argument companion. Exact ownership is not known. This pass deliberately does **not** name these commands camera selection, actor selection, trigger, commit, start, stop, or end.

Status:

- physical/relational observations: `CORPUS_CONFIRMED`;
- selector-or-index + companion interpretation: `SEMANTIC_CANDIDATE`;
- exact semantics: `PRESERVED_UNDECODED`.

## Opcode `0x36`

- Arity: **2**.
- Count: **406**.
- Coverage: **15** byte-available files.

Argument 0:

- 61 distinct values;
- common values include `100` (84), `101` (37), `102` (27), `200` (24), `300` (18), `201` (17), `202` (15), `500` (13), `400` (12);
- other sparse values include `31`, `444`, `650`, `777`, `888`, `1200`, `1201`, `1301`;
- one `0xFFFFFFFF` sentinel is observed.

Argument 1 is a three-value domain:

```text
0 = 335
1 = 62
2 = 9
```

Neighbourhoods:

- `0x36 -> 0x36`: **123** occurrences;
- `0x36 -> 0x47`: **72** occurrences;
- `0x70 -> 0x36`: **55** occurrences.

A particularly clean repeated pattern in `EventTbl21.bin` is:

```text
36(100,0) 47(0)
36(101,0) 47(1)
36(102,0) 47(2)
```

Other files contain related multi-command runs, including sequences of several `0x36` definitions followed by `0x47` selectors.

The first argument therefore behaves like an identifier domain more than an unconstrained numeric scalar, while the second behaves like a small variant/mode domain. The current evidence does **not** establish whether arg0 names rooms, stages, entities, event records, camera records, enemies, or another identifier family.

Status:

- domains and neighbourhoods: `CORPUS_CONFIRMED`;
- identifier + small-mode interpretation: `SEMANTIC_CANDIDATE`;
- concrete field names: `PRESERVED_UNDECODED`.

## Opcode `0x49`

- Arity: **2**.
- Count: **280**.
- Coverage: **16** byte-available files.

Argument domains are unusually small:

```text
arg0: 0=218, 1=38, 2=24
arg1: 0=103, 1=177
```

Immediate predecessors:

```text
0x31 84
0x2C 68
0x58 25
0x4C 19
0x0D 16
```

Immediate successors:

```text
0x4D 102
0x72  51
0x37  26
0x4A  25
0x44  16
```

Representative motifs:

```text
3D 31 49 72 4D
0E 0D 49 4D 4F
31 58 49 37 4D
31 58 49 4D 37
```

The tiny enum/boolean-like argument domains and adjacency to `0x31`, `0x2C`, and the `0x4D/0x4F` pair support a small control/state-selector role as a semantic candidate. Exact meaning remains unresolved.

## Relationship to pass 01

Pass 02 makes the pass-01 cluster more structured without assigning unsupported gameplay labels:

```text
... 2C -> 3D -> 31 -> 49 -> 4D -> 4F ...
```

is a repeatedly observed family of neighbourhoods, with variations involving `0x72`, `0x37`, `0x44`, `0x45`, `0x58`, and `0x4C`.

This is sufficient to treat these commands as a connected control subsystem for further reverse work, but not sufficient to call it cutscene, camera, gameplay-start, room transition, or any other product-facing category.

## Executable-evidence boundary

Searches of the currently connected repository and reverse documentation did not locate handler-level evidence that maps `0x4D`, `0x4F`, `0x36`, or `0x49` to exact runtime operations. The known EventTbl action dispatcher anchor remains `0x1401A6510`; the next closure step is to recover the dispatcher case targets and field/call accesses for these opcodes from the canonical executable.

Until then, the canonical API should expose raw opcode/arguments plus evidence level rather than guessed labels.

## Reproducibility

Corpus statistics are regenerated from the normalized SQLite database with:

```bash
python research/sql/analyze_eventtbl_semantics.py \
  --opcode 0x4D --opcode 0x4F --opcode 0x36 --opcode 0x49 \
  --output research-private/eventtbl-semantic-pass02.json
```

Committed evidence rows are in:

```text
research/sql/004_eventtbl_semantic_pass02.sql
```

## Next closure targets

The strongest directly connected unresolved commands for the next pass are `0x4C`, `0x58`, `0x47`, `0x72`, `0x37`, and `0x65`. They appear at the boundaries of both pass-01 and pass-02 motifs and should provide more leverage than simply taking the next opcodes by global frequency.
