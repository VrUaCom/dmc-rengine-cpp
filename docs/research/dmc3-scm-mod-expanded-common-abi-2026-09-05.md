# DMC3 HD SCM/MOD expanded common Model Family ABI — 2026-09-05

**Status:** canonical research addendum / architecture promotion  
**Branch:** `model-family`  
**Canonical executable:** `dmc3.exe` SHA-256 `e454272ed0fb0247fcbcf300e5d55d7a3e96d50b89b9ffaff81bb978dcbdd082`

## Purpose

The previous Model Family pass extracted only the directly identical `0x50` mesh subset. This pass asks a broader question:

> How much of SCM and MOD is actually one recovered model-document ABI/runtime family, and where do the formats genuinely diverge?

The answer is now substantially larger than `MeshCoreAbi`, but it still does **not** make SCM and MOD byte-interchangeable formats.

Evidence used here:

- canonical `dmc3.exe` machine code;
- three hash-bound real MOD payloads (`8785` combined vertices);
- the existing `68`-SCM corpus evidence;
- fresh field-by-field corpus reconciliation on all `42` outer records and `48` inner records of the three available MOD payloads.

## 1. High-level result

The safe recovered architecture is now:

```text
ModelDocumentCore
├── ModelObjectCore
│   └── ModelMeshCore
├── NodeDomainCore
│   └── TransformCore
└── format extensions
    ├── MOD: blend indices + packed skin weights + skeleton/palette behavior
    └── SCM: stage object binding + RGB/topology + stage-specific compatibility
```

There are separate format-specific normalizers and runtime object layouts, but both formats feed common lower-level manager, node, transform and mesh/material helpers.

## 2. Document core — shared header shell

Both recovered SCM and all three real MOD payloads use a `0x40`-byte header shell:

```text
+0x04  f32 version ~= 1.01
+0x10  u8 outer/object count
+0x11  u8 node-domain count
+0x12  u8 serialized texture-slot-domain count/mirror
+0x13  u8 runtime-carried mode/metadata byte (semantic name open)
+0x14  u32 runtime-carried metadata (semantic interpretation is format-specific)
+0x20  qword pointer/offset to node-domain block
+0x40  first outer/object record
```

The shared manager initializer `0x1402F9570` is called from both model setup families. It copies:

```text
raw +0x14 -> manager +0xE4
raw +0x10 -> manager +0xE8
raw +0x11 -> manager +0xEA
raw +0x13 -> manager +0xFA
```

Runtime texture count is **not** taken from serialized `+0x12`; `0x1402F9570` reads the external companion at `manager+0x110` and stores its count at `manager+0xEC`. The serialized `+0x12` byte is therefore a file-side domain/mirror value rather than the live authority in this path.

Do not project SCM's decoded `LegacyResourceCode` meaning of `+0x14` onto MOD. MOD values in the fresh three-file corpus are `217`, `100407`, `217`, which do not follow the SCM decimal resource-code family.

## 3. Object/outer core — shared 0x40 record

The common record is now much stronger than count/table coincidence:

```text
record size 0x40
+0x00 u8  child mesh/inner count
+0x01 u8  alpha/control byte
+0x02 u16 aggregate element/vertex count
+0x08 qword child mesh/inner table pointer
+0x10 u32 source flags
+0x30 vec3 bounding center
+0x3C f32 bounding radius
```

### 3.1 Aggregate count

On SCM, `+0x02 == sum(mesh.vertex_count)` for `254/254` objects.

On the three bound MOD payloads, `+0x02 == sum(inner.element_count)` for all `42/42` outer records.

### 3.2 Alpha/control byte is shared

This pass closes a previous overly conservative boundary.

MOD/EFM object initializer `0x1403029E0` and SCM object initializer `0x140302F10` both:

```text
serialized outer/object +0x01
    -> runtime object +0x07
    -> common <=0x80 / >0x80 alpha-control state machine
```

Both initializers also allow the low source-flag mode to force effective control `0x80`. SCM then applies two narrow resource-signature compatibility corrections (`0xEA` and `0xC4`) that are not generic format law.

All `42` MOD outer records in the current corpus serialize `+0x01 = 0x80`.

Safe family interpretation:

> `+0x01` is a shared model-object alpha/control byte with format-specific compatibility behavior layered on top.

### 3.3 Source flags are shared raw state

Both object initializers copy serialized `+0x10` verbatim into runtime object `+0x10` and mutable/effective `+0x14`.

Current MOD outer-flag values across the three files include:

```text
0x00000000
0x00100001
0x00120002
0x00200000
```

