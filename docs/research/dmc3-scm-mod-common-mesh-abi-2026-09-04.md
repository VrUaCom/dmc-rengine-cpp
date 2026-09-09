# DMC3 HD SCM/MOD common model-mesh ABI — 2026-09-04

**Branch:** `model-family`  
**Status:** STRUCTURAL CROSS-FORMAT EVIDENCE  
**Scope:** identify only the serialized mesh fields that are genuinely shared by the recovered SCM and MOD revisions.

## Result

SCM and MOD use a closely related 0x50-byte mesh-record family. This supports one semantic/model toolkit, but does not make the full records interchangeable.

### Shared core

| Offset | SCM mesh | MOD inner mesh | Shared semantic |
|---:|---|---|---|
| `+0x00` | `u16 vertex_count` | `u16 element_count` | element/vertex count |
| `+0x10` | position stream field | position stream field | `float3[count]` positions |
| `+0x18` | normal stream field | normal stream field | `float3[count]` normals |
| `+0x20` | UV stream field | UV stream field | `int16x2[count]`, scale `1/4096` |
| `+0x40` | generated-topology workspace field | generated-topology workspace field | runtime topology workspace |
| `+0x48` | generated index count | generated topology count | runtime-generated topology count |

Both records have stride:

```text
0x50
```

The common stream element sizes are also identical:

```text
position = 12 bytes
normal   = 12 bytes
UV       = 4 bytes
UV scale = 1 / 4096
```

These facts are exposed by `formats/model_mesh_core.hpp`.

## Divergent extension lanes

The shared core must not hide meaningful format-specific differences.

### SCM

SCM uses additional lanes for stage/static rendering state, including:

- `+0x02` external texture slot index;
- `+0x04..+0x0B` legacy GS CLAMP REGION_REPEAT parameters;
- `+0x28` recovered continuation contract;
- `+0x38` RGB + topology/control stream;
- object-level alpha and texture-filter state outside the inner mesh core.

### MOD

MOD uses corresponding extension space for skeletal-model data, including:

- `+0x28` `u8x4[count]` BLENDINDICES-correlated matrix-row indices;
- `+0x30` `u16[count]` packed three-way skin weights plus independent `0x8000` topology/control bit;
- skeleton/node domain reached through the model document block.

Therefore the evidence supports:

```text
shared mesh ABI core + format-specific extensions
```

and rejects:

```text
full SCM record == full MOD record
```

## Architectural consequence

The correct DMC Rengine design is one Model Family semantic surface with separate SCM and MOD binary adapters.

Common code can safely begin with:

- positions;
- normals;
- fixed-point UV conversion;
- common 0x50 record-domain constants;
- node/transform presentation;
- geometry inspection;
- ModViz selection/visualization contracts;
- common validation/diagnostic vocabulary.

The following remain adapter-specific until stronger evidence proves common runtime implementation:

- topology control encoding;
- texture/material state;
- skinning;
- writer layout details;
- generated-workspace capacity policy;
- object/group record semantics.

## Next proof target

Compare the canonical EXE normalizers/consumers for SCM and MOD around their `+0x10/+0x18/+0x20/+0x40/+0x48` fields to determine whether they share one internal mesh helper or only preserve a historical serialized ABI. Only then should more binary code be physically deduplicated.
