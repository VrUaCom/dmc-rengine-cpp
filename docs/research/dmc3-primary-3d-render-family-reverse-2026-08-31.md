# DMC3 primary 3D / render family reverse — 2026-08-31

**Status:** CANONICAL RESEARCH ADDENDUM  
**Target:** canonical unpacked DMC3 HD analysis executable  
**SHA-256:** `e454272ed0fb0247fcbcf300e5d55d7a3e96d50b89b9ffaff81bb978dcbdd082`

## 1. Question

This pass answers the narrower question:

> Which of `MOD / EFM / SCM / MRP / SHW` are actual mesh-bearing 3D resources, and which are render/shadow companions?

The answer must not collapse runtime recognition, geometry ownership, shader input,
post-load fixup and exact on-disk schema into one claim.

## 2. Current canonical classification

| Family | Runtime identity | 3D/model conclusion | Current boundary |
|---|---|---|---|
| `MOD` | EXE confirmed by registry + family-mask probes | **mesh-bearing actor/object model** | exact full schema/writer still partial |
| `EFM` | EXE confirmed by registry + family-mask probes | **mesh-bearing effect model** | exact mapping of every on-disk stream still partial |
| `SCM` | EXE confirmed by registry + family-mask probes | **mesh-bearing stage/scene model** | exact full schema/writer still partial |
| `MRP` | EXE confirmed by two byte-backed classifiers | **render/model-side companion; not proven as standalone mesh** | no normal generic fixup recovered; exact schema open |
| `SHW` | EXE confirmed by registry + family-mask probes; real payload bound | **self-contained shadow-hull mesh geometry** | positions, triangles, exact adjacency and per-vertex transform-matrix selectors confirmed; selected palette ownership remains open |

The previous shorthand `MOD + SCM = geometry; EFM/MRP/SHW = companions` is therefore
superseded. `EFM` belongs on the mesh-bearing side.

## 3. Runtime type systems remain separate

The evidence split from `dmc3-runtime-type-evidence-split-2026-08-31.md` applies:

```text
registry_content_probe @ 0x1402DB1F0
    MOD / EFM / SCM / MRP / SHW

container_dispatch @ 0x1401B9FA0
    MOD / EFM / SCM / SHW -> normal post-load handlers
    EFW / EFE -> recognized sentinel prefixes

family_mask_probe @ 0x1402FD650
    MOD  -> 0x10000000
    EFM  -> 0x20000000
    SCM  -> 0x30000000
    MRP  -> 0x40000000
    MCV  -> 0x50000000
    SHW  -> 0x60000000
```

This document uses those classifiers as identity evidence, not as a substitute for
geometry evidence.

## 4. MOD / EFM / SCM are one related model-document family

The original post-load normalizers are:

```text
MOD -> 0x1402FE3B0
EFM -> 0x1402F7A90
SCM -> 0x1403051B0
```

Direct disassembly shows a shared high-level document topology:

```text
resource header
  +0x10  outer/object count-like byte
  +0x20  base-relative pointer relocated in place

outer records from +0x40
  stride 0x40
  +0x00  inner/mesh count-like byte
  +0x08  inner-record array pointer

inner records
  stride 0x50
  multiple base-relative stream pointers
  format-specific topology/fixup work
```

This establishes a related original-runtime document family. It does **not** imply
that every field is identical across all three formats.

### 4.1 MOD

`0x1402FE3B0` relocates mesh-side pointers and rebuilds an index/topology stream.
Observed inner-record behavior includes:

```text
inner +0x10 -> base-relative data stream
inner +0x18 -> base-relative data stream
inner +0x20 -> base-relative data stream
inner +0x28 -> base-relative data stream
inner +0x30 -> base-relative u16 topology/index stream
inner +0x40 -> inner-record-relative generated/output stream
inner +0x48 -> generated/output count
```