which overlaps the existing SCM source-flag vocabulary. Not every bit has identical semantics, so the core exposes the raw source-flags lane while bit promotion remains evidence-specific.

One important shared downstream bit is now directly connected:

```text
source flags & 0x00004000
    -> generic mesh/material helper 0x1402F9890
    -> legacy GS TEX1 filtering 0x60 (linear) or 0 (nearest)
```

### 3.4 Bounding sphere is identical

Fresh MOD validation over all 42 outer records:

```text
outer +0x30 = center.xyz
outer +0x3C = radius
radius == max distance(center, all vertices in all child inners)
```

Maximum observed absolute discrepancy in the three-file MOD sweep is below `3.9e-6` world units.

This is the same contract already proven on `254/254` SCM objects.

Therefore `ModelObjectCore` may safely expose one bounding-sphere representation for both adapters.

## 4. Mesh core — shared material fields added

The original common mesh ABI already contained:

```text
record size 0x50
+0x00 element count
+0x10 position float3[]
+0x18 normal float3[]
+0x20 UV int16x2[] / 4096
+0x40 generated topology workspace
+0x48 generated topology count
```

This pass promotes additional common fields:

```text
+0x02 u16 texture-table slot
+0x04 u16 GS CLAMP MINU
+0x06 u16 GS CLAMP MAXU
+0x08 u16 GS CLAMP MINV
+0x0A u16 GS CLAMP MAXV
```

### 4.1 Texture slot — direct runtime proof

The MOD-specific builder `0x1402FE6A0` retains the raw MOD inner pointer in the runtime mesh record. The shared mesh/material helper `0x1402F9890` then:

```text
rawMesh = runtimeMesh+0x10
slot = u16(rawMesh+0x02)
textureRecord = textureTable + slot * 0x40
textureDescriptor = textureRecord+0x20
```

The same helper is also used by the related runtime material path after family-specific construction.

Fresh MOD corpus consistency:

```text
slot_0001 (2).mod: header +0x12 = 3, observed inner +0x02 = {0,1,2}
slot_0001 (3).mod: header +0x12 = 4, observed inner +0x02 = {0,1}
slot_0012.mod:     header +0x12 = 3, observed inner +0x02 = {0,1}
```

Every observed MOD texture slot is below serialized header `+0x12`.

### 4.2 GS CLAMP is shared

`0x1402F9890` reads the four u16 values at raw mesh `+0x04/+0x06/+0x08/+0x0A` and packs:

```text
(minU << 4)
| 0xF
| (maxU << 14)
| (minV << 24)
| (maxV << 34)
```

This is the previously recovered legacy PS2 GS `CLAMP` REGION_REPEAT register contract. The current 48 MOD inner records serialize all four fields as zero, but executable support is active and common.

Thus texture-slot selection and legacy GS sampler/wrap state are **Model Family** capabilities, not SCM-only features.

## 5. Shared runtime material convergence

The MOD/EFM runtime mesh path at `0x1402F9DA0` is format-specialized only for raw mesh construction:

```text
MOD -> 0x1402FE6A0
EFM -> 0x1402F7D60
```

After that specialization, every inner runtime mesh is sent through:

```text
0x1402F9890  generic texture / GS sampler descriptor builder
0x1402F9A80  common follow-up mesh state
```

`0x1402F9DA0` passes runtime outer `+0x14` (the effective copy of serialized source flags) into `0x1402F9890`, closing source-flag provenance for the common `0x4000` texture-filter override.

SCM uses a related specialized mesh-object allocation path (`0x1402F9F20`) but reaches the same underlying serialized material ABI in its mesh descriptors. The family therefore shares lower-level material semantics even though the runtime outer object sizes differ.

## 6. NodeDomainCore is shared

The common attachment helper `0x1402F1DB0` is called from both model setup paths. It reads raw header `+0x20`, resolves the four relative u32 offsets and stores:

```text
block +0x00 -> manager +0x08
block +0x04 -> manager +0x10
block +0x08 -> manager +0x18
block +0x0C -> manager +0x20
```

Across all three MOD samples, relative-array placement follows the exact SCM formula:

```text
parentRel  = 0x20
orderRel   = 0x20 + align4(N)
adapterRel = 0x20 + 2*align4(N)
transRel   = align16(0x20 + 3*align4(N))
```

Examples:

```text
N=24 -> 0x20,0x38,0x50,0x70
N=23 -> 0x20,0x38,0x50,0x70
N=33 -> 0x20,0x44,0x68,0x90
```

For all three MOD payloads:

- `order[]` is an exact permutation of `0..N-1`;
- parent array has exactly one `-1` root;
- parents are topologically valid.

