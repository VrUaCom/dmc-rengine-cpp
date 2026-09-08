# DMC3 HD em000 effect pack — T wrapper metrics and A normalized-grid relation (2026-09-08)

**Branch:** `reverse/effect-pack-em000-20260908`  
**Corpus:** `em000-extract.zip`  
**Corpus SHA-256:** `306130125f09824811289366324f4208c3c1aba880c5a7efa3953a88d566d07b`

## Scope

This pass tightens the relation between `A` and `T` without promoting unproven historical names.

Previously established:

- `A +0x01` selects a T manifest ID in 11/11 records;
- A owns 33 fixed-capacity 10-byte rectangle-like entries;
- A grid values lie in a 0..256 domain with quantum 32;
- T is a `0x70` DMC wrapper followed by an embedded DDS resource.

This pass validates the wrapper fields against the embedded DDS metadata and shows that the A grid is independent of the physical 128/256 texture dimensions.

---

# 1. T corpus

The bound effect pack contains eight T members.

Embedded DDS properties:

```text
T25 : 256x256, DXT5, 9 mip levels
T26 : 128x128, DXT5, 8 mip levels
T32 : 128x128, DXT5, 8 mip levels
T13 : 128x128, DXT5, 8 mip levels
T33 : 128x128, DXT5, 8 mip levels
T9  : 128x128, DXT5, 8 mip levels
T27 : 128x128, DXT5, 8 mip levels
T48 : 128x128, DXT5, 8 mip levels
```

All eight embedded images are DXT5 in the bound pack.

---

# 2. DMC 0x70 wrapper fields mirror DDS metrics

For every T member, the following relations hold exactly.

## 2.1 Packed dimension words

Wrapper fields:

```text
+0x10 u32 packed_dimension_word
+0x44 u32 duplicated_packed_dimension_word
```

For all eight square textures:

```text
128x128 -> 0x00800080
256x256 -> 0x01000100
```

Because all bound samples are square, the exact width/height half-word ordering is not independently proven. Safe statement:

> Both fields carry a duplicated packed dimension value matching the embedded DDS dimensions for 8/8 bound T records.

## 2.2 Reciprocal dimensions

Wrapper:

```text
+0x48 f32
+0x4C f32
```

Observed:

```text
256 texture -> 0.00390625 = 1/256
128 texture -> 0.00781250 = 1/128
```

Both fields equal the reciprocal physical dimension in all eight square samples.

With non-square samples still absent, axis ordering remains unproven.

## 2.3 DDS extents

Wrapper:

```text
+0x38 u32
+0x64 u32
```

For 8/8:

```text
+0x64 == complete embedded DDS extent from "DDS " to end of T member
+0x38 == embedded DDS extent - 0x80-byte DDS header
```

Examples:

```text
T25:
  DDS extent       = 87536
  +0x64            = 87536
  compressed bytes = 87408
  +0x38            = 87408

T26 etc.:
  DDS extent       = 22000
  +0x64            = 22000
  compressed bytes = 21872
  +0x38            = 21872
```

## 2.4 Width-times-four field

Wrapper `+0x18` is:

```text
T25 256-wide -> 1024
Txx 128-wide ->  512
```

So:

```text
+0x18 == physical_width * 4
```

for 8/8 bound T records.

The high-level runtime meaning (for example decompressed row bytes versus another buffer pitch) is not promoted.

---

# 3. A coordinates are not raw physical texture pixels

There are 54 non-zero A grid entries across the 11 A records.

A grid coordinates use the same 0..256 domain even when the selected T texture is physically only 128x128.

Example:

```text
A32 -> T26 (128x128 DDS)
A grid rectangles include x/w values such as 64
```

A raw grid value 64 cannot be interpreted as one universal physical-pixel coordinate system shared by both 128 and 256 textures.

The normalized scaling:

```text
physical_pixel_x = grid_x * texture_width  / 256
physical_pixel_y = grid_y * texture_height / 256
physical_pixel_w = grid_w * texture_width  / 256
physical_pixel_h = grid_h * texture_height / 256
```

produces exact integral physical pixel coordinates for **54 / 54** non-zero A entries in the bound corpus.

Examples:

```text
T25 = 256x256
A grid width 64 -> 64 physical pixels

T26 = 128x128
A grid width 64 -> 32 physical pixels

T9 = 128x128
A grid width 32 -> 16 physical pixels
```

Safe promotion:

> A uses a texture-dimension-independent 0..256 coordinate grid. Mapping that grid through the selected T physical dimensions produces exact pixel rectangles for every bound entry.

Evidence status:

- independent 0..256 logical grid: `CORPUS_CONFIRMED`;
- exact integral scaling to bound DDS pixels: `CORPUS_CONFIRMED`;
- interpretation as normalized UV/atlas rectangles: `SEMANTIC_CANDIDATE` pending executable consumer.

---

# 4. A entry flag correlation

Across 11 A records:

```text
9 records: first active entry raw_flags = 1, all following active entries = 0
2 records: all active entry raw_flags = 0
```

The two all-zero-flag records are exactly the two bound records with:

```text
A header +0x03 == 2
```

All other observed `+0x03` values (`1`, `3`, `6`) have a first-entry flag of `1`.

This is a real corpus correlation, but no runtime meaning is assigned to either `+0x03` or `raw_flags` yet.

Status: `CORPUS_CONFIRMED correlation / PRESERVED_UNDECODED semantics`.

---

# 5. G/V high-byte modifier census correction

The earlier machine receipt listed only a partial set of G/V selector high bytes. A complete re-census gives:

```text
G high-byte values:
0    x2
48   x3
76   x1
202  x2
252  x2
255  x2

V high-byte values:
0    x17
1    x3
17   x1
82   x1
206  x14
208  x1
238  x4
255  x9
```

Combined observed set:

```text
0, 1, 17, 48, 76, 82, 202, 206, 208, 238, 252, 255
```

The low byte remains a fully validated target-kind selector. The high byte remains raw and must not be assigned a flag/mode meaning yet.

---

# 6. G/V does not cover every P/E record

The 62 G/V outgoing edges reach:

```text
18 / 34 unique P records
15 / 45 unique E records
```

Therefore 16 P records and 30 E records have no inbound G/V edge in the bound graph.

This rejects an overly strong model in which every effect invocation must begin at G/V.

Possible explanations still requiring executable evidence:

1. gameplay can request P/E records directly;
2. unresolved fields in other effect families reference those P/E records;
3. some records are dormant/unused alternatives in the retail pack;
4. there are additional root-selection tables outside the bound effect manifest graph.

Safe conclusion:

> G/V is a routing layer for a subset of effect nodes, not proven to be the universal root layer for every effect resource.

---

# 7. Next executable targets

1. locate the runtime consumer of the G/V packed selector and recover high-byte semantics;
2. locate T wrapper materialization and bind the dimension/extent fields to runtime texture descriptors;
3. locate the A 10-byte entry consumer and verify the 0..256-to-UV conversion;
4. recover the A header `+0x02/+0x03` mode switch and entry `raw_flags` behavior;
5. identify how gameplay chooses a root P/E/G/V logical ID;
6. determine whether P/E records with no G/V inbound edge are direct roots, externally referenced, or unused.

## Hard non-claims

This pass does not promote A to an original historical "atlas" format name, does not assign semantics to G/V high-byte modifiers, and does not establish writer authority.