The topology pass reads `u16` elements from `+0x30`, interprets bit `0x8000` as a
strip/control marker and clears that bit from the source values while generating a
second index sequence.

MOD additionally reads header byte `+0x11` and compares it with `1`; that field must
not be projected onto EFM or SCM merely because the outer shape is related.

### 4.2 EFM

`0x1402F7A90` is not a parameter-only effect-table fixup. It traverses the same
`0x40` outer / `0x50` inner document topology and relocates:

```text
inner +0x10
inner +0x18
inner +0x20
inner +0x28
inner +0x30
inner +0x38
```

It resolves `inner +0x40` relative to the inner record itself and uses the `+0x30`
`u16` stream plus the same `0x8000` control bit to generate triangle/index topology,
storing the resulting count at `inner +0x48`.

That alone is strong mesh-bearing evidence. The canonical executable adds an even
stronger independent proof through embedded DMC3 HLSL source.

## 5. EFM shader contract: direct EXE proof of an effect model

The canonical EXE embeds multiple effect-model vertex shader sources:

```text
DMC3_EFM.hlsl
DMC3_EFM_SP.hlsl
DMC3_EFM_STX.hlsl
DMC3_EFM_VA.hlsl
DMC3_EFM_VA_SP.hlsl
```

The source itself contains the explicit engine comment:

```text
// EFM models have extra vertex RGB that you modulate in...
```

The recovered `VS_IN` contract is:

```hlsl
struct VS_IN
{
    float3 position : POSITION;
    float3 normal   : NORMAL;
    float2 tex0     : TEXCOORD0;
    uint4  matIndex : BLENDINDICES;
    float  flags    : PSIZE;
    float4 rgba     : COLOR0;
};
```

The shader consumes `position`, `normal`, `tex0`, matrix/blend indices and per-vertex
RGBA. It also converts the texture coordinates from fixed-point PS2-style units and
uses `rgba` to modulate lit effect colour.

This promotes the safe purpose boundary to:

> **EFM is an effect-model / mesh-bearing model family with extra per-vertex colour data.**

Exact on-disk stream-to-semantic mapping is still not declared field-perfect until a
real EFM payload is bound against these runtime inputs.

## 6. MOD shader corroboration

The embedded `DMC3_MOD.hlsl` family exposes the closely related input contract:

```hlsl
struct VS_IN
{
    float3 position : POSITION;
    float3 normal   : NORMAL;
    float2 tex0     : TEXCOORD0;
    uint4  matIndex : BLENDINDICES;
    float  flags    : PSIZE;
};
```

The important structural comparison is therefore:

```text
MOD vertex input
    position + normal + UV + blend/matrix indices + flags

EFM vertex input
    MOD-like inputs + per-vertex RGBA
```

This independently agrees with the related MOD/EFM post-load layouts.


### 6.1 Real MOD payload binding

A hash-bound real MOD payload now closes the gap between the handler-derived pointer
shape and actual stream bytes:

```text
source          slot_0001 (2).mod
size            216544
SHA-256         e219e89285604cb6d800b0afdd3bec6684a6b00cd1862d464a669d2861ff3c89
outer records   17
inner records   18
stream elements 5316
```

For every outer record, the u16 at `outer +0x02` equals the sum of the inner
`+0x00` counts. The five pointer groups have exact aligned spans:

```text
+0x10  count * float3
+0x18  count * float3
+0x20  count * int16x2
+0x28  count * u8x4
+0x30  count * u16
```

One outer contains two inner records and proves that payload streams are field-major
within the outer group. The two float3 streams are position/normal-shaped; every
vector in the second stream is unit length within `4.9e-8`. The remaining widths
match the embedded MOD shader contract for fixed-point UV, `BLENDINDICES` and
`PSIZE` flags. Exact evidence and the confidence boundary are recorded in the
[real MOD / SHW payload binding](dmc3-real-mod-shw-payload-binding-2026-09-01.md).


