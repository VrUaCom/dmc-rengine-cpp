# DMC3 Dante MOD + SHW paired corpus evidence — 2026-09-01

**Status:** CANONICAL RESEARCH ADDENDUM  
**Canonical EXE SHA-256:** `e454272ed0fb0247fcbcf300e5d55d7a3e96d50b89b9ffaff81bb978dcbdd082`  
**Branch:** `research/dmc3-primary-3d-format-abi-20260831`

## 1. Source boundary

User supplied a known Dante pair for analysis:

- MOD source SHA-256: `e219e89285604cb6d800b0afdd3bec6684a6b00cd1862d464a669d2861ff3c89`
- MOD size: `216544` bytes
- SHW source SHA-256: `cb392ef2e874addb887d32bc44d409299a32a83a4845afcbdef31698283f2e7e`
- SHW size: `9488` bytes

The copyrighted game payloads are not committed to the repository; only derived structural evidence and hashes are recorded.

## 2. Pair identity evidence

MOD header:

```text
magic              = "MOD "
version            = 1.01
objectCount +0x10  = 17
boneCount   +0x11  = 24
textureCount+0x12  = 3
skeletonOffset     = 0x2CE50
```

SHW header:

```text
magic                    = "SHW "
version                  = 0.5
recordCount +0x10        = 17
transformMatrixCount+0x11= 24
```

The exact `24 == 24` match between the MOD skeleton/bone palette and SHW transform-matrix palette is direct paired-corpus corroboration of the recovered runtime contract where SHW vertices are transformed through the model's matrix palette.

The equal `17` counts are noted as correlation only; a strict MOD-object-to-SHW-record one-to-one semantic is not promoted from count equality alone.

## 3. MOD geometry

The MOD contains 17 object records. Their mesh records contain a total of:

```text
5316 source vertices
```

Recovered model-space bounding box across all position streams:

```text
min = (-88.593384, -0.000231, -13.323054)
max = ( 88.593384,181.352325,  23.001238)
size= (177.186768,181.352556,  36.324292)
```

## 4. SHW geometry

The Dante SHW contains:

```text
17 records
152 vertices
236 triangles
24-entry transform palette capacity
17 distinct transform indices actually referenced
```

Referenced transform indices:

```text
2, 3, 4, 5,
7, 8, 9,
11, 12, 13, 14, 15, 16, 17,
19, 20, 21
```

Each SHW record uses exactly one transform-matrix index across all its vertices in this sample.

Recovered SHW model-space bounding box:

```text
min = (-88.302162, 0.036567, -15.676764)
max = ( 88.302162,182.277161, 23.016979)
size= (176.604324,182.240594, 38.693743)
```

The SHW proxy therefore covers almost exactly the full Dante silhouette envelope while using only 152 vertices versus 5316 MOD vertices (~2.86% of the source vertex count).

Approximate envelope ratios SHW/MOD:

```text
X width  ~= 99.67%
Y height ~=100.49%
Z depth  ~=106.52%
```

The slightly larger depth is consistent with a conservative shadow-caster proxy envelope, but the exact authoring policy is not promoted without more paired samples.

## 5. SHW records are closed manifold proxy shells

Every one of the 17 SHW records satisfies:

```text
all triangle indices in bounds
all triangle padding fields = 0
all adjacency padding fields = 0
all vertex W = 1.0
all adjacency entries resolve to the triangle sharing the corresponding edge
```

Additionally, for every record:

```text
triangleCount = 2 * vertexCount - 4
all undirected edges have exactly two incident triangles
V - E + F = 2
```

Therefore each record is a closed triangulated genus-0 manifold shell. This is stronger than treating SHW as an arbitrary triangle soup.

Do not promote the stronger claim that every record is a strict convex hull: the topology is closed and low-poly, but exact face triangulation/shape can be non-convex relative to a naive convex-hull reconstruction.

## 6. Body-part / skeleton interpretation

Spatial symmetry plus transform indices show a segmented articulated shadow proxy.

High-confidence spatial groupings from this sample:

```text
central upper chain: transforms 2,3,4,5
left arm chain:      transforms 7,8,9
right arm chain:     transforms 11,12,13
pelvis/lower center: transform 14
left leg chain:      transforms 15,16,17
right leg chain:     transforms 19,20,21
```

Example record envelopes:

```text
transform 5  -> center ~ (0,170,1), small head-sized shell
transform 4  -> center ~ (0,158,-1), neck-sized shell
transform 3  -> center ~ (0,148,-2), upper torso shell
transform 14 -> center ~ (0,107,-1), pelvis shell
transform 15/19 -> mirrored upper-leg shells
transform 16/20 -> mirrored lower-leg shells
transform 17/21 -> mirrored foot/lower-end shells
transform 7/11, 8/12, 9/13 -> mirrored arm chains
```

These labels are spatial/structural interpretations of the known Dante pair. Exact canonical bone names remain separate from numeric matrix indices unless recovered from an authoritative skeleton naming source.

## 7. Practical authoring implication

This paired sample strongly supports an SHW-generation strategy for DMC Rengine:

```text
MOD skeleton + model-space geometry
        ↓
select shadow-relevant bone/body regions
        ↓
construct very low-poly closed proxy shell per selected transform
        ↓
assign one transform index per proxy shell (as observed here)
        ↓
build triangle adjacency table
        ↓
serialize SHW
```

A simple one-shot global mesh decimator would not reproduce the observed Dante organization. The stock SHW is segmented by transform/bone region and preserves articulation while drastically reducing geometry.

This opens a realistic future `Generate SHW from MOD` tool, but canonical generation rules still require additional paired MOD+SHW samples and live-game validation before a production writer is declared safe.

## 8. Status promotions

| Claim | Status |
|---|---|
| Dante SHW shares the 24-entry transform palette cardinality with its MOD skeleton | DATA_CONFIRMED |
| Dante SHW is a full-body low-poly articulated shadow proxy | DATA_CONFIRMED |
| SHW proxy envelope closely matches MOD envelope | DATA_CONFIRMED |
| each Dante SHW record is a closed triangulated manifold shell | DATA_CONFIRMED |
| each Dante SHW record uses one transform index | DATA_CONFIRMED for this sample |
| exact stock SHW generation algorithm | RESEARCH_REQUIRED |
| automatic generated SHW accepted by game | GAME TEST REQUIRED |
