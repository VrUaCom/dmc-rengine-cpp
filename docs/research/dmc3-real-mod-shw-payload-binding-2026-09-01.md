# DMC3 real MOD / SHW payload binding — 2026-09-01

**Status:** CANONICAL RESEARCH ADDENDUM  
**Scope:** one hash-bound real `MOD` payload and one hash-bound real `SHW` payload,
cross-checked against the canonical DMC3 HD executable.

## Artifacts

| Family | Source name | Size | SHA-256 |
|---|---:|---:|---|
| `SHW` | `slot_0008.shw` | 9,488 | `cb392ef2e874addb887d32bc44d409299a32a83a4845afcbdef31698283f2e7e` |
| `MOD` | `slot_0001 (2).mod` | 216,544 | `e219e89285604cb6d800b0afdd3bec6684a6b00cd1862d464a669d2861ff3c89` |

The payloads remain external evidence inputs. They are not committed to the
repository.

## 1. SHW correction

The real payload rejects the earlier statement that SHW topology only references a
spatial vertex pool external to the payload.

This artifact contains the full geometry needed by its 17 shadow hull records:

| Quantity | Value |
|---|---:|
| hull records | 17 |
| vertices | 152 |
| triangles | 236 |

Header and record grammar:

```text
SHW header
  +0x00  "SHW "
  +0x04  f32 version = 0.5
  +0x10  u8 hull_count = 17
  +0x20  hull records, stride 0x40

hull record
  +0x00  u16 vertex_count
  +0x02  u16 triangle_count
  +0x10  relptr triangle_count * { u32 a, b, c, zero }
  +0x18  relptr triangle_count * { u16 n0, n1, n2, zero }
  +0x20  relptr vertex_count * float4 position
  +0x28  relptr vertex_count * u8 transform selector, then zero padding to 0x10
```

The four pointers are exactly the four qwords relocated by the canonical normalizer
at `0x1403204C0`.

### 1.1 Triangle and adjacency proof

Across all 236 triangle records:

- the fourth u32 lane is zero;
- each of the first three indices is below the hull's `vertex_count`;
- the referenced `float4` positions are finite and have `w = 1.0`.

Across all 236 parallel u16 records:

- the fourth u16 lane is zero;
- each of the first three values is below `triangle_count`;
- every value points to a triangle sharing exactly one edge with the current
  triangle;
- each triplet equals the complete set of the current triangle's three edge
  neighbours.

All 17 hulls also satisfy:

```text
triangle_count = 2 * vertex_count - 4
```

Together with the exact three-edge adjacency, this is direct structural evidence of
closed triangulated shadow hulls.

### 1.2 Correct SHW classification

The safe classification is now:

> **SHW is a self-contained shadow-hull mesh resource containing positions,
> triangle topology and exact triangle adjacency.**

It remains distinct from a MOD/SCM textured model document: there are no recovered
normal, UV, material or texture streams in this SHW layout, and the embedded
`DMC3_SHW.hlsl` consumes only `POSITION`.

The `u8` stream at record `+0x28` is now EXE-bound. The runtime builder
`0x14031FD30` maps the raw vertex pointer to runtime hull `+0x38` and this byte
stream to runtime hull `+0x40`. At `0x1403202F0`, each byte is multiplied by
`0x40` and added to a transform-matrix base. `0x140030A70` then multiplies the raw
`float4` vertex by the selected 64-byte matrix and stores the transformed vertex.

Therefore the field is an **EXE-confirmed per-vertex transform-matrix selector**.
The selected matrix palette's ownership and exact cross-resource model/bone mapping
remain open.

The runtime code near `0x1403204F0` receives the vertex array separately from its
runtime record. The payload proves that this separation is a runtime-object layout
detail, not proof that the vertices live outside the SHW file.

## 2. Real MOD stream binding

The MOD payload confirms the related document shell recovered from
`0x1402FE3B0`:

```text
MOD header
  +0x00  "MOD "
  +0x04  f32 version ~= 1.01
  +0x10  u8 outer_count = 17
  +0x20  relptr document-side region = 0x2CE50
  +0x40  outer records, stride 0x40

outer record
  +0x00  u8 inner_count
  +0x02  u16 aggregate element_count
  +0x08  relptr inner records

inner record, stride 0x50
  +0x00  u16 element_count
  +0x10  relptr count * float3
  +0x18  relptr count * float3
  +0x20  relptr count * int16x2
  +0x28  relptr count * u8x4
  +0x30  relptr count * u16
  +0x38  zero in this artifact
  +0x40  inner-relative generated-topology buffer
  +0x48  generated-topology count, initially zero
```

The file contains 17 outer records and 18 inner records. One outer record contains
two inners, which proves an important packing detail: stream data is grouped by
field across all inner records in an outer group. It is not always stored as five
complete per-inner stream blocks.

For every outer record, `outer +0x02` equals the sum of all inner `+0x00` counts.
All five source-stream spans match `count * stride`, rounded to `0x10` alignment.
The final source stream ends exactly at the header pointer `0x2CE50`.

### 2.1 Stream semantics and confidence

| Inner pointer | Data-confirmed shape | Semantic binding | Status |
|---|---|---|---|
| `+0x10` | `float3[count]` | position | `HIGH_CONFIDENCE`, shader-correlated |
| `+0x18` | `float3[count]` | normal | `HIGH_CONFIDENCE`; every vector has unit length within `4.9e-8` |
| `+0x20` | `int16x2[count]` | fixed-point UV | `HIGH_CONFIDENCE`; MOD shader converts texture coordinates by `1/4096` |
| `+0x28` | `u8x4[count]` | blend/matrix row indices | `HIGH_CONFIDENCE`; MOD shader consumes `uint4 BLENDINDICES` and the observed values are multiples of four |
| `+0x30` | `u16[count]` | vertex flags plus strip/control bit | high bit behavior `EXE_CONFIRMED`; lower packed-weight semantics shader-correlated |

The artifact has 5,316 stream elements. The `+0x30` stream contains 1,636 values
with bit `0x8000` set. The canonical normalizer uses that bit as a strip/control
marker, clears it in place, emits a triangle-index sequence into `+0x40`, and writes
the generated count to `+0x48`.

This is strong enough for a bounded read-only MOD stream parser. It is not evidence
for a safe writer or universal revision coverage.

## 3. What changes

| Previous statement | New status |
|---|---|
| SHW references a spatial pool external to the payload | **REJECTED** by the real payload |
| SHW is not proven self-contained geometry | **SUPERSEDED** |
| SHW is a MOD/SCM-style textured model | still **REJECTED** |
| SHW contains self-contained closed shadow hulls | **DATA_CONFIRMED + EXE corroborated** |
| MOD stream pointer grammar was only handler-derived | now **real-payload bound** |
| MOD exact safe writer is known | still **NOT PROVEN** |

## 4. Remaining boundary

- bind the EXE-confirmed SHW transform selectors to the owning matrix palette and
  model/bone resource path;
- acquire additional MOD/SHW variants and prove the grammar is revision-stable;
- implement guarded read-only parsing before any writer work;
- require rebuild, reopen and original-game consumption evidence before promoting
  mutation support.
