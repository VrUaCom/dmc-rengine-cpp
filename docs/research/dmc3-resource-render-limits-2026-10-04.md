# DMC3 resource and rendering limits — evidence ledger

Date: 2026-10-04
Canonical executable SHA-256: e454272ed0fb0247fcbcf300e5d55d7a3e96d50b89b9ffaff81bb978dcbdd082

## Rule

Do not collapse these domains into one "engine maximum":

1. PS2/GS hardware encoding limits.
2. DMC3 serialized-format representable limits.
3. DMC3 runtime/allocator limits proven from the canonical HD executable.
4. Retail-corpus observed maxima.
5. HD/D3D11 authoring/runtime domain.

A representable maximum is not a proven runtime-safe maximum.

## Geometry / SCM

Confirmed serialized fields:
- header object_count: u8 -> representable 0..255.
- header scene_node_count: u8 -> representable 0..255.
- header texture_slot_count: u8 -> representable 0..255.
- object mesh_count: u8 -> representable 0..255.
- object total_vertex_count: u16 -> representable 0..65535.
- mesh vertex_count: u16 -> representable 0..65535.

These are STRUCTURAL_CONFIRMED encoding ceilings, not claims that the original game accepts all maximum values.

Hash-bound retail SCM corpus:
- 68 unique SCM payloads.
- 254 objects.
- 481 meshes.
- 328 scene nodes.
- 182,612 serialized vertices total across the corpus.
- largest SCM file: st114.scm, 1,038,816 bytes.
- vertices per mesh observed: 3..10,196.
- texture-slot counts observed: 6, 16, 17.
- mesh count per object observed: 1..10.

The consolidated corpus receipt also records a largest per-file vertex total of 27,057 for st114.scm.

Status: CORPUS_CONFIRMED for observed values; STRUCTURAL_CONFIRMED for field widths.

## SCM normalizer and runtime allocation ceiling

Fresh canonical-EXE analysis closes a second limit domain that is independent of
the per-frame D3D11 ring.

### Normalizer count domain

SCM normalizer `0x1403051B0` reads `mesh.vertex_count` as a `u16`, zero-extends
it and compares a 32-bit loop counter directly against that value. Generated
workspace indices are written as `u16`.

At the serialized maximum `vertex_count = 65,535`, the largest source vertex
index is `65,534`, which is still exactly representable as `u16`. No lower
explicit vertex-count cap was found in this normalizer.

The canonical workspace policy

`align16(6 * (vertexCount - 2))`

also provides the required worst-case storage domain for its generated u16
sequence.

This proves that the normalizer itself does **not** lower the per-mesh u16
representational ceiling. It does not by itself prove that a 65,535-vertex mesh
is universally gameplay-safe because the complete runtime blob and per-frame
renderer budgets are separate limits.

Status: EXE_CONFIRMED_NORMALIZER_U16_DOMAIN.

### Runtime blob planner

For SCM family mask `0x30000000`, `0x1402FD8D0` computes the runtime allocation
as:

`align16(commonSize + scmSpecificSize)`

using common planner `0x1402FD9C0` plus SCM planner `0x1402FDD10`.

The SCM-specific term is exactly:

`0xA0 + 0x740*O + 0x220*M + 0x100*sum(ceil(V_i/60)) + 2*A`

where:

- `O` = object count;
- `M` = total mesh count;
- `V_i` = each mesh vertex count;
- `A` = the auxiliary size returned by `0x1402FE1B0` for one of the two passes.

The `0x100*ceil(V/60)` contribution comes from two passes, each allocating
`0x80` bytes for every 60-vertex chunk.

The common term is:

`0xB0 + 0xE0*N + 0x40*T + optionalTextureState`

with `N` scene nodes and `T` texture-companion entries. When the runtime
texture-state flags are non-zero, the optional term includes `0x20`,
`0x50*T*L` for the first parsed texture level count `L`, and an additional
`0x30*T` when flag bit `0x02` is active.

One mesh at the serialized maximum of 65,535 vertices contributes only
`0x44F00 = 282,368` bytes to the SCM-specific term for a one-object/one-mesh
resource before common and auxiliary terms. Therefore this runtime-size planner
also does not, by itself, reduce the one-mesh u16 ceiling.

### Game runtime arena and SCM pool route

Startup `0x140030190` requests `0x10400000` bytes through
`0x1400490D0`, which is the canonical `VirtualAlloc` reserve/commit wrapper.
That is a 260 MiB top-level block.

The game then initializes a `0x10000000` = 256 MiB master arena at
`0x1402C60E0..0x1402C6119`.

SCM does **not** allocate its runtime blob from that entire 256 MiB capacity.
At `0x1400899FB..0x140089A12` it requests selector `-2` through allocator
router `0x1402C6150`.

Normal selector-`-2` routing selects pool index 1:

- span: `0x500000` = 5 MiB;
- block size: `0x400` = 1,024 bytes;
- alignment: 16 bytes;
- occupancy slots: **5,114**;
- usable block payload in an empty pool: **5,236,736 bytes = 0x4FE800**.

The allocator rounds a request to
`ceil(requestBytes / 1024)` blocks and `0x1403374A0` must find one contiguous
free run. Fragmentation or existing occupants can therefore lower the actual
available maximum.

There is also a live routing override. When byte `0x140CA8AB1 == 1`, the same
selector class is forced to pool index 2:

- span: 4 MiB;
- block size: 512 bytes;
- alignment: 64 bytes;
- slots: **8,175**;
- empty-pool usable payload: **4,185,600 bytes = 0x3FDE00**.

Thus there is no honest single statement such as "SCM has 5 MiB available".
The strongest current statement is:

> SCM runtime materialization is bounded by a block-pool route whose normal
> empty-pool contiguous ceiling is 5,236,736 bytes, with a live override route
> that lowers the corresponding empty-pool ceiling to 4,185,600 bytes.

