# DMC3 HD em000 effect pack — G/V selector graph and A texture-grid grammar (2026-09-08)

**Branch:** `reverse/effect-pack-em000-20260908`  
**Corpus:** `em000-extract.zip`  
**Corpus SHA-256:** `306130125f09824811289366324f4208c3c1aba880c5a7efa3953a88d566d07b`

## Scope

This pass continues the em000 effect-pack reverse without assigning invented historical names to the `G/V/E/P/T/A/M` kind letters.

Two major contracts are recovered:

1. `G` and `V` are not opaque fixed-size records: both contain the same packed **directed manifest-selector edge** grammar;
2. `A` is directly texture-linked and contains a fixed-capacity sequence of 10-byte grid rectangles that is strongly consistent with atlas/UV-region data.

The latter atlas interpretation remains a semantic candidate until the executable consumer is recovered; the byte grammar and cross-references are corpus-confirmed.

---

# 1. Shared G/V selector pair

The em000 manifest gives disjoint logical kind domains. A selector can therefore be validated by both its kind code and its identifier.

## 1.1 V selector

All 50 `V` records are 0x170 bytes. At the beginning of every V record:

```text
V +0x04 u16 packed_selector
V +0x06 u16 target_manifest_id
```

Split the packed selector as:

```text
kind_code = packed_selector & 0x00FF
modifier  = packed_selector >> 8
```

Observed low-byte kind code mapping:

```text
0 -> P
1 -> E
2 -> G
3 -> V
```

For **50 / 50** V records, `target_manifest_id` exists in exactly the manifest kind domain selected by this low byte.

This resolves otherwise ambiguous numeric identifiers. Examples:

```text
V 26   selector low=2, id=16   -> G 16   # not A16/P16
V 84   selector low=0, id=18   -> P 18   # not M18
V 8    selector low=1, id=10   -> E 10   # not M10
V 165  selector low=1, id=273  -> E 273  # not V273
V 317  selector low=1, id=499  -> E 499  # not V499
```

The high byte varies and is preserved as a raw modifier. Observed values include `0`, `1`, `17`, `82`, `206`, `238`, and `255`. No high-level meaning is assigned.

## 1.2 G selector

All 12 `G` records are 0x60 bytes. The same pair occurs at:

```text
G +0x24 u16 packed_selector
G +0x26 u16 target_manifest_id
```

The same low-byte code map holds **12 / 12**:

```text
0 -> P
1 -> E
2 -> G
3 -> V
```

Observed em000 G edges:

```text
G13  -> P92
G75  -> V108
G152 -> P44
G153 -> V210
G187 -> P92
G16  -> P89
G608 -> P72
G313 -> E68
G104 -> E796
G405 -> E1114
G406 -> E1115
G443 -> P312
```

The selector grammar is therefore shared structurally between G and V, but this does **not** prove that G and V have identical runtime roles.

---

# 2. em000 effect graph is a directed acyclic routing graph

Across the complete bound G/V set there are exactly 62 selector edges:

```text
V -> P : 27
V -> E : 11
V -> G :  8
V -> V :  4
G -> P :  6
G -> E :  4
G -> V :  2
--------------
          62
```

No selector edge fails the manifest-domain check.

Within the G/V subgraph:

- no cycle exists in the bound em000 pack;
- every G/V node has exactly one outgoing selector edge;
- every chain terminates in a `P` or `E` node;
- the longest observed path is two edges / three logical nodes.

Examples:

```text
G75  -> V108 -> P141
G153 -> V210 -> P192
V26  -> G16  -> P89
V33  -> G608 -> P72
V70  -> V69  -> E94
V439 -> G313 -> E68
V440 -> G104 -> E796
```

Terminal result across all G/V start nodes:

```text
41 chains -> P
21 chains -> E
```

Safe semantic statement:

> G/V form an evidenced directed routing/control graph over effect-pack manifest entries, with P/E as terminal record families in the current em000 graph.

Do **not** expand G or V into artistic names without executable evidence.

---

# 3. A record is directly T-linked

All 11 A records are exactly 0x150 bytes.

Observed four-byte header:

```text
A +0x00 u8 constant_01       # 1 in 11/11
A +0x01 u8 t_manifest_id     # valid T id in 11/11
A +0x02 u8 raw_selector_a
A +0x03 u8 raw_selector_b
```

Observed `A +0x01` values:

```text
25, 26, 32, 9, 33
```

Every value exists in the bound `T` manifest domain.

This is independent of E's optional A selector. In nine of ten E->A references, the E-selected T and the A-owned T are the same. One valid counterexample exists:

```text
E203 -> T32 + A20
A20  -> T25
```