## 7. Runtime construction/allocation path confirms the split

A higher-level factory/construction path at `0x140248140` calls the four-byte
family-mask classifier.

Recovered behavior:

```text
MOD / EFM -> shared branch, size argument 0x780
SCM       -> separate branch, size argument 0x580
other masks (MRP / MCV / SHW) -> no object from this factory path
```

Exact function window for this bounded factory evidence:

```text
VA          0x140248140
file offset 0x247540
size        0xA0
SHA-256     8ae56885624cbcbc89ece904fe8dc38ba3ea89291ac6575e0f0cf1c16a6f8079
```

This is a strong independent reason to group EFM with MOD as a model/render object
family rather than treating EFM as effect metadata only.

## 8. Runtime memory-footprint specialization

`0x1402FD8D0` independently calls the family-mask classifier and calculates runtime
memory requirements.

Its type-specialized branch is:

```text
MOD / EFM -> 0x1402FDB40 size/layout contribution
SCM       -> 0x1402FDD10 size/layout contribution
MRP/MCV/SHW -> no MOD/EFM/SCM-specific contribution in this function
```

Bounded window:

```text
VA          0x1402FD8D0
file offset 0x2FCCD0
size        0xF0
SHA-256     972aa71b8c6a33be636e727bae8747b9496017ed2de2bd42da51cea7844d3abe
```

`0x1402FDB40` repeatedly accounts for the `0x40` outer and `0x50` inner document
structures, while the SCM path has its own related size model.

A census of the obvious high-nibble family checks in the surrounding model-runtime
code likewise specializes `0x10000000`, `0x20000000` and `0x30000000`; no equivalent
MRP/MCV/SHW branch was recovered in that model-specific neighborhood. This is a
bounded negative result, not a claim that those families have no consumers anywhere.

## 9. SCM / stage model corroboration

SCM remains a confirmed stage/scene mesh family. The original `0x1403051B0` fixup is
related to MOD/EFM but format-specific, and external corpus tooling independently
decodes SCM positions, normals, UVs, faces and vertex colours.

The canonical executable also contains stage vertex-shader inputs with static-model
characteristics (`POSITION`, `NORMAL`, `TEXCOORD0`, `COLOR0`), consistent with the
known SCM vertex-colour path. Shader naming alone is not used as the SCM format
identity proof; it is corroboration of the already established stage-render model.

## 10. SHW is self-contained shadow-hull mesh geometry

The SHW post-load fixup remains format-specific:

```text
0x1403204C0
header +0x10 -> record count
records from +0x20, stride 0x40
four base-relative qword pointers relocated per record
```

A real payload now binds all four pointers:

```text
source        slot_0008.shw
size          9488
SHA-256       cb392ef2e874addb887d32bc44d409299a32a83a4845afcbdef31698283f2e7e
hull records  17
vertices      152
triangles     236
```

Each `0x40` record contains:

```text
+0x00  u16 vertex_count
+0x02  u16 triangle_count
+0x10  triangle_count * { u32 a, b, c, 0 }
+0x18  triangle_count * { u16 n0, n1, n2, 0 }
+0x20  vertex_count * float4 position
+0x28  vertex_count * u8 selector, padded to 0x10
```

Every triangle index is in vertex range. Every adjacency triplet is the exact set of
three triangles sharing the current triangle's edges. All vertex vectors are finite
and have `w = 1`. All 17 records satisfy
`triangle_count = 2 * vertex_count - 4`, consistent with their verified closed
triangulated topology.

The runtime path near `0x1403204F0` passes the vertex array separately from the
runtime record and `0x140320BB0` calculates an edge cross product. The real payload
shows that this is a runtime-object separation: the spatial vertex pool itself is
inside SHW. The earlier file-level “external vertex pool” statement is rejected.

The embedded `DMC3_SHW.hlsl` input remains minimal:

```hlsl
struct VS_IN
{
    float3 position : POSITION;
};
```

