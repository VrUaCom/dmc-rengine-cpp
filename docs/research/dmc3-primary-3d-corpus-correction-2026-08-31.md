# DMC3 primary 3D corpus correction — EFM / SHW / MRP — 2026-08-31

**Status:** CANONICAL RESEARCH CORRECTION CANDIDATE  
**Scope:** local preserved stage-drop corpus + canonical `dmc3.exe` correlation  
**Canonical EXE SHA-256:** `e454272ed0fb0247fcbcf300e5d55d7a3e96d50b89b9ffaff81bb978dcbdd082`  
**Corpus archive SHA-256:** `7680a9ddb700b958ca1591be0629c2ff1da53efa1b723141bbee0ae4b4c7ff6f`

This pass corrects and extends `dmc3-primary-3d-render-family-reverse-2026-08-31.md`. It does not erase the older evidence trail. The earlier EXE-only conclusions remain useful, but three boundaries change after binding real local payloads.

## 1. Corrections

1. **EFM real-payload gap CLOSED for the available local stage-drop corpus.** Nine EFM payloads validate the recovered MOD-like mesh layout and a sixth 4-byte-per-vertex stream correlated with shader `COLOR0`.
2. **SHW external-spatial-pool interpretation REJECTED.** Sixteen SHW payloads prove that each SHW record contains its own float4 vertex pool, triangle table, exact edge-adjacency table and per-vertex transform/matrix-index bytes.
3. **MRP remains open.** Its runtime identity is independently confirmed twice, but no normal model path, exact downstream owner, strict payload signature or local sample was recovered in the bounded corpus.

The evidence hierarchy used here is: bounded raw payload structure -> cross-sample invariants -> canonical EXE fixup/consumer -> embedded shader corroboration. Filename alone is not used as semantic proof.

## 2. Corpus provenance boundary

The payloads come from preserved `analysis_inputs/stage_drops` inside local archive `DMC 3 RENGINE (6).zip`. They are strong local corpus evidence. This pass does **not** independently prove the upstream retail acquisition lineage of every extracted payload, so it deliberately says “local stage-drop corpus” rather than laundering that into “retail-confirmed”.

## 3. EFM corpus closure

Observed: **9 files, 2,058 vertices**; magic `EFM `; versions `1.00` and `1.01`. All nine use the recovered `0x40` outer-record / `0x50` mesh-record family.

For mesh vertex count `N`, all nine samples satisfy:

```text
+0x10 -> align16(N * 12)  // float3 POSITION correlation
+0x18 -> align16(N * 12)  // float3 NORMAL correlation
+0x20 -> align16(N * 4)   // two 16-bit TEXCOORD values
+0x28 -> align16(N * 4)   // blend/matrix-index stream
+0x30 -> align16(N * 2)   // packed weight/flag + strip-control source
+0x38 -> align16(N * 4)   // per-vertex COLOR0/RGBA stream
```

The `+0x38` stream ends exactly at `skeletonOffset` in **9/9** samples. Together with the embedded EFM shader contract (`COLOR0`) and the engine comment that EFM models carry extra vertex RGB, this is sufficient to promote `+0x38` to a **4-byte per-vertex colour stream**. Exact channel normalization/vertex-declaration packing remains open.

`mesh + 0x40` is not a seventh authoritative source stream. It is a mesh-relative destination/reserve used by runtime topology reconstruction; raw `mesh + 0x48` generated count is zero in 9/9 samples. The canonical source topology/control information remains in the `+0x30` stream and uses bit `0x8000` in the recovered normalizer.

### EFM sample receipts

