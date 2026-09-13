# HITS repeat reverse — 2026-09-04

**Branch:** `hits`  
**Canonical executable:** `dmc3.exe`  
**SHA-256:** `e454272ed0fb0247fcbcf300e5d55d7a3e96d50b89b9ffaff81bb978dcbdd082`  
**Purpose:** independent second-pass verification of the serialized HITS format, spatial index, triangle records and clean writer assumptions.

## Executive result

The repeat pass confirmed most of the existing HITS architecture but found one material serialized-type error and one material writer edge-case error.

### Corrected

1. Header `+0x20/+0x24/+0x28` is **not** `vec3f cell_size`.
   It is three little-endian `u32` integer cell extents.
2. The previous writer encoded those fields as floats and therefore did not emit the canonical serialized ABI.
3. The previous triangle candidate-cell range could omit the preceding cell when a triangle lay exactly on an internal cell boundary. This affected 192 / 4,420 triangles in the stock corpus before SAT filtering could run.

### Reconfirmed

- four-byte `HITS` magic;
- `0x44` header;
- `+8` relative-offset base;
- `0x38` triangle stride;
- `-1` cell-list terminator;
- x/y/z flatten order `((x*Y)+y)*Z+z`;
- triangle point/normal/plane layout;
- spatial cell membership based on triangle-vs-cell-box SAT;
- 16-byte physical file alignment with zero tail padding in the tested stock corpus;
- `0x18060001` is a raw flag value, not a universal record marker.

---

## 1. Corpus

Repeat pass used 16 HITS payload instances from the available stage extraction corpus, representing 14 unique binary payload hashes because `st111` and `st434` contain identical HITS pairs.

Totals:

```text
payload instances              16
unique payload SHA-256 values  14
triangles                       4,420
cell -> triangle references    40,789
unique raw flag values              24
```

### Corpus inventory

| container | payload | size | end | cell extent | grid | triangles | refs | SHA-256 |
|---|---|---:|---:|---|---|---:|---:|---|
| st000.zip | st000_003.ukn | 11056 | 11056 | 300,300,300 | 6x4x5 | 127 | 729 | a3458863ac9ed0e66e2feebb4a884e00c96aeb6259c09f2c5013f46079ff34c6 |
| st000.zip | st000_006.ukn | 2592 | 2580 | 300,300,300 | 6x3x5 | 20 | 168 | d6f8f2cb2d22e0d7eeb8d576cb76c8ac0a03a8d06531381a816e92fbcdb87d9b |
| st001.zip | st001_003.ukn | 26160 | 26148 | 500,500,500 | 9x6x7 | 294 | 1648 | 80b4b64318d84748c34a9e4f08065e35998e2e14e3766affb653e0f26adab77a |
| st001.zip | st001_006.ukn | 1648 | 1644 | 500,500,500 | 4x2x2 | 20 | 82 | 0f3d9952b61e3d1346dc9910c9c401fc0214d0d6fb2593791e0c000cc4599f64 |
| st111.zip | st111_003.ukn | 155440 | 155428 | 300,300,300 | 27x20x20 | 736 | 6936 | 83346491bb17874b60ac75c8a8c3fde051de5e991059df445184dfd5fa735e9b |
| st111.zip | st111_006.ukn | 52928 | 52924 | 300,300,300 | 23x13x17 | 60 | 2208 | 9b10aca8932ee8f4201377f028b5968f9de566d5d9900593c7fa9e960c7c2c37 |
| st143.zip | st143_003.ukn | 74544 | 74540 | 600,600,600 | 11x15x11 | 762 | 4320 | 7276793a1a3fa0aebfc681392aff8ea04689a7400e1aedb32b576ce1c30bf4cf |
| st225.zip | st225_003.ukn | 137072 | 137072 | 300,300,300 | 20x21x27 | 485 | 4781 | 8a5e01357fa822083b185105ac3442ad01bfb44cd63240d1d087a3f700810b98 |
| st434.zip | st434_003.ukn | 155440 | 155428 | 300,300,300 | 27x20x20 | 736 | 6936 | 83346491bb17874b60ac75c8a8c3fde051de5e991059df445184dfd5fa735e9b |
| st434.zip | st434_006.ukn | 52928 | 52924 | 300,300,300 | 23x13x17 | 60 | 2208 | 9b10aca8932ee8f4201377f028b5968f9de566d5d9900593c7fa9e960c7c2c37 |
| st449.zip | st449_003.ukn | 82128 | 82124 | 300,300,300 | 18x26x18 | 48 | 2994 | 59174de9ffcf92290e1bf622fce41e385bce46d4c1244bbbaf89ed488e8b0c7e |
| st449.zip | st449_006.ukn | 800 | 800 | 300,300,300 | 6x1x11 | 1 | 37 | 037bde95adf5bb5755bd1870bd2ecd1d23c13a4df969a390bf0f4430584da0a9 |
| st600.zip | st600_003.ukn | 58192 | 58184 | 250,250,250 | 12x6x11 | 642 | 3957 | 8f4b4ad2aff67d0ed2de9b6da98adf589199a2eb5268a2b7659cea555ab7cce0 |
| st600.zip | st600_006.ukn | 2320 | 2316 | 300,300,300 | 3x2x4 | 28 | 122 | 2bab883a541485af410ab1d9a3c8021c112f948f4e660034255a06de44d07f83 |
| st600.zip | st601_003.ukn | 36144 | 36132 | 300,300,300 | 14x9x19 | 129 | 2422 | d3f02d5c195dc9f6d3297d5254b91c3d37bce8315cae7875b8d0dfebd14c1ff0 |
| st900.zip | st900_003.ukn | 23152 | 23144 | 600,600,600 | 12x5x6 | 272 | 1241 | 74aaf33f13193992ed229b77415f7a86c11fea18f091bdfd3e8a7a6262f2024e |

