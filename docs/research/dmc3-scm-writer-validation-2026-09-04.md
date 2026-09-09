# DMC3 HD SCM writer / authoring validation — consolidated 2026-09-09

## Scope

This record began as the first evidence-bounded C++20 SCM writer/authoring pass on historical branch `scm`. The implementation and evidence are now consolidated on the existing canonical model-format branch:

`reverse/mod-completion-20260907`

Canonical runtime authority remains:

- executable: `dmc3.exe`;
- SHA-256: `e454272ed0fb0247fcbcf300e5d55d7a3e96d50b89b9ffaff81bb978dcbdd082`;
- ImageBase: `0x140000000`.

This record does **not** promote SCM to `game_validated`, production-ready or `100%`. Original-game acceptance, real size-changing authoring breadth and SCM container reintegration remain separate evidence gates.

## 1. Writer foundation

The parser materializes the authoring payloads required by the current writer:

- positions: `float32 x/y/z`;
- normals: `float32 x/y/z`;
- UV: exact signed `int16 u/v` serialized representation;
- color/topology: `u8 r/g/b/topologyFlags`.

Serialized state that is not semantically decoded is represented or preserved rather than silently discarded:

- object bytes `+0x14..+0x2F`;
- scene header bytes `+0x10..+0x1F`;
- header reserved fields;
- mesh reserved fields;
- scene transform `+0x1C`;
- the original recognized byte image for same-layout preservation.

The implementation deliberately avoids treating zero-filled reconstruction as authority for unknown source bytes.

## 2. Writer modes

### `WriteMode::preserve_layout`

Purpose:

- same-layout editing;
- source offsets remain physical authority;
- unknown padding/workspace bytes remain byte-preserved;
- object/mesh/node counts and mesh vertex counts may not change.

Current typed edits include:

- vertex position and normal;
- UV;
- texture index;
- alpha-control;
- confirmed nearest/linear texture-filter flag;
- GS CLAMP REGION_REPEAT;
- node translation and XYZ rotation.

### `WriteMode::canonical_rebuild`

Purpose:

- deterministic layout derived from typed IR;
- changed mesh vertex counts and resulting layout changes;
- derived object totals, continuation spans, offsets and index-workspace locations;
- `0x1212` regenerated index-workspace sentinel;
- reset of runtime-generated index count in rebuilt mesh records.

This is a **DMC Rengine canonical rebuild policy**, not a claim that Capcom's offline authoring tools would emit identical bytes for every hypothetical edited resource.

## 3. Mandatory post-write gate

Every successful writer output must reopen through the canonical SCM parser/validator:

```text
serialize
  -> Parser::parse(output)
  -> structural validation
  -> hierarchy validation
  -> stream/workspace validation
```

Writer output is not accepted merely because serialization completed.

## 4. Unknown-byte policy for canonical reflow

A layout-changing rebuild can move typed regions, so source-bound bytes that have no modeled reflow policy cannot simply be normalized away.

When retained source bytes exist, `canonical_rebuild` now:

1. reparses the retained source through the canonical parser;
2. marks every source span whose bytes are typed, explicitly raw-preserved, or intentionally regeneratable;
3. scans all remaining source bytes;
4. rejects reflow if any unmodeled remaining byte is non-zero.

Stable diagnostic:

```text
scm.writer-canonical-reflow-unmodeled-nonzero-source
```

Synthetic regression places `0xA5` at source offset `0xF4`, an alignment-padding byte outside the modeled regions. Observed contract:

```text
preserve_layout                     -> PASS, exact source bytes
layout-changing canonical_rebuild   -> REJECT
reported offending offset           -> 0xF4
```

The index workspace is intentionally classified as regeneratable under canonical rebuild and therefore is not treated as an unknown transplant domain.

## 5. Dependent-field policy

### Object vertex totals

```text
object.totalVertexCount = sum(mesh vertex counts)
```

### Mesh continuation

```text
non-final mesh +0x28 = 0x50
final mesh     +0x28 = 0
```

### Translation magnitude

If translation XYZ changes:

```text
translationMagnitude = sqrt(x*x + y*y + z*z)
```

No-edit preserve-layout retains original bits.

### Bounding radius

When geometry or the existing bounding center changes, the current writer derives a conservative radius from that center:

```text
radius = max(distance(center, vertex))
```

It does not invent an unrecovered center-generation algorithm.

## 6. Safe editing API