The third array **must stay adapter-specific**:

- SCM: geometry object binding by node index;
- MOD: current values behave as another node class/type domain (`0/1/2` observed), exact semantic name still open.

## 7. TransformCore is shared; previous SCM function attribution corrected

A provenance correction discovered during this pass is important.

The previous SCM notes attributed local SCM transform construction to `0x1402FA080`. Whole setup-chain reconciliation now proves:

```text
MOD/EFM setup 0x1403039C0 -> transform initializer 0x1402FA080
SCM setup     0x140303C10 -> transform initializer 0x1402FA360
```

The earlier function ownership was wrong, but the recovered transform math remains correct because **both initializers call the same helpers on the same transform lanes**:

```text
rotation vec3 at +0x10 -> 0x140330450
translation vec3 at +0x00 -> 0x140031200
```

The shared serialized transform shell is:

```text
record size 0x20
+0x00 vec3 translation
+0x0C f32 precomputed length(translation)
+0x10 vec3 XYZ Euler radians
+0x1C f32 zero/reserved
```

Fresh MOD corpus validation:

- all three MOD payloads satisfy `+0x0C == length(XYZ)` within at most `1.6e-6`;
- `+0x1C == 0` in the inspected records;
- one sample (`slot_0012.mod`) contains real non-zero rotation values, so the second vec3 is not merely unused padding.

Both transform initializers call `0x140330450`, which applies X then Y then Z rotation and therefore yields `Rz * Ry * Rx` in the recovered DMC3 row-vector convention. Both use `0x140031200` for translation; its mask preserves matrix W and excludes serialized `+0x0C` from homogeneous translation.

Therefore the local transform semantic core is now shared between SCM and MOD/EFM.

## 8. Common world propagation, specialized initialization

`0x1402F9700` is a generic node-world update helper using manager node count/order state and the common matrix pool. It composes each node local matrix with its already selected parent/root matrix via `0x140030E40`.

However, initialization remains specialized:

- `0x1402FA080` (MOD/EFM) additionally constructs matrix/inverse state needed by the skeletal/model pipeline and calls rigid inverse helper `0x140030DC0`;
- `0x1402FA360` (SCM) builds the static/stage node local matrices and parent/root pointers but does **not** perform that same inverse-cache initialization path.

This corrects the earlier overclaim that direct `0x140030DC0` inverse-cache ownership was SCM-specific. The arithmetic helper itself is valid; its observed direct initialization ownership belongs to the MOD/EFM path.

This distinction is highly relevant to the next skinning target: the MOD/EFM inverse path is a likely bridge toward bind/inverse-bind and matrix-palette ownership.

## 9. Format-specific differences that remain real

Do not deduplicate these simply because the shell is shared:

### SCM

```text
mesh +0x28 continuation span
mesh +0x38 RGB/topology stream, bit 0x02
node +0x08 array = object binding
stage-specific alpha compatibility signatures
stage texture-companion authoring rules
```

### MOD

```text
mesh +0x28 u8x4 blend/matrix-row indices
mesh +0x30 packed 3x5-bit skin weights + topology bit 0x8000
node +0x08 third array has different semantics
skeletal matrix/inverse/palette behavior
```

The two normalizers remain separate:

```text
MOD 0x1402FE3B0
SCM 0x1403051B0
```

They are homologous algorithm-family members, not one function.

## 10. Product/API consequence

The Model Family toolkit can now safely expose four shared ABI layers:

```text
ModelDocumentCoreAbi
ModelObjectCoreAbi
ModelMeshCoreAbi
NodeDomainCoreAbi + TransformCoreAbi
```

MOD capability promotion from this pass:

```text
texture_binding   = true
alpha_control     = true
legacy_gs_sampler = true
```

Skinning remains MOD-only. SCM authoring maturity does not grant MOD writer authority.

## 11. Next reverse frontier

1. Close exact semantics of MOD node-domain third array (`block +0x08`).
2. Follow the MOD/EFM `0x140030DC0` inverse-matrix products into matrix-palette ownership.
3. Recover bind pose / inverse bind relation and animated skin matrix construction.
4. Compare the shared generic world-update calls across actual MOD and SCM draw/update call sites.
5. Expand MOD corpus to additional revisions/models before any MOD writer promotion.
6. Bind texture companion payloads for real MOD samples and verify header `+0x12` against companion count, not only mesh-slot range.

The main architectural result is now evidence-backed: SCM and MOD are not merely two visually similar formats. They are specialized members of one recovered DMC3 model-document runtime family with shared document, object, mesh, node and local-transform cores.
