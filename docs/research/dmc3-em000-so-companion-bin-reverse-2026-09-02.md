# DMC3 em000 SO/companion binary reverse — 2026-09-02

**Status:** CORPUS-CONFIRMED STRUCTURE + PARTIAL RUNTIME SEMANTICS  
**Branch:** `research/dmc3-primary-3d-format-abi-20260831`

## User-supplied em000 payloads

```text
slot_0038.bin  size 6144
slot_0039.bin  size   96
slot_0040.bin  size 1840
```

These three files were supplied as adjacent resources from `em000`.

## Authoritative corpus relation

Retail-derived extraction manifests provide the strongest naming evidence currently available:

```text
em035_011.so   5568
em035_012.ukn   144
em035_013.ukn  2880

pl011_005.so   73120
pl011_006.so    1008
pl011_007.ukn  20080
```

`slot_0038` structurally matches the large `.so` family. `slot_0039` structurally matches `pl011_006.so` exactly as a compact `4 + N*4` SO table. `slot_0040` matches the headerless `N*0x50` `.ukn` companion family.

## 1. slot_0038.bin — large SO typed-chunk graph

### Root header

First bytes:

```text
06 00 98 0B FF FF 08 00 CC 09 18 0B FF FF BC 00 ...
```

Interpreted as u16:

```text
+0x00  0x0006   root type = 6
+0x02  0x0B98   relative/in-file child offset
+0x04  0xFFFF   absent/optional reference sentinel
+0x06  0x0008   child type = 8
+0x08  0x09CC   structured subtable offset
+0x0A  0x0B18   structured subtable offset
+0x0C  0xFFFF   absent/optional reference sentinel
+0x0E  ...      u16 entry-offset table
```

The strongest structural proof is that `+0x02` lands on another typed SO chunk and `+0x06` predicts its type:

```text
em000 slot_0038:  type 6 -> 0x0B98 -> first u16 = 8
em035_011.so:     type 6 -> 0x0DD6 -> first u16 = 10
em035_038.so:     type 6 -> 0x01B6 -> first u16 = 10
em035_043.so:     type 6 -> 0x026E -> first u16 = 10
pl011_005.so:     type 6 -> 0xD740 -> first u16 = 4
```

Therefore large `.SO` is not a flat record file. It is a **typed recursive chunk/object graph with relative offsets**.

### Root entry table

The first entry offset is `0x00BC`. If the offset table begins at `0x0E`, this self-describes:

```text
(0xBC - 0x0E) / 2 = 87 top-level entry offsets
```

The offsets are increasing and point to variable-length compact records. Many spans are `0x16` bytes, while others are larger. Records contain `0xFFFF`, `0x7FFF`, references to other in-file structures and compact values/opcode-like words such as `0x0802`, `0x0202`, `0x0602`, `0x0110`, `0x0111`.

### Child TYPE 8 chunk at 0x0B98

Its start is:

```text
08 00 8E 0A FE 0B FF FF 9E 00 AE 00 FE 00 42 01 ...
```

The first record offset is `0x009E`; with an 8-byte header:

```text
(0x9E - 0x08) / 2 = 75 entry offsets
```

References `0x0A8E` and `0x0BFE`, interpreted relative to the TYPE-8 chunk base, land on valid structured data at absolute offsets `0x1626` and `0x1796`.

### Current classification

- `.SO` family: **DATA_CONFIRMED / HIGH**.
- typed recursive chunk graph: **STRUCTURE_CONFIRMED**.
- type IDs observed in corpus include at least 4, 6, 8 and 10 in linked chunks.
- exact semantic expansion of the acronym `SO`: **UNKNOWN**. Existing `Stage Object Candidate` wording is too narrow because the same family is heavily used in enemy/player actor packages.
- actor action/state/sequence-control interpretation: **HIGH candidate**, direct consumer still required.

## 2. slot_0039.bin — compact SO link/attachment table

Exact layout:

```text
size = 96 = 4-byte header + 23 * 4-byte records
```

Header:

```text
06 00 00 00
```

This is **not count=6**. Authoritatively named `pl011_006.so` begins with the same `06 00 00 00` header and has:

```text
1008 = 4 + 251 * 4
```

Therefore `6` is a SO subtype/class/discriminator candidate, not record count.

Each record is:

```text
byte0  mode/kind/group candidate
byte1  joint/bone selector candidate
byte2  link/next/reference index candidate
byte3  zero/reserved in observed samples
```

For the supplied `em000` sample, `byte2` predominantly follows `i+1`, with zero/other links breaking or redirecting chains. In `pl011`, this field reaches values far above the character skeleton bone count, proving it is not simply a bone index.

Current status:

- compact SO table form: **HIGH / corpus-confirmed by authoritative `.so` sample**.
- byte2 as link/index: **HIGH**.
- byte1 as joint/bone selector: **MEDIUM-HIGH candidate**.
- byte0 exact mode semantics: **OPEN**.

