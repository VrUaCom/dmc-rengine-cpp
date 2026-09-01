# DMC3 em000 SO/companion binary reverse — 2026-09-02

**Status:** CORPUS-CONFIRMED STRUCTURE + PARTIAL SEMANTICS  
**Branch:** `research/dmc3-primary-3d-format-abi-20260831`

## User-supplied em000 payloads

```text
slot_0038.bin  size 6144
slot_0039.bin  size   96
slot_0040.bin  size 1840
```

These three files were supplied as adjacent resources from `em000`.

## Strong corpus match

A legacy retail-derived `em035` extraction contains the same three structural families adjacent at top level:

```text
em035_011.so   5568
em035_012.ukn   144
em035_013.ukn  2880
```

`em035_011.so` and `slot_0038.bin` share the same compact u16-oriented SO-family fingerprint: small leading selectors, internal offsets, `0xFFFF` sentinels, `0x7FFF` sentinels and compact command/control streams.

The small table and 0x50 table form a repeated companion pair across several actors. `pl011` gives an even stronger example:

```text
pl011_005.so    73120   # large SO-family actor stream
pl011_006.so     1008   # 4 + 251*4 companion table
pl011_007.ukn   20080   # 251*0x50 companion records
```

Therefore the extensions assigned by historical extraction are not sufficient to define the actual structural family: the 4-byte companion table can be labeled `.so` in one corpus and `.ukn` in another.

## slot_0038.bin

### Classification

**SO-family: DATA_CONFIRMED by structural corpus match.**

Current layout observations:

- no ASCII magic;
- u16-oriented packed structure;
- multiple self-relative/in-file offsets;
- extensive `0xFFFF` terminators;
- recurrent `0x7FFF` sentinels;
- compact opcode/control-like values such as `0x0802`, `0x1002`, etc.;
- internal offsets such as `0x09CC`, `0x0B18`, `0x0B98` land on valid structured substreams.

The same family scales strongly with actor complexity:

```text
em035 large SO:   5568 bytes
em000 candidate:  6144 bytes
pl011 large SO:  73120 bytes
```

This makes an actor action/state/sequence-control role a **strong candidate**, but the exact runtime consumer is not yet closed and this semantic name must not be promoted to CONFIRMED yet.

## slot_0039.bin

Exact size:

```text
4-byte header + 23 * 4-byte records = 96 bytes
```

Header:

```text
u32 = 6
```

Each record is four bytes:

```text
byte0  mode/group-like value
byte1  joint/bone-like selector candidate
byte2  link/index candidate
byte3  always zero in this sample
```

Observed value ranges in `em000`:

```text
byte0: 0..4
byte1: 0..20
byte2: 0..22
byte3: 0
```

Cross-corpus `pl011` proves `byte2` is not simply a bone number: in its 251-entry table the field reaches 235, while the character MOD has only about two dozen skeleton bones. Therefore `byte2` is much more likely a table link/next/reference index.

`byte1` remains a strong joint/bone-selector candidate because its range tracks actor skeleton-sized values across samples, but direct EXE consumer evidence is still required.

## slot_0040.bin

Exact size:

```text
23 * 0x50-byte records = 1840 bytes
```

This exactly matches the 23 companion records in `slot_0039.bin` for this `em000` sample.

Cross-corpus pairs show the same relation:

```text
pl011: 251 x 4-byte metadata records + 251 x 0x50 records
em035: closely related pair; one extra/default 0x50 record exists and needs separate investigation
```

### Geometric primitive evidence

The 0x50 records contain a leading integer type and float/vector payloads.

`em000` type distribution:

```text
type 2: 22 records
type 4:  1 record
```

Type-2 examples have the unmistakable shape:

```text
+0x10 float4 center-like vector, w=1
+0x20 float radius-like scalar
```

Example:

```text
center = (0, 50, 0, 1)
radius = 50
```

The single type-4 record has:

```text
+0x10 float4 endpoint A = (0, 231, 106, 1)
+0x20 float4 endpoint B = (0,-120, 106, 1)
+0x30 float radius-like scalar = 95
```

This is strongly diagnostic of a capsule-like geometric volume.

The larger `pl011` corpus contains type 2, type 3 and type 4 records in the same 0x50 family. This makes a collision/hit/interaction-volume descriptor interpretation **HIGH confidence**.

The canonical EXE independently contains primitive vocabulary and parser logic for `sphere`, `box`, `cylinder`, `capsule`, `hit_rot`, `hit_jnt`, `hitattr`, `charhit`, `camhit`, etc. That parser uses a different compact intermediate record layout, so it corroborates the engine's primitive vocabulary but does **not** yet prove the exact enum mapping of the 0x50 companion records.

Do not yet assert `type 2 == sphere` or `type 4 == capsule` as canonical enum names until the direct 0x50 consumer is traced, even though the type-4 payload is geometrically capsule-shaped.

## Relationship to MOD skeleton

The supplied `em000` MOD has 23 bones. `slot_0039` has 23 metadata records and `slot_0040` has 23 primitive records. This is a very strong per-actor correlation, but cross-corpus evidence shows that record count is **not universally equal to skeleton bone count** (`pl011` has 251 records). Therefore these should not be modeled as a simple per-bone array globally.

A safer model is:

```text
Actor SO/control subsystem
├─ large SO-family control/state stream
├─ compact metadata/link table (4 + N*4)
└─ geometric primitive/volume table (N*0x50)
```

Some metadata records likely attach entries to joints/bones, while other indices link entries within the table.

## CSpreadBone evidence

The canonical executable contains RTTI for class `CSpreadBone`. Its relationship to these exact files is currently a **candidate**, not a proven file-to-class binding. The combination of joint-sized selectors, linked metadata and geometric volumes makes it a valuable downstream target for the next EXE trace.

## Current status

- `slot_0038.bin` belongs to the same structural family as actor-level `.so` resources: **DATA_CONFIRMED**.
- exact purpose of large SO family: **HIGH candidate: actor action/state/sequence control; EXE trace required**.
- `slot_0039.bin` is a compact N-entry metadata/link companion table: **DATA_CONFIRMED**.
- `slot_0040.bin` is an N-entry 0x50 geometric-volume companion table: **DATA_CONFIRMED**.
- collision/hit/interaction-volume role for 0x50 records: **HIGH confidence**, exact subsystem and enum names still open.
- direct link to `CSpreadBone`: **candidate**.

## Next reverse gates

1. Trace the direct consumer of the `N*0x50` table and recover exact type enum names/field meanings.
2. Trace the `4+N*4` table consumer and resolve byte0/byte1/byte2 semantics.
3. Trace the large SO-family parser/VM and identify command/opcode meanings.
4. Acquire `em000.index` to recover authoritative extracted names for slots 0038/0039/0040.
5. Validate the pair against additional enemy/player corpus.
