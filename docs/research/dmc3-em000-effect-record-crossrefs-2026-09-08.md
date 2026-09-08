# DMC3 HD em000 effect pack — E cross-references and P substructure (2026-09-08)

**Branch:** `reverse/effect-pack-em000-20260908`  
**Base corpus:** `em000-extract.zip`  
**Corpus SHA-256:** `306130125f09824811289366324f4208c3c1aba880c5a7efa3953a88d566d07b`  
**Target:** logical records in `em000_041.pnst`

## Scope

This pass does not assign invented names to effect kind letters. It recovers relationships that are visible directly in the em000 bytes and manifest identifier domains.

The effect manifest provides independent identifier sets for kinds:

```text
G 12
V 50
E 45
P 34
T  8
A 11
M 13
```

That makes it possible to test whether fields inside one record family are selectors into another manifest family rather than guessing from numeric similarity.

---

# 1. E records are a fixed 0x220-byte family

All 45 `E` records in em000 are 544 bytes (`0x220`).

The first four bytes have a highly constrained byte layout:

```text
+0x00 u8  constant type byte = 0x09     # 45/45
+0x01 u8  subtype selector
+0x02 u16 raw modifier
```

Observed `(subtype, modifier)` histogram:

```text
subtype 1, modifier 0x0000 :  8
subtype 1, modifier 0xFFFF :  7
subtype 2, modifier 0x0000 : 10
subtype 2, modifier 0xFFFF :  1
subtype 5, modifier 0x0000 : 19
--------------------------------
                            45
```

Safe names are deliberately structural: `type_byte`, `subtype_selector`, `raw_modifier`.

## 1.1 Subtypes 1 and 2 select T records

For every subtype-1 or subtype-2 E record (`26 / 26`), the low 16 bits of `E +0x04` equal an identifier that exists in the eight-entry `T` manifest domain.

Examples:

```text
E 29   subtype 2   +0x04 = 0x00000019 -> T 25
E 103  subtype 2   +0x04 = 0x0000000D -> T 13
E 61   subtype 1   +0x04 = 0x0000001B -> T 27
E 1480 subtype 1   +0x04 = 0x00000030 -> T 48
E 309  subtype 1   +0x04 = 0xFF000021 -> low16 = T 33
E 1    subtype 1   +0x04 = 0xFF000009 -> low16 = T 9
```

Therefore the safe promotion is:

> For observed E subtypes 1/2, `+0x04 low16` is a **T-kind manifest identifier selector**.

The high 16 bits are not discarded. They vary (`0x0000`, `0xFF00`, `0xFF01`, etc.) and remain preservation/runtime-semantics targets.

Status: `CORPUS_CONFIRMED`.

## 1.2 E +0x08 optionally selects A records

For subtype 1/2, the low 16 bits of `E +0x08` are either:

```text
0xFFFF               -> no observed A selector
or
an identifier present in the A manifest domain
```

Observed A identifiers reached from E include:

```text
20
117
174
178
```

Examples:

```text
E 29    +0x08 = 0xFFFF0014 -> A 20
E 796   +0x08 = 0xFFFF0075 -> A 117
E 1115  +0x08 = 0xFFFF00AE -> A 174
E 1114  +0x08 = 0xFFFF00B2 -> A 178
```

Across all subtype-1/2 records, every non-sentinel low16 value at this field belongs to the A identifier domain in the bound pack.

Safe promotion:

> `E +0x08 low16` is an **optional A-kind manifest identifier selector** for the observed subtype-1/2 layouts; `0xFFFF` is the observed no-selector value.

High bits remain raw.

## 1.3 Subtype 5 selects M records instead of T

All 19 subtype-5 E records show the complementary layout:

```text
E +0x04 low16 = 0xFFFF or raw sentinel form
E +0x08 low16 = 0xFFFF
E +0x28 u32    = identifier in M manifest domain   # 19/19
```

Examples:

```text
E 42   +0x28 = 18  -> M 18
E 58   +0x28 = 17  -> M 17
E 37   +0x28 = 338 -> M 338
E 169  +0x28 = 15  -> M 15
E 797  +0x28 = 105 -> M 105
E 997  +0x28 = 259 -> M 259
E 1001 +0x28 = 261 -> M 261
```

`M 0` is a real manifest identifier, so an observed zero at `E +0x28` for the matching record is a valid selector rather than a generic null assumption.

Safe promotion:

> For observed E subtype 5, `+0x28` is an **M-kind manifest identifier selector**.

Since each M logical entry owns a primary MOD resource plus an optional companion physical slot, this creates the first explicit manifest-level path from an E record to a grouped model resource:

```text
E(subtype 5)
    -> M manifest id
        -> M logical group
            -> primary MOD
            -> optional 16-byte companion
```