The final accepted resource must also satisfy the common/texture planner,
auxiliary validation, current pool occupancy and contiguous-run availability.

Reproducer:
`scripts/reverse/audit_scm_runtime_allocation_limits.py`

Machine receipt:
`data/reverse/dmc3-scm-runtime-allocation-limit-20261004.json`

Status: EXE_CONFIRMED_SCM_RUNTIME_ALLOCATION_LIMIT_PATH.

## Retail reconstructed triangle workload

A reproducible census now applies the same topology-break rule used by
`src/formats/scm_topology.cpp` to the hash-bound 68-unique-SCM corpus.

Population:
- 68 unique SCM files;
- 481 meshes;
- 182,612 serialized vertices;
- **114,051 reconstructed non-degenerate triangles**.

Largest file workload:
- `st114.scm`;
- SHA-256 `fd3ade343a5cac15a174fbdfc2ff3d848f33c0928bcef171eec0ca6e19c1cd7d`;
- 41 objects / 72 meshes;
- 27,057 vertices;
- **16,248 triangles**.

Largest individual mesh workload:
- `m20_b00_004_000.scm`, object 17 / mesh 1;
- 10,196 vertices;
- **5,696 triangles**.

For comparison with the EXE-confirmed shared normal-frame dynamic vertex ring,
if every triangle in `st114.scm` were submitted in one rendered frame and no
other dynamic draw consumed the ring, its byte use would be:

| SCM output stride | bytes/triangle | st114 bytes | fraction of 0x3C0000 |
|---:|---:|---:|---:|
| 20 | 60 | 974,880 | 24.79% |
| 28 | 84 | 1,364,832 | 34.71% |
| 36 | 108 | 1,754,784 | 44.63% |

This is deliberately a conservative file-level comparison, not a claim that all
geometry in an SCM is visible/submitted every frame. Multiple SCM resources and
non-SCM dynamic draws may share the same rendered-frame ring.

Reproducer:
`scripts/reverse/census_scm_triangle_budget.py`

Machine receipt:
`data/reverse/dmc3-scm-triangle-budget-census-20261004.json`

Status: CORPUS_CONFIRMED using the EXE-confirmed topology rule.

## Generated-index workspace

For each SCM mesh:
capacityBytes = align16(6 * (vertexCount - 2))
serialized sentinel = 0x1212
generated_index_count is zero before runtime normalization.

The canonical normalizer reconstructs u16 indices from topology flags. This means vertex count, generated-index capacity, and transient normalized geometry cost must be treated together when stress-testing higher geometry loads.

Status: EXE_AND_CORPUS_CONFIRMED.

## HD D3D11 draw path

Direct canonical-EXE analysis of 0x140043F90 establishes a real D3D11 draw wrapper.

Key behavior:
- mode 5 binds an index buffer through ID3D11DeviceContext::IASetIndexBuffer;
- the bound index format is DXGI_FORMAT_R32_UINT (numeric value 42);
- mode 5 then calls DrawIndexed;
- non-mode-5 paths call Draw;
- D3D11 Draw/DrawIndexed count arguments are 32-bit UINT values on this path.

For mode 5 the wrapper derives quad groups as floor(vertexCount / 4) and issues
DrawIndexed(6 * quadCount, 0, 0).

This proves that the HD GPU backend is not restricted to the serialized SCM u16
index representation. It does not prove that arbitrary SCM meshes can exceed their
serialized u16 count fields.

Status: EXE_CONFIRMED.

## HD dynamic vertex ring — corrected EXE result

Fresh direct analysis of the canonical executable supersedes the earlier
"98,304 vertices independent of stride" wording.

Relevant ranges:
- common dynamic vertex-buffer initialization: 0x140042E79..0x140042EC7;
- dynamic vertex conversion/upload: 0x140043190..0x14004378A;
- draw wrapper: 0x140043F90..0x140044114.

The common dynamic D3D11 vertex buffer has a fixed byte capacity:

ByteWidth = 0x3C0000 = 3,932,160 bytes.

The upload path derives an output stride from the active compatibility vertex flags,
maps the shared buffer, converts/copies vertices, unmaps it, binds it through
IASetVertexBuffers and advances renderer+0x17A0 by:

consumedBytes = vertexCount * outputStride.

Observed output-layout examples from the EXE input-layout/vertex conversion path:
- flags 0x00000002 -> stride 12;
- flags 0x40000002 -> stride 16;
- flags 0x00000102 -> stride 20;
- flags 0x00000042 -> stride 28;
- flags 0x00000142 -> stride 36;
- additional registered layouts include stride 40, 48 and 56.

Therefore 98,304 vertices is only the quotient 0x3C0000 / 40 for one 40-byte
layout. There is no layout-independent 98,304-vertex ceiling.

Map mode is selected from the tracked offset:
- offset == 0 -> discard-style map;
- offset != 0 -> no-overwrite-style map.

No preflight test of (currentOffset + vertexCount*outputStride) against 0x3C0000
was found inside 0x140043190. Capacity safety is therefore a caller/lifecycle property,
not a local guard in the upload routine.

Status: EXE_CONFIRMED_FIXED_BYTE_CAPACITY; previous layout-independent vertex-count
interpretation REJECTED.

## SCM geometry -> HD draw path

The canonical EXE now supplies a provenance-clean SCM-specific route rather than a
generic renderer inference.

SCM geometry GIF construction at 0x14030A320..0x14030A38D emits a PRE-enabled
PACKED stream with register descriptors ST / RGBAQ / XYZF2. The PRIM/control
formula forces the low three primitive bits to numeric value 5.

The compatibility dispatcher maps packed register descriptor 4 (XYZF2) to
0x14002D6F6. That handler updates the compatibility XYZ state and reaches
0x14002B160, whose primitive jump table maps low-three-bit primitive value 5 to
0x14002B56E.