Safe conclusion:

> **SHW is a self-contained shadow-hull mesh resource containing positions,
> triangle topology and exact triangle adjacency. It is not a MOD/SCM-style
> textured model document.**

The per-vertex selector byte is also EXE-bound. The runtime builder
`0x14031FD30` maps raw vertices to runtime hull `+0x38` and selectors to
`+0x40`. At `0x1403202F0`, the selector is multiplied by `0x40` to choose a
64-byte transform matrix, and `0x140030A70` multiplies the raw `float4` vertex
by that matrix. The field is therefore an EXE-confirmed per-vertex
transform-matrix selector. Ownership of the selected matrix palette remains open.


## 11. MRP remains the major open member

MRP is real runtime identity twice over:

```text
registry probe: MRP -> type 3
family mask:    MRP<space> -> 0x40000000
```

But:

- registrar path has no immediate MOD/EFM/SCM/SHW-style handler for type 3;
- container dispatcher has no MRP handler branch;
- the `0x140248140` model/render construction path does not construct the
  MOD/EFM/SCM object type for MRP;
- the `0x1402FD8D0` model memory-footprint calculation does not take a dedicated MRP
  branch;
- no MRP-specific embedded shader filename/string was recovered in the current
  printable-string census.

Therefore the evidence supports **runtime model/render-side companion identity**, but
not standalone mesh ownership.

The next MRP gate is downstream ownership: locate where an object carrying
`family_mask == 0x40000000` is consumed, or acquire a real retail MRP payload and
bind its fields to that consumer.


### 11.1 Whole-`.text` census of direct `object + 0xE0` consumers

A later whole-`.text` pass scanned every direct dword load from `[base+0xE0]` and
retained only sites followed by a family discriminator: `AND 0xF0000000`,
`SHR 0x1C`, high-bit tests or direct family-mask comparisons.

The bounded result is **10 direct discriminator sites in 9 functions**. Every
recovered branch specializes only:

```text
MOD -> 0x10000000
EFM -> 0x20000000
SCM -> 0x30000000
```

No direct `object + 0xE0` downstream branch for `MRP / MCV / SHW` was recovered.
This strengthens the MRP negative boundary but does not prove that an indirect
consumer, copied mask or different object layout does not exist.

Exact VAs, 0x30-byte window hashes and the reproducible scan method are recorded in
the [family-mask object+0xE0 consumer census](dmc3-family-mask-object-e0-consumer-census-20260831.md)
and its machine-readable evidence packet.


## 12. Real-payload boundary

The evidence set now includes hash-bound standalone MOD and SHW payloads. This
closes the MOD stream-shape and SHW self-contained-topology gaps for the observed
artifacts.

Standalone EFM and MRP payloads remain absent from the current bound corpus.
Therefore:

```text
MOD / SHW
  EXE evidence + real-payload structural binding

EFM / MRP
  EXE evidence; exact real-payload field binding still open
```

One sample per family does not prove universal revision coverage, and no writer or
original-game mutation round trip is promoted by this pass.


## 13. Canonical viewer implication

A future DMC3 3D viewer should not be architected as a single `MOD/SCM parser`.
The evidence supports a layered design:

```text
PrimaryModelDocument
  -> shared object / mesh traversal concepts
  -> MOD variant
       skinned model inputs
  -> EFM variant
       MOD-like inputs + vertex RGBA / effect rendering
  -> SCM variant
       stage/static geometry variant

ShadowHullDocument
  -> SHW records
  -> self-contained float4 positions
  -> triangle topology + exact adjacency
  -> per-vertex transform-matrix selector
  -> selected palette ownership still open
  -> shadow plane/normal generation

RenderCompanion
  -> MRP
  -> exact schema/ownership still open
```

Do not force SHW into the MOD/SCM mesh ABI and do not invent MRP fields to make the
viewer API look uniform.
