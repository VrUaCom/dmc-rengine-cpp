# DMC3 HD em000 effect system — how the recovered graph works (2026-09-08)

**Branch:** `reverse/effect-pack-em000-20260908`  
**Corpus:** `em000-extract.zip`  
**Corpus SHA-256:** `306130125f09824811289366324f4208c3c1aba880c5a7efa3953a88d566d07b`

## Purpose

This document explains the current recovered effect system as a **resource graph**, not as a list of unrelated files.

The central correction is:

```text
EffectPack != flat file list
EffectPack  = serialized typed graph / effect program
```

The graph lives inside the em000 effect PNST and uses logical manifest kinds `G/V/E/P/T/A/M`. The letters are preserved as original record-kind identifiers; no historical word expansion is invented without executable evidence.

---

# 1. Physical storage versus logical effect graph

The effect resource is physically:

```text
outer PNST
  slot 0 -> ASCII manifest
  slot 1 -> inner PNST
              -> physical record members
```

The manifest gives logical IDs. Most logical entries own one physical member. `M` is the observed exception:

```text
M logical entry
  -> primary MOD physical slot
  -> optional 16-byte companion physical slot
```

Therefore a future editor must preserve two parallel concepts:

```text
logical effect node/group
physical PNST member(s)
```

A logical graph view must never collapse physical slot identity needed for extraction/rebuild.

---

# 2. G and V are routing/control nodes

G and V contain the same structural selector grammar at different offsets.

```text
V +0x04 u16 packed_selector
V +0x06 u16 target_manifest_id

G +0x24 u16 packed_selector
G +0x26 u16 target_manifest_id
```

Low byte of `packed_selector` selects the target manifest family:

```text
0 -> P
1 -> E
2 -> G
3 -> V
```

This validates for every bound G/V selector edge in em000.

The high byte is a separate raw modifier. Its runtime meaning is not recovered yet.

Examples:

```text
G75  -> V108 -> P141
G153 -> V210 -> P192
V26  -> G16  -> P89
V70  -> V69  -> E94
V439 -> G313 -> E68
```

Current safe interpretation:

> G/V form a typed routing/control layer whose outgoing edge selects another G/V node or a terminal P/E family.

In the bound em000 graph:

- 62 G/V selector edges exist;
- every selector resolves to the manifest domain selected by its low byte;
- the G/V subgraph is acyclic;
- each G/V node has one outgoing selector;
- chains terminate in P or E;
- longest observed chain length is two edges.

The acyclic property is corpus-specific and must not be generalized without a second pack.

---

# 3. E has at least two materially different execution layouts

All 45 E records are `0x220` bytes and begin with:

```text
+0x00 u8  type_byte = 0x09
+0x01 u8  subtype
+0x02 u16 raw_modifier
```

Observed subtype values are `1`, `2`, `5`.

## 3.1 E subtype 1/2 — texture-oriented branch

For every subtype 1/2 record:

```text
E +0x04 low16 -> T manifest ID
E +0x08 low16 -> A manifest ID or 0xFFFF
```

Safe graph:

```text
E subtype 1/2
  -> T
  -> optional A
         -> own T
```

`A` owns its own texture selector; it is not merely an alias of the E texture field.

This branch is therefore demonstrably texture-linked, but high-level labels such as sprite/emitter remain unproven.

## 3.2 E subtype 5 — model-backed branch

For all 19 subtype-5 records:

```text
E +0x28 u32 -> M manifest ID
```

and M physically resolves to:

```text
M
  -> MOD
  -> optional raw companion
```

So the effect system supports a direct model-backed path:

```text
E subtype 5
  -> M
      -> MOD geometry/resource
```

This is a separate structural execution path from the texture-oriented subtype 1/2 layout.

---

# 4. T is the texture-bearing endpoint

Each T member is physically:

```text
0x70-byte DMC texture descriptor
-> embedded DDS starting at +0x70
```

It is therefore a wrapped DDS resource, not a plain DDS file starting at byte zero and not a full PTX bundle.

T IDs are selected by E, A, and one P variant.

