# DMC3 HD MOD — full recursive `em000` corpus sweep (2026-09-07)

**Branch:** `reverse/mod-completion-20260907`  
**Corpus:** `em000-extract.zip`  
**Corpus SHA-256:** `306130125f09824811289366324f4208c3c1aba880c5a7efa3953a88d566d07b`  
**Canonical executable authority:** `dmc3.exe` SHA-256 `e454272ed0fb0247fcbcf300e5d55d7a3e96d50b89b9ffaff81bb978dcbdd082`

## Scope

This pass recursively inspects every `.mod` present in the supplied `em000` extraction and asks two separate questions:

1. does every payload fit the currently recovered MOD structural grammar; and
2. does any payload contain a new non-zero serialized area or field pattern outside the already known/preserved MOD sections?

The scan is evidence-only. It does not promote writer authority merely because a corpus invariant is stable.

## Corpus size

The recursive extraction contains:

- **35 unique MOD payloads**;
- **22 top-level MODs** under `em000/`;
- **13 nested MODs** under `em000_041.pnst/...`;
- **136 outer/object records**;
- **147 mesh records**;
- **14,804 vertices**;
- **226 node/transform positions**;
- **628,192 MOD bytes** total.

All 35 MOD SHA-256 hashes are unique.

## Primary result

**No new non-zero serialized block was found outside the currently known MOD section model.**

A byte-coverage pass marked:

- the 0x40 document header;
- the complete 0x40 outer/object records;
- the complete 0x50 mesh records;
- position, normal, UV, BLENDINDICES and packed-control streams;
- node-domain header and arrays;
- local transform records;
- generated-topology workspaces.

After those ranges were marked, the remaining uncovered space across all 35 files was **5,106 bytes**, and every one of those bytes was zero alignment/padding.

This is strong corpus evidence that the current MOD section map covers every non-zero region in this `em000` recursive corpus. It is not a proof that another actor family cannot contain an additional section.

## New finding 1 — retail MOD revision range is wider than 1.01

The current parser was originally conservative around the 1.01 corpus revision. This full sweep finds four retail version values while the same structural grammar remains valid:

| Serialized version | Files |
|---:|---:|
| `1.01` | 24 |
| `1.00` | 6 |
| `0.84` | 4 |
| `0.82` | 1 |

Therefore `0.82`, `0.84` and `1.00` are not malformed by themselves in this retail `em000` corpus.

For all eleven non-1.01 payloads, the same recovered contracts still hold:

- header size 0x40;
- object stride 0x40;
- mesh stride 0x50;
- absolute stream pointers;
- node-domain relative layout;
- topological parent/order arrays;
- 0x20 local-transform records;
- skin packing;
- 16-byte stream alignment;
- generated-workspace geometry.

This is `CORPUS_CONFIRMED` revision compatibility, not yet a claim that every DMC3 MOD revision globally uses identical semantics.

## New finding 2 — real retail use of object `+0x18/+0x1C`

`em000_033.mod` supplies concrete retail data for fields that were already EXE-confirmed as live render-path inputs:

```text
object 1:
    source_flags = 0x00100201
    +0x18 f32     = 1.25
    +0x1C u32     = 2

object 2:
    source_flags = 0x00100201
    +0x18 f32     = 1.25
    +0x1C u32     = 2
```

All other 134 objects in this corpus store zero in both fields.

Because flag `0x00000200` is active in `0x00100201`, this retail observation matches the canonical executable path that copies `+0x18/+0x1C` into runtime state. This strengthens those fields from “EXE-live but synthetic/corpus-unspecified” to **EXE_AND_CORPUS_CONFIRMED as genuinely populated retail inputs**. Their artistic meaning remains open.

## New finding 3 — header `+0x14` has a strong decimal structure in MOD too

All 35 MOD values at serialized header `+0x14` admit the same arithmetic split:

```text
raw = family_class * 100000
    + model_set    * 100
    + sub_index
```

Examples from this corpus:

