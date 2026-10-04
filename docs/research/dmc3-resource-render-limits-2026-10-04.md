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

## Texture-slot / DDS framing

DMC3 HD texture slots use descriptor + standard DDS framing. The evidenced descriptor stores width and height in 16-bit halves and the parser currently accepts dimensions up to 0xffff structurally. This is a descriptor encoding domain, not an acceptance/safety claim.

The old Dmc3DdsSafety product envelope (64..1024 per dimension) must NOT be documented as the universal DMC3 HD maximum. Audit of the same historical v6 retail corpus used by the texture work found a real DXT5 DDS at 1024x2048. Therefore a single max_dimension=1024 policy is narrower than the observed HD corpus.

Action: retain the existing conservative authoring guard until a dedicated writer-policy change is tested, but classify 1024x1024 as a PRODUCT_SAFETY_LIMIT, not an engine ABI maximum.

Status of 1024x2048 specimen: CORPUS_CONFIRMED. Original-game runtime acceptance of newly-authored textures at/above this size remains open.

## PTX runtime budgets

Canonical executable reverse evidence currently records:
- PTX runtime payload: fixed 0x258 bytes.
- payload texture-record pointer array: 64 entries.
- global PTX pool: 128 records.
- record stride: 0x50 bytes.
- global pool span: 0x2800 bytes.
- record layout includes D3D11 texture and shader-resource-view pointers.

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

1. Reverse the exact allocation/validation path for oversized SCM object/mesh vertex counts.
2. Determine transient vertex/index buffer allocation and draw splitting behavior.
3. Determine whether any loader/renderer rejects SCM counts before their serialized integer ceiling.
4. Census all texture dimensions and per-slot texture counts from the complete retail resource population.
5. Trace D3D11 texture creation failures/guards and maximum mip/resource dimensions in the canonical executable.
6. Stress-test authored geometry at increasing counts in the original game.
7. Stress-test authored DDS dimensions separately from retail-observed dimensions.
8. Measure simultaneous stage resource residency: geometry + textures + effects, not isolated file maxima.

Until those gates close, do not publish a single number as "the maximum polygons/textures DMC3 can handle."