## 3. slot_0040.bin — headerless N x 0x50 typed geometric-volume table

Exact layout:

```text
1840 = 23 * 0x50
```

There is no independent file header. Every `0x50` record begins with its own u32 type.

Corpus:

```text
em000:         23 records: 22 x type2, 1 x type4
em035_013.ukn: 36 records: type2 family
pl011_007.ukn: 251 records: type2/type3/type4 family
```

### Type-2 geometry

Typical payload:

```text
+0x10 float4 center-like vector, w=1
+0x20 radius-like scalar
```

Example:

```text
center = (0, 50, 0, 1)
radius = 50
```

This is strongly sphere-like, but the canonical enum name is not yet promoted.

### Type-4 geometry

The supplied record has:

```text
+0x10 A = (0, 231, 106, 1)
+0x20 B = (0,-120, 106, 1)
+0x30 radius-like = 95
```

`pl011` has the same structural type-4 pattern. This is geometrically capsule-like with very high confidence, but exact enum naming still awaits the direct loader/dispatcher.

### Relationship to slot_0039

```text
em000:  23 compact SO metadata records <-> 23 volume records
pl011: 251 compact SO metadata records <-> 251 volume records
```

`em035` has a one-record mismatch that remains an explicit open issue.

`slot_0039.byte0` does not equal the volume type, so the compact SO record is metadata/linkage, not a duplicate shape enum.

## 4. EXE runtime evidence for 0x50 volume arrays

Function `0x140056220` is now a strong runtime consumer candidate for **0x50-stride geometric arrays**.

Behavior:

```text
input: base pointer, count, output index
loop:
    test current element
    current += 0x50
    if match -> return index
```

The stride is explicit in machine code:

```asm
addq $0x50, %rdi
```

Each element is passed into `0x1402CEB30`, which performs spatial overlap/inside-style tests using vector/scalar fields and calls lower-level geometry routines. This materially strengthens the collision/hit/interaction-volume interpretation for the 0x50 family, though a direct file-loader-to-runtime-struct trace is still needed before naming the raw file format semantically.

A neighboring transform helper `0x1400562A0` operates on a runtime 0x50 geometry structure as four vec4-like blocks plus a trailing field at `+0x40`, transforms/copies the vectors and preserves the trailing field. This suggests the raw file records may be converted/repacked into a related runtime representation rather than consumed byte-for-byte.

## 5. CSpreadBone correction

Earlier suspicion that the SO companion/0x50 volume pair might directly belong to `CSpreadBone` is **REJECTED / corrected**.

Canonical EXE RTTI and direct methods show `CSpreadBone` is a separate secondary-bone physics system:

```text
init/build    0x140322210
wrapper       0x140322580
update        0x1403225B0
offset-all    0x1403227C0
```

Recovered runtime facts:

```text
count at object +0xB10
entry stride = 0x40
maximum entries = 0x2C = 44
entries start around object +0x10
```

The update loop integrates per-entry motion/lag and writes transforms back into target bone matrices. This is relevant to CLT/secondary physics research, but **is not the N*0x50 geometric-volume table**.

## 6. Current actor subsystem model

```text
Actor package
|
+-- large SO typed chunk graph
|   +-- TYPE 6 root
|   +-- linked TYPE 4 / 8 / 10 child chunks
|   +-- variable compact command/control records
|
+-- compact SO metadata/link table
|   +-- mode/kind candidate
|   +-- joint selector candidate
|   +-- next/link index
|
+-- headerless N*0x50 volume table (.ukn in authoritative manifests)
    +-- typed geometric primitive descriptor
    +-- spatial vectors/scalars
    +-- high-confidence collision/hit/interaction role
```

## 7. Evidence status

- `slot_0038` -> large `.SO`: **HIGH / DATA_CONFIRMED**.
- large SO = recursive typed chunk graph: **CONFIRMED**.
- `slot_0039` -> compact `.SO` table form: **HIGH / authoritative corpus match**.
- `slot_0040` -> headerless `.ukn` N*0x50 geometric table: **CONFIRMED structure**, semantic role **HIGH**.
- `CSpreadBone` direct relation to 0039/0040: **REJECTED**.
- `CSpreadBone` as independent secondary-bone physics system: **EXE-CONFIRMED**.
- exact SO acronym and type-4/6/8/10 meanings: **OPEN**.
- exact 0x50 raw shape enum mapping: **OPEN**.

## 8. Next reverse gates

1. Trace the SO type dispatcher and recover exact meanings for TYPE 4 / 6 / 8 / 10.
2. Trace the compact `06 00 00 00 + N*4` table loader and lock byte0/byte1/byte2 semantics.
3. Find the raw `type-first 0x50` loader and prove how it converts into the runtime 0x50 geometry representation used around `0x140056220`.
4. Recover exact raw shape enum mapping for type2/type3/type4.
5. Acquire authoritative `em000.index` if available to lock the original extracted names for slots 0038/0039/0040.
