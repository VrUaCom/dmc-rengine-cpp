# HITS Collision Resource

**Reconciled:** 2026-08-26  
**Status:** CURRENT CLEAN-C++ STRUCTURAL AUTHORITY / COLLISION PURPOSE CONFIRMED  
**Supersedes:** the obsolete `HITS$` + universal `0x18060001` marker-scanner interpretation.

HITS is a DMC3 spatial collision resource. The current clean C++ parser models a bounded header, a three-dimensional spatial grid, per-cell triangle references and fixed-size triangle/plane records.

## Canonical magic

```text
HITS
```

The current parser and GDSpaces classifier use the first **four bytes** `HITS`.

`HITS$` is not the current canonical magic. Historical documentation that required five-byte `HITS$` is superseded.

## Structural envelope

Constants in the current C++ format module:

```text
header size     = 0x44
triangle stride = 0x38
relative-offset base = 0x08
```

Header fields currently parsed:

```text
+0x00 char[4]  magic = HITS
+0x04 u32      end_offset
+0x08 vec3f    bounds_min
+0x14 vec3f    bounds_max
+0x20 vec3f    cell_size
+0x2C u32      grid_count_x
+0x30 u32      grid_count_y
+0x34 u32      grid_count_z
+0x38 u32      triangle_count
+0x3C u32      spatial_table_relative_offset
+0x40 u32      triangle_array_relative_offset
```

The parser validates arithmetic/ranges before slicing the spatial table or triangle array.

## Spatial grid

The resource partitions the collision space into a 3-D grid derived from:

```text
bounds_min / bounds_max
cell_size
(grid_count_x, grid_count_y, grid_count_z)
```

The spatial table provides per-cell references into lists of triangle byte offsets. Lists terminate with `-1`.

This is structural collision acceleration/index data, not merely a flat list of anonymous 56-byte records.

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

The clean implementation can evaluate the plane equation against triangle vertices as a structural/diagnostic check.

### Flag boundary

`0x18060001` has been observed as a raw flag value in historical research. It is **not** a universal record marker and must not be used to discover every HITS triangle by byte scanning.

Exact bit-level semantics of `raw_flags` remain evidence-gated.

## Purpose

**Confirmed/bounded purpose:** stage/spatial collision geometry plus an acceleration grid mapping world cells to triangle planes.

This is the collision family exposed to Stage/inspection tooling. Stronger semantics such as material, collision class, traversal policy or gameplay response must be attached only after direct evidence for the relevant flag/consumer path.

## Product boundary

Current clean C++ support is structural:

- exact four-byte magic recognition;
- bounded header parsing;
- grid dimension/range validation;
- spatial-cell list decoding;
- triangle/plane decoding;
- structured diagnostics;
- integration as a collision resource family.

A product working-copy/write policy does not by itself prove topology-changing output is accepted by the original game. Original-game writer/consumer acceptance remains a separate gate.

## Architecture rule

HITS parsing belongs in the shared format/GDSpaces path. Stage Ops, ModViz and other consumers must not create private collision parsers or infer HITS from `.ukn` alone.

A misleading filename such as `*.ukn` may still contain HITS; validated bytes/structure outrank the extension.

## Related authority

- `include/dmc_rengine/formats/hits.hpp`
- `src/formats/hits.cpp`
- `src/integration/format_registry.cpp`
- `src/gdspaces/classifier.cpp`
- [DMC3 HD format and purpose catalog](dmc3-hd-format-catalog.md)
- `docs/gdspaces/l3-residual-format-pass-2026-08-26.md`