```text
100407 -> (1, 4, 7)
100200 -> (1, 2, 0)
100201 -> (1, 2, 1)
100202 -> (1, 2, 2)
202400 -> (2, 24, 0)
202900 -> (2, 29, 0)
601700 -> (6, 17, 0)
601715 -> (6, 17, 15)
700000 -> (7, 0, 0)
700007 -> (7, 0, 7)
700601 -> (7, 6, 1)
```

This is materially stronger MOD corpus evidence than the previous single-flow fact `serialized +0x14 -> manager +0xE4`.

However, this pass **does not promote SCM's high-level `LegacyResourceCode` semantic into MOD**. Until a MOD-specific executable consumer is bound, the safe state is:

```text
serialized arithmetic shape = CORPUS_CONFIRMED
runtime carriage to +0xE4 = EXE_CONFIRMED
MOD high-level semantics = SEMANTIC_CANDIDATE / PRESERVED_UNDECODED
```

## New finding 4 — generated topology workspace capacity is exact across all 147 meshes

For every mesh:

```text
workspace_capacity = align16(6 * (vertex_count - 2))
```

matches the physical span from that mesh's generated-workspace pointer to the next workspace (or EOF for the final mesh).

Result:

```text
147 / 147 exact matches
```

Workspace bytes are the known `0x12` sentinel pattern. In every one of the 35 payloads the final file tail is:

```text
12 12 12 12 12 12 12 12 12 12 12 12 12 12 00 00
```

This is a strong corpus constraint useful for future layout planning. It is not yet standalone writer authority.

## Complete unknown/reserved audit

No previously preserved unknown field became non-zero in this corpus:

| Area | Result |
|---|---:|
| header `+0x08..+0x0F` | zero in 35/35 |
| header `+0x18..+0x1F` | zero in 35/35 |
| header `+0x28..+0x3F` | zero in 35/35 |
| object `+0x04..+0x07` | zero in 136/136 |
| object `+0x14..+0x17` | zero in 136/136 |
| object `+0x20..+0x2F` | zero in 136/136 |
| mesh `+0x0C` | zero in 147/147 |
| mesh `+0x38` | zero in 147/147 |
| mesh generated count `+0x48` | zero in 147/147 serialized records |
| mesh `+0x4C` | zero in 147/147 |
| node-domain header `+0x10..+0x1F` | zero in 35/35 |
| transform `+0x1C` | zero in 226/226 |
| `BLENDINDICES.x` | zero in 14,804/14,804 |
| uncovered bytes after known-range marking | 5,106/5,106 zero |

These values remain preservation constraints, not permission for a writer to synthesize zeros globally.

## Structural/skin validation across all 35

All 35 payloads satisfy the current structural grammar:

- magic `MOD `;
- all object tables in bounds;
- all mesh tables in bounds;
- every outer aggregate count equals the sum of its child mesh counts;
- all five serialized vertex streams are in bounds;
- all stream starts are 0x10 aligned;
- all node-domain offsets match the recovered core layout;
- every node-order array is a complete permutation;
- every hierarchy is topological/acyclic under the recovered semantics;
- all local transform components are finite;
- every translation magnitude matches `length(translation.xyz)` within tolerance;
- every active blend-index lane is divisible by four;
- every active bone index is in range;
- every vertex has `q0 + q1 + q2 == 31`;
- every vertex has `q0 >= q1 >= q2`;
- active influences are unique per vertex;
- all position/normal floats are finite.

Skin census over 14,804 vertices:

| Active influences | Vertices |
|---:|---:|
| 1 | 11,995 |
| 2 | 2,353 |
| 3 | 456 |

Topology-break high bit `0x8000` is present on **4,693 vertices**.

Normals are effectively unit length over the full corpus. Serialized UVs stay inside:

```text
U: 0 .. 4096
V: -180 .. 4096
```

No UV int16 overflow case appears in this corpus.

## Material/state coverage limits of em000

The corpus is broad structurally, but it does **not** exercise every render-state possibility.

Observed facts:

- all 136 objects use alpha/control `0x80`;
- all 147 mesh GS CLAMP fields are `(0,0,0,0)`;
- texture slots observed are 0..3 and are always below the header texture-domain mirror;
- the header texture mirror is a domain/capacity value, not merely `max_used_slot + 1` (10 files reserve unused slots).

Observed source-flag bits across 136 objects:

```text
0x00000001 : 38 objects
0x00000002 : 2 objects
0x00000200 : 2 objects
0x00004000 : 1 object
0x00020000 : 17 objects
0x00100000 : 36 objects
0x00200000 : 7 objects
```

`0x00200000` is especially important: it occurs in seven retail objects, but the current MOD object-runtime projection does not yet give it a promoted high-level semantic. It remains an active Wave-A reverse target rather than “padding”.

## Motion groups

Across 226 node order positions:

```text
group 0 = 132
group 1 = 91
group 2 = 3
```

No group value above 2 appears.

The non-zero groups remain concentrated in the large skeleton models already identified; all small/nested one-node models use group 0 only. This adds no new group value semantics.

## Per-file census

| MOD | ver | obj/mesh/vtx | nodes | tex | Jnt | +0x14 | groups | flags union | note |
|---|---:|---:|---:|---:|---:|---:|---|---:|---|
| `em000_001.mod` | 1.01 | 19/22/2641 | 23 | 4 | 1 | 100407 | 0:9,1:13,2:1 | `0x00320002` | canonical-shape |
| `em000_003.mod` | 1.01 | 1/1/48 | 5 | 4 | 1 | 100407 | 0:5 | `0x00200000` | canonical-shape |
| `em000_004.mod` | 1.01 | 2/2/292 | 23 | 1 | 1 | 100673 | 0:9,1:13,2:1 | `0x00000000` | canonical-shape |
| `em000_005.mod` | 1.01 | 20/23/2242 | 23 | 2 | 1 | 100407 | 0:9,1:13,2:1 | `0x00320002` | canonical-shape |
| `em000_007.mod` | 1.01 | 1/1/48 | 5 | 2 | 1 | 100407 | 0:5 | `0x00200000` | canonical-shape |
| `em000_008.mod` | 1.01 | 22/23/1851 | 22 | 4 | 1 | 100200 | 0:9,1:13 | `0x00120001` | canonical-shape |
| `em000_010.mod` | 1.00 | 1/1/34 | 6 | 4 | 1 | 100201 | 0:6 | `0x00100001` | revision variant |
| `em000_012.mod` | 1.00 | 1/1/34 | 6 | 4 | 1 | 100202 | 0:6 | `0x00100001` | revision variant |
| `em000_013.mod` | 1.01 | 18/20/1597 | 22 | 4 | 1 | 100300 | 0:9,1:13 | `0x00100001` | canonical-shape |
| `em000_015.mod` | 1.00 | 1/1/52 | 5 | 4 | 1 | 100301 | 0:5 | `0x00100001` | revision variant |
| `em000_017.mod` | 1.00 | 1/1/33 | 5 | 4 | 1 | 100302 | 0:5 | `0x00100001` | revision variant |
| `em000_018.mod` | 1.01 | 3/4/1295 | 22 | 4 | 1 | 100407 | 0:9,1:13 | `0x00120001` | canonical-shape |
| `em000_019.mod` | 1.01 | 19/20/1355 | 22 | 4 | 1 | 100407 | 0:9,1:13 | `0x00120001` | canonical-shape |
| `em000_021.mod` | 1.01 | 1/1/3 | 5 | 4 | 1 | 100407 | 0:5 | `0x00200000` | canonical-shape |
| `em000_026.mod` | 0.84 | 1/1/119 | 2 | 1 | 0 | 202400 | 0:2 | `0x00000000` | revision variant |
| `em000_027.mod` | 1.00 | 1/1/150 | 2 | 1 | 0 | 202500 | 0:2 | `0x00100001` | revision variant |
| `em000_028.mod` | 0.84 | 1/1/192 | 2 | 1 | 0 | 202600 | 0:2 | `0x00000000` | revision variant |
| `em000_029.mod` | 0.82 | 1/1/66 | 2 | 1 | 0 | 203000 | 0:2 | `0x00000000` | revision variant |
| `em000_030.mod` | 0.84 | 1/1/128 | 2 | 1 | 0 | 202500 | 0:2 | `0x00100001` | revision variant |
| `em000_031.mod` | 0.84 | 2/2/199 | 2 | 1 | 0 | 202600 | 0:2 | `0x00000000` | revision variant |
| `em000_033.mod` | 1.01 | 3/3/376 | 3 | 3 | 1 | 202800 | 0:3 | `0x00120201` | retail +18/+1C live |
| `em000_034.mod` | 1.00 | 3/3/379 | 4 | 2 | 1 | 202900 | 0:4 | `0x00120001` | revision variant |
| `em000_pnst0041_pnst0001_160.mod` | 1.01 | 1/1/206 | 1 | 1 | 0 | 601700 | 0:1 | `0x00000000` | nested |
| `em000_pnst0041_pnst0001_162.mod` | 1.01 | 1/1/59 | 1 | 1 | 0 | 700001 | 0:1 | `0x00020001` | nested |
| `em000_pnst0041_pnst0001_164.mod` | 1.01 | 1/1/122 | 1 | 1 | 0 | 700000 | 0:1 | `0x00020001` | nested |
| `em000_pnst0041_pnst0001_166.mod` | 1.01 | 1/1/42 | 1 | 1 | 0 | 602002 | 0:1 | `0x00020001` | nested |
| `em000_pnst0041_pnst0001_168.mod` | 1.01 | 1/1/149 | 1 | 1 | 0 | 700007 | 0:1 | `0x00020000` | nested |
| `em000_pnst0041_pnst0001_170.mod` | 1.01 | 1/1/18 | 1 | 1 | 0 | 601708 | 0:1 | `0x00000000` | nested |
| `em000_pnst0041_pnst0001_172.mod` | 1.01 | 1/1/403 | 1 | 1 | 0 | 601715 | 0:1 | `0x00024000` | nested |
| `em000_pnst0041_pnst0001_174.mod` | 1.01 | 1/1/139 | 1 | 1 | 0 | 700601 | 0:1 | `0x00000000` | nested |
| `em000_pnst0041_pnst0001_176.mod` | 1.01 | 1/1/294 | 1 | 1 | 0 | 700002 | 0:1 | `0x00020000` | nested |
| `em000_pnst0041_pnst0001_178.mod` | 1.01 | 1/1/122 | 1 | 1 | 0 | 700003 | 0:1 | `0x00020000` | nested |
| `em000_pnst0041_pnst0001_180.mod` | 1.01 | 1/1/32 | 1 | 1 | 0 | 700004 | 0:1 | `0x00020000` | nested |
| `em000_pnst0041_pnst0001_182.mod` | 1.01 | 1/1/72 | 1 | 1 | 0 | 700005 | 0:1 | `0x00020000` | nested |
| `em000_pnst0041_pnst0001_184.mod` | 1.01 | 1/1/12 | 1 | 1 | 0 | 700006 | 0:1 | `0x00020000` | nested |

## Conclusion

For the exact recursive `em000` corpus:

**Every one of the 35 MODs is structurally covered by the current MOD grammar. No payload contains a new non-zero serialized section outside the known header/object/mesh/streams/node-domain/workspace map.**

What *is* new is variation inside that grammar:

1. real retail MOD versions `0.82`, `0.84`, `1.00`, and `1.01`;
2. real non-zero object `+0x18/+0x1C` inputs in `em000_033.mod`;
3. much stronger MOD corpus structure for header `+0x14`;
4. exact 147/147 generated-workspace capacity behavior;
5. a seven-object retail census for still-open source bit `0x00200000`.

So the next reverse work should not search `em000` for a missing sixth stream or hidden section—the full byte sweep finds none. The higher-value targets are now **semantics of fields already located**, especially header `+0x14`, source flag `0x00200000`, source `0x00100000`, and then cross-family comparison against `pl###`, `id###`, boss/weapon and other actor MOD corpora.