No artistic label such as "model emitter" is promoted yet.

---

# 2. P records contain a real internal name and a repeated structural grammar

The 34 `P` records are not one fixed extent. Observed sizes:

```text
0x150 / 336 bytes : 21
0x210 / 528 bytes : 10
0x2C0 / 704 bytes :  1
0x380 / 896 bytes :  2
```

Despite that variation, all 34 fit one arithmetic envelope.

## 2.1 Offset-table span at +0x10

Treat file offset `0x10` as the base of a small relative-offset table.

```text
P +0x10 : u32 table_span
```

Observed:

```text
aux blocks 0 -> table_span 0x10
aux blocks 1 -> table_span 0x10
aux blocks 2 -> table_span 0x10
aux blocks 3 -> table_span 0x20
```

For all 34 records:

```text
table_span = align16(4 * (aux_block_count + 2))
```

The table contains exactly `aux_block_count + 2` non-zero entries followed by zero entries to its aligned span.

## 2.2 Primary block is fixed 0x130 bytes

The primary P block begins immediately after the offset table:

```text
primary_offset = 0x10 + table_span
primary_size   = 0x130
```

This closes on all 34 records.

At `primary_offset + 0x02` there is a NUL-terminated printable ASCII name in every P record.

Examples:

```text
ee000-21p1
ee000-21p2
ee000-42p0
ee008-21p1
ec021-40p0
ec021-40p1
ec021-40p4
ec017-a1p0
```

The two bytes preceding the name vary in the observed set (`02 03`, `01 01`, `00 04`) and remain raw structural selectors.

Safe promotion:

> P carries an **embedded internal source/name token** inside its primary block.

Do not infer the expansion of `ee`, `ec`, `p`, or `a` without executable/producer evidence.

## 2.3 Optional auxiliary area

When `aux_block_count > 0`, the primary block is followed by:

```text
0x10-byte auxiliary header
N * 0xB0-byte auxiliary blocks
```

The complete file-size identity is therefore:

```text
if N == 0:
    size = 0x10 + table_span + 0x130
else:
    size = 0x10 + table_span + 0x130 + 0x10 + N*0xB0
```

This identity holds `34 / 34`.

Observed N distribution:

```text
N=0 : 21 records
N=1 : 10 records
N=2 :  1 record
N=3 :  2 records
```

For N > 0 the non-zero relative table entries follow:

```text
entry[0] = table_span
entry[1] = table_span + 0x130
entry[2] = entry[1] + 0x10
entry[3] = entry[2] + 0xB0
entry[4] = entry[3] + 0xB0
...
```

For N=0 the observed second non-zero entry is `table_span + 0x10`; its exact purpose remains unresolved. It must not be normalized to the N>0 interpretation.

Status of the arithmetic envelope: `CORPUS_CONFIRMED`.

---

# 3. Cross-resource graph now evidenced in em000

The effect pack can no longer be modeled as unrelated opaque records.

At minimum the bound corpus supports:

```text
E subtype 1/2
    -> T id
    -> optional A id

E subtype 5
    -> M id
    -> MOD + optional companion

P
    -> embedded internal name
    -> optional repeated 0xB0 auxiliary blocks
```

The semantic meaning of G/V/E/P/A letters remains open, but their physical and referential contracts are becoming recoverable independently.

---

# 4. Required modular architecture

Do not turn `effect_pack.cpp` into a larger monolith.

Target direction:

```text
include/dmc_rengine/formats/effect_pack/
    manifest.hpp
    group_map.hpp
    e_record_abi.hpp
    p_record_abi.hpp
    p_record_parser.hpp

src/formats/effect_pack/
    ...

include/dmc_rengine/analysis/effect_pack/
    cross_references.hpp
```

The generic PNST parser stays unaware of E/T/A/M/P semantics.

---

# 5. Next reverse targets

1. bind E subtype byte `1/2/5` to canonical EXE dispatch behavior;
2. recover the high 16-bit packing of E `+0x04/+0x08` references;
3. identify A record runtime purpose by following E optional A selectors;
4. identify the consumer of the P embedded name;
5. decode the fixed `0x130` P primary block and `0xB0` auxiliary block independently;
6. test whether P auxiliary-count arithmetic holds in a second effect pack;
7. recover G/V graph relationships rather than assigning names from letter intuition.

## Hard non-claims

This pass does **not** claim:

- E = emitter;
- P = particle;
- A = animation;
- T = texture as a letter expansion, even though T physical members are wrapped DDS;
- M = model as a historical Capcom name, even though its primary members are MOD;
- any meaning for E subtype values 1/2/5 beyond their observed selector layouts.