| sample | size | SHA-256 | vertices |
|---|---:|---|---:|
| `em035_057_001_105.efm` | 9568 | `fda36531db3f5d7f3e510e56f95a571ec56ab37745f2da731f2d5ee1c45e0424` | 210 |
| `em035_057_001_111.efm` | 2352 | `2aa78d1abaea8d39e79b97b4bb50cfd661a67695c2006d666c1acc44d0a64d72` | 46 |
| `em035_057_001_121.efm` | 6624 | `238ff08cc46656f7fa6ffdc37ff608bd544fe9ab498ca12e3f880cf6c68159f1` | 144 |
| `em035_057_001_123.efm` | 2352 | `fdd95e42002653f09534f41acd5bfda9b1d400b6fc90ea4f3b8c318b92326458` | 46 |
| `m20_s00_004_037.efm` | 22592 | `dc839d19073b6f2ef98e5c96e34d037bd12f5676f369070e88ea880890eb5b43` | 506 |
| `m20_s00_004_038.efm` | 16128 | `3e38bc2bd5657ce52f72f834cb56f413531f2f0d20ca7b33c01f0c84a787b223` | 360 |
| `m20_s00_004_039.efm` | 11296 | `0b3041adf09d77879e3bfa44c2205da652ad665d0fe735c2210efe90399fe66a` | 249 |
| `m20_s00_004_042.efm` | 9184 | `3e030af67b4431cd7171f8be72e7cfb398a8faad9b2a68b34405fa67ef4c6352` | 201 |
| `m20_s00_004_044.efm` | 13312 | `f7403ee9d8ae78b7316fe728282b7b84cdf4461c9e32eda73a801f831faa3726` | 296 |

## 4. SHW exact local payload ABI

Observed: **16 files, 158 records, 1,882 vertices, 3,104 triangles**; every sample has magic `SHW ` and version `0.5`.

Evidence-safe header boundary:

```cpp
struct ShwHeaderEvidence {
    // ...
    uint8_t recordCount;       // +0x10
    uint8_t matrixCountLike;   // +0x11; every observed vertex matrix index is below it
    uint8_t unknown12;         // +0x12
    // ...
    // records begin at +0x20
};
```

Each record has stride `0x40`:

```cpp
struct ShwRecordEvidence {
    int16_t vertexCount;       // +0x00
    int16_t triangleCount;     // +0x02
    // ...
    rel64 triangles;           // +0x10 -> triangleCount * 0x10
    rel64 adjacency;           // +0x18 -> triangleCount * 0x08
    rel64 vertices;            // +0x20 -> vertexCount * 0x10
    rel64 matrixIndices;       // +0x28 -> vertexCount bytes
};
```

Triangle record:

```cpp
struct ShwTriangleEvidence { int32_t v0, v1, v2, zero; };
```

Adjacency record:

```cpp
struct ShwAdjacencyEvidence {
    uint16_t acrossEdge01;
    uint16_t acrossEdge12;
    uint16_t acrossEdge20;
    uint16_t zero;
};
```

Vertex record is `float4 x,y,z,w`; `w == 1.0` in **1,882/1,882** observed vertices. The `+0x28` byte stream is used as a per-vertex transform/matrix selector; canonical runtime code scales the selected index by `0x40`, matching a 4x4 float matrix stride.

### Adjacency proof

All **9,312/9,312** adjacency references were checked. For every triangle `(v0,v1,v2)`:

- adjacency slot 0 targets a triangle sharing undirected edge `v0-v1`;
- slot 1 shares edge `v1-v2`;
- slot 2 shares edge `v2-v0`.

There were **0 invalid adjacency references**. All triangle padding words and adjacency padding words were zero in the checked corpus. Therefore `+0x18` is not a generic control table: its per-edge neighbour semantics are data-confirmed.

### SHW correction

The earlier statement that SHW triangle indices reference an **external** 16-byte spatial/vector pool is rejected. The `+0x20` pointer resolves inside the same SHW payload and owns `vertexCount * 16` float4 records. SHW is therefore a **self-contained shadow-geometry resource** with its own vertices/topology/adjacency, while still not being a textured MOD/SCM-style model document.

### SHW sample receipts