The 0x14002B56E path assembles one triangle into compatibility scratch and calls
the real D3D11 draw wrapper 0x140043F90 with exactly three output vertices.
The recovered SCM-compatible output variants are:

| vertex flags | output stride | bytes per 3-vertex triangle | empty-ring full triangles |
|---:|---:|---:|---:|
| 0x00000102 | 20 | 60 | 65,536 |
| 0x00000042 | 28 | 84 | 46,811 |
| 0x00000142 | 36 | 108 | 36,408 |

The draw wrapper uses non-indexed mode 3 for this path. Its topology lookup resolves
to numeric topology value 4, and Draw is issued with vertexCount=3. Thus the recovered HD compatibility path receives standard GS `PRIM=5 = TRIANGLE_FAN` state and converts each drawable fan triangle into an independent three-vertex D3D11 triangle-list draw.

These triangle counts are NOT a claim that a stage may contain that many polygons.
They are the arithmetic capacity of an initially empty shared 0x3C0000-byte dynamic
vertex ring for a single recovered SCM output layout. Other compatibility draws consume
the same ring, and lifecycle/reset timing controls reuse.

Important consequence: the SCM serialized u16 vertex ceiling is not automatically a
runtime-safe ceiling. For 28- and 36-byte variants, a sufficiently large continuous fan workload can exhaust the shared byte ring before reaching the u16 representable
maximum unless higher-level reset/chunking intervenes. No such capacity guard exists
inside the recovered upload routine itself.

Status: EXE_CONFIRMED_SCM_TO_D3D11_DRAW_PATH.

## Dynamic ring lifecycle — normal rendered-frame boundary

The reset function containing 0x140042567 clears:
- renderer+0x17A0: shared dynamic vertex-ring byte offset;
- renderer+0x17D8: dynamic u16-index byte offset;
- renderer+0x1810: dynamic u32-index byte offset.

It also toggles renderer state at +0x1828 and clears active offsets for the
16-entry layout-buffer table when their resources are present.

Fresh whole-EXE caller closure identifies the normal scheduling chain:

dmc3_main 0x1402C5DF0
 -> startup 0x1402C5E57 calls 0x140337F70 with mode 1
 -> 0x140337F70 stores mode 1 at graphics-config +0x13
 -> normal timed loop 0x1402C5EB0..0x1402C5FE2
 -> one render-dispatch call 0x140337DF0 at 0x1402C5F85
 -> 0x140337EC0 indexes table 0x1405D1B70 by graphics-config +0x13
 -> mode 1 selects 0x140337FA0
 -> first call in 0x140337FA0 is 0x1400335C0
 -> renderer/input-layout reset clears the three dynamic ring offsets
 -> render work proceeds
 -> dmc3_main eventually jumps back to the timing-loop head.

The dispatch can be skipped/changed by alternate runtime state, so this is not a claim
that every possible application state renders identically. It does prove that the
ordinary mode-1 rendered path resets these dynamic rings once at the start of each
normal rendered loop iteration.

Consequently the 0x3C0000 dynamic vertex ring is best modeled as a **shared normal
per-rendered-frame byte budget**. It is shared with other dynamic compatibility draws;
the SCM-only triangle capacities above are empty-frame upper bounds, not guaranteed
polygon budgets.

Status: EXE_CONFIRMED_NORMAL_RENDERED_FRAME_RESET.

## HD dynamic and prebuilt index buffers

Renderer initialization creates two dynamic index-buffer classes:

- 16-bit index buffer: ByteWidth 0x4E000, element size 2 bytes
  -> 159,744 indices.
- 32-bit index buffer: ByteWidth 0x9C000, element size 4 bytes
  -> 159,744 indices.

It also creates a prebuilt R32_UINT quad index buffer:

- ByteWidth 0x3A8000;
- 958,464 uint32 indices;
- generated as 159,744 quad groups * 6 indices;
- covers 638,976 sequential quad vertices in the generated index pattern.

The existence of this larger prebuilt index table does not override the separate dynamic
vertex upload capacity. The effective draw ceiling for a particular path is the minimum of
all participating resource budgets and caller semantics.

Status: EXE_CONFIRMED.








## Correction: GS primitive 5 is TRIANGLE_FAN

A PS2 GS register cross-check exposed a naming error in the first version of this
pass. Standard GS PRIM encoding is:

```text
0 point
1 line
2 line strip
3 triangle list
4 triangle strip
5 triangle fan
6 sprite
```

The canonical EXE independently confirms the same domain through the jump table
at `0x14002BA98`:

- primitive 0 -> threshold 1;
- 1/2 -> threshold 2;
- 3/4 -> threshold 3;
- **5 -> threshold 3 plus dedicated fan-anchor initialization at 0x14002B20C**;
- 6 -> threshold 2.

The second dispatcher table at `0x14002BAB4` routes primitive 5 specifically
to `0x14002B56E`. That handler consumes the saved first-vertex anchor plus the
current/previous vertices.

Therefore the source-bound SCM geometry formula
`((runtimeObject+0x98 & 0x7F8) | 5)` means **GS TRIANGLE_FAN**, not
TRIANGLE_STRIP.

The earlier TRIANGLE_STRIP label in the 2026-10-04 intermediate evidence is
**REJECTED** and superseded by the corrected fan evidence. The numerical
`N-2` triangle count and three uploaded vertices per emitted triangle remain
unchanged.

This correction does **not** rename the separate serialized SCM topology
workspace algorithm, which may still use strip-style source reconstruction.
It corrects only the HD compatibility GS PRIM=5 render path.

## SCM `ceil(vertexCount/60)` term — planner headroom, not draw batching

The previously ambiguous `ceil(meshVertexCount / 60)` allocation term is now
closed against the complete canonical executable.

### Planner

`0x1402FDD10` computes, for each mesh and for each of two runtime/frame
domains:

```text
+0x40
+0x80 * ceil(meshVertexCount / 60)
```

The exact ceiling division is:

```text
(vertexCount + 0x3B) / 0x3C
```

A whole-image instruction sweep finds this exact `+59 / 60` operation only in
the SCM size planner.

### Constructor

The complete SCM construction sequence is:

```text
0x1402F9570
 -> 0x1402F1DB0
 -> 0x1402FA360
 -> 0x14030D040 / 0x14030D430
 -> 0x1402F9120
 -> 0x1402F92B0
 -> 0x140302F10
 -> 0x1402F9F20
 -> 0x140309C60
 -> align16
 -> manager+0xF0 = actual constructed bytes
```

Neither `0x1402F9F20` nor the other construction stages reproduce the
`/60` mesh loop.

For each mesh in each of the two command domains, `0x140309C60` creates:

```text
one 0x40 material command block
one 0x80 geometry command block
```

The geometry block is a fixed `0x5B00001C` descriptor carried by a 7-qword
inline command. The runtime mesh stores its two frame-domain pointers at
`+0x100/+0x108`.

A whole-image indexed-field census finds:
- SCM write: `0x14030A6A3 -> runtimeMesh+0x100+frame*8`;
- SCM read: `0x140304672 <- runtimeMesh+0x100+frame*8`;
- no SCM consumer deriving `base + chunkIndex*0x80` from that field.

Therefore the extra planner reservation for a mesh is:

```text
plannerHeadroom(mesh)
 = 2 * 0x80 * (ceil(vertexCount/60) - 1)
 = 0x100 * (ceil(vertexCount/60) - 1)
```

for `vertexCount > 0`.

It is allocator headroom reserved beyond the command blocks actually constructed
by this HD path. It must not be described as a 60-vertex D3D11 draw batch.

### Bounded retail-corpus projection

Across the 67 unique SCM payloads inside the exact historical
`DMC 3 RENGINE (6).zip` archive:

- 61/67 have non-zero planner headroom;
- 6/67 have zero headroom;
- aggregate headroom: 610,816 bytes;
- median: 1,792 bytes/file;
- mean: about 9,116.66 bytes/file;
- maximum: **87,808 bytes** in `st001_002.scm`
  (23,049 vertices, 77 meshes).

Other high observations:
- `m20_b00_004_000.scm`: 77,056 bytes;
- `m20_s00_004_032.scm`: 65,024 bytes;
- `st445_005_040.scm`: 58,880 bytes;
- `st445_002.scm`: 57,088 bytes.

Status:
**EXE_CONFIRMED_PLANNER_HEADROOM + CORPUS_CONFIRMED_HEADROOM_CENSUS**.

This does not mean the reserved memory can automatically be reclaimed safely by
Rengine. The original allocator contract requests it, so binary/runtime parity
must preserve that budget unless a separate compatibility patch intentionally
changes the allocation policy.


## Compatibility layout stride code and static draw-call ring census

A fresh whole-EXE pass closes the meaning of compatibility global
`0x1405D95F4` and makes per-call dynamic-ring accounting explicit.

At `0x14002B284..0x14002B2D2`, the compatibility renderer selects these
layout states:

| vertex flags | `0x1405D95F4` | recovered output stride |
|---:|---:|---:|
| `0x00000102` | 5 | 20 bytes |
| `0x00000042` | 7 | 28 bytes |
| `0x00000142` | 9 | 36 bytes |

The dynamic uploader `0x140043190` independently recomputes the same output
strides from the flag bits:

```text
base = 16 if (flags & 0x40000002) == 0x40000002 else 12
+8  if flags & 0x00000100
+16 if flags & 0x00000040
+4  if flags & 0x00020000
```

For the SCM/legacy compatibility primitive handlers, the byte-count argument
to `0x140043F90` is formed from `0x1405D95F4` as:

```text
8  * strideQuarter = 2 * outputStride
12 * strideQuarter = 3 * outputStride
16 * strideQuarter = 4 * outputStride
```

The wrapper divides this byte count by the registered layout stride to recover
vertex count, calls the uploader, and the uploader advances
`renderer+0x17A0` by `vertexCount * outputStride`. For these recovered
layouts, the incoming byte-count argument is therefore the exact dynamic-ring
consumption for that draw.

### Whole-image direct-call census

The canonical EXE has 49 direct callsites to `0x140043F90`.

A bounded local constant-propagation pass resolves flags/mode/byte-count
completely at **42/49** callsites after closing two formerly runtime-looking branches. Three additional compatibility callsites are bounded to exact layout-dependent byte sets, leaving only four genuinely generic runtime-count helpers. Fixed signatures are:

| flags | mode | ring bytes / invocation | direct callsites |
|---:|---:|---:|---:|
| `0x102` | 5 | 80 | 20 |
| `0x102` | 3 | 60 | 17 |
| `0x142` | 5 | 144 | 1 |
| `0x102` | 4 | 60 | 1 |
| `0x42`  | 4 | 84 | 1 |
| `0x102` | 4 | 80 | 1 |
| `0x142` | 5 | 96 | 1 |

If every one of those 42 fixed callsites executed exactly once, they would add
3,084 bytes to the shared ring. **This is not a per-frame occupancy claim**:
many callsites are inside loops/conditional render paths and may execute zero,
one or many times per rendered frame.

Three compatibility primitive callsites are not single constants but are fully bounded by the recovered layout state:

- `0x14002B407`: mode 1, **40 / 56 / 72 bytes**;
- `0x14002B555`: mode 3, **60 / 84 / 108 bytes**;
- `0x14002BA5B`: mode 5, **80 / 112 / 144 bytes**.

The two newly closed fixed sites are:

- `0x14003D8A7`: the draw is reachable only on the `ESI==0` branch, reducing `r9=ESI+0x50` and mode `ESI+5` to **80 bytes, mode 5, flags 0x102**;
- `0x14003F395`: `r13d` is unconditionally set to 1 before the draw and is not rewritten, reducing the call to **80 bytes, mode 5, flags 0x102**.

Only four direct callsites retain genuinely runtime-derived counts:
`0x14003F4C8`, `0x140040344`, `0x140044CDF`, and `0x1400457CD`.
Their exact per-frame occupancy depends on caller-supplied iteration/count state.

Status:
**EXE_CONFIRMED_LAYOUT_STRIDE_MAPPING +
EXE_CONFIRMED_STATIC_CALLSITE_ARGUMENT_CENSUS**.


## Shared dynamic-ring caller census

A whole-image direct-call census closes the ownership question for the HD
dynamic vertex ring:

- `0x140043190` (Map/convert/Unmap/IASetVertexBuffers uploader) has one direct
  executable caller: `0x140043F90`;
- `0x140043F90` has **49 direct callsites** in the canonical executable;
- SCM compatibility primitive handlers are among those callsites
  (`0x14002B407`, `0x14002B555`, `0x14002BA5B`);
- many other direct callsites lie outside that compatibility primitive cluster,
  including the `0x14003Dxxx..0x140041xxx`, `0x140044CDF`,
  `0x1400457CD` and `0x14032C718` regions.

Therefore the dynamic vertex ring at `renderer+0x17A0` is conclusively a
**shared renderer resource**, not an SCM-private allocation.

This closes the structural ownership question but not the dynamic occupancy
question: static reverse can prove who may consume the ring, while the exact
remaining headroom at a particular point in a retail frame requires either a
full scheduler/draw-order reconstruction or runtime instrumentation.

Status: **EXE_CONFIRMED_SHARED_DYNAMIC_VERTEX_RING**.


## Loader allocation envelope vs rendered-frame ring envelope

The canonical executable now gives two different geometry budgets that must not
be conflated.

### Empty-pool SCM allocation envelope

For the normal SCM allocation route, the recovered size planner is:

```text
common =
    0xB0
  + 0xE0 * sceneNodeCount
  + 0x40 * textureCount
  + optional texture-state terms

scmSpecific =
    0xA0
  + 0x740 * objectCount
  + 0x220 * meshCount
  + 0x100 * sum(ceil(meshVertexCount / 60))
  + 2 * auxiliarySize
```

The `ceil(vertexCount/60)` term is therefore an **EXE-confirmed allocation
reserve quantum**. The current pass does not rename every reserved 0x80-byte
sub-block as a draw chunk: the proven HD draw path operates independently as
3-vertex triangle draws.

Using the most geometry-dense structural arrangement for this allocator
(one mesh per object, every object at the u16 total-vertex ceiling, zero
auxiliary bytes, no texture overhead, and one helper/root node in addition to
the geometry objects), the mathematical empty-pool envelope is:

| Route | Empty usable bytes | Max full-u16 objects | Serialized vertices | Planned bytes |
|---|---:|---:|---:|---:|
| normal pool 1 | 5,236,736 | 18 | 1,179,630 | 5,084,336 |
| alternate pool 2 | 4,185,600 | 14 | 917,490 | 3,954,608 |

The next full-u16 object would require 5,366,768 bytes on the normal scenario
and 4,237,040 bytes on the alternate scenario, exceeding the respective
empty-pool usable spans.

These values are **conditional allocator envelopes**, not playable-scene
claims. Existing pool occupancy, fragmentation, textures, auxiliary state,
additional nodes/meshes and other allocations reduce them.

### Worst-case continuous-fan rendered-frame envelope

The SCM HD path converts `TRIANGLE_STRIP` into one non-indexed 3-vertex draw
for each drawable strip triangle. The dynamic vertex ring is fixed at
`0x3C0000 = 3,932,160` bytes and `0x140043190` contains no local
`offset + uploadBytes <= ringSize` clamp or wrap.

The renderer-cluster direct-reference census for `renderer+0x17A0` shows:
reset/initialization, zero/non-zero selection for discard vs no-overwrite,
mapped-write base calculation, IA binding, and the final
`offset += vertexCount * outputStride`. No direct capacity comparison exists
in this upload function.

For one continuous fan starting with an empty ring:

| Output stride | Bytes/triangle | Empty-ring triangles | Max continuous source vertices |
|---:|---:|---:|---:|
| 20 | 60 | 65,536 | 65,535 (u16 SCM ceiling dominates) |
| 28 | 84 | 46,811 | 46,813 |
| 36 | 108 | 36,408 | 36,410 |

A maximum-size u16 fan (`65,535` source vertices -> `65,533` triangles)
would consume:

- 3,931,980 bytes at stride 20, leaving only 180 bytes;
- 5,504,772 bytes at stride 28, exceeding the ring by 1,572,612 bytes;
- 7,077,564 bytes at stride 36, exceeding the ring by 3,145,404 bytes.

This explains why the format/normalizer u16 ceiling is not by itself a safe
rendering ceiling. Topology breaks reduce actual triangle draws; other dynamic
draws in the same rendered frame consume the same ring and reduce available
headroom.

Status:
- allocator formulas/pool routes: **EXE_CONFIRMED**;
- ring size/upload increment/no local guard: **EXE_CONFIRMED**;
- tables above: deterministic projections from those confirmed contracts;
- successful original-game rendering exactly at these projected boundaries:
  **OPEN / requires controlled stress test**.


## SCM TRIANGLE_FAN -> per-triangle D3D11 draw conversion

Fresh canonical-EXE tracing closes the geometry chunking mechanism.

The SCM geometry GIFtag now has a fully source-bound primitive code:
`PRIM=5 = TRIANGLE_STRIP`.

The packed `XYZF2` handler at `0x14002D6F6` does the following for every
incoming geometry vertex:

1. increments compatibility vertex count at `0x1405D9680`;
2. shifts the rolling vertex-state window:
   `0x1405D9630 <- old 0x1405D9628`,
   `0x1405D9628 <- old 0x1405D9620`,
   then publishes the new vertex into `0x1405D9620`;
3. calls primitive dispatcher `0x14002B160` on the drawable XYZF2 path.

For standard GS primitive 5 (**TRIANGLE_FAN**), the dispatcher threshold is exactly **3 vertices** and selects
handler `0x14002B56E`.

That handler submits exactly `3 * 12 = 36` packed source bytes to
`0x140043F90`. The draw wrapper divides by the recovered 12-byte packed input
stride and therefore sends **3 vertices** into `0x140043190`, followed by a
non-indexed triangle draw.

Primitive-5 setup at `0x14002B20C` snapshots the first fan vertex into the dedicated anchor state (`0x1405D9638/0x1405D9658/0x1405D9678`). The XYZF2 handler maintains current/previous state at `0x1405D9620/0x1405D9628` (with their attribute companions). Handler `0x14002B56E` builds each drawable triangle from **anchor + current + previous**. After the first two fan vertices, each additional drawable vertex therefore produces one 3-vertex triangle draw while the anchor remains fixed.

Therefore the HD SCM path is:

```text
SCM TRIANGLE_FAN
 -> XYZF2 stream
 -> rolling last-three vertex window
 -> one D3D11 triangle draw per subsequent fan vertex
 -> 3 uploaded vertices per triangle
```

Consequences:

- an SCM mesh with thousands of vertices is **not uploaded as one giant draw**;
- the 3.75 MiB dynamic vertex ring is consumed cumulatively by duplicated
  per-triangle vertices;
- starting from an empty ring, SCM-only triangle capacity is therefore
  `floor(0x3C0000 / (3*outputStride))`:
  - 20-byte output layout: 65,536 triangles;
  - 28-byte output layout: 46,811 triangles;
  - 36-byte output layout: 36,408 triangles;
- these are per-empty-ring capacities, not stage/game polygon maxima, because
  the ring is shared with other dynamic draws during the rendered frame.

Status: **EXE_CONFIRMED_SCM_TRIANGLE_STRIP_ROLLING_DRAW**.


## SCM runtimeObject+0x98 producer — closed

Fresh whole-EXE analysis closes the producer that was previously left open.

SCM object materialization at `0x140302F10` reads serialized
`object+0x10` flags and calls `0x140302640(runtimeObject, sourceFlags)`.

Inside `0x140302640`:

```text
runtimeObject+0x98 =
    0x1C
  | ((sourceFlags & 0x00010000) == 0 ? 0x20 : 0)
  | ((sourceFlags & 0x0000000F) != 0 ? 0x40 : 0)
```

So the produced runtime values are exactly:

```text
0x1C / 0x3C / 0x5C / 0x7C
```

The geometry tag constructor `0x14030A320..0x14030A38D` then uses:

```text
((runtimeObject+0x98 & 0x7F8) | 5)
```

as the PRE-enabled GIFtag PRIM/control field.

Under the standard GS PRIM bit layout this means:

- primitive code is forced to **5 = TRIANGLE_STRIP**;
- IIP is enabled;
- TME is enabled;
- FGE is enabled when serialized source flag `0x00010000` is **clear**;
- ABE is enabled when the serialized source-flags low nibble is **non-zero**.

This creates a direct provenance-clean chain:

```text
SCM object+0x10
 -> 0x140302F10
 -> 0x140302640
 -> runtimeObject+0x98
 -> 0x14030A320 geometry GIFtag
 -> PRIM/control
 -> compatibility renderer
```

Status: **EXE_CONFIRMED_SERIALIZED_TO_RUNTIME_GS_PRIM_CONTROL**.

This closes the old `runtimeObject+0x98 producer` frontier.


## PTX preflight reservation gate — auxiliary SCM size frontier closed

Fresh canonical-EXE analysis closes the previously open
`0x1402FE1B0 -> 0x1402FE030` frontier.

`0x1402FE1B0` builds a temporary PTX record context from the model's
texture source through `0x140336E90`. `0x1402FE030` then iterates the
resulting 0x50-byte PTX records and counts the active primary allocation plus
an eligible secondary allocation when `record+0x28` exists and
`0x140330FF0(record)` accepts it.

The terminal comparison is:

```text
active_span_count <= ceil_signed(pool[+0xCB14] / 32)
```

For the canonical global PTX pool base `0x140D5FB70`,
`pool+0xCB14 == 0x140D6C684`.

The producer is `0x140331D90`:

```text
blocks << 5 -> pool+0xCB14
```

Therefore, for the normal non-overflow domain, the comparison reduces to:

```text
active PTX primary/secondary spans <= configured reservation blocks
```

This is a texture-placement/reservation gate. It is **not an additional SCM
vertex-count or polygon-count ceiling**.

Status: **EXE_CONFIRMED_PTX_RESERVATION_GATE**.

## Complete preserved DDS census for the v6 research archive

The exact historical archive
`DMC 3 RENGINE (6).zip`
(SHA-256 `7680a9ddb700b958ca1591be0629c2ff1da53efa1b723141bbee0ae4b4c7ff6f`)
contains:

- 243 DDS paths;
- 154 unique DDS payloads by SHA-256;
- 60 unique DXT1;
- 94 unique DXT5;
- 15 unique width/height/compression combinations.

Observed dimensions span 128x64 through 1024x2048.
The unique maximum by pixel area and maximum height is the previously identified
1024x2048 DXT5 texture, with 12 mip levels and 2,796,368 total DDS bytes.

