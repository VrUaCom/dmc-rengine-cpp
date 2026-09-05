# DMC3 SCM deep reverse — 2026-09-02

## Purpose

Close the serialized structural core of DMC3-HD `.SCM` using three independent evidence classes:

1. canonical `dmc3.exe` machine-code consumers;
2. hash-bound real SCM payloads;
3. broad corpus invariants, without laundering prior Blender/DMC Rengine assumptions into canonical semantics.

No proprietary game/resource bytes are committed.

## Canonical authorities

Canonical analysis executable:

```text
file   dmc3.exe
size   6,356,432
sha256 e454272ed0fb0247fcbcf300e5d55d7a3e96d50b89b9ffaff81bb978dcbdd082
```

Runtime/type anchors retained from current reverse work:

```text
0x1402DB1F0  registry three-byte content probe; SCM is a recognized family
0x1402FD650  four-byte family-mask classifier; `SCM ` is a primary render family
0x1403051B0  SCM post-load normalizer / generated-index reconstruction
0x1402FDD10  SCM mesh consumer using fixed physical mesh stride 0x50
```

This pass resolves the historical blocker where `0x1403051B0` appeared to imply a variable mesh record because it advances through `mesh+0x28`.

## Corpus

Historical external archive used as one corpus source:

```text
DMC 3 RENGINE (6).zip
size   237,658,858
sha256 7680a9ddb700b958ca1591be0629c2ff1da53efa1b723141bbee0ae4b4c7ff6f
SCM entries 77
unique SCM payloads by SHA-256 67
```

Additional real payload:

```text
st114.scm
size   1,038,816
sha256 fd3ade343a5cac15a174fbdfc2ff3d848f33c0928bcef171eec0ca6e19c1cd7d
```

`st001.scm` is already present in the historical archive and was also validated independently:

```text
size   887,760
sha256 3ed787cc8a41f4c21b972664cf4174739382230ce0010f166c32c2a9e3626ba5
```

Combined deduplicated validation population:

```text
unique SCM files    68
objects             254
meshes              481
scene nodes          328
vertices             182,612
file size min/max    464 / 1,038,816
mesh count/object    1..10
vertices/mesh        3..10,196
version              1.01 on 68/68
```

The new C++20 parser returns `recognized=true`, `ok=true`, `diagnostics=0` for all 68 unique payloads.

## Reconciled findings

### 1. SCM is not MOD-with-another-magic

SCM is a dedicated mesh-bearing stage/scene family with its own post-load normalizer, its own scene-node structure and no MOD skin-weight/bone-index vertex streams in the confirmed serialized path.

### 2. Header count semantics

The former ambiguous count names are now separated:

```text
+0x10 object count
+0x11 scene-node count
+0x12 texture-slot count
```

`+0x11` is not a texture count. The independent scene-node block and object-binding array prove the second count belongs to scene hierarchy.

### 3. Object total vertex count

For every object:

```text
object.totalVertexCount == sum(mesh.vertexCount)
```

Result: `254/254`, zero contradictions.

### 4. Object bounding sphere

`object+0x30` is:

```text
vec3 center
f32 radius
```

For all 254 objects, radius equals the maximum Euclidean distance from the stored center to serialized object vertices within float tolerance. Largest observed absolute error in this sweep was below `0.002` world units on an ~84k radius.

### 5. Fixed mesh record plus continuation contract

Two runtime observations are simultaneously true:

```text
physical mesh record size = 0x50
non-final mesh +0x28      = 0x50
final mesh +0x28          = 0
```

Therefore `mesh+0x28` is not evidence for arbitrary record size. It is an explicit continuation span that agrees with the fixed physical stride in canonical serialized data.

Validation: `481/481` meshes.

### 6. Vertex-stream ABI

Serialized streams are:

```text
positions  f32[3]
normals    f32[3]
UV         i16[2], scale 1/4096
RGB/flags  u8[4]
```

The fourth RGB/flags byte uses only `0x00` and `0x02` in all 182,612 validated vertices. Runtime index reconstruction consumes the `0x02` bit as a strip break/skip. The field must not be modeled as alpha.

### 7. Exact disk placement

For each object, stream families are grouped and each individual mesh stream is 16-byte aligned:

```text
meshDefs
align16
position(mesh0); align16
position(mesh1); align16
...
normal(mesh0); align16
...
UV(mesh0); align16
...
colorFlags(mesh0); align16
...
next object meshDefs
```

After the final object's color stream comes the scene-node block, then aligned generated-index reserves. The deterministic formula reaches exact EOF on `68/68` files.

