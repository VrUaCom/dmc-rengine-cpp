# DMC3 HD `.MOD` completion audit — 2026-09-07

**Branch:** `reverse/mod-completion-20260907`  
**Base:** `main@1a029daace6790e1c832e13ba4841ae45004a147`  
**Canonical executable authority:** `dmc3.exe` SHA-256 `e454272ed0fb0247fcbcf300e5d55d7a3e96d50b89b9ffaff81bb978dcbdd082`

## Purpose

Define what “fully researched MOD” means and turn the remaining uncertainty into a finite evidence program. This is a research/audit slice only. It does not promote writer authority, invent semantics, or weaken any existing evidence boundary.

A MOD domain is considered closed only when its claims are classified separately for:

1. serialized layout;
2. corpus behavior / variant coverage;
3. canonical executable producer/consumer behavior;
4. runtime representation and derived state;
5. cross-resource ownership;
6. read/validation authority;
7. writer/serialization authority;
8. parent-container reintegration;
9. original-game authored-resource acceptance and rollback.

Reader success is not writer authority. A homologous SCM/EFM offset is not MOD semantic proof.

## Current closure matrix after the full recursive em000 sweep

| Domain | Current strongest state | Remaining proof before full closure |
|---|---|---|
| document magic/version/count shell | strong structural + EXE-backed common ABI; `0.82/0.84/1.00/1.01` structural variants corpus-confirmed | actor-family/version coverage beyond em000 |
| header `+0x12` texture-domain mirror | EXE-confirmed flow; companion is runtime authority; unused reserved slots observed in 10 em000 files | broader mismatch behavior + writer coherence rule |
| header `+0x13` | EXE-confirmed `default_joint_index` | mutation range + original-game edited acceptance |
| header `+0x14` | EXE-confirmed copy to manager `+0xE4`; decimal arithmetic shape corpus-confirmed | MOD-specific downstream consumer/xref; do not import SCM semantics |
| object `0x40` core | EXE-confirmed core fields; 136 retail records swept | full flag semantics; mutation rules |
| object `+0x18/+0x1C` | EXE_AND_CORPUS_CONFIRMED as genuinely populated retail inputs (`em000_033.mod`) | artistic meaning + safe ranges + authored acceptance |
| object source flags | many direct runtime projections closed; `0x00200000` observed in 7 retail objects | finish `0x00100000/0x00200000` downstream semantics and any remaining packet effects |
| mesh `0x50` live core | strong structural + EXE runtime mapping; 147 retail meshes swept | actor-family coverage + writer layout proof |
| mesh `+0x0C/+0x38/+0x4C` | preserved; zero 147/147 in em000; absent from confirmed post-load/runtime-mesh construction | global xref census + wider actor-family corpus, then writer policy |
| positions/normals | structural/read authority; all 14,804 finite; normals effectively unit length | authoring normalization/recalculation policy + edited acceptance |
| fixed-point UV | structural/read authority; em000 range U `0..4096`, V `-180..4096` | broader range/overflow corpus + authoring behavior |
| texture slot / GS CLAMP | EXE-confirmed; all em000 slots valid against header mirror; CLAMP zero 147/147 | nonzero CLAMP corpus + safe authoring domain + companion coherence + game acceptance |
| texture companion envelope | EXE-confirmed read path | complete-enough TIM2 internals + companion writer + replacement acceptance |
| runtime texture descriptor | EXE-confirmed in-memory ABI | remaining `+0x38..+0x3F`; complete producer/consumer map where relevant |
| topology high bit `0x8000` | EXE-confirmed CPU post-load + shader-independent weight split; 4,693 corpus hits | canonical serializer/workspace planning + game acceptance |
| generated topology workspace | `147/147` em000 meshes match `align16(6*(vertex_count-2))`; sentinel pattern stable | broader family confirmation + writer planner + game acceptance |
| blend-index y/z/w `/4` | EXE-confirmed CPU + shader linkage; all active values valid across 14,804 vertices | editing/writer acceptance |
| `BLENDINDICES.x` | zero 14,804/14,804 in em000; semantic open | producer/consumer xref or nonzero corpus variant |
| packed 5+5+5 weights `/31` | EXE-confirmed shader + CPU domain; all em000 q-sums=31 and active bones valid | mutation quantization rules + game acceptance |
| hierarchy parent/order | EXE-confirmed semantics; all 35 hierarchies complete/topological | broader actor-family/revision corpus + writer acceptance |
| motion-group table | EXE-confirmed selector into `CMotionJoint+0xF8`; em000 histogram `0:132,1:91,2:3` | high-level meanings of values, if any, through direct behavior/correlation |
| local transform XYZ + magnitude | structural + EXE-backed transform construction; all em000 magnitudes match `length(xyz)` | mutation policy; broader variant coverage |
| transform `+0x1C` | zero 226/226 em000 records; preserved | direct consumer/producer census or nonzero corpus evidence |
| world matrices | EXE-confirmed row-vector hierarchy | external/root placement ownership for every use-case |
| inverse-rest + skin palette | EXE-confirmed | authoring/rest-pose mutation acceptance |
| MOD-side animation binding | canonical hierarchy/group/evaluated-local -> palette composition | complete MOT serialized decode/evaluator ownership |
| SHW relation | SHW matrix selector EXE-confirmed | exact selected-palette ownership and MOD node/bone mapping |
| SO relation | bounded cardinality/correlation analysis | semantic cross-resource identity only if direct evidence appears |
| no-edit writer | not promoted | deterministic layout planner + byte/preservation parity across broad corpus |
| edited writer | not promoted | bounded edits, reopen, derived-state correctness |
| PAC/PNST/NBZ reintegration | not MOD-authority closed | slot-preserving child rebuild -> parent rebuild -> rematerialization |
| original `dmc3.exe` acceptance | open | deterministic visible effect, load proof, failure/rollback proof |