---

# 5. A is a texture-linked region table

Every A record is `0x150` bytes:

```text
0x04-byte header
33 * 0x0A-byte entries
0x02-byte tail
```

Header:

```text
+0x00 u8 constant 1        # 11/11
+0x01 u8 T manifest ID     # 11/11
+0x02 u8 raw selector
+0x03 u8 raw selector
```

Each 10-byte entry is structurally:

```text
u16 raw_flags
u16 grid_x
u16 grid_y
u16 grid_w
u16 grid_h
```

Corpus invariants:

- non-zero entries form a contiguous prefix;
- active-entry counts are 2, 4, 10, or 16;
- grid values are multiples of 32;
- all rectangles stay within a 0..256 domain;
- the tail is zero in 11/11 records.

Because A independently selects a T texture and its entries form bounded rectangles, an atlas/UV-region interpretation is strong, but the original runtime field names remain open.

Example shape:

```text
A29 -> T25

(1,  0,  0, 64, 64)
(0, 64,  0, 64, 64)
(0,  0, 64, 64, 64)
(0, 64, 64, 64, 64)
```

---

# 6. P is a parameterized terminal family, not one fixed struct

P records use one arithmetic envelope despite four observed physical sizes:

```text
336
528
704
896 bytes
```

Recovered layout:

```text
relative-offset table
-> fixed 0x130 primary block
-> optional 0x10 auxiliary header
-> N * 0xB0 auxiliary blocks
```

The primary block contains a NUL-terminated internal production/source name such as:

```text
ee000-21p1
ee000-42p0
ec021-40p4
```

Primary bytes `[0],[1]` split em000 P into at least three layout variants:

```text
02 03 -> 30 records
01 01 ->  3 records
00 04 ->  1 record
```

For variant `02 03` only:

```text
primary +0xE0 low16 -> T manifest ID
primary +0x106 u16  -> A manifest ID
```

and the graph is coherent:

```text
P -> T
P -> A -> same T
```

The other P variants use different data at those offsets, so one universal P texture layout is explicitly rejected.

---

# 7. Current recovered execution graph

```text
G/V routing node
  -> G/V routing node (optional)
  -> P or E terminal family

P variant 02 03
  -> T
  -> A
      -> T

E subtype 1/2
  -> T
  -> optional A
      -> T

E subtype 5
  -> M
      -> MOD
      -> optional companion
```

This is the strongest current evidence-backed explanation of how em000 effect resources relate to each other.

---

# 8. What the game probably does — evidence boundary

The resource graph itself is corpus-confirmed. The missing top-level runtime edge is:

```text
enemy/gameplay event
    -> root effect logical ID
    -> effect graph traversal
```

A plausible runtime pipeline is:

```text
CEm*/gameplay event
 -> request effect root
 -> evaluate G/V routing if present
 -> reach P or E
 -> resolve T/A or M/MOD dependencies
 -> construct render/effect runtime object
 -> renderer
```

Only the graph below the requested root is currently recovered from bytes. The exact gameplay-to-root selector and the runtime traversal functions still require canonical EXE reverse.

---

# 9. Editor architecture implied by the reverse

A future Enemy/Effect Editor should not present this as a flat file browser. It should expose a graph while preserving physical storage:

```text
Effect Graph
  G/V nodes
  P/E terminal nodes
  T texture nodes
  A region-table nodes
  M model-group nodes
  MOD resources
```

Every graph node must retain:

```text
manifest kind + logical ID
physical PNST slot(s)
raw bytes / SHA-256
reference edges
semantic evidence status
```

This would make effect authoring possible later without destroying the original PNST structure.

---

# 10. Hard non-claims

This document does not claim:

- G = generator;
- V = variation;
- E = emitter;
- P = particle;
- A = animation/atlas as an original Capcom name;
- M = model as the original expansion;
- that high-byte G/V modifiers are flags of any specific meaning;
- that A coordinates are confirmed UVs by EXE;
- that every DMC3 effect pack has an acyclic G/V graph;
- writer authority.

Those names/behaviors remain reverse targets.