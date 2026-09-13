# DMC3 HD SCM reverse completion audit — 2026-09-13

**Branch:** `reverse/mod-completion-20260907`  
**Canonical executable:** `dmc3.exe`  
**SHA-256:** `e454272ed0fb0247fcbcf300e5d55d7a3e96d50b89b9ffaff81bb978dcbdd082`  
**Scope:** serialized/runtime SCM reverse. Writer breadth, PAC/NBZ delivery, authored-resource acceptance and complete HD-backend shader/resource-key enumeration are separate gates.

## Completion rule

SCM uses the same terminal evidence rule as the rest of DMC Rengine. A field does not require a guessed Capcom source name to be closed. Every serialized byte/bit/domain instead terminates as one of:

- `EXE_CONFIRMED`;
- `CORPUS_CONFIRMED`;
- `EXE_AND_CORPUS_CONFIRMED`;
- `STRUCTURAL_CONFIRMED`;
- `SEMANTIC_CANDIDATE` where that is the strongest honest statement;
- `PRESERVED_UNDECODED`;
- `RESERVED_OBSERVED_ZERO`;
- `REJECTED`.

`RESEARCH_REQUIRED` is not a terminal field state. Backend-selection questions may remain open without reopening the serialized SCM ABI when the owning serialized field already has a terminal technical role.

## Result

**The DMC3 HD SCM serialized/runtime reverse is complete for the canonical project scope.**

The 2026-09-13 renderer pass strengthens that result: live SCM material/object state is now traced through the HD compatibility command pipeline to real D3D11 vertex- and pixel-shader binding.

This does **not** claim:

- Capcom source-symbol recovery;
- original offline-builder equivalence;
- unrestricted/canonical rebuild writer authority;
- original-game acceptance of arbitrary authored SCMs;
- a complete numeric pixel-shader key formula for every auxiliary runtime state;
- exact SCM material-state provenance to every D3D11 SRV/sampler slot.

The last two items are narrow HD-backend selection targets, not unknown SCM file-format fields.

## Expanded retail authority

The direct `st000.pac`–`st003.pac` occurrence census adds **51 structurally valid, SHA-256-distinct SCM payloads** to the evidence base and confirms the same recovered grammar at serialized versions:

```text
0.83 / 0.90 / 1.00 / 1.01
```

Across that expanded stage-PAC set the recovered objects/meshes/nodes/vertices remain compatible with the canonical typed layout. The runtime-source flag `0x00080000` is present on **46 of 345** SCM objects, which is important because its renderer path is therefore retail-live rather than theoretical.

## Header — terminal map

| Offset | Terminal result |
|---:|---|
| `+0x00` | `SCM ` magic — `EXE_CONFIRMED` |
| `+0x04` | versions `0.83 / 0.90 / 1.00 / 1.01` observed under the same structural grammar — `CORPUS_CONFIRMED` |
| `+0x08..+0x0F` | zero on bounded corpus; no typed runtime effect in the bounded source-pointer census — `RESERVED_OBSERVED_ZERO` + `PRESERVED_UNDECODED` |
| `+0x10` | object count -> manager `+0xE8` — `EXE_CONFIRMED` |
| `+0x11` | scene-node count -> manager `+0xEA` — `EXE_CONFIRMED` |
| `+0x12` | serialized texture-count mirror; live runtime count comes from the external texture companion — `EXE_AND_CORPUS_CONFIRMED` |
| `+0x13` | `lighting_reference_node_index`: manager `+0xFA` -> selected scene-node world matrix -> lighting query / `MDL_LIGHT_MAT` constants — `EXE_CONFIRMED` |
| `+0x14` | decimal `LegacyResourceCode = family_class*100000 + model_set*100 + sub_index`; copied to manager `+0xE4`; observed families `3/4/7/8`; high-level manager role remains neutral — structural semantic terminal |
| `+0x18..+0x1F` | zero corpus + bounded model-source dormancy — `RESERVED_OBSERVED_ZERO` + `PRESERVED_UNDECODED` |
| `+0x20` | absolute scene-node-block offset — `STRUCTURAL_CONFIRMED` and runtime-bound |
| `+0x28..+0x3F` | zero corpus + bounded model-source dormancy — `RESERVED_OBSERVED_ZERO` + `PRESERVED_UNDECODED` |

`+0x13` therefore supersedes the historical “runtime-carried undecoded byte” classification. It is a typed authoring domain in the clean IR, constrained to the serialized scene-node range.

## Object record — terminal map

Object stride is `0x40`; SCM runtime object stride is `0x3C0`.

