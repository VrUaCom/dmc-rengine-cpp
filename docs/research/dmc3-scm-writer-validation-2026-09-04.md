# DMC3 HD SCM writer / authoring validation — 2026-09-04

## Scope

This record documents the first evidence-bounded C++20 SCM writer/authoring implementation on branch `scm`.

Canonical runtime authority remains:

- executable: `dmc3.exe`;
- SHA-256: `e454272ed0fb0247fcbcf300e5d55d7a3e96d50b89b9ffaff81bb978dcbdd082`;
- ImageBase: `0x140000000`.

This pass does **not** promote SCM registry maturity to `editable` or `game_validated`. Original-game consumption and full-corpus roundtrip are still required.

## 1. Writer foundation

The parser now materializes authoring payloads rather than keeping only offsets:

- positions: `float32 x/y/z`;
- normals: `float32 x/y/z`;
- UV: exact signed `int16 u/v` serialized representation;
- color/topology: `u8 r/g/b/topologyFlags`.

Unresolved serialized state is explicitly preserved:

- object bytes `+0x14..+0x2F`;
- scene header bytes `+0x10..+0x1F`;
- existing header/mesh reserved fields;
- the recognized original byte image for same-layout preservation.

The implementation intentionally avoids a `memset-and-reconstruct-known-fields` model for parsed resources.

## 2. Writer modes

### `WriteMode::preserve_layout`

Purpose:

- same-size edits;
- source offsets remain authoritative;
- unknown padding/workspace bytes remain byte-preserved;
- object/mesh/node counts and mesh vertex counts may not change.

Expected use:

- vertex position / normal changes;
- UV changes;
- texture index changes;
- alpha-control changes;
- object flag changes such as confirmed TEX1 nearest/linear override;
- GS CLAMP REGION_REPEAT changes;
- node translation/rotation changes.

### `WriteMode::canonical_rebuild`

Purpose:

- deterministic layout derived from typed IR;
- supports changed mesh vertex counts and resulting file-size changes;
- derives object totals, continuation spans, offsets and index workspace positions;
- writes `0x1212` as the recovered index-workspace regeneration sentinel for newly planned workspace regions;
- resets runtime-generated index count in rebuilt mesh records.

This is currently a **DMC Rengine canonical rebuild policy**, not a claim that Capcom's original offline tool emitted exactly the same bytes for every theoretical resource.

## 3. Mandatory post-write gate

Every writer output is immediately reparsed by the canonical SCM parser/validator.

Writer success requires:

```text
serialize
  -> Parser::parse(output)
  -> structural validation
  -> hierarchy validation
  -> stream/workspace validation
```

Failure emits stable diagnostic:

```text
scm.writer-reparse-failed
```

## 4. Dependent-field policy

### Object vertex totals

Writer derives:

```text
object.totalVertexCount = sum(mesh vertex counts)
```

### Mesh continuation

Writer derives:

```text
non-final mesh +0x28 = 0x50
final mesh     +0x28 = 0
```

### Translation magnitude

If serialized translation XYZ is edited, writer emits:

```text
translationMagnitude = sqrt(x*x + y*y + z*z)
```

No-edit preserves the original float bits.

### Bounding radius

If mesh positions or the preserved bounding center are edited, writer recomputes a conservative radius from the existing confirmed center:

```text
radius = max(distance(center, vertex))
```

The writer does **not** invent a new center algorithm because the exact original center-generation policy has not been independently recovered.

## 5. Safe editing API

`scm_edit.hpp` provides bounded edit operations:

```text
set_vertex_position
set_vertex_normal
set_uv / set_uv_raw
set_texture_slot
set_alpha_control
set_texture_filter_nearest
set_region_repeat
set_node_translation
set_node_rotation
```

Safety properties:

- object/mesh/vertex/node indices are range checked;
- non-finite position/normal/transform values are rejected;
- float UV authoring is quantized to signed int16 at the confirmed `1/4096` scale and rejected if not representable;
- texture index is checked against the SCM mirror count when non-zero;
- safe GS CLAMP editing rejects values above the 10-bit hardware width;
- nearest/linear filtering changes only confirmed source bit `0x00004000` and preserves all other object flags.

Unknown source flag `0x00200000` has no semantic setter and remains preserved raw state.

## 6. Real-file baseline currently executable from preserved Library specimens

Two preserved DMC3 HD SCM specimens are directly available to the current validation environment:

| file | size |
|---|---:|
| `st001.scm` | 887,760 |
| `st114.scm` | 1,038,816 |

Observed result from the writer foundation validation pass:

```text
st001.scm
  parse                    PASS
  preserve-layout write    PASS
  preserve byte identity   PASS
  canonical rebuild        PASS
  canonical reparse        PASS
  canonical byte identity  PASS

st114.scm
  parse                    PASS
  preserve-layout write    PASS
  preserve byte identity   PASS
  canonical rebuild        PASS
  canonical reparse        PASS
  canonical byte identity  PASS
```

This is strong positive evidence for the implementation but is **not** a substitute for the required 68+ resource corpus gate.

## 7. Size-changing synthetic acceptance

Regression fixture exercises:

```text
3 vertices
 -> add fourth position
 -> add fourth normal
 -> add fourth UV
 -> add fourth color/topology entry
```

Expected/observed contract:

```text
preserve_layout   -> rejected
canonical_rebuild -> accepted
reparse           -> accepted
mesh vertexCount  -> 4
object total      -> 4
file size         -> changed
```

## 8. Corpus verifier

The normal `dmc-rengine` CLI now exposes:

```text
dmc-rengine verify-scm-corpus <directory> [--json <report.json>]
```

Input bytes are acquired through `LocalDirectorySource` / `SourceRegistry`, preserving the GDSpaces-only resource-access architecture.

For every `.scm`, the verifier records:

- source size;
- objects;
- meshes;
- scene nodes;
- vertices;
- parse status;
- preserve-layout write and byte-identity status;
- canonical write status;
- canonical reparse status;
- canonical byte-identity status;
- first mismatch offset;
- diagnostic count.

JSON schema identifier:

```text
dmc-rengine.scm-corpus-report.v1
```

A non-empty corpus returns success only when every file parses and both no-edit writer paths satisfy their current acceptance contract.

## 9. Current maturity decision

Current evidence supports:

```text
reader              strong
semantic IR         strong
safe same-size edit implemented
writer              experimental
2-file no-edit      bit-identical confirmed
size-changing IR    synthetic reparse confirmed
full corpus         pending
game acceptance     pending
PAC/NBZ reintegration pending
texture companion authoring pending
```

Therefore registry maturity must remain below `editable/game_validated`.

## 10. Next gates

1. Run `verify-scm-corpus` against the complete preserved 68+ unique SCM corpus.
2. Classify every non-identical result as structural error, writer defect, or explained canonical difference.
3. Add texture-companion bundle validation/writer contract.
4. Perform controlled real-resource edits for position, UV, alpha, TEX1 filter, GS CLAMP and transform.
5. Reintegrate through canonical PAC/NBZ Layer-1 authoring.
6. Produce original `dmc3.exe` load/visual/rollback acceptance receipts.

No 100% SCM or production-ready claim is made by this record.