---

## 2. st001 independent byte-level pass

`st001.pac` contains two HITS payloads corresponding to the extracted member-3/member-6 collision resources.

### st001_003.ukn

```text
SHA-256 = 80b4b64318d84748c34a9e4f08065e35998e2e14e3766affb653e0f26adab77a
size    = 26160
end     = 26148

bounds_min = (485, -437, 700)
bounds_max = (4985, 2563, 4200)

+0x20 = 500 u32
+0x24 = 500 u32
+0x28 = 500 u32

grid = 9 x 6 x 7
triangles = 294
spatial_relative = 0x3C
triangle_relative = 0x25CC
triangle_base = 0x25D4
```

Checks:

```text
4985 - 485   = 4500 = 500 * 9
2563 - (-437)= 3000 = 500 * 6
4200 - 700   = 3500 = 500 * 7
```

### st001_006.ukn

```text
SHA-256 = 0f3d9952b61e3d1346dc9910c9c401fc0214d0d6fb2593791e0c000cc4599f64
size    = 1648
end     = 1644

bounds_min = (1350, -91, 2450)
bounds_max = (3350, 909, 3450)

+0x20 = 500 u32
+0x24 = 500 u32
+0x28 = 500 u32

grid = 4 x 2 x 2
triangles = 20
spatial_relative = 0x3C
triangle_relative = 0x204
triangle_base = 0x20C
```

Reading `F4 01 00 00` as float yields approximately `7.0e-43`; reading it as little-endian DWORD yields `500`. The integer interpretation is independently confirmed by executable code.

---

## 3. Corrected header ABI

```text
+0x00 char[4] HITS
+0x04 u32     end_offset
+0x08 vec3f   bounds_min
+0x14 vec3f   bounds_max
+0x20 u32     cell_extent_x
+0x24 u32     cell_extent_y
+0x28 u32     cell_extent_z
+0x2C u32     grid_count_x
+0x30 u32     grid_count_y
+0x34 u32     grid_count_z
+0x38 u32     triangle_count
+0x3C u32     spatial_table_relative_offset
+0x40 u32     triangle_array_relative_offset
```

### EXE CONFIRMED — integer cell extents

Canonical world-to-grid function `0x1402D2F50`:

- subtracts `bounds_min` from the world coordinate;
- converts the float delta to integer (`cvttss2si` path);
- divides by DWORD fields at header `+0x20/+0x24/+0x28`;
- clamps to `0 .. grid_count-1` using `+0x2C/+0x30/+0x34`;
- flattens the coordinate.

This directly rejects the previous float cell-size ABI.

### EXE CONFIRMED — AABB half extents

Broadphase function `0x1402D2A10` reads the same DWORD extents, converts them to float and multiplies by `0.5f` (`0x3F000000`) when constructing cell half-extents.

Function `0x1402D2E80` is consistent with cell-centre construction from:

```text
bounds_min + extent * coordinate + extent * 0.5
```

---

## 4. Relative offsets and spatial lists

### EXE CONFIRMED

Cell-list resolver `0x1402D29F0` resolves:

```text
spatial_base = raw_file + header[0x3C] + 8
list_offset  = raw_file + sign_extend(spatial[cell_index]) + 8
```

Runtime initialization `0x1402D3060` also resolves both spatial and triangle bases using the same `+8` origin.

Record resolver `0x1402D3050` resolves a triangle as:

```text
triangle_base + signed_triangle_byte_offset
```

### Stock corpus layout

All 16 instances use `spatial_table_relative_offset = 0x3C`, making the physical pointer table begin at `0x44`.

For every tested file:

```text
pointer_table_bytes = cell_count * 4
list_bytes          = (total_reference_count + cell_count) * 4
triangle_base       = 0x44 + pointer_table_bytes + list_bytes
```

Every list terminates with signed `-1`.

---

## 5. Flatten order

### EXE CONFIRMED

```text
flat = ((x * grid_count_y) + y) * grid_count_z + z
```

### Corpus cross-check

All 40,789 references were tested against the six possible x/y/z axis-order permutations. Only the canonical x->y->z slow-to-fast order reproduces the full corpus.

---

## 6. Stock grid fitting

Across all 16 instances:

```text
bounds_min_axis = floor(global minimum vertex coordinate)

grid_count_axis = max(
    1,
    ceil((global maximum vertex coordinate - bounds_min_axis)
         / cell_extent_axis)
)

bounds_max_axis = bounds_min_axis + cell_extent_axis * grid_count_axis
```

This reproduces every tested stock header exactly.

**Evidence boundary:** this is an exact stock-file corpus reconstruction. The original Capcom authoring/writer function has not yet been identified, so original writer provenance remains `RESEARCH REQUIRED` even though the clean writer can reproduce the observed rule.

---

## 7. Triangle records

Layout remains confirmed:

```text
+0x00 u32    raw_flags
+0x04 vec3f  A
+0x10 vec3f  B
+0x1C vec3f  C
+0x28 vec3f  normal
+0x34 f32    plane_d
```

Corpus geometry statistics:

```text
triangles                            4,420
degenerate triangles                    0
max |length(normal)-1|       ~4.30e-08
minimum dot(stored, cross-dir) ~0.999999959
max stored plane residual      ~4.76e-04
```

Stored normal orientation agrees with:

```text
normalize(cross(B-A, C-A))
```

for the entire corpus.

However, exact authoring arithmetic is not bit-closed:

```text
float32-style recompute exact normal bits: 1784 / 4420
float32-style recompute exact plane-D bits: 2572 / 4420
double-normal then f32 exact normal bits:   3490 / 4420
double-derived plane-D exact bits:          3407 / 4420
```

Therefore exact original normalization/precision/order remains `RESEARCH REQUIRED`.

---

## 8. Exact spatial membership

A triangle-vs-AABB SAT implementation was compared against the serialized cell lists across the complete repeat corpus.

Result:

```text
triangle membership sets compared  4,420
exact membership sets               4,420
mismatches                               0
serialized references              40,789
```

This establishes the clean file-level assignment rule for the tested corpus.

### Boundary correction

The old clean writer first reduced each triangle to the cells containing its min/max points. That candidate range is insufficient for closed boxes when a triangle lies exactly on an internal grid boundary.

Observed impact:

```text
triangles whose serialized membership includes a cell
outside the old min/max candidate range: 192 / 4,420
```

Correct candidate-axis range before SAT:

```text
lower = ceil((triangle_min - bounds_min) / extent) - 1
upper = floor((triangle_max - bounds_min) / extent)

clamp both to [0, grid_count-1]
```

SAT then determines the exact membership.

### EXE support

`0x1402D2670` is a triangle-vs-AABB SAT-style collision routine in the broadphase consumer path. This is consistent with the corpus-exact SAT reconstruction. The repeat pass does not claim that this runtime function is literally the offline authoring writer.

---

## 9. Raw flags

The 4,420-triangle corpus contains 24 raw values:

```text
0x00000001  1723
0x10040001   358
0x18540001   337
0x10000002   292
0x18260001   248
0x00040001   218
0x18060001   194
0x00000000   188
0x182E0001   151
0x100A0002   150
0x00000002   118
0x00000009    83
0x18000001    76
0x100C0002    60
0x18040001    58
0x10000001    49
0x0000000C    37
0x182E0000    32
0x00000003    25
0x184C0000     8
0x0000000A     6
0x0000000B     4
0x00040003     3
0x00000004     2
```

No new gameplay/material names are promoted from frequency. Exact bit meanings beyond already proven query filters remain `RESEARCH REQUIRED`.

---

## 10. File end/alignment

For every tested stock payload:

```text
end_offset = triangle_base + triangle_count * 0x38
file_size  = align16(end_offset)
```

Tail bytes after `end_offset`, when present, are zero.

Observed tail sizes were `0`, `4`, `8` or `12` bytes.

---

## 11. Implementation corrections made by this pass

The `hits` branch is being corrected to match the repeat reverse:

- `Header::cell_size` now stores a `CellExtent {u32 x,y,z}` serialized ABI;
- parser reads DWORD extents;
- world-to-grid uses integer-extent semantics;
- Binary Inspector displays `u32_le[3]` extents;
- spatial comparison compares extents as integers;
- writer serializes DWORD extents;
- fit-grid floors the global minimum before count derivation;
- writer candidate-cell enumeration is boundary-inclusive before SAT;
- regression tests encode real integer extents and include an exact-boundary triangle case.

---

## 12. Current closure / open gates

### Closed strongly

- serialized header field types and offsets;
- spatial and triangle relative bases;
- grid dimensions and flatten formula;
- cell list structure/terminator/reference domain;
- triangle record layout;
- integer cell AABB construction;
- stock-corpus grid-fit formula;
- tested-corpus cell membership via boundary-inclusive SAT;
- physical end/alignment pattern.

### Still open

1. Complete semantic names for raw flags.
2. Exact original authoring arithmetic for normal and plane-D bit parity.
3. Original Capcom offline/authoring writer identity and provenance.
4. Controlled original-game acceptance of arbitrary topology-changing rebuilt HITS.
5. Whether unobserved HITS variants permit layout deviations not present in this 16-instance corpus.

The repeat pass therefore materially strengthens HITS, but does not promote topology-changing custom output to `GAME VERIFIED` or `DL8`.
