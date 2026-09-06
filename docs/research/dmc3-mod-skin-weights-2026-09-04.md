# DMC3 HD MOD skin-weight reverse — 2026-09-04

**Branch:** `mod-skin-reverse`  
**Canonical executable:** `dmc3.exe` SHA-256 `e454272ed0fb0247fcbcf300e5d55d7a3e96d50b89b9ffaff81bb978dcbdd082`  
**Scope:** decode the serialized per-vertex bone influence representation needed for Skeleton/Rig and Skin Weight visualization/editing. This is a MOD track, not SCM: the confirmed SCM serialized path has scene-node transforms but no MOD-style skin-index/weight vertex streams.

## Evidence inputs

| File | Size | SHA-256 | Header bone/node count | Vertices |
|---|---:|---|---:|---:|
| `slot_0001 (2).mod` | 216544 | `e219e89285604cb6d800b0afdd3bec6684a6b00cd1862d464a669d2861ff3c89` | 24 | 5316 |
| `slot_0001 (3).mod` | 110096 | `34f2c03795b007eac67abdb3a5808f675f1b1fd419ea6f39d4b77b8142c9e7ce` | 23 | 2641 |
| `slot_0012.mod` | 35696 | `7a2be875b3702f59a607655f7a0a412801a6aea639dcb6e3b23d9b0a09c7e740` | 33 | 828 |

Combined fresh census: **8785 vertices**.

## 1. Relevant serialized MOD streams

The already recovered MOD inner-record shell remains:

```text
inner record, stride 0x50
+0x00  u16 element_count
+0x10  relptr count * float3       position
+0x18  relptr count * float3       normal
+0x20  relptr count * int16x2      UV
+0x28  relptr count * u8x4         blend/matrix row indices
+0x30  relptr count * u16          packed weights + topology/control high bit
+0x40  generated topology workspace
+0x48  generated topology count
```

The canonical executable contains compiled `DMC3_MOD.hlsl` input semantics including `POSITION`, `NORMAL`, `TEXCOORD`, `BLENDINDICES`, and `PSIZE`. The `u8x4` stream therefore remains shader-correlated with `BLENDINDICES`; the packed u16 stream remains the corresponding compact scalar input/control stream.

## 2. Exact packed-weight arithmetic

For every one of the 8785 inspected vertices:

```text
raw16 = vertex_control_word
strip_or_topology_bit = raw16 & 0x8000
packed_weights        = raw16 & 0x7FFF

q0 = (packed_weights >>  0) & 0x1F
q1 = (packed_weights >>  5) & 0x1F
q2 = (packed_weights >> 10) & 0x1F
```

**Corpus invariant with zero exceptions:**

```text
q0 + q1 + q2 == 31
```

The quantized weights are therefore:

```text
w0 = q0 / 31.0
w1 = q1 / 31.0
w2 = q2 / 31.0
```

and:

```text
w0 + w1 + w2 == 1.0
```

within the exact 5-bit quantization model.

The quantized components are also observed in non-increasing order:

```text
q0 >= q1 >= q2
```

with zero exceptions in the current three-file corpus.

This proves a maximum of **three non-zero serialized skin influences per vertex** in the recovered MOD revision.

### Influence-count census

Across 8785 vertices:

| non-zero influences | vertices |
|---:|---:|
| 1 | 6594 |
| 2 | 1823 |
| 3 | 368 |

No vertex required a fourth non-zero quantized weight.

## 3. Bone/matrix index encoding

For every inspected vertex, the `u8x4` stream at inner `+0x28` satisfies:

```text
lane[0] == 0
lane[1] % 4 == 0
lane[2] % 4 == 0
lane[3] % 4 == 0
```

The non-zero weight components map naturally to lanes 1..3:

```text
bone0 = lane[1] / 4
bone1 = lane[2] / 4
bone2 = lane[3] / 4
```

The factor of four is consistent with row addressing into a float4 matrix-row palette: one rigid transform occupies four float4 rows.

Cross-file range validation is exact:

- 24-node sample: all non-zero weighted indices resolve below 24;
- 23-node sample: all non-zero weighted indices resolve below 23;
- 33-node sample: all non-zero weighted indices resolve below 33, and all node indices 0..32 are represented somewhere in the weighted mesh data.

No non-zero influence in the 8785-vertex census references an out-of-range node.

The zero-weight lanes may retain placeholder matrix indices and must not be interpreted as active influences.

## 4. Skeleton hierarchy block cross-binding

