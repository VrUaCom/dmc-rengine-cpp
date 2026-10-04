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

## Dynamic ring lifecycle/reset boundary

The function containing 0x140042567 clears:
- renderer+0x17A0: dynamic vertex byte offset;
- renderer+0x17D8: dynamic u16-index byte offset;
- renderer+0x1810: dynamic u32-index byte offset.

The same routine also toggles renderer state at +0x1828 and clears active offsets for
the 16-entry layout-buffer table when their resources are present.

0x1400335C0 calls this renderer reset and then the input-layout reset. A higher
wrapper at 0x140337FA0 calls 0x1400335C0 before renderer work and is present in an
indirect lifecycle/function-pointer table. This is strong evidence for a render-cycle
reset boundary, but the current static pass does not yet promote it to the exact claim
"once per frame".

Status: EXE_CONFIRMED_RESET_BEHAVIOR; exact scheduler cadence remains open.

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

## Texture-slot / DDS framing

DMC3 HD texture slots use descriptor + standard DDS framing. The evidenced descriptor stores width and height in 16-bit halves and the parser currently accepts dimensions up to 0xffff structurally. This is a descriptor encoding domain, not an acceptance/safety claim.

The old Dmc3DdsSafety product envelope (64..1024 per dimension) must NOT be documented as the universal DMC3 HD maximum. Audit of the same historical v6 retail corpus used by the texture work found a real DXT5 DDS at 1024x2048. Therefore a single max_dimension=1024 policy is narrower than the observed HD corpus.

Action: retain the existing conservative authoring guard until a dedicated writer-policy change is tested, but classify 1024x1024 as a PRODUCT_SAFETY_LIMIT, not an engine ABI maximum.

Status of 1024x2048 specimen: CORPUS_CONFIRMED. Original-game runtime acceptance of newly-authored textures at/above this size remains open.

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

1. Close the exact scheduler cadence of 0x140337FA0 / 0x1400335C0 and prove whether the shared dynamic rings reset once per frame or at another render-cycle boundary.
2. Reverse the exact allocation/validation path for oversized SCM object/mesh vertex counts.
3. Determine whether any loader/renderer rejects SCM counts before their serialized integer ceiling.
4. Close the producer of SCM runtimeObject+0x98, now that the complete canonical EXE is available.
5. Census all texture dimensions and per-slot texture counts from the complete retail resource population.
6. Trace D3D11 texture creation failures/guards and maximum mip/resource dimensions in the canonical executable.
7. Stress-test authored geometry at increasing counts in the original game.
8. Stress-test authored DDS dimensions separately from retail-observed dimensions.
9. Measure simultaneous stage resource residency: geometry + textures + effects, not isolated file maxima.

Until those gates close, do not publish a single number as "the maximum polygons/textures DMC3 can handle."