No preserved DDS in this exact bounded archive exceeds width 1024 or height 2048.
This is a **CORPUS_CONFIRMED observed maximum for this archive**, not a claim
that the full retail game contains no larger resource.

Machine authority:
`data/reverse/dmc3-dds-full-corpus-census-20261004.json`.

## 16-bit index count clarification

The 159,744-element dynamic 16-bit index buffer is valid.

A 16-bit index format limits the **numeric value stored in each index** to
0..65,535. It does not limit the buffer to 65,535 index elements. A buffer may
contain 159,744 uint16 entries (319,488 bytes) as long as each entry references
a vertex representable by the 16-bit index domain.

This distinction is important for DMC3 because the executable contains both:
- a 159,744-entry 16-bit dynamic index buffer; and
- a separate 159,744-entry 32-bit dynamic index buffer.

Status: **EXE_CONFIRMED_BUFFER_CAPACITY + TYPE_SEMANTIC_CLARIFICATION**.



## D3D11 device feature-level requirement

The canonical executable imports `D3D11CreateDeviceAndSwapChain` through IAT
`0x14034F650`; the import thunk at `0x1403453EA` has one executable callsite:
`0x140042ADD`.

The callsite reconstructs the complete Win64 argument list. In particular:

```text
pFeatureLevels = &stackFeatureLevel
FeatureLevels  = 1
SDKVersion     = 7
pFeatureLevel  = 0x140BEB3E8
```

Immediately before the call:

```text
0x1400429FC: stackFeatureLevel = 0xB000
```

`0xB000` is `D3D_FEATURE_LEVEL_11_0`.

Therefore DMC3 HD does not ask D3D11 to negotiate down through a list of 10.x/9.x
feature levels on this canonical path: it explicitly requests exactly one level,
**11_0**, and device creation fails if the call returns a failing HRESULT.

This closes an important texture-limit ambiguity. Microsoft defines the
feature-level-11 Texture2D U/V resource limit as **16,384 texels**, which matches
the executable's own recovered pre-`CreateTexture2D` checks:

```text
width  <= 0x4000
height <= 0x4000
```

Thus the 16,384-per-axis guard is not an arbitrary Rengine or DDS-parser number:
it aligns with both the canonical game's requested D3D feature level and the
Direct3D 11 resource domain.

It remains an API/validation ceiling, not a guarantee that a maximal texture can
be allocated alongside the game's other resident resources.

Status: **EXE_CONFIRMED_D3D_FEATURE_LEVEL_11_0**.

## Texture-slot / DDS framing and EXE-backed Texture2D ceiling

DMC3 HD texture slots use descriptor + standard DDS framing. The evidenced slot
descriptor stores width and height in 16-bit halves; that 0xffff-per-axis encoding
domain is only a serialized representation ceiling.

The historical Dmc3DdsSafety authoring envelope (64..1024 per dimension) is also
not an engine ABI ceiling. The preserved retail corpus contains a real
1024x2048 DXT5 image, so 1024x1024 is classified only as a PRODUCT_SAFETY_LIMIT.

Fresh canonical-EXE reverse now closes the real descriptor-wrapped DDS -> D3D11
Texture2D path used by the non-TM2 PTX fallback:

0x1403365B0 PTX parser
 -> 0x140046510 installs/uses texture-resource vtable 0x1404C5388
 -> vtable +0x10 = 0x140046AF0 GPU realization
 -> 0x1400499C0 / 0x140049BA0 DDS loader
 -> 0x140049490 D3D resource creation helper
 -> dimension-3 branch 0x1400495DC
 -> device vtable +0x28 at 0x140049642 = ID3D11Device::CreateTexture2D
 -> optional device vtable +0x38 = CreateShaderResourceView.

The embedded DDS loader performs explicit checks before resource creation:
- mip count <= 0x0F = 15 at 0x140049D51;
- for the Texture2D branch, array size <= 0x800 = 2,048 at 0x140049D97;
- width <= 0x4000 = 16,384 at 0x140049DA4;
- height <= 0x4000 = 16,384 at 0x140049DB0.

Therefore the strongest current static statement is:

**DMC3 HD's recovered PTX descriptor-wrapped DDS path accepts the Texture2D
dimension domain up to 16,384 x 16,384 at its explicit pre-CreateTexture2D
validation layer.**

This is an EXE-side acceptance ceiling, not a promise that an arbitrary
16,384-square authored DXT texture is practical or will survive all memory,
bundle, residency and GPU-allocation constraints. The effective maximum remains
the minimum of this dimension guard, serialized DDS/PTX framing, compressed byte
size, available GPU/system memory and concurrent residency.

For ordinary legacy-header DXT1/DXT5 DMC3 textures, the array-size branch is not
the primary authoring domain; it is retained as a recovered generic DDS-loader
constraint rather than advertised as a DMC3 texture-array feature.

Statuses:
- 1024x2048 retail specimen: CORPUS_CONFIRMED;
- 16,384-per-axis Texture2D pre-create ceiling: EXE_CONFIRMED;
- successful authored original-game acceptance at 16,384 x 16,384: OPEN.

## PTX runtime budgets

Canonical executable reverse evidence currently records:
- PTX runtime payload: fixed 0x208 bytes (520 bytes).
- payload texture-record pointer array: 64 entries.
- global PTX pool: 128 records.
- record stride: 0x50 bytes.
- record array span: 0x2800 bytes.
- record layout feeds D3D11 texture/shader-resource publication paths.

These are HD runtime resource budgets and must not be conflated with PS2 GS limits.

Status: EXE_CONFIRMED.


## PS2 hardware envelope — separate from HD runtime

The original PlayStation 2 hardware domain is now recorded separately from the
HD Collection executable. These are hardware/register constraints, not claims
about DMC3's original PS2 allocator.

### GS local memory and register domains

