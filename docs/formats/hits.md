# HITS Collision Resource

**Reconciled:** 2026-09-04  
**Status:** CURRENT CLEAN-C++ STRUCTURAL AUTHORITY / EXE-CONFIRMED INTEGER GRID ABI / CORPUS-VERIFIED SPATIAL MEMBERSHIP  
**Supersedes:** the obsolete `HITS$`, universal `0x18060001` marker-scan interpretation, and the corrected `+0x20 vec3f cell_size` model.

HITS is a DMC3 spatial collision resource. The canonical parser models a bounded header, a three-dimensional integer-cell grid, per-cell triangle references and fixed-size triangle/plane records.

## Canonical magic

```text
HITS
```

The first **four bytes** are the magic. Historical `HITS$` was a false five-byte interpretation: in observed resources the following byte belongs to the little-endian `end_offset` at `+0x04`.

## Structural envelope

```text
header size          = 0x44
triangle stride      = 0x38
relative-offset base = 0x08
```

Canonical header:

```text
+0x00 char[4]  magic = HITS
+0x04 u32      end_offset
+0x08 vec3f    bounds_min
+0x14 vec3f    bounds_max
+0x20 u32      cell_extent_x
+0x24 u32      cell_extent_y
+0x28 u32      cell_extent_z
+0x2C u32      grid_count_x
+0x30 u32      grid_count_y
+0x34 u32      grid_count_z
+0x38 u32      triangle_count
+0x3C u32      spatial_table_relative_offset
+0x40 u32      triangle_array_relative_offset
```

### Correction: `+0x20` is not `vec3f`

The previous clean-C++ model treated `+0x20..+0x2B` as three floats. A repeat reverse pass disproved that model.

Across 16 stock HITS payloads these fields are positive little-endian integers such as `250`, `300`, `500` and `600`. The canonical executable independently confirms DWORD usage: world-to-grid code reads these fields as integers and performs integer division; broadphase code converts them to float only when constructing cell AABBs.

The stock corpus satisfies exactly on every axis:

```text
bounds_max = bounds_min + cell_extent * grid_count
```

## Spatial grid

Cell count:

```text
cell_count = grid_count_x * grid_count_y * grid_count_z
```

Flattening order is:

```text
flat = ((x * grid_count_y) + y) * grid_count_z + z
```

This order is both EXE-confirmed and the only axis ordering that reproduces all 40,789 observed stock cell-to-triangle references in the repeat corpus.

### Spatial table

The spatial table begins at:

```text
file_base + 8 + spatial_table_relative_offset
```

It starts with `cell_count` signed 32-bit relative list pointers. Each pointer resolves relative to `file_base + 8`.

Each cell list contains zero or more triangle byte offsets followed by signed `-1`.

Observed stock layout also satisfies:

```text
spatial_size = cell_count * 4
             + (total_triangle_references + cell_count) * 4
```

and cell lists are serialized contiguously after the pointer table.

## Cell geometry

For cell coordinate `(x,y,z)`:

```text
cell_min.x = bounds_min.x + cell_extent_x * x
cell_min.y = bounds_min.y + cell_extent_y * y
cell_min.z = bounds_min.z + cell_extent_z * z

cell_max = cell_min + cell_extent
```

The executable converts the serialized integer full extents to float and multiplies them by `0.5f` when constructing half-extents/centres for collision tests.

## Stock grid fitting rule

The 16-file repeat corpus reproduces the stock headers with:

```text
bounds_min_axis = floor(global_vertex_min_axis)

grid_count_axis = max(
    1,
    ceil((global_vertex_max_axis - bounds_min_axis) / cell_extent_axis)
)

bounds_max_axis = bounds_min_axis + cell_extent_axis * grid_count_axis
```

This is now the canonical clean-writer fit-grid rule. Custom authoring padding is applied before flooring the minimum and before deriving the required count.

## Triangle record

Each triangle record is `0x38` bytes:

```text
+0x00 u32    raw_flags
+0x04 vec3f  point_a
+0x10 vec3f  point_b
+0x1C vec3f  point_c
+0x28 vec3f  normal
+0x34 f32    plane_d
```

For all 4,420 triangles in the repeat corpus:

- no degenerate triangle was observed;
- stored normals are unit-length within small float error;
- stored normal orientation agrees with `normalize(cross(B-A, C-A))`;
- vertices satisfy `dot(normal, point) + plane_d = 0` within small float residual.

Exact original authoring precision/operation order is **not** yet claimed bit-for-bit because recomputation does not reproduce every stored normal/plane-D bit pattern.

## Exact cell-membership rule

The repeat corpus compared every serialized triangle membership against triangle-vs-cell-AABB Separating Axis Theorem testing.

Result:

```text
4,420 / 4,420 triangles exact
40,789 serialized cell references covered
0 membership mismatches
```

Therefore the current file-level spatial rebuild rule is:

1. derive a boundary-inclusive candidate cell range;
2. test each candidate cell's **closed AABB** against the triangle with triangle-box SAT;
3. emit the triangle byte offset into every intersecting cell list.

A critical boundary rule follows: if a triangle lies exactly on an internal cell plane, both adjacent closed cells may contain the triangle. Candidate-range construction must therefore include the preceding cell before SAT filtering. The earlier writer omitted this case for 192 of 4,420 stock triangles.

## Triangle flags

`0x18060001` is an observed raw flag value, **not** a record marker.

The repeat corpus contains 24 distinct 32-bit flag values. The field remains losslessly preserved as `raw_flags`; only independently proven query masks may be named semantically. Corpus frequency alone must not be promoted to material/surface/gameplay names.

## File termination/alignment

Across the repeat corpus:

```text
end_offset = triangle_base + triangle_count * 0x38
file_size  = align16(end_offset)
```

Bytes between `end_offset` and physical file end were zero in all tested stock payloads.

## Purpose

**Confirmed/bounded purpose:** stage/spatial collision geometry plus an acceleration grid mapping world cells to triangle planes.

This is structural collision data, not a flat anonymous record table.

## Product boundary

Current clean C++ support includes:

- four-byte magic recognition;
- corrected integer cell-extent header parsing;
- EXE-compatible world-to-grid mapping;
- spatial pointer/list decoding;
- triangle/plane decoding;
- Binary Inspector exposure;
- topology-preserving edits;
- fit-grid reconstruction;
- boundary-inclusive triangle-box SAT spatial rebuild;
- deterministic serialization and parser round-trip checks.

The stock file-level writer algorithm is corpus-verified. This still does **not** prove that every topology-changing custom output is accepted by the original game. Controlled in-game validation remains a separate gate.

## Architecture rule

HITS parsing belongs in the shared format/GDSpaces path. Stage Ops, ModViz and editors must not create private HITS parsers or infer HITS from `.ukn` alone.

A misleading filename such as `*.ukn` may contain HITS; validated bytes/structure outrank extension.

## Related authority

- `include/dmc_rengine/formats/hits.hpp`
- `src/formats/hits.cpp`
- `include/dmc_rengine/hits/runtime.hpp`
- `src/hits/writer.cpp`
- `src/formats/hits_binary.cpp`
- `docs/research/hits-repeat-reverse-2026-09-04.md`
- `docs/research/hits-canonical-reverse-through-pass10.md`
- `data/reverse/dmc3-hits-repeat-20260904.json`
- `src/integration/format_registry.cpp`
- `src/gdspaces/classifier.cpp`