Therefore A owns its own T selector rather than merely inheriting the parent E texture selector.

Safe promotion:

> `A +0x01` is a T-kind manifest identifier selector for the bound em000 layout.

Status: `CORPUS_CONFIRMED`.

---

# 4. A fixed-capacity 10-byte grid-entry grammar

After the 4-byte A header, the 0x150-byte record fits exactly:

```text
0x04-byte header
33 * 0x0A-byte entries
0x02-byte zero tail
----------------------
0x150 bytes
```

Treat each 0x0A entry structurally as five u16 values:

```text
+0x00 u16 raw_flags
+0x02 u16 grid_x
+0x04 u16 grid_y
+0x06 u16 grid_w
+0x08 u16 grid_h
```

The names `grid_x/y/w/h` describe only the arithmetic pattern, not yet the original engine field names.

Across all non-zero entries in all 11 A records:

- values in the four grid fields are multiples of 32;
- observed values are only `0, 32, 64, 96, 128, 192`;
- `grid_x + grid_w <= 256` for every entry;
- `grid_y + grid_h <= 256` for every entry;
- non-zero entries form a contiguous prefix, followed by all-zero entries;
- observed active-entry counts are `2`, `4`, `10`, or `16`;
- final 2 bytes of every A record are zero in the bound corpus.

Representative entries:

```text
A29 -> T25
    (1,   0,  0, 64, 64)
    (0,  64,  0, 64, 64)
    (0,   0, 64, 64, 64)
    (0,  64, 64, 64, 64)

A61 -> T32
    (1,  64,  0, 32, 32)
    (0,  96,  0, 32, 32)
    (0,  64, 32, 32, 32)
    (0,  96, 32, 32, 32)
```

The 0..256 bounded grid, texture linkage, and rectangle-like arithmetic make a normalized UV/atlas-region interpretation very strong.

Evidence status:

- 4 + 33*10 + 2 structural envelope: `CORPUS_CONFIRMED`;
- T selector at A+0x01: `CORPUS_CONFIRMED`;
- bounded grid arithmetic: `CORPUS_CONFIRMED`;
- "texture atlas / UV rectangles" high-level interpretation: `SEMANTIC_CANDIDATE` pending executable consumer.

---

# 5. P primary variants now split cleanly

The previously recovered P envelope contains a fixed 0x130-byte primary block. Its first two bytes divide the 34 em000 records into three observed layout tuples:

```text
primary[0:2] = 02 03 : 30 records
primary[0:2] = 01 01 :  3 records
primary[0:2] = 00 04 :  1 record
```

For the `02 03` variant only, all 30 records contain two additional selectors at fixed positions:

```text
primary +0xE0 low16  -> T manifest id   # 30/30
primary +0x106 u16    -> A manifest id   # 30/30
```

The other four P records do not use those fields as T/A selectors. At `primary +0xE0` they contain negative floating-point values instead, including approximately `-0.5`, `-0.7`, `-0.75`, and `-1.0`.

Therefore a universal `P + fixed texture selector` interpretation is rejected. The texture/A binding belongs specifically to the observed `02 03` primary variant.

Safe promotion:

> P primary bytes `[0],[1]` select at least three different layouts in em000; variant `02 03` carries T/A manifest references at `+0xE0/+0x106`.

Status: `CORPUS_CONFIRMED`.

---

# 6. Effect graph after this pass

The currently evidenced graph is now:

```text
G/V router node
    -> G/V router node (optional, bounded depth in em000)
    -> P or E terminal family

E subtype 1/2
    -> T
    -> optional A
          -> own T

E subtype 5
    -> M
         -> MOD
         -> optional raw companion

P variant 02 03
    -> T
    -> A
         -> T
```

This is the first point where the em000 effect pack is better described as a typed resource graph than as a list of opaque records.

---

# 7. Next reverse gates

1. recover executable consumers for the shared G/V packed selector pair;
2. determine the high-byte modifier meaning;
3. bind the A 10-byte grid entries to runtime texture-coordinate/atlas code;
4. identify the meaning of A header bytes `+0x02/+0x03` and `raw_flags` in each grid entry;
5. recover the P primary-layout switch for `02 03`, `01 01`, `00 04`;
6. bind P `02 03` T/A selectors to the same runtime texture path as E/A;
7. repeat this graph census on a second enemy/effect pack before globalizing the variant tables.

## Hard non-claims

This pass does not claim:

- G = generator;
- V = variation/value;
- P = particle;
- A = animation/atlas as an original Capcom name;
- any artistic meaning for selector high-byte modifiers;
- that the em000 acyclic graph is a universal property of all DMC3 effect packs.