`scm_edit.hpp` currently exposes:

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

Safety properties include:

- range checks for object/mesh/vertex/node indices;
- rejection of non-finite position, normal and transform values;
- UV quantization to signed int16 at the confirmed `1/4096` scale, failing if not representable;
- texture-index bounds against the SCM texture-count mirror when non-zero;
- 10-bit GS CLAMP field bounds;
- nearest-filter editing limited to confirmed source bit `0x00004000` while preserving all other object flags.

Unknown source flag `0x00200000` has no semantic setter and remains preservation-only.

## 7. Provenance-bound consolidated retail corpus gate

The current corpus verifier is:

```text
dmc-rengine verify-scm-corpus <directory> [--json <report.json>]
```

Retail payload bytes are externally held and are not committed. The machine receipt is:

`data/reverse/dmc3-scm-consolidated-corpus-20260909.json`

Observed result:

```text
paths scanned                         78
unique SHA-256 inputs                 68
duplicate paths                       10
parse                                 78 / 78 PASS
preserve-layout write                 78 / 78 PASS
preserve-layout exact byte identity   78 / 78 PASS
canonical rebuild                     78 / 78 PASS
canonical output reparse              78 / 78 PASS
canonical exact byte identity         78 / 78 PASS
canonical no-edit parity              100%
```

This supersedes the earlier two-file-only baseline (`st001.scm` and `st114.scm`) as the strongest current no-edit writer evidence.

The result proves the current writer reproduces all 68 unique hash-bound inputs byte-for-byte in both no-edit modes. It does **not** prove every possible SCM layout or edited layout accepted by the original game.

## 8. Size-changing synthetic acceptance

Regression expands one synthetic mesh from three to four vertices while extending all four parallel streams.

Observed contract:

```text
preserve_layout   -> rejected as required
canonical_rebuild -> accepted
canonical reparse -> accepted
mesh vertexCount  -> 4
object total      -> 4
edited payload    -> survives reopen
```

This proves the deterministic reflow implementation mechanically for the covered synthetic topology. Real-retail size-changing authoring remains a separate gate.

## 9. Texture companion coherence gate

`ScmResourceBundleWriter` reuses the canonical texture framing and packed-reflow infrastructure rather than introducing another texture writer.

Before SCM output is accepted it requires:

- texture companion framing parse success;
- `SCM header +0x12 == external texture count`;
- every `mesh.texture_index < external texture count`;
- output texture companion framing reparse success;
- no texture-slot count addition/removal in the current safe contract;
- SCM writer/reparse success.

Synthetic regression currently proves:

- coherent SCM + one wrapped texture companion is accepted;
- unchanged companion bytes remain exact;
- mismatched SCM texture-count mirror is rejected;
- out-of-range mesh texture index is rejected.

The texture reflow implementation exists behind this bundle gate, but a provenance-bound retail SCM texture rewrite has **not** yet been promoted by this record.

## 10. Current maturity

Current evidence supports:

```text
reader                                strong
semantic IR                           strong
safe same-layout edit API             implemented
writer                                experimental
retail no-edit corpus                 78 paths / 68 unique PASS
preserve-layout retail byte parity    68 / 68 unique PASS
canonical no-edit retail byte parity  68 / 68 unique PASS
canonical output reparse              68 / 68 unique PASS
size-changing rebuild                 synthetic PASS only
unknown-byte reflow protection        fail-closed regression PASS
texture companion coherence           synthetic bounded PASS
retail texture rewrite                pending
SCM PAC/PNST/NBZ reintegration        pending
original dmc3.exe acceptance          pending
full SCM writer authority             false
```

Registry maturity must therefore remain below game-validated/production authoring authority.

## 11. Next evidence frontier

Do not repeat the already-closed 68-unique no-edit corpus pass. The next useful SCM evidence is:

1. provenance-bound real SCM same-layout edits across representative domains: geometry, UV, alpha/filter, GS CLAMP and transforms;
2. provenance-bound retail texture-companion authoring where texture state actually changes;
3. real size-changing SCM canonical rebuild with exact preservation accounting;
4. PAC/PNST reintegration through the existing Layer-1 authored-child/container path;
5. NBZ overlay emission/reopen through the existing NBZ writer path;
6. original `dmc3.exe` load/visual/rollback acceptance.

No `100% SCM`, Capcom-tool equivalence, arbitrary authoring or production-ready claim is made.
