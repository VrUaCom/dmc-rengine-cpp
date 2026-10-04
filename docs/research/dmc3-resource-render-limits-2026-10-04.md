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
to numeric topology value 4, and Draw is issued with vertexCount=3. Thus the recovered
HD compatibility path converts the serialized SCM strip topology into independent
three-vertex triangle-list draw batches.

These triangle counts are NOT a claim that a stage may contain that many polygons.
They are the arithmetic capacity of an initially empty shared 0x3C0000-byte dynamic
vertex ring for a single recovered SCM output layout. Other compatibility draws consume
the same ring, and lifecycle/reset timing controls reuse.

Important consequence: the SCM serialized u16 vertex ceiling is not automatically a
runtime-safe ceiling. For 28- and 36-byte variants, a sufficiently large continuous
SCM workload can exhaust the shared byte ring before reaching the u16 representable
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

### Worst-case continuous-strip rendered-frame envelope

The SCM HD path converts `TRIANGLE_STRIP` into one non-indexed 3-vertex draw
for each drawable strip triangle. The dynamic vertex ring is fixed at
`0x3C0000 = 3,932,160` bytes and `0x140043190` contains no local
`offset + uploadBytes <= ringSize` clamp or wrap.

The renderer-cluster direct-reference census for `renderer+0x17A0` shows:
reset/initialization, zero/non-zero selection for discard vs no-overwrite,
mapped-write base calculation, IA binding, and the final
`offset += vertexCount * outputStride`. No direct capacity comparison exists
in this upload function.

For one continuous strip starting with an empty ring:

| Output stride | Bytes/triangle | Empty-ring triangles | Max continuous source vertices |
|---:|---:|---:|---:|
| 20 | 60 | 65,536 | 65,535 (u16 SCM ceiling dominates) |
| 28 | 84 | 46,811 | 46,813 |
| 36 | 108 | 36,408 | 36,410 |

A maximum-size u16 strip (`65,535` source vertices -> `65,533` triangles)
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


## SCM TRIANGLE_STRIP -> per-triangle D3D11 draw chunking

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

For primitive 5, the dispatcher threshold is exactly **3 vertices** and selects
handler `0x14002B56E`.

That handler submits exactly `3 * 12 = 36` packed source bytes to
`0x140043F90`. The draw wrapper divides by the recovered 12-byte packed input
stride and therefore sends **3 vertices** into `0x140043190`, followed by a
non-indexed triangle draw.

The compatibility counter is not reset after every primitive-5 triangle.
Because the XYZF2 handler itself maintains the rolling last-three-vertex window,
after the first two vertices every additional strip vertex produces another
3-vertex triangle draw from the newest triplet.

Therefore the HD SCM path is:

```text
SCM TRIANGLE_STRIP
 -> XYZF2 stream
 -> rolling last-three vertex window
 -> one D3D11 triangle draw per subsequent strip vertex
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
