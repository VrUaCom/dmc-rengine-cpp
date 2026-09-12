# DMC3 HD SCM reverse completion audit — 2026-09-13

**Branch:** `reverse/mod-completion-20260907`  
**Canonical executable:** `dmc3.exe`  
**SHA-256:** `e454272ed0fb0247fcbcf300e5d55d7a3e96d50b89b9ffaff81bb978dcbdd082`  
**Scope:** serialized/runtime reverse only. Writer breadth, container/NBZ delivery and original-game authored-resource acceptance are separate gates.

## Completion rule

SCM uses the same terminal evidence rule already applied to the completed MOD reverse. A serialized field does not require an invented artistic/gameplay name to be considered closed. Every byte/bit/domain must instead end in one of the canonical terminal states:

- `EXE_CONFIRMED`;
- `CORPUS_CONFIRMED`;
- `EXE_AND_CORPUS_CONFIRMED`;
- `STRUCTURAL_CONFIRMED`;
- `SEMANTIC_CANDIDATE` only where the candidate itself is the strongest honest terminal statement;
- `PRESERVED_UNDECODED`;
- `RESERVED_OBSERVED_ZERO`;
- `REJECTED`.

`RESEARCH_REQUIRED` is not a terminal state and is removed from the current SCM field map. Historical evidence files are not rewritten; this audit supersedes their old frontier lists where newer evidence exists.

## Result

**The DMC3 HD SCM reverse is complete for the canonical project scope.**

Completion means:

1. the complete serialized layout has a typed or preservation-aware representation;
2. every live serialized domain has a bounded runtime/structural role;
3. every remaining zero/opaque field has a terminal preservation disposition rather than an invented meaning;
4. disproven historical interpretations are explicitly rejected;
5. no current direct-EXE field-consumer gate remains classified `RESEARCH_REQUIRED`.

This is not a claim of Capcom source-symbol recovery, original offline-builder equivalence, unrestricted writer authority or original-game acceptance of arbitrary edited SCM files.

## Header — terminal map

| Offset | Terminal result |
|---:|---|
| `+0x00` | `SCM ` magic — `EXE_CONFIRMED` |
| `+0x04` | version `1.01` on the canonical 68-unique corpus — `CORPUS_CONFIRMED` |
| `+0x08..+0x0F` | zero on bounded corpus; shared model-manager whole-image source-pointer census finds no typed runtime effect — `RESERVED_OBSERVED_ZERO` + `PRESERVED_UNDECODED` |
| `+0x10` | object count -> manager `+0xE8` — `EXE_CONFIRMED` |
| `+0x11` | scene-node count -> manager `+0xEA` — `EXE_CONFIRMED` |
| `+0x12` | serialized texture-count mirror; live runtime authority comes from the external texture companion — `EXE_AND_CORPUS_CONFIRMED` |
| `+0x13` | copied to manager `+0xFA`; zero in bounded corpus; no higher semantic required — runtime-carried `PRESERVED_UNDECODED` |
| `+0x14` | legacy resource/provenance code; decimal split `family_class*100000 + model_set*100 + sub_index` is corpus-confirmed and value is copied to manager `+0xE4`; official names of classes 3/4 are not evidenced — neutral structural semantic is terminal |
| `+0x18..+0x1F` | zero corpus + whole-image model-source dormancy — `RESERVED_OBSERVED_ZERO` + `PRESERVED_UNDECODED` |
| `+0x20` | absolute scene-node-block offset — `STRUCTURAL_CONFIRMED` / runtime-bound through the common node binder |
| `+0x28..+0x3F` | zero corpus + whole-image model-source dormancy — `RESERVED_OBSERVED_ZERO` + `PRESERVED_UNDECODED` |

The shared model-header closure is provenance-gated by the canonical family classifier (`SCM -> 0x30000000`) and manager source pointer, not by raw displacement equality.

## Object record — terminal map

Object stride is `0x40`; SCM runtime object stride is `0x3C0`.

| Offset | Terminal result |
|---:|---|
| `+0x00` | mesh count — `EXE_CONFIRMED` |
| `+0x01` | `alpha_control`: common values `<=0x80` project to packet alpha `value/255`; control values `>0x80` retain an override code and project alpha `1.0`; SCM additionally has bounded C4/EA compatibility rewrites — `EXE_CONFIRMED` |
| `+0x02` | total vertex count = sum of child mesh counts — `STRUCTURAL_CONFIRMED` |
| `+0x04..+0x07` | zero on bounded corpus; no direct source-object semantic recovered; exact bytes remain source-preserved — `RESERVED_OBSERVED_ZERO` + `PRESERVED_UNDECODED` |
| `+0x08` | absolute mesh-table pointer — `EXE_CONFIRMED` |
| `+0x10` | source/effective object flags — runtime projection is `EXE_CONFIRMED`; individual technical semantics are listed below |
| `+0x14..+0x2F` | zero on bounded corpus; preservation-only serialized state — `RESERVED_OBSERVED_ZERO` + `PRESERVED_UNDECODED` |
| `+0x30..+0x3B` | bounding-sphere center — `CORPUS_CONFIRMED` |
| `+0x3C` | bounding-sphere radius — `CORPUS_CONFIRMED` |