Cross-check authority:
- PCSX2 `PCSX2/pcsx2@81526d4dc7cc70e4ae75abb35a789417456c6d43`,
  `pcsx2/GS/GSRegs.h` and `pcsx2/GS/GSState.cpp`;
- PS2 Developer Wiki hardware/memory-map documentation.

Confirmed GS structure:
- local GS video memory: **4 MiB = 4,194,304 bytes**;
- `TEX0.TBP0`: 14 bits, block address domain;
- `TEX0.TBW`: 6 bits;
- `TEX0.PSM`: 6 bits;
- `TEX0.TW` / `TH`: 4-bit log2 fields;
- GS specification maximum used by PCSX2: **TW/TH = 10**, therefore
  **1024 x 1024** maximum specified texture dimensions;
- the raw bitfields can encode larger exponents, but that is not the valid GS
  texture-size specification and must not be advertised as hardware support;
- `CLAMP.MINU/MAXU/MINV/MAXV` are 10-bit fields, domain **0..1023**.

The 10-bit GS CLAMP domain directly matches the 10-bit REGION_REPEAT fields
already recovered from SCM. This is strong architectural evidence that those
serialized SCM lanes preserve the original PS2 GS material ABI.

PCSX2 additionally documents a narrower **MTBA automatic mip-base** condition:
for 32-bit swizzled texture formats the maximum automatic MTBA size is 512,
while 16-bit and ordinary 8/4-bit formats may reach 1024. This is a mip-address
generation constraint, not a replacement for the general TEX0 1024x1024
dimension ceiling.

### Why 1024x1024 does not mean a practical 1024x1024 DMC3 texture budget

Ignoring swizzle/alignment/CLUT/mip overhead, a 1024x1024 image requires:

- 32 bpp: 4,194,304 bytes — the **entire GS local memory**;
- 16 bpp: 2,097,152 bytes;
- 8 bpp: 1,048,576 bytes plus palette state;
- 4 bpp: 524,288 bytes plus palette state.

The GS local memory must also hold framebuffer(s), depth buffer, texture
working sets and other render targets. Therefore **1024x1024 is an addressing /
format ceiling, not a realistic simultaneous-residency promise**.

A full mip chain further increases the raw texel footprint by roughly one third
before GS layout/alignment effects. Streaming from main RAM can change
residency over time but does not enlarge the physical 4 MiB GS local-memory
window.

### EE / VU memory context

PS2 memory-map evidence:
- EE main RAM: **32 MiB**;
- VU0 instruction memory: 4 KiB;
- VU0 data memory: 4 KiB;
- VU1 instruction memory: **16 KiB**;
- VU1 data memory: **16 KiB**;
- EE scratchpad RAM: **16 KiB**.

These limits explain why PS2 engines stream geometry through DMA/VIF/VU/GIF
rather than requiring an entire scene to fit in VU memory. VU1's 16 KiB data
memory is a working-set / micro-batch constraint, **not a whole-scene polygon
count**.

### Throughput is not geometry capacity

Published GS peak figures include approximately:
- 2.352 Gpixel/s untextured peak fill;
- 1.2 Gpixel/s with texturing;
- 75 million/s small-polygon peak;
- lower rates for larger textured/Z/alpha/fogged primitives.

These are idealized throughput figures. They do not specify how many polygons
may exist in one DMC3 stage, mesh, resource or frame. Resource size is instead
bounded by game data structures, EE/main-memory allocation, VU/DMA batching,
GS 4 MiB residency and frame-time workload.

### Evidence boundary for original DMC3 PS2

No original DMC3 PS2 `SLUS/SLES/ELF` executable is present in the currently
connected project sources. Consequently:

- PS2 **hardware** limits above are structurally confirmed;
- HD `dmc3.exe` legacy-GS compatibility semantics are EXE-confirmed;
- an exact **DMC3 PS2 game allocator / mesh / texture runtime maximum remains
  OPEN** until the original PS2 executable is acquired and reversed.

Do not use the HD D3D11 16,384x16,384 DDS guard as a PS2 texture limit.
Do not use the PS2 1024x1024 GS ceiling as an HD texture limit.

## PS2 GS vs DMC3 HD

The original PS2 renderer and the HD Collection D3D11 renderer are separate limit domains. Legacy GS texture-register encoding and VRAM constraints cannot be used as the HD DDS maximum, and HD DDS specimens cannot be projected backward as proof of PS2 capability.

For Rengine, every future limit entry must carry:
- domain: PS2_GS / DMC3_SERIALIZED / DMC3_HD_RUNTIME / RETAIL_CORPUS / PRODUCT_SAFETY
- status: EXE_CONFIRMED / CORPUS_CONFIRMED / STRUCTURAL_CONFIRMED / SEMANTIC_CANDIDATE / PRESERVED_UNDECODED / REJECTED
- evidence source
- whether the value is representable, observed, runtime-proven, or merely a tool safety bound.

## Open gates before claiming "maximum DMC3"

1. Close the remaining auxiliary-size validation domain behind 0x1402FE1B0/0x1402FE030 and quantify its contribution for retail PTX companions.
2. Quantify how much of the shared per-rendered-frame ring is consumed by non-SCM dynamic draws in representative retail frames.
3. Determine whether any later runtime consumer imposes a vertex-count restriction below the normalizer's now-confirmed full u16 domain.
4. Close the producer of SCM runtimeObject+0x98, now that the complete canonical EXE is available.
5. Census all texture dimensions and per-slot texture counts from the complete retail resource population.
6. Determine practical allocation/residency failure points below the now-closed 16,384-per-axis Texture2D validation ceiling.
7. Stress-test authored geometry at increasing counts in the original game.
8. Stress-test authored DDS dimensions separately from retail-observed dimensions.
9. Measure simultaneous stage resource residency: geometry + textures + effects, not isolated file maxima.

Until those gates close, do not publish a single number as "the maximum polygons/textures DMC3 can handle."