Header byte `+0x11` matches the node domain required by the blend-index stream on all three samples.

The structure reached through header qword `+0x20` begins with four relative offsets. The first two arrays have node-count length and reproduce the same topological pair shape seen elsewhere in DMC3 resources:

```text
relative +0x00 -> parent-by-order-position array
relative +0x04 -> node-at-order-position array
relative +0x08 -> third node-class/type array (semantic name still open)
relative +0x0C -> transform block
```

Observed relative layouts:

```text
24 nodes: 0x20, 0x38, 0x50, 0x70
23 nodes: 0x20, 0x38, 0x50, 0x70
33 nodes: 0x20, 0x44, 0x68, 0x90
```

The second array is an exact permutation of `0..nodeCount-1` on all three samples and the first array has exactly one `-1` root. This is strong structural evidence that header `+0x11` is the MOD skeleton/node count used by the skin palette.

Exact runtime ownership of the final skin matrix palette and bind-pose/inverse-bind construction still requires EXE closure before writer authority.

## 5. Canonical read-only decoder

The evidence-backed decoder should expose an influence list rather than raw packed fields:

```cpp
struct ModSkinInfluence {
    std::uint8_t bone_index;
    std::uint8_t quantized_weight; // 1..31
    float weight;                  // quantized_weight / 31.0f
};

struct ModVertexSkin {
    std::array<ModSkinInfluence, 3> influences;
    std::uint8_t influence_count;
    bool topology_break;
};
```

Pseudo-decoder:

```cpp
const uint16_t raw = packed_word;
const uint16_t p = raw & 0x7FFF;
const uint8_t q[3] = {
    uint8_t((p >> 0)  & 0x1F),
    uint8_t((p >> 5)  & 0x1F),
    uint8_t((p >> 10) & 0x1F),
};

for (int i = 0; i < 3; ++i) {
    if (q[i] == 0) continue;
    assert(blend_indices[i + 1] % 4 == 0);
    influence.bone_index = blend_indices[i + 1] / 4;
    influence.quantized_weight = q[i];
    influence.weight = float(q[i]) / 31.0f;
}
```

Hard validator:

```text
q0 + q1 + q2 == 31
active blend index % 4 == 0
bone_index < header.node_count
active bone indices are unique per vertex
```

Current corpus: all pass.

## 6. ModViz visualization contract

This decode is sufficient for the requested "zones of the 3D figure influenced by a bone" view.

Selecting one bone `B`:

```text
for every vertex V:
    influence = sum(weight for active influences where bone_index == B)
```

Render a heat map:

```text
0.00 -> no influence / hidden
0.01..0.24 -> weak
0.25..0.49 -> medium-low
0.50..0.74 -> strong
0.75..1.00 -> dominant
```

The UI should also support:

- show all three influences for a selected vertex;
- select a bone and highlight its weighted vertex zone;
- threshold slider (`weight >= X`);
- dominant-bone mode;
- overlapping influence visualization;
- raw quantized value `0..31` beside normalized float;
- diagnostics for invalid sums/index alignment/range.

Do not interpolate or invent a fourth weight.

## 7. Writer boundary

A safe encoder is mechanically possible:

```text
three normalized weights
-> quantize to integers summing exactly 31
-> sort descending together with their bone indices
-> encode q0 | (q1 << 5) | (q2 << 10)
-> preserve/derive the independent 0x8000 topology bit
-> encode matrix-row indices as bone_index * 4
```

However, **writer authority is not promoted by this pass**. Before skin editing is enabled, still close:

1. exact shader instruction mapping from `BLENDINDICES` + packed scalar to skin matrices;
2. exact runtime matrix-palette owner and construction;
3. bind pose / inverse-bind relationship;
4. whether node type/class array alters matrix selection;
5. multi-MOD revision corpus;
6. no-edit byte-identical MOD rebuild;
7. edited MOD -> PAC/NBZ -> original-game acceptance and rollback.

Until then the correct product maturity is:

```text
skin_weights = read_only_decoded
skin_weight_visualization = allowed
skin_weight_editing = blocked
```

## 8. SCM boundary

Do not add these streams to the SCM IR. The confirmed SCM path is static/stage scene geometry with scene-node transforms and does not contain MOD-style `BLENDINDICES`/packed-weight vertex streams. Skin-weight work belongs to the MOD model/skeleton pipeline; ModViz may present SCM and MOD through the same high-level 3D editor while keeping their binary contracts separate.
