# DMC3 HD SCM serialized format

**Reverse status:** COMPLETE for the canonical DMC3 HD SCM scope.  
**Product status:** structural reader + bounded authoring stack; generic product write authority remains separately gated.  
**Profile:** `dmc3-hd`.  
**Canonical analysis EXE:** SHA-256 `e454272ed0fb0247fcbcf300e5d55d7a3e96d50b89b9ffaff81bb978dcbdd082`.  
**Primary runtime normalizer:** `0x1403051B0`.  
**Independent fixed-stride SCM mesh consumer:** `0x1402FDD10`.  
**SCM local-transform initializer:** `0x1402FA360`.  
**Completion audit:** [`../research/dmc3-scm-reverse-completion-2026-09-13.md`](../research/dmc3-scm-reverse-completion-2026-09-13.md).  
**Machine receipt:** [`../../data/reverse/dmc3-scm-reverse-completion-20260913.json`](../../data/reverse/dmc3-scm-reverse-completion-20260913.json).

SCM is a mesh-bearing stage/static-scene geometry family. It is not a MOD alias. The current serialized/runtime reverse is terminal under the DMC Rengine evidence rule: every domain is typed/structural, explicitly preservation-only, or a rejected historical hypothesis. Writer breadth, container/NBZ delivery and original-game acceptance remain separate proof tracks and do not reopen the reverse.

## Evidence vocabulary

Terminal SCM results use the same closed evidence vocabulary as the completed MOD reverse:

```text
EXE_CONFIRMED
CORPUS_CONFIRMED
EXE_AND_CORPUS_CONFIRMED
STRUCTURAL_CONFIRMED
SEMANTIC_CANDIDATE
PRESERVED_UNDECODED
RESERVED_OBSERVED_ZERO
REJECTED
```

`RESERVED_OBSERVED_ZERO` describes the corpus only. It never grants permission to zero-normalize source bytes. `PRESERVED_UNDECODED` is a terminal reverse result when the strongest honest contract is exact preservation rather than an invented semantic name.

## Header — 0x40 bytes

| Offset | Type | Canonical meaning/status |
| ---: | --- | --- |
| `+0x00` | `char[4]` | exact magic `SCM ` — `EXE_CONFIRMED` |
| `+0x04` | `f32` | version; `1.01` on 68/68 unique canonical files — `CORPUS_CONFIRMED` |
| `+0x08` | `u64` | zero in bounded corpus; canonical model-source path dormant — `RESERVED_OBSERVED_ZERO`, `PRESERVED_UNDECODED` |
| `+0x10` | `u8` | object count -> manager `+0xE8` — `EXE_CONFIRMED` |
| `+0x11` | `u8` | scene-node count -> manager `+0xEA` — `EXE_CONFIRMED` |
| `+0x12` | `u8` | serialized texture-count mirror; live runtime count comes from external texture companion — `EXE_AND_CORPUS_CONFIRMED` |
| `+0x13` | `u8` | copied to manager `+0xFA`; zero in bounded corpus — runtime-carried `PRESERVED_UNDECODED` |
| `+0x14` | `u32` | legacy resource/provenance code, see below |
| `+0x18` | `u64` | zero corpus + canonical model-source dormancy — `RESERVED_OBSERVED_ZERO`, `PRESERVED_UNDECODED` |
| `+0x20` | `u64` | absolute serialized scene-node-block offset — `STRUCTURAL_CONFIRMED` |
| `+0x28/+0x30/+0x38` | `u64` | zero corpus + canonical model-source dormancy — `RESERVED_OBSERVED_ZERO`, `PRESERVED_UNDECODED` |

Shared manager initializer `0x1402F9570` binds the canonical model-family source and copies the known header lanes. A whole-image source-pointer census, provenance-gated by family classifier `0x1402FD650` (`SCM -> 0x30000000`), closes the secondary header shell as dormant in the audited canonical runtime. Equal raw displacements in unrelated owners are not evidence.

### Header `+0x14` — legacy resource/provenance code

`0x1402F9570` copies the value to manager `+0xE4`. Across the preserved SCM corpus it has a stable arithmetic decomposition:

```text
raw = family_class * 100000
    + model_set    * 100
    + sub_index
```

Canonical C++ API:

```text
LegacyResourceCode
decode_legacy_resource_code()
encode_legacy_resource_code()
```

The component split is `CORPUS_CONFIRMED`; runtime carry is `EXE_CONFIRMED`. The original source-level names for family classes 3/4 are not present in current evidence, so DMC Rengine deliberately keeps neutral structural names. Lack of guessed class names is not an open reverse blocker.

## Object record — 0x40 bytes

Objects begin at file offset `0x40`, fixed serialized stride `0x40`. The SCM runtime object family uses stride `0x3C0`.

| Offset | Type | Canonical meaning/status |
| ---: | --- | --- |
| `+0x00` | `u8` | mesh count — `EXE_CONFIRMED` |
| `+0x01` | `u8` | alpha-control byte — `EXE_CONFIRMED` |
| `+0x02` | `u16` | total vertex count; equals sum of child mesh vertex counts — `STRUCTURAL_CONFIRMED` |
| `+0x04` | `u32` | zero corpus; source-preserved secondary state — `RESERVED_OBSERVED_ZERO`, `PRESERVED_UNDECODED` |
| `+0x08` | `u64` | absolute mesh-table offset — `EXE_CONFIRMED` |
| `+0x10` | `u32` | source/effective object flags; technical runtime projections below |
| `+0x14..+0x2F` | bytes | zero corpus; preservation-only serialized state — `RESERVED_OBSERVED_ZERO`, `PRESERVED_UNDECODED` |
| `+0x30` | `vec3f` | bounding-sphere center — `CORPUS_CONFIRMED` |
| `+0x3C` | `f32` | bounding-sphere radius — `CORPUS_CONFIRMED` |

The stored radius matches the maximum distance from the stored center to serialized geometry across the confirmed corpus within float tolerance.

### Object `+0x01` — alpha control

SCM initializer `0x140302F10` copies source `+0x01` to runtime object `+0x07`. After the narrow target-specific C4/EA corrections, the common path is:

```text
if effectiveControl <= 0x80:
    runtime override = 0
    MDL_PARTS_COLOR_PKT.alpha.w = effectiveControl / 255
else:
    runtime override = effectiveControl
    MDL_PARTS_COLOR_PKT.alpha.w = 1.0
```

A non-zero low object-mode nibble forces effective control `0x80`. Embedded `DMC3_STG.hlsl` forwards the resulting packet alpha into stage `COLOR0.a`, and relevant pixel variants use that alpha in final output/alpha testing.

Therefore `+0x01` is an alpha-control byte, not a generic unknown class and not plain opacity.

### Object flags `+0x10`

SCM initializer `0x140302F10` plus shared helper `0x140302640` establish the exact source/effective runtime projection.

| Source condition | Terminal technical result |
|---|---|
| low nibble nonzero | helper/render mode; runtime flag projection; effective alpha control forced to `0x80` |
| `0x00000020` | confirmed runtime flag projection; higher-level category intentionally unnamed |
| `0x00010000` | runtime bit + secondary-boolean projection |
| `0x00020000` | runtime bit + unit vector `(1,1,1,0)` initialization |
| `0x00040000` | confirmed runtime flag projection |
| `0x00080000` | confirmed runtime flag projection |
| `0x00100000` | shared legacy GS packet path selects `TEST_1.AREF 0 <-> 16` and `ZBUF_1.ZMSK 1 <-> 0` in active low-mode path |
| `0x00200000` | observed on 19 SCM objects and runtime-preserved; whole-image immediate/bit-test candidates provenance-rejected; terminal `PRESERVED_UNDECODED` |
| `0x00004000` | legacy GS TEX1 filter: set -> `0` nearest, clear -> `0x60` linear |
| high nibble `0x0F000000` | confirmed high-mode runtime projection |

The current 68-unique corpus union mask is `0x003A0003`. EXE-supported bits need not appear in that corpus.

Source bit `0x00200000` is not called unused or reserved. Its strongest evidence is runtime preservation plus bounded negative consumer evidence, so exact preservation is the terminal contract.

## Mesh record — 0x50 bytes

The physical record is fixed-size `0x50`; helper `0x1402FDD10` and normalizer `0x1403051B0` independently agree with that stride.

| Offset | Type | Canonical meaning/status |
| ---: | --- | --- |
| `+0x00` | `u16` | vertex count — `STRUCTURAL_CONFIRMED` |
| `+0x02` | `u16` | external texture-companion slot index — `EXE_CONFIRMED` |
| `+0x04` | `u16` | GS CLAMP `MINU` |
| `+0x06` | `u16` | GS CLAMP `MAXU` |
| `+0x08` | `u16` | GS CLAMP `MINV` |
| `+0x0A` | `u16` | GS CLAMP `MAXV` |
| `+0x0C` | `u32` | zero corpus; source-preserved, no promoted semantic — `RESERVED_OBSERVED_ZERO`, `PRESERVED_UNDECODED` |
| `+0x10` | `u64` | absolute positions stream offset |
| `+0x18` | `u64` | absolute normals stream offset |
| `+0x20` | `u64` | absolute UV stream offset |
| `+0x28` | `u64` | continuation span: `0x50` except final mesh `0` |
| `+0x30` | `u64` | zero corpus; preservation-only — `RESERVED_OBSERVED_ZERO`, `PRESERVED_UNDECODED` |
| `+0x38` | `u64` | absolute RGB/topology-flags stream offset |
| `+0x40` | `u64` | mesh-relative generated-index-workspace offset |
| `+0x48` | `u32` | runtime-generated index word count; zero in serialized retail corpus before normalization |
| `+0x4C` | `u32` | zero corpus; preservation-only — `RESERVED_OBSERVED_ZERO`, `PRESERVED_UNDECODED` |

### Legacy GS CLAMP

`0x1402F9890` consumes `+0x04/+0x06/+0x08/+0x0A` as the four payload fields of a PlayStation 2 GS CLAMP register. For non-zero `MINU` it packs:

```text
WMS = 3 = REGION_REPEAT
WMT = 3 = REGION_REPEAT
MINU/MAXU/MINV/MAXV = serialized u16 values
```

`MINU == 0` is the runtime disabled/sentinel case. The current retail HD corpus has zero in all four fields, but the executable feature is live.

## Vertex streams

Per mesh:

```text
positions  = f32 x,y,z            stride 12
normals    = f32 x,y,z            stride 12
UV         = i16 u, i16 v         stride 4, scale 1/4096
colorFlags = u8 r,g,b,flags       stride 4
```

The fourth color/flags byte is topology state, not alpha. The confirmed corpus uses `0x00` and `0x02`; runtime topology reconstruction consumes bit `0x02` as the triangle-run break/skip bit. Other bits, if a new corpus exposes them, remain source-preserved until separately evidenced.

## Exact serialized placement

Given the header/object shapes, the remaining physical layout is deterministic:

```text
0x40
  object records [objectCount] x 0x40

for each object:
  mesh records [meshCount] x 0x50
  align16
  positions per mesh; align16 each
  normals per mesh;   align16 each
  UV per mesh;        align16 each
  RGB/topology per mesh; align16 each

scene-node block
align16

for every mesh in object/mesh order:
  generated-index-workspace reserve

EOF
```

This formula reaches exact EOF on all 68 unique canonical retail payloads. No hidden serialized SCM tail/table is required by the confirmed corpus.

## Scene-node block

Header `+0x20` points to:

```text
+0x00 u32 parentRel
+0x04 u32 orderRel
+0x08 u32 objectBindingRel
+0x0C u32 transformRel
+0x10..+0x1F preservation shell
```

Common binder `0x1402F1DB0` resolves the first four arrays into manager pointers.

For `N = sceneNodeCount`:

```text
parentRel        = 0x20
orderRel         = 0x20 + align4(N)
objectBindingRel = 0x20 + 2*align4(N)
transformRel     = align16(0x20 + 3*align4(N))
```

Arrays:

- `parentByOrderPosition`: `i8[N]`;
- `nodeAtOrderPosition`: `u8[N]` permutation;
- `objectBindingByNodeIndex`: `i8[N]`, `-1` or object index;
- `transformByNodeIndex`: `N * 0x20`.

The hierarchy/order/object-binding contract is `EXE_AND_CORPUS_CONFIRMED`. Scene nodes are not equivalent to geometry objects; helper nodes use object binding `-1`.

### Scene shell `+0x10..+0x1F`

