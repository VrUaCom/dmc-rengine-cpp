# DMC3 HD SCM deep-reader reverse pass — 2026-09-13

**Branch:** `reverse/mod-completion-20260907`  
**Canonical executable:** `dmc3.exe`  
**SHA-256:** `e454272ed0fb0247fcbcf300e5d55d7a3e96d50b89b9ffaff81bb978dcbdd082`  
**Scope:** read/reverse path only. This pass does not promote writer or original-game acceptance authority.

## Why this pass exists

The SCM parser already had a strong canonical typed IR, but the Native Reader integration stopped after recognition and parser diagnostics. That meant much of the reverse knowledge existed in `formats::scm` without being exposed through the shared Binary Inspector document model.

This pass moves SCM from “parser exists” to an evidence-aware deep reader:

```text
resource bytes
 -> canonical SCM parser
 -> typed IR
 -> evidence-aware byte map
 -> Native Reader / Binary Inspector
 -> point-selection provenance
 -> corpus delta census
```

The goal is not to invent names for opaque bytes. The goal is to make every known byte range visible with its current evidence boundary and to make genuinely new corpus/runtime evidence easy to detect.

## Implementation promoted in this pass

### `scm_binary`

New canonical adapter:

```text
include/dmc_rengine/formats/scm_binary.hpp
src/formats/scm_binary.cpp
```

`build_binary_document(...)` projects a successfully parsed SCM into the shared `binary::Document` model.

Mapped domains:

- header and every currently modeled header field;
- fixed `0x40` object records;
- fixed `0x50` mesh records;
- position streams;
- normal streams;
- signed fixed-point UV streams;
- RGB/topology streams;
- scene-node header;
- parent/order/object-binding arrays;
- `0x20` scene transforms;
- generated-index workspaces and the serialized `0x1212` sentinel.

The document also carries ownership claims and evidence annotations. Preservation-only fields are deliberately represented as preservation-only rather than silently omitted.

### Native Reader promotion

`src/integration/native_reader/scm_module.cpp` now attaches the SCM `binary::Document` after a successful canonical parse.

This closes the old integration gap:

```text
old:
SCM -> recognized + diagnostics

current:
SCM -> recognized + diagnostics + typed Binary Inspector document
```

### Read-only CLI

New command:

```text
dmc-rengine inspect-scm <file>
dmc-rengine inspect-scm <file> --json
dmc-rengine inspect-scm <file> --offset 0x...
```

Modes:

- default: structural summary and preservation frontier;
- `--json`: deterministic full Binary Inspector manifest;
- `--offset`: point selection showing regions, fields, ownership and evidence annotations covering one byte.

The offset mode is intended for reverse engineering: an investigator can start from a suspicious byte and immediately see its current parser identity, physical owner and evidence boundary.

### Corpus reverse census

`verify-scm-corpus` now retains its no-edit writer parity gate but also emits read/reverse census data:

- union of observed source object flags;
- union of currently preservation-only source flag bits;
- union of topology bits;
- union of topology bits outside confirmed `0x02`;
- resource-code decomposition;
- texture-slot count;
- non-zero GS CLAMP mesh count;
- non-zero header/object/mesh/scene/transform preservation domains;
- unexpected serialized non-zero generated-index counts.

This turns a new corpus into a contradiction detector. A new non-zero field or unseen bit should become visible immediately instead of being hidden behind a generic successful parse.

## Binary map evidence policy

### Confirmed typed fields

The deep reader may use technical names where the existing reverse already supports them, including:

- `alpha_control`;
- external texture-companion slot;
- GS CLAMP `MINU/MAXU/MINV/MAXV`;
- position/normal/UV streams;
- RGB/topology stream;
- scene parent/order/object-binding arrays;
- translation, translation magnitude and XYZ-radian rotation;
- generated-index workspace/count.

### Preservation-only fields

The deep reader explicitly annotates, rather than hides or renames:

- header `+0x08..+0x0F`;
- header `+0x13`;
- header `+0x18..+0x1F`;
- header `+0x28..+0x3F`;
- object `+0x04..+0x07`;
- object `+0x14..+0x2F`;
- source object flag `0x00200000` when present;
- mesh `+0x0C`;
- mesh `+0x30`;
- mesh `+0x4C`;
- scene header `+0x10..+0x1F`;
- transform `+0x1C`.