### Object flag `+0x10`

Terminal technical map:

- low nibble: helper/render mode; non-zero also forces effective alpha control `0x80` — `EXE_CONFIRMED`;
- `0x00000020`: recovered runtime flag projection — `EXE_CONFIRMED`, high-level category intentionally unnamed;
- `0x00010000`: runtime bit/secondary-boolean projection — `EXE_CONFIRMED`, high-level category intentionally unnamed;
- `0x00020000`: runtime bit plus `(1,1,1,0)` initialization — `EXE_CONFIRMED`;
- `0x00040000`: runtime flag projection — `EXE_CONFIRMED`;
- `0x00080000`: runtime flag projection — `EXE_CONFIRMED`;
- `0x00100000`: in the shared `0x140302640` low-mode packet path selects legacy GS `TEST_1.AREF 0 <-> 16` and `ZBUF_1.ZMSK 1 <-> 0`; technical renderer semantic is `EXE_CONFIRMED`;
- `0x00200000`: observed on 19 SCM objects, preserved into runtime source/effective state; whole-image immediate/bit-test candidates were provenance-rejected as unrelated owners. Terminal status: `PRESERVED_UNDECODED` with bounded negative evidence;
- `0x00004000`: common material helper selects legacy GS `TEX1=0` nearest instead of `TEX1=0x60` linear — `EXE_CONFIRMED`;
- high nibble `0x0F000000`: recovered runtime mode projection — `EXE_CONFIRMED`.

A missing artistic/material name is not an open reverse gate when the binary technical effect or preservation disposition is already terminal.

## Mesh record — terminal map

Mesh stride is exactly `0x50`.

| Offset | Terminal result |
|---:|---|
| `+0x00` | vertex count — `STRUCTURAL_CONFIRMED` |
| `+0x02` | external texture-companion slot index; runtime table stride `0x40` — `EXE_CONFIRMED` |
| `+0x04/+0x06/+0x08/+0x0A` | legacy PS2 GS CLAMP `MINU/MAXU/MINV/MAXV`; packed with `WMS=WMT=REGION_REPEAT` — `EXE_CONFIRMED` |
| `+0x0C` | zero in current SCM corpus; canonical material helper consumes preceding CLAMP words and skips it; SCM normalizer does not assign a semantic — `RESERVED_OBSERVED_ZERO` + `PRESERVED_UNDECODED` |
| `+0x10` | positions `float3[]` — `EXE_AND_CORPUS_CONFIRMED` |
| `+0x18` | normals `float3[]` — `EXE_AND_CORPUS_CONFIRMED` |
| `+0x20` | signed fixed-point UV `i16/i16`, scale `1/4096` — `EXE_AND_CORPUS_CONFIRMED` |
| `+0x28` | continuation span: `0x50` for non-final mesh, `0` for final — `EXE_AND_CORPUS_CONFIRMED` |
| `+0x30` | zero in canonical corpus; no promoted serialized semantic; source-preserved — `RESERVED_OBSERVED_ZERO` + `PRESERVED_UNDECODED` |
| `+0x38` | RGB + topology-flags stream — `EXE_AND_CORPUS_CONFIRMED` |
| `+0x40` | mesh-relative generated-index workspace — `EXE_CONFIRMED` |
| `+0x48` | runtime-generated index word count; serialized retail value zero before normalization — `EXE_AND_CORPUS_CONFIRMED` |
| `+0x4C` | zero in canonical corpus; no promoted serialized semantic; source-preserved — `RESERVED_OBSERVED_ZERO` + `PRESERVED_UNDECODED` |

The topology byte is `RGB + flags`, not RGBA. Bit `0x02` is the confirmed triangle-run break/skip bit. Other bits remain preservation-gated if encountered.

## Serialized physical layout

The canonical layout formula is closed and reaches exact EOF on the 68-unique retail corpus:

```text
header 0x40
object table [N] * 0x40
for each object:
  mesh records [M] * 0x50
  align16
  position streams per mesh, align16 each
  normal streams per mesh, align16 each
  UV streams per mesh, align16 each
  RGB/topology streams per mesh, align16 each
scene-node block
align16
index-workspace reserves per mesh
EOF
```

No hidden serialized tail/secondary table is required by the confirmed corpus.

## Scene-node block — terminal map

The block starts with four relative dwords:

```text
+0x00 parentByOrderPosition
+0x04 nodeAtOrderPosition
+0x08 objectBindingByNodeIndex
+0x0C transformByNodeIndex
```

All are runtime-bound by `0x1402F1DB0`. The arrays and hierarchy constraints are structurally/corpus confirmed.

`+0x10..+0x1F` is zero in the bounded corpus. The canonical model layout planner reads byte `+0x10` once into a dead local with zero later reads; `+0x11..+0x1F` has no provenance-confirmed model-domain consumer and the raw shell pointer does not escape. Terminal state: `RESERVED_OBSERVED_ZERO` + `PRESERVED_UNDECODED`, with canonical runtime effect dormant/no effect.

## Scene transform — terminal map

Each transform is `0x20` bytes:

```text
+0x00 float3 translation
+0x0C float  translation_magnitude
+0x10 float3 rotation_xyz_radians
+0x1C float  preservation-only fourth lane
```

Closed runtime path:

```text
SCM setup 0x140303C10
 -> SCM local initializer 0x1402FA360
 -> XYZ rotation helper 0x140330450
 -> Rz * Ry * Rx
 -> translation helper 0x140031200
 -> parent/root pointer setup
 -> generic world update 0x1402F9700
 -> world = local * parentOrRootWorld
```

`+0x0C` is the precomputed Euclidean length of translation and is excluded from homogeneous matrix W. `+0x1C` is zero in the canonical corpus and is not consumed by the confirmed SCM local matrix construction; terminal status is `PRESERVED_UNDECODED` / `RESERVED_OBSERVED_ZERO`.

The old attribution of `0x1402FA080` to SCM is `REJECTED`; that initializer belongs to MOD/EFM.

## Generated-index workspace

Closed contract:

```text
capacityBytes = align16(6 * (vertexCount - 2))
serialized first u16 = 0x1212
serialized generated count = 0
runtime normalizer 0x1403051B0 reconstructs u16 indices
```

Triangle reconstruction from the topology byte is part of the canonical C++ implementation.

## Texture/material ownership boundary

SCM does not own texture payload bytes internally.

```text
manager +0x108 = SCM resource
manager +0x110 = external texture companion
companion count -> manager +0xEC
mesh +0x02 -> runtime texture table slot
```

Header `+0x12` is a serialized mirror/consistency count, not the live runtime texture-table authority. This ownership chain is reverse-closed even though retail texture authoring remains a separate writer gate.

## Historical hypotheses rejected/superseded

The completion audit explicitly rejects or supersedes these old statements:

- SCM is a MOD alias — `REJECTED`;
- object `+0x01` is merely an unknown classification byte — superseded by `alpha_control` runtime proof;
- mesh `+0x04..+0x0B` is reserved — superseded by GS CLAMP REGION_REPEAT proof;
- object flag `0x00004000` only selects an opaque descriptor `0/0x60` — superseded by GS TEX1 nearest/linear decoding;
- object flag `0x00100000` only selects opaque `0x5000D/0x5010D` state — superseded by GS TEST_1/ZBUF_1 decoding;
- nearby `AND 0x00200000` sites prove the SCM object bit-21 semantic — `REJECTED` by pointer provenance;
- `0x1402FA080` is the SCM local-transform initializer — `REJECTED`; SCM uses `0x1402FA360`;
- SCM local-to-world composition is open — superseded by the corrected hierarchy/world-transform record;
- header `+0x14` must have an official guessed gameplay name before reverse can close — `REJECTED`; its arithmetic structure/runtime carry are known and unproven class labels remain neutral.

## Reverse-complete boundary

The following are **not reverse blockers** and must not reopen SCM reverse by themselves:

- production writer promotion;
- real size-changing authored SCM acceptance;
- retail texture-companion rewrite;
- PAC/PNST/NBZ delivery breadth;
- Native Reader/ModViz edit UX;
- original `dmc3.exe` acceptance of authored resources;
- Capcom offline-builder byte equivalence;
- official source-code class/member names that are absent from the executable evidence.

Those are authoring, integration or original-runtime acceptance tracks.

A future SCM reverse pass is justified only by a genuine contradiction, a new non-zero corpus class, a new executable consumer/producer that changes a terminal disposition, or a previously unseen format revision.