The bounded corpus is zero. The family-aware model layout planner reads byte `+0x10` once into a dead local with zero later reads; `+0x11..+0x1F` has no provenance-confirmed model-domain consumer and the raw shell pointer does not escape.

Terminal status:

```text
RESERVED_OBSERVED_ZERO
PRESERVED_UNDECODED
canonical audited runtime effect = dormant / no effect
```

## Scene transform — 0x20 bytes

```text
+0x00 f32 translationX
+0x04 f32 translationY
+0x08 f32 translationZ
+0x0C f32 translationMagnitude
+0x10 f32 rotationX radians
+0x14 f32 rotationY radians
+0x18 f32 rotationZ radians
+0x1C f32 preservation-only fourth lane
```

`translationMagnitude == length(X,Y,Z)` is corpus-confirmed. Runtime translation construction excludes this lane from homogeneous W.

The corrected SCM-specific path is:

```text
0x140303C10
 -> 0x1402FA360
 -> 0x140330450  // X, then Y, then Z
 -> Rz * Ry * Rx
```

`0x1402FA080` is the MOD/EFM initializer and must not be attributed to SCM.

SCM parent/root matrix setup plus generic world updater `0x1402F9700` closes composition as:

```text
root:
  world[root] = local[root] * rootBase

non-root:
  world[current] = local[current] * world[parent]
```

Thus local-to-world composition is no longer an open SCM reverse target.

Transform `+0x1C` is zero in the canonical corpus and is not consumed by the confirmed SCM local matrix construction. It remains exact-byte-preserved as `PRESERVED_UNDECODED` / `RESERVED_OBSERVED_ZERO`.

## Generated-index workspace

`mesh+0x40` is a mesh-relative offset to a runtime-generated `u16` index workspace, not an opaque command stream.

Serialized capacity:

```text
capacityBytes = align16(6 * (vertexCount - 2))
```

for `vertexCount >= 3`.

The first serialized word is `0x1212`; `mesh+0x48` is zero before normalization. `0x1403051B0` rebuilds the u16 sequence from per-vertex topology flags and writes the generated count to `+0x48`.

DMC Rengine exposes:

- `scm::index_workspace_capacity_bytes()`;
- `scm::generate_triangle_strip_indices()`.

## Texture companion ownership

The runtime binds SCM and textures as separate resources:

```text
manager +0x108 = SCM resource
manager +0x110 = texture companion
companion count = manager +0xEC
mesh.texture_index -> runtime texture table slot, stride 0x40
```

The companion begins with a count/block table and payloads; the confirmed runtime includes a TM2 path plus fallback handling. This closes SCM-side texture ownership/binding. Retail texture rewriting is writer evidence, not a remaining SCM-format semantic reverse blocker.

## Reverse completion boundary

Canonical SCM reverse is complete because every serialized/runtime domain now has a terminal evidence state. The following are deliberately separate and must not be presented as reverse incompleteness:

- broader/production writer authority;
- real size-changing authored SCM acceptance;
- provenance-bound retail texture rewrite;
- PAC/PNST/NBZ authored-resource delivery breadth;
- Native Reader/ModViz edit UX;
- original `dmc3.exe` acceptance of authored SCM;
- Capcom offline-packer equivalence;
- unrecovered official source-level class/member names.

A future SCM reverse pass should begin only for a genuine contradiction, new non-zero corpus class, new format revision, or new executable producer/consumer that changes one of these terminal dispositions.

## Historical corrections

The following older claims are superseded/rejected:

- SCM = MOD alias — `REJECTED`;
- `object+0x01` unknown control/classification — superseded by `alpha_control`;
- `mesh+0x04..+0x0B` reserved — superseded by GS CLAMP REGION_REPEAT;
- `object flag 0x00004000` only opaque descriptor state — superseded by GS TEX1 filtering;
- `object flag 0x00100000` only opaque selector — superseded by GS TEST_1/ZBUF_1 technical semantics;
- nearby `0x00200000` masks are SCM bit-21 consumers — provenance `REJECTED`;
- `0x1402FA080` is SCM transform initializer — `REJECTED`; SCM uses `0x1402FA360`;
- SCM world composition remains open — superseded by corrected hierarchy/world evidence.