`PRESERVED_UNDECODED` does not mean “probably unused.” It means the current reader knows the physical ownership and preservation rule but does not claim a semantic editor control.

## Topology-reading boundary

The fourth byte of each color/topology tuple is not alpha.

Current technical contract:

```text
byte0 = R
byte1 = G
byte2 = B
byte3 = topology flags
```

Bit `0x02` is the confirmed triangle-run break/skip condition. The deep reader annotates the complete topology stream and the corpus census reports any bit outside `0x02` as an unconfirmed topology delta.

This is intentionally stricter than accepting an arbitrary RGBA interpretation.

## Object flag bit `0x00200000`

The deep reader keeps the current terminal state:

```text
DATA_CONFIRMED
RUNTIME_PRESERVED
BOUNDED_CONSUMERS_NEGATIVE
PRESERVED_UNDECODED
```

It does not turn bit 21 into an editor toggle.

The next pass must look for transformed or indirect dataflow, not repeat only exact-immediate `0x200000` searches. The acquisition plan therefore reacquires both the SCM object initializer/helper chain and the two previously rejected bit-21 sites as provenance regression anchors.

## Mesh preservation frontier

### `mesh +0x0C`

Existing SCM-specific evidence remains:

- zero in bounded direct specimens;
- primary materialization function `0x1402F9BB0` reads `+0x00/+0x02/+0x10/+0x18/+0x20/+0x28/+0x38` but not `+0x0C`;
- helper `0x140308C00` does not consume serialized `+0x0C`;
- known nearby `+0x0C` accesses with different pointer provenance are rejected.

The deep-reader acquisition plan reacquires this path rather than upgrading the negative result from old notes alone.

### `mesh +0x30` and `mesh +0x4C`

Both remain zero/preservation-only in the canonical SCM field map. The new plan explicitly asks the SCM normalizer, primary runtime mesh materializer and downstream fixed-stride consumer to census these offsets.

Cross-family MOD evidence may be used as a search hint only. It is not SCM authority.

## Fresh canonical EXE acquisition plan

Added:

```text
data/reverse/dmc3-scm-deep-reader-window-plan.v1.json
```

The plan covers 14 windows:

1. `0x1403051B0` — SCM post-load normalizer;
2. `0x1402F9BB0` — runtime mesh materialization;
3. `0x1402F9890` — common material/texture helper;
4. `0x140302F10` — SCM object materialization;
5. `0x140302640` — source-flag helper;
6. `0x1402F9570` — manager/header load;
7. `0x140304AE0` — SCM + texture-companion pair binding;
8. `0x140303C10` — SCM scene setup;
9. `0x1402F1DB0` — node-array binder;
10. `0x1402FA360` — corrected SCM local-transform initializer;
11. `0x1402FDD10` — fixed-stride downstream mesh consumer;
12. `0x140304100` — alpha/color packet construction;
13. `0x140303E80` — rejected manager bit-21 candidate regression;
14. `0x1402F4B80` — rejected unrelated runtime bit-21 candidate regression.

The packet is now a CI-validated acquisition plan. Validation proves plan structure and canonical artifact binding only; it does not substitute for acquiring the actual executable windows.

## Regression coverage

New `scm_binary_tests.cpp` constructs a canonical synthetic SCM and checks:

- header/object/mesh/stream/scene/workspace regions;
- resource-code exposure;
- source-flag projection text;
- texture-slot field;
- explicit bit-21 preservation annotation;
- scene preservation annotation;
- topology annotation;
- zero region and ownership conflicts.

The test is registered as `dmc_rengine_scm_binary_tests` in the normal CTest matrix.

## What is closed by this pass

This pass closes a product/reverse integration gap:

```text
canonical SCM reverse knowledge
    ==
Native Reader byte/evidence representation
```

for the currently modeled SCM domains.

It also creates a repeatable path for finding contradictions in new retail/legacy/modded SCM corpora.

## What is not claimed

This pass does not claim:

- a new semantic meaning for `0x00200000`;
- new semantics for zero/preservation-only mesh/object/header lanes;
- universal SCM revisions outside the evidenced DMC3 HD family;
- writer promotion;
- retail texture rewrite authority;
- original-game acceptance of arbitrary authored SCM.

A preservation terminal state changes only when new corpus bytes, a provenance-clean executable consumer/producer, or controlled runtime evidence justifies the change.