| Offset | Terminal result |
|---:|---|
| `+0x00` | mesh count — `EXE_CONFIRMED` |
| `+0x01` | `alpha_control`; packet alpha/control projection plus bounded C4/EA compatibility rewrites — `EXE_CONFIRMED` |
| `+0x02` | total vertex count = sum child mesh counts — `STRUCTURAL_CONFIRMED` |
| `+0x04..+0x07` | zero/preservation-only in bounded corpus — `RESERVED_OBSERVED_ZERO` + `PRESERVED_UNDECODED` |
| `+0x08` | absolute mesh-table pointer — `EXE_CONFIRMED` |
| `+0x10` | source/effective object flags with source-proven runtime/render projections — `EXE_CONFIRMED` |
| `+0x14..+0x2F` | preservation-only serialized state — `RESERVED_OBSERVED_ZERO` + `PRESERVED_UNDECODED` |
| `+0x30..+0x3B` | bounding-sphere center — `CORPUS_CONFIRMED` |
| `+0x3C` | bounding-sphere radius — `CORPUS_CONFIRMED` |

### Object flag `+0x10`

Terminal technical map includes:

- low nibble -> runtime helper/render mode; non-zero forces effective alpha control `0x80` — `EXE_CONFIRMED`;
- `0x00000020` -> recovered runtime flag projection — `EXE_CONFIRMED`;
- `0x00010000` -> runtime bit/secondary-boolean projection — `EXE_CONFIRMED`;
- `0x00020000` -> runtime bit plus `(1,1,1,0)` initialization — `EXE_CONFIRMED`;
- `0x00040000` -> runtime flag projection — `EXE_CONFIRMED`;
- `0x00080000` -> runtime bit `0x20` -> `runtimeObject+0x05` selector `2` (otherwise `3`) -> compatibility command `0x5C000002/3` — `EXE_CONFIRMED`;
- `0x00100000` -> legacy GS `TEST_1.AREF 0↔16` and `ZBUF_1.ZMSK 1↔0` in the recovered helper path — `EXE_CONFIRMED`;
- `0x00200000` -> preserved through runtime source/effective flags, but no source-proven terminal consumer — `PRESERVED_UNDECODED` with bounded negative evidence;
- `0x00004000` -> legacy GS `TEX1_1`: nearest `0x00` instead of default linear `0x60` — `EXE_CONFIRMED`;
- high nibble `0x0F000000` -> recovered runtime mode projection — `EXE_CONFIRMED`.

### `0x00080000` compatibility consequence

The exact jump table at `0x1400446C4` maps compatibility selectors:

| Selector | VS base key |
|---:|---:|
| 2 | 13 |
| 3 | 13 |
| 4 | 5 |
| 5 | 8 |
| 6 | no shader-bind path |
| 7 | 10 |
| 8 | 11 |
| 9 | 9 |
| 10 | 7 |
| 11 | 12 |
| 12 | 6 |

SCM source initialization currently produces selectors `2/3`. Both alias VS base key `13`, so source bit `0x00080000` changes compatibility state without switching the base VS family.

## Mesh record — terminal map

Mesh stride is `0x50`.

| Offset | Terminal result |
|---:|---|
| `+0x00` | vertex count — `STRUCTURAL_CONFIRMED` |
| `+0x02` | external texture-companion slot index; runtime descriptor stride `0x40` — `EXE_CONFIRMED` |
| `+0x04/+0x06/+0x08/+0x0A` | legacy GS CLAMP `MINU/MAXU/MINV/MAXV`, `WMS=WMT=REGION_REPEAT` — `EXE_CONFIRMED` |
| `+0x0C` | corpus-zero, no promoted source semantic — `RESERVED_OBSERVED_ZERO` + `PRESERVED_UNDECODED` |
| `+0x10` | positions `float3[]` — `EXE_AND_CORPUS_CONFIRMED` |
| `+0x18` | normals `float3[]` — `EXE_AND_CORPUS_CONFIRMED` |
| `+0x20` | signed fixed-point UV `i16/i16`, scale `1/4096` — `EXE_AND_CORPUS_CONFIRMED` |
| `+0x28` | continuation span `0x50` non-final / `0` final — `EXE_AND_CORPUS_CONFIRMED` |
| `+0x30` | corpus-zero/preservation-only — `RESERVED_OBSERVED_ZERO` + `PRESERVED_UNDECODED` |
| `+0x38` | RGB + topology-flags stream; bit `0x02` breaks/skips strip run — `EXE_AND_CORPUS_CONFIRMED` |
| `+0x40` | mesh-relative generated-index workspace — `EXE_CONFIRMED` |
| `+0x48` | runtime-generated index count; retail serialized value zero before normalization — `EXE_AND_CORPUS_CONFIRMED` |
| `+0x4C` | corpus-zero/preservation-only — `RESERVED_OBSERVED_ZERO` + `PRESERVED_UNDECODED` |

## Real SCM material packet

The material producer `0x1402F99B0 / 0x1402F9AC0` builds a real legacy-GIF compatibility packet:

```text
GIFtag = 0x4000000000008001
REGS   = 0x000000000020EEEE
NLOOP  = 1
EOP    = 1
FLG    = PACKED
NREG   = 4
```

Four A+D writes:

```text
0x06 -> TEX0_1
0x14 -> TEX1_1
0x08 -> CLAMP_1
0x34 -> MIPTBP1_1
```

Ownership is source-bound as:

```text
mesh texture_index +0x02 -> runtime descriptor +0x20 -> TEX0_1
object flag 0x00004000 -> nearest/linear -> TEX1_1
mesh +0x04..+0x0B -> REGION_REPEAT -> CLAMP_1
runtime texture descriptor +0x28 -> MIPTBP1_1
```

The SCM material packet does **not** use A+D address `0x7D`; treating the separate backend `0x7D` handler as SCM-specific shader authority is `REJECTED`.

## Compatibility command → D3D11 shader bind

Closed provenance:

```text
0x140309DF0 -> 0x57000000
  -> active converter 0x57
  -> 0x5C000002 / 0x5C000003
  -> compatibility selector
  -> 0x5B00001C + mesh descriptor
  -> 0x140032D7A
  -> 0x140044310
  -> 0x140045FE0
      -> 0x14004C340 -> ID3D11DeviceContext::VSSetShader
      -> 0x14004C140 -> ID3D11DeviceContext::PSSetShader
```

Status: `EXE_CONFIRMED_TO_D3D11_SHADER_BIND`.

The exact **PS key formula** remains narrower than this bind proof: auxiliary mesh/runtime state and conditional `+0x20/+0x40` variants participate. This is a backend-selection frontier, not an unknown serialized SCM field.

## D3D11 texture-resource boundary

Backend texture/SRV/sampler wrappers around `0x14003F7A0 / 0x14003F580` are confirmed renderer infrastructure. What is not yet promoted is the complete provenance-clean bridge:

```text
SCM TEX0_1/TEX1_1/CLAMP_1/MIPTBP1_1
  -> exact compatibility resource state
  -> exact D3D11 SRV/sampler slots
  -> backend wrapper calls
```

Canonical status: backend family confirmed; `SRV_SAMPLER_SOURCE_BINDING_OPEN` for the exact SCM source-to-slot mapping.

## Scene-node and transform closure

The scene block remains:

```text
+0x00 parentByOrderPosition
+0x04 nodeAtOrderPosition
+0x08 objectBindingByNodeIndex
+0x0C transformByNodeIndex
```

All four arrays are runtime-bound by `0x1402F1DB0`. Transform records remain:

```text
+0x00 float3 translation
+0x0C float translation_magnitude
+0x10 float3 rotation_xyz_radians
+0x1C preservation-only lane
```

Closed matrix path:

```text
0x140303C10 -> 0x1402FA360 -> 0x140330450 (Rz*Ry*Rx)
-> 0x140031200 -> 0x1402F9700 -> world = local * parentOrRootWorld
```

`+0x1C` remains `RESERVED_OBSERVED_ZERO / PRESERVED_UNDECODED`.

## Generated-index workspace

Closed contract:

```text
capacityBytes = align16(6 * (vertexCount - 2))
serialized first u16 = 0x1212
serialized generated count = 0
runtime normalizer 0x1403051B0 reconstructs u16 indices
```

## Historical hypotheses rejected/superseded

Current authority supersedes or rejects:

- SCM is a MOD alias — `REJECTED`;
- object `+0x01` is a plain opacity/unknown classification byte — superseded by `alpha_control`;
- mesh `+0x04..+0x0B` is reserved — superseded by GS CLAMP REGION_REPEAT;
- object flag `0x00004000` merely selects opaque `0/0x60` — superseded by TEX1 nearest/linear;
- `+0x13` is only runtime-carried undecoded state — superseded by `lighting_reference_node_index` and lighting-matrix provenance;
- runtimeObject`+0x05` producer is unknown — superseded by source flag `0x00080000` -> selector `2/3`;
- A+D handler `0x7D` is the SCM material/shader path — `REJECTED`;
- nearby `AND 0x00200000` sites prove SCM bit-21 semantic — `REJECTED` by pointer provenance;
- `0x1402FA080` initializes SCM transforms — `REJECTED`; SCM uses `0x1402FA360`.

## Reverse-complete boundary

The following do not reopen the serialized SCM reverse by themselves:

- production writer promotion;
- size-changing authored SCM acceptance;
- texture-companion rewrite breadth;
- PAC/PNST/NBZ reintegration;
- Native Reader/ModViz editing UX;
- original `dmc3.exe` authored-resource acceptance;
- Capcom offline-builder equivalence;
- exact artistic/source names absent from executable evidence;
- complete enumeration of PS key variants;
- exact D3D11 SRV/sampler slot binding, provided the serialized material-state ownership remains unchanged.

The renderer-specific continuation is now deliberately narrow: finish the PS-key formula and SCM material-state → SRV/sampler slot provenance. The detailed authority is recorded in `dmc3-scm-hd-command-to-d3d11-2026-09-13.md` and `data/reverse/dmc3-scm-hd-command-compatibility-pipeline-20260913.json`.