## Full recursive em000 corpus result

Hash-bound corpus:

`em000-extract.zip`  
SHA-256 `306130125f09824811289366324f4208c3c1aba880c5a7efa3953a88d566d07b`

Recursive inventory:

- 35 unique MOD payloads;
- 22 top-level MODs;
- 13 nested MODs;
- 136 object records;
- 147 mesh records;
- 14,804 vertices;
- 226 node/transform positions;
- 628,192 MOD bytes total.

### Byte coverage

The sweep marked all currently known serialized regions: document header, object/mesh records, every vertex stream, node-domain header/arrays, local transforms and generated-topology workspaces.

After marking these ranges, **5,106 bytes remained uncovered and all 5,106 were zero**.

Current corpus conclusion:

```text
no new non-zero serialized MOD section exists in recursive em000
outside the current known section map
```

Status: `CORPUS_CONFIRMED` for this bound corpus only.

This does not prove that player, weapon, boss, item or effect model families cannot contain additional sections or nonzero variants.

### Retail revisions

Observed versions:

```text
1.01 -> 24 files
1.00 -> 6 files
0.84 -> 4 files
0.82 -> 1 file
```

All preserve the same recovered structural grammar in this corpus. The parser/code contract must therefore not describe only `1.01` as the retail structural revision.

### Header `+0x14`

All 35 values have the observed arithmetic projection:

```text
raw = high_component * 100000
    + middle_component * 100
    + low_component
```

Examples:

```text
100407 -> 1 / 4 / 7
202900 -> 2 / 29 / 0
601715 -> 6 / 17 / 15
700601 -> 7 / 6 / 1
```

This projection is now `CORPUS_CONFIRMED` and is codified as a neutral arithmetic view. The names `family_class/model_set/sub_index` remain candidate-level until a MOD-specific `manager+0xE4` downstream consumer is proven.

### Object `+0x18/+0x1C`

`em000_033.mod` contains two retail records:

```text
source_flags = 0x00100201
+0x18 f32     = 1.25
+0x1C u32     = 2
```

Because source flag `0x200` is active and the executable already proves the conditional runtime copy of these fields, they are now `EXE_AND_CORPUS_CONFIRMED` as real live serialized inputs. Their artistic/high-level meaning is still open.

### Generated topology workspace

For all `147/147` meshes:

```text
capacity = align16(6 * (vertex_count - 2))
```

matches the physical workspace span. This is codified as an em000 corpus observation for future layout planning. It does not yet authorize a production serializer.

### Zero-preservation matrix

- header `+0x08..+0x0F`: zero 35/35;
- header `+0x18..+0x1F`: zero 35/35;
- header `+0x28..+0x3F`: zero 35/35;
- object `+0x04..+0x07`: zero 136/136;
- object `+0x14..+0x17`: zero 136/136;
- object `+0x20..+0x2F`: zero 136/136;
- mesh `+0x0C`: zero 147/147;
- mesh `+0x38`: zero 147/147;
- mesh serialized `+0x48`: zero 147/147;
- mesh `+0x4C`: zero 147/147;
- node-domain header `+0x10..+0x1F`: zero 35/35;
- transform `+0x1C`: zero 226/226;
- `BLENDINDICES.x`: zero 14,804/14,804.

All remain corpus-specific preservation evidence, not global writer-zero rules.

## Code/evidence synchronization requirements from this sweep

The following findings must exist simultaneously in canonical code and evidence documents:

1. structural version policy includes corpus-confirmed `0.82/0.84/1.00/1.01`;
2. header `+0x14` has a neutral decimal projection helper without SCM semantic promotion;
3. generated-workspace observed capacity formula is available to analysis/layout planning;
4. em000 statistics and zero-field invariants are machine-readable;
5. object `+0x18/+0x1C` documentation states that real retail nonzero values are observed;
6. parser source bytes remain preservation authority for all unresolved fields.

Canonical implementation locations for these corpus facts:

```text
include/dmc_rengine/formats/mod/version.hpp
include/dmc_rengine/analysis/mod/corpus_observations.hpp
src/analysis/mod/corpus_observations.cpp
data/reverse/dmc3-mod-em000-full-corpus-sweep-20260907.json
```

## Completion program

### Wave A — serialized/ABI unknown closure

A1. Header `+0x14`: exhaustive manager `+0xE4` downstream xref. Corpus shape is now closed; semantic consumer is not.  
A2. Transform `+0x1C`: direct initializer/consumer census + nonzero-variant search outside em000.  
A3. Mesh `+0x0C/+0x38/+0x4C`: global load/render/tool-side xref census + broader actor-family histogram.  
A4. `BLENDINDICES.x`: full CPU/GPU producer-consumer census and variant search outside em000.  
A5. Finish unresolved source-flag semantics, especially active retail bits that still lack high-level names.

### Wave B — animation closure

B1. Promote current-main MOT structural parser/evidence without importing stale branch history.  
B2. Recover exact MOT channel targeting, interpolation/key representation and evaluation order.  
B3. Bind MOT output to the already-canonical MOD `animation_binding` input contract.  
B4. Identify whether motion-group values have stable semantic classes or are merely authored selector IDs.

### Wave C — cross-resource model closure

C1. Close SHW selected matrix-palette ownership.  
C2. Prove or reject direct SHW-selector <-> MOD-node mapping per runtime path.  
C3. Complete model texture companion/TIM2 semantics needed for safe replacement.  
C4. Re-run the exact same full byte-coverage sweep across player, weapon, boss, item and other actor families.

### Wave D — authoring closure

D1. Canonical layout planner preserving all unresolved bytes.  
D2. No-edit parse -> write -> parse and byte-diff proof over broad corpus.  
D3. Bounded edit policies for geometry, UV, transforms, skin weights, texture slots and material flags.  
D4. Parent PAC/PNST reintegration with physical slot preservation.  
D5. NBZ/overlay rematerialization and provenance receipt.  
D6. Original-game edited-resource acceptance with deterministic visible effect and rollback.

## Evidence acquisition discipline

For every remaining field:

```text
serialized field
 -> parser preservation
 -> corpus histogram
 -> exact EXE producer/consumer
 -> runtime destination
 -> downstream behavior
 -> regression
 -> writer rule
 -> reopen
 -> original-game acceptance
```

If any arrow is missing, the corresponding stronger claim stays open.

## Definition of “MOD fully researched”

The project may call MOD fully researched only when:

- every serialized byte range has a typed meaning or an explicitly proven preservation rule;
- every live field has a bounded producer/consumer/runtime contract;
- all derived runtime state is separated from serialized authority;
- variant coverage is broad enough to reject one-corpus assumptions;
- cross-resource dependencies are proven, not inferred;
- no-edit rebuild is deterministic and preservation-correct;
- supported edits reopen correctly and survive parent-container rebuild;
- the canonical game accepts those edits with a reproducible visible effect and rollback path.

Until then, MOD remains **strong read/reverse support with incomplete authoring authority** rather than “100% complete.”