This corrects the weaker wording that only group boundaries were aligned.

### 8. Scene hierarchy is real and independent

Scene block relative-array formula is exact on `68/68`:

```text
parentRel        = 0x20
orderRel         = 0x20 + align4(N)
objectBindingRel = 0x20 + 2*align4(N)
transformRel     = align16(0x20 + 3*align4(N))
```

`order[]` is a permutation, parents are `-1`/valid node indexes, and object bindings are `-1`/valid object indexes.

Node/object count deltas:

```text
+1 : 66 files
+2 : 1 file
+6 : 1 file (st001)
```

This rejects a one-object-one-node simplification.

### 9. Scene transform

Transform record `0x20`:

```text
+00 vec3 translation
+0C f32  length(translation)
+10 vec3 rotation candidate
+1C f32  zero
```

The translation magnitude invariant holds on `328/328` nodes. The second vec3 contains radian-like values including common fractions of pi, but Euler order/coordinate semantics remain `HIGH_CONFIDENCE`, not `EXE_CONFIRMED`.

### 10. `mesh+0x40` is generated-index workspace

The old opaque-command interpretation is rejected.

Canonical serialized reserve:

```text
workspaceOffset = meshRecordOffset + mesh.relative40
capacityBytes   = align16(6 * (vertexCount - 2))
first word      = 0x1212
mesh+0x48       = 0 before runtime normalization
```

All reserves are sequential after the aligned scene-node block and the last reserve ends exactly at EOF.

The normalizer generates `u16` indices from topology bit `0x02` and publishes the generated word count at `mesh+0x48`.

## Corpus-observed unresolved fields

These values are preserved, not semantically renamed:

Header `+0x14`:

- 21 distinct values in this population;
- structured/id-like distribution;
- exact runtime meaning remains open.

Object `+0x01`:

```text
0x80
0xC0 0xC1 0xC2 0xC3 0xC4 0xC5
```

Object `+0x10` flags observed:

```text
0x000000
0x000001
0x020000
0x080000
0x080001
0x080002
0x100001
0x180001
0x200000
```

Do not name these bits until consumer evidence isolates behavior.

Texture-slot counts in the 68-file population:

```text
16 : 55 files
17 : 9 files
6  : 4 files
```

Every validated mesh texture index is within its header texture-slot count.

## C++20 implementation

New modules:

```text
include/dmc_rengine/formats/scm.hpp
include/dmc_rengine/formats/scm_layout.hpp
include/dmc_rengine/formats/scm_topology.hpp
src/formats/scm.cpp
src/formats/scm_layout.cpp
src/formats/scm_topology.cpp
tests/scm_tests.cpp
```

Design rules:

- no packed C++ struct overlay on untrusted bytes;
- explicit little-endian reads;
- bounds checked before every variable region;
- absolute disk offsets stay absolute;
- `mesh+0x40` is converted only through the evidenced mesh-relative formula;
- unknown fields preserved in typed IR with unresolved names;
- corpus-only invariants warn when safe; structural contradictions error;
- parser remains read-only until writer acceptance is independently closed.

Local verification before branch publication:

```text
C++20 compile: -Wall -Wextra -Wpedantic -Wconversion
synthetic scm_tests: PASS
st001.scm: recognized=1 ok=1 diagnostics=0
st114.scm: recognized=1 ok=1 diagnostics=0
68 unique SCM corpus: 68 PASS / 0 FAIL / 0 diagnostics
```

## Writer promotion gate

The layout is now sufficient to build a clean writer, but writer status remains intentionally open until all gates pass:

1. typed authoring IR owns every changed semantic field;
2. unknown/reserved fields have explicit preservation policy;
3. no-edit rebuild is byte-identical across the supported 68-file population;
4. deterministic generated-index reserve creation matches canonical serialized layout;
5. edited mesh/scene rebuild reparses with zero structural errors;
6. container reintegration succeeds through the common authoring pipeline;
7. original DMC3 consumes the edited SCM without loader failure;
8. intended geometry/transform effect is observed;
9. rollback/original artifact integrity is verified.

Until then the integration registry must advertise SCM as `structural / read-only`, not editable/exportable.

## Remaining reverse targets

1. exact consumer semantics for header `+0x14`;
2. object `+0x01` class/flag meaning;
3. bit-level object `+0x10` flags;
4. exact transform rotation order and coordinate convention;
5. material/texture-slot mapping into the stage texture bundle;
6. original runtime ownership of normalized/generated index workspace;
7. writer + game-consumption acceptance.
