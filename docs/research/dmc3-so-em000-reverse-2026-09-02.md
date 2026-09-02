# DMC3 HD SO / em000 structural reverse

**Snapshot:** 2026-09-02  
**Branch:** `research/so-cpp20`  
**Scope:** read-only reverse of the em000 SO working set and its direct companion payloads.  
**Canonical EXE SHA-256:** `e454272ed0fb0247fcbcf300e5d55d7a3e96d50b89b9ffaff81bb978dcbdd082`

## Naming boundary

`SO` is the current project working name for this structure family. The observed archive leaves used in this pass are named `slot_0038.bin`, `slot_0039.bin` and `slot_0040.bin`; no claim is made yet that `.so` is an embedded/original filename extension or runtime lookup extension. Identity is therefore context/evidence bound, not extension-authoritative.

## Hash-bound corpus

| Role | Source | Size | SHA-256 |
|---|---|---:|---|
| SO graph/control payload | `slot_0038.bin` | 6144 | `99bfcb7259a3db9b8f97ab441e07d30afdcc16425edcd694a5d758190fa3fb50` |
| compact companion/link table | `slot_0039.bin` | 96 | `147d63c8ca5dac72c78ad869fbc3c20505ef425d087b378051f6bca01b77eb13` |
| fixed-stride spatial-volume payload | `slot_0040.bin` | 1840 | `e188cc929df800a09b71838142d15461ee3b6d7c6104da370b16e839d355158a` |
| companion MOD | `slot_0001 (3).mod` | 110096 | `34f2c03795b007eac67abdb3a5808f675f1b1fd419ea6f39d4b77b8142c9e7ce` |

The source binaries are evidence inputs and are not committed.

## DATA_CONFIRMED: `slot_0038`

The payload is not a flat mesh. It contains at least two indexed structural blocks.

### Type-6 block

- starts at file offset `0x0000`;
- first u16 is `6`;
- u16 at `+0x02` is `0x0B98`; at that exact file offset an indexed type-8 block begins;
- the currently bounded type-6 header is `0x0E` bytes;
- the first indexed-entry offset is `0x00BC`;
- `(0x00BC - 0x0E) / 2 == 87`, therefore the table contains exactly **87 strictly increasing u16 entry offsets**;
- entries are variable length. The dominant adjacent-offset delta is `0x16`, but larger spans occur and must not be normalized to a fixed stride.

The semantic meaning of header words `+0x04..+0x0C` is still open. In particular, the in-range offsets `0x09CC` and `0x0B18` identify auxiliary regions, but their role is not named yet.

### Type-8 block

- starts at `0x0B98`;
- first u16 is `8`;
- bounded header size is `0x08` bytes;
- first entry offset relative to the block is `0x009E`;
- `(0x009E - 0x08) / 2 == 75`, therefore the block contains exactly **75 strictly increasing u16 entry offsets**;
- type-8 entries are variable length; no fixed record stride is claimed.

The current C++20 parser deliberately exposes raw header words and offset tables, not invented opcode names.

## DATA_CONFIRMED: `slot_0039`

`96 / 4 == 24`, therefore this companion is exactly **24 four-byte records** in the current corpus.

Observed shape:

```text
record := u8 field0, u8 field1, u8 field2, u8 field3
```

All observed `field3` bytes are zero. Record 0 is `06 00 00 00` and behaves as a header-like prefix for correlation purposes only; exact semantics remain open.

Do not yet rename `field1` to `joint`, `parent`, `bone` or similar. Those interpretations are candidates pending direct runtime consumer evidence.

## DATA_CONFIRMED: `slot_0040`

`1840 / 0x50 == 23`, therefore this payload contains exactly **23 records of 0x50 bytes**.

Bounded raw layout:

```text
+0x00 u32 type
+0x04 12 bytes unknown/reserved in this corpus
+0x10 float4 vector0
+0x20 float4 vector1
+0x30 float4 vector2
+0x40 float4 vector3
```

Observed type census:

- `22 x type 2`
- `1 x type 4`

For type 2, `vector0.xyz` behaves geometrically like a center candidate and `vector1.x` like a radius/extent candidate. Example record 0 contains `(0, 50, 0, 1)` and scalar `50`.

For type 4, the single observed record contains:

```text
vector0 = (0, 231, 106, 1)
vector1 = (0, -120, 106, 1)
vector2.x = 95
```

This is strongly consistent with a two-endpoint, radius-bearing capsule-like volume. The editor/visualizer may render it as a **candidate capsule** but must not label it hitbox/hurtbox/collision until consumer semantics are confirmed.

## Cross-file correlation

### `0039 <-> 0040`

There are 24 compact records and 23 volume records. Therefore the current corpus satisfies exactly:

```text
0039 record 0        -> header-like prefix candidate
0039 records 1..23   -> 23 records available for one-to-one correlation
0040 records 0..22   -> 23 spatial volume records
```

Status: **DATA_CONFIRMED cardinality; HIGH_CONFIDENCE one-to-one correlation; field semantics open**.

### `MOD <-> SO companion cardinality`

The companion MOD is a valid `MOD ` v1.01 stream document with:

- 19 outer records;
- 22 inner records;
- 2641 elements;
- matrix-index byte values exactly `0,4,8,...,88`.

The existing MOD reverse establishes those bytes as matrix/blend-index selectors with values divided by four for the 0..22 selector universe. This em000 MOD therefore exposes exactly **23 matrix selector indices: 0 through 22**.

That count is identical to the **23 volume records** in `slot_0040`. Together with the **23 post-prefix records** in `slot_0039`, this is strong evidence that the SO companion set participates in the actor transform/skeleton space rather than being an unrelated global table.

Status: **HIGH_CONFIDENCE cross-resource transform-palette correlation; not yet EXE_CONFIRMED ownership/binding**.

## Product boundary in `research/so-cpp20`

Implemented in the first C++20 slice:

- fail-closed read-only parser for the observed type-6 -> type-8 graph structure;
- raw compact-link-table parser;
- raw 0x50 spatial-volume parser;
- correlation helper for the `1 + N` link-table / `N` volume relation;
- synthetic regression test;
- real-corpus development probe passed outside the repository.

Not implemented:

- writer or mutation;
- opcode semantics;
- exact skeleton/joint binding;
- runtime ownership/lifetime;
- hitbox/hurtbox/damage/collision labels;
- type-3 semantics;
- universal SO variant coverage.

## Next reverse passes

1. Recover the canonical EXE consumer that reads the 0x50 volume record and prove the switch semantics for types 2/4 (and search for type 3).
2. Trace the 96-byte compact table consumer and determine whether `field1` / `field2` are matrix/joint/parent/next references.
3. Bind the 23-record companion relation to the MOD matrix palette in machine code.
4. Recover the producer/consumer of type-6 and type-8 indexed blocks and identify the auxiliary regions at `0x09CC`, `0x0B18`, and type-8 internal offsets.
5. Only after those steps, promote ModViz overlays from candidate geometry to named gameplay semantics.