| sample | size | SHA-256 | records | vertices | triangles |
|---|---:|---|---:|---:|---:|
| `em035_005.shw` | 9952 | `5a17a4d435d562929f641f29fe4f45d1996a794e107d065ae5c1ad999f07a205` | 17 | 159 | 250 |
| `em035_006.shw` | 4992 | `ef39b3ad8e713c1b14a8ed69571d03fdc6660db6cbb48266e007bacb10e11e2d` | 7 | 79 | 130 |
| `em035_021.shw` | 15920 | `7e6244d695e5407e556d3049f7945c60acc3c1b1ebd18a8ac6b9ae2865cc3c6e` | 17 | 251 | 434 |
| `em035_022.shw` | 5840 | `dc7884346b35c76a83c246b07bb5e616b8ea48f60dc591e2b4b549ef7e7da598` | 5 | 91 | 162 |
| `em035_023.shw` | 3904 | `b51e70ce704205f0a4b00fdf5418db718ac800e5c508b366eb69c5ae6e93c5d6` | 1 | 60 | 116 |
| `em035_026.shw` | 1952 | `7181f038c045f5878a5a7bbda192231dcb4adb16191f2aed02e5d9938e30b8a3` | 1 | 30 | 56 |
| `em035_029.shw` | 4832 | `cf187353af67a90c29d80440cf0c3d80d5a5efb53a0c6de56ceca77e9755998c` | 4 | 80 | 132 |
| `em035_033.shw` | 2352 | `1e582a42ce678c8376813f822233d951aa478b40b0abb6055233b5dbf10df434` | 1 | 39 | 66 |
| `em035_048.shw` | 13776 | `f9d743690cae40e756cd351de824371be2e2cb0e7662756832c3715e6d455581` | 17 | 218 | 368 |
| `em035_049.shw` | 5792 | `cc5d678a0cc92445e2e5b0e1d85ca66883573f2a4ffad828dddf1c14dc9ea86e` | 4 | 90 | 164 |
| `em035_059.shw` | 2352 | `59e690b1eb2f1fc4afc514349949f1cbfab0cde5aaee180f77e50a44c9c20fd0` | 1 | 39 | 66 |
| `m20_s00_004_004.shw` | 10976 | `21b326601078d426990fa10cdb3fb3e303dd7b31dfd886a1f51fb675e43198e4` | 17 | 175 | 282 |
| `m20_s00_004_023.shw` | 8960 | `658fad1fe31e02fbc622da6e243a8549b904f1a93959faac77ef9b9575c829ce` | 15 | 143 | 226 |
| `m20_s00_004_028.shw` | 8592 | `93488d5853359268d995145b7247cfe1d98c9bac082ee6d6f354a74abfcb81e1` | 17 | 138 | 208 |
| `m20_s00_004_030.shw` | 8592 | `20c8f446b50c7fd0e4db34465b3ae4c072af0552dc43477b982552adc6a2bb74` | 17 | 138 | 208 |
| `pl011_008.shw` | 9488 | `cb392ef2e874addb887d32bc44d409299a32a83a4845afcbdef31698283f2e7e` | 17 | 152 | 236 |

## 5. MRP bounded negative result

MRP identity remains direct EXE fact:

```text
0x1402DB1F0 : MRP -> registry type 3
0x1402FD650 : "MRP " -> family mask 0x40000000
```

But no MRP-specific branch is currently recovered in the normal generic post-load dispatcher, MOD/EFM/SCM model factory, model-memory specialization, or bounded direct-caller family-mask census. The `Model` parsing route accepts MOD/EFM/SCM rather than MRP.

Corpus scan boundary:

- 23,986 non-directory archive entries / 566,890,888 uncompressed bytes inspected;
- after excluding `node_modules` and `.npm-cache`: 2,969 project-content entries / 123,296,119 bytes;
- strict signatures `MRP ` and `MRP\0`: **0 hits**;
- generic ASCII `MRP` hits in the unfiltered archive resolve to npm/Vite HMR identifiers and are rejected as game-resource evidence.

This does **not** prove MRP is unused or absent from every DMC3 build/distribution. It only closes the bounded corpus result. Canonical status should therefore be: `identity=EXE_CONFIRMED`, `purpose=RESEARCH_REQUIRED`, `schema=OPEN`.

## 6. Corrected 3D-family boundary

```text
MOD  -> mesh-bearing actor/object model
EFM  -> mesh-bearing effect model + per-vertex COLOR0
SCM  -> mesh-bearing stage/scene model
SHW  -> self-contained shadow geometry + triangle edge adjacency + transform indices
MRP  -> runtime family identity confirmed; exact purpose/schema still open
```

The product architecture implication is a shared Geometry Core with format adapters for MOD/EFM/SCM/SHW, but MRP must not be forced into that interface until payload or consumer evidence proves it belongs there.

## 7. Remaining gates

1. Recover the exact EFM vertex declaration/normalization and confirm byte-channel mapping for COLOR0.
2. Name SHW header `+0x11` only after its exact count/ownership consumer is recovered; current `matrixCountLike` is evidence-safe, not a final semantic name.
3. Recover complete SHW lifecycle/writer behavior and validate byte-preserving round-trip before editor write support.
4. For MRP, acquire a real payload or trace a classifier-derived `0x40000000` identity into a downstream owner; do not chase unrelated immediate `0x40000000` constants.
5. Keep local stage-drop provenance distinct from independently reacquired retail-source provenance.
