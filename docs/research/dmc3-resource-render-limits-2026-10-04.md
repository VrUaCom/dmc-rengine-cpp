# DMC3 resource and rendering limits — evidence ledger

Date: 2026-10-04  
Branch: `reverse/dmc3-resource-render-limits-20261004`  
Canonical executable SHA-256: `e454272ed0fb0247fcbcf300e5d55d7a3e96d50b89b9ffaff81bb978dcbdd082`

## Rule

Do not collapse these domains into one "engine maximum":

1. PS2/GS hardware encoding limits.
2. DMC3 serialized-format representable limits.
3. DMC3 runtime/allocator limits proven from the canonical HD executable.
4. Retail-corpus observed maxima.
5. HD/D3D11 authoring/runtime domain.
6. DMC Rengine product safety limits.

A representable maximum is not a proven runtime-safe maximum. A retail-observed maximum is not automatically the runtime ceiling.

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
- largest SCM file: `st114.scm`, 1,038,816 bytes.
- largest per-file vertex total observed: 27,057 in `st114.scm`.
- vertices per mesh observed: 3..10,196.
- mesh count per object observed: 1..10.
- texture-slot counts observed: 6, 16, 17.

Status: CORPUS_CONFIRMED for observed values; STRUCTURAL_CONFIRMED for field widths.

## Generated-index workspace

For each SCM mesh:

```text
capacityBytes = align16(6 * (vertexCount - 2))
serialized sentinel = 0x1212
generated_index_count = 0 before runtime normalization
```

The canonical normalizer reconstructs u16 indices from topology flags. In the no-break triangle-strip case this corresponds to:

```text
generated indices = 3 * (vertexCount - 2)
```

This means serialized vertex count, topology breaks, generated-index capacity and the downstream HD dynamic index budget must be treated together.

Status: EXE_AND_CORPUS_CONFIRMED for the workspace algorithm.

## HD dynamic geometry backend — fresh canonical-EXE pass

### Generic dynamic vertex buffers

Renderer initialization creates two ping-pong dynamic D3D11 vertex buffers.

Per buffer:

- ByteWidth = `0x3C0000` = 3,932,160 bytes = 3.75 MiB.
- Usage = `D3D11_USAGE_DYNAMIC`.
- BindFlags = `D3D11_BIND_VERTEX_BUFFER`.
- CPUAccessFlags = `D3D11_CPU_ACCESS_WRITE`.
- initialization-side stride field = 40 bytes.

The generic streaming upload at `0x140043190` maps one selected buffer, converts the legacy vertex packet to the HD vertex layout, unmaps it, binds it through `IASetVertexBuffers`, and advances the current byte offset by `vertex_count * output_stride`.

The function itself does not contain a local capacity comparison. This is a bounded negative observation only; a caller-side/global rollover guard is still being traced.

Status: EXE_CONFIRMED_GENERIC_HD_BACKEND.

### Dynamic index buffers

Renderer initialization creates two ping-pong buffers for each index width.

R16 set:

- element width = 2 bytes.
- ByteWidth = `0x4E000` = 319,488 bytes.
- capacity = `0x27000` = 159,744 indices per buffer.
- dynamic D3D11 index buffer, CPU-write.

R32 set:

- element width = 4 bytes.
- ByteWidth = `0x9C000` = 638,976 bytes.
- capacity = `0x27000` = 159,744 indices per buffer.
- dynamic D3D11 index buffer, CPU-write.

The upload/draw helper at `0x1400425B0` selects R16 when element width is 2 and R32 otherwise, maps the current dynamic index buffer, copies the incoming index stream, unmaps, binds via `IASetIndexBuffer`, advances its current index offset, and reaches `DrawIndexed`.

Status: EXE_CONFIRMED.

### SCM-specific indexed draw

The source-proven SCM converter at `0x140044310` calls `0x1400425B0` with index-element width exactly 2.

Therefore the confirmed SCM HD render path uses:

- `DXGI_FORMAT_R16_UINT`;
- the 159,744-index dynamic R16 buffer;
- `DrawIndexed`.

Status: EXE_CONFIRMED_SCM_TO_D3D11_R16_INDEXED_DRAW.

This is stronger than the generic backend observation and is directly relevant to the maximum geometry question.

### Derived single-mesh no-break bound

For an unbroken SCM strip:

```text
indices = 3 * (vertices - 2)
dynamic R16 capacity = 159,744 indices
3 * (vertices - 2) <= 159,744
vertices <= 53,250
```

Therefore **53,250 vertices** is a useful derived upper bound for one unbroken mesh only under all of these assumptions:

- the dynamic R16 index buffer starts empty for that allocation window;
- there is no draw splitting;
- there is no caller-side rollover before the upload;
- there are no topology breaks reducing generated indices;
- no previous draw shares the current dynamic index budget.

It is **not** promoted as the final DMC3 maximum.

Status: DERIVED_BOUND_FROM_EXE_CONFIRMED_CAPACITY.

For comparison, the largest retail mesh observed so far has 10,196 vertices. Its worst-case unbroken-strip index requirement would be 30,582 indices, about 19.1% of the 159,744-index buffer.

This explains an important architectural distinction: the serialized u16 mesh vertex count permits 65,535, but the downstream dynamic index budget can become the tighter constraint before that encoding ceiling is reached.

### Generic 32-bit quad path

The generic draw wrapper at `0x140043F90` supports several primitive modes. Its special mode 5 binds a 32-bit index buffer (`DXGI_FORMAT_R32_UINT`) and issues `DrawIndexed`.

A separate static quad-index buffer is built with:

- 159,744 quads;
- 958,464 u32 indices;
- 3,833,856 bytes;
- references up to 638,976 vertices.

This proves the HD renderer itself is not globally limited to 16-bit indices. The R16 restriction above is specifically established for the SCM converter path.

Status: EXE_CONFIRMED_GENERIC_HD_BACKEND.

## Renderer reset / rollover boundary

The reset path around `0x140042270` switches the ping-pong buffer selection and zeros current vertex/index offsets. The reviewed call chain reaches it from `0x1400335C0`.

The exact scheduling semantic — for example whether this corresponds one-to-one with a frame boundary — is not yet promoted. Until that is proven, documentation uses the neutral term **renderer reset window**.

Open question: whether SCM uploads that would cross the 159,744-index budget are split, rejected, rolled over to the alternate buffer, or can overrun the intended allocation domain.

## Texture-slot / DDS framing

DMC3 HD texture slots use descriptor + standard DDS framing. The evidenced descriptor stores width and height in 16-bit halves and the parser structurally accepts dimensions through the u16 domain. This is a descriptor encoding domain, not an acceptance/safety claim.

The old `Dmc3DdsSafety` product envelope (64..1024 per dimension) must NOT be documented as the universal DMC3 HD maximum. Audit of the same historical retail corpus used by the texture work found a real DXT5 DDS at 1024x2048.

Therefore a single `max_dimension=1024` policy is narrower than the observed HD corpus.

Action: retain the conservative writer safety guard until a dedicated writer-policy change is tested, but classify 1024 as a PRODUCT_SAFETY_LIMIT, not an engine ABI maximum.

Status of the 1024x2048 specimen: CORPUS_CONFIRMED. The exact newly-authored runtime acceptance ceiling remains open.

## PTX runtime budgets

Canonical executable reverse evidence records:

- PTX runtime payload: fixed `0x208` = 520 bytes.
- payload texture-record pointer array: 64 entries.
- global PTX pool: 128 records.
- record stride: `0x50` = 80 bytes.
- first 128 records occupy `0x2800` bytes before downstream pool state.
- render finalization publishes texture resources into the HD resource registry.

These are HD runtime resource budgets and must not be conflated with PS2 GS limits.

Status: EXE_CONFIRMED.

## PS2 GS vs DMC3 HD

The original PS2 renderer and the HD Collection D3D11 renderer are separate limit domains.

Legacy GS texture-register encoding and VRAM constraints cannot be used as the HD DDS maximum, and HD DDS specimens cannot be projected backward as proof of PS2 capability.

For Rengine, every future limit entry must carry:

- domain: PS2_GS / DMC3_SERIALIZED / DMC3_HD_RUNTIME / RETAIL_CORPUS / PRODUCT_SAFETY;
- evidence status;
- source artifact/address;
- whether the value is representable, observed, executable-proven, derived, runtime-tested, or merely a tool safety bound.

## Open gates before claiming "maximum DMC3"

1. Close SCM dynamic-index pre-call capacity guard / rollover / split behavior.
2. Prove renderer reset cadence instead of assuming "per frame".
3. Trace SCM-specific vertex-stream storage/capacity; do not substitute the generic 3.75 MiB vertex buffer without source provenance.
4. Reverse exact validation/allocation behavior for oversized SCM object/mesh vertex counts.
5. Census all texture dimensions and per-slot texture counts from the complete retail resource population.
6. Trace the DDS/D3D11 texture creation path to exact dimension/feature-level rejection behavior.
7. Stress-test authored geometry at increasing counts in the original game.
8. Stress-test authored DDS dimensions separately from retail-observed dimensions.
9. Measure simultaneous stage residency: geometry + textures + effects, not isolated file maxima.

Until those gates close, do not publish a single number as "the maximum polygons/textures DMC3 can handle."
