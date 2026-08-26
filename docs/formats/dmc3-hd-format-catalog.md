# DMC3 HD Format and Resource-Purpose Catalog

**Snapshot:** 2026-08-26  
**Initial reconciliation base:** `main@453373ff0977fc0aa1f6fab39273cdd9716da6af`  
**Scope:** DMC3 HD Collection resource families currently observed in the project corpus, canonical analysis executable, current C++ product code, or preserved reverse research.

This document is the canonical human-readable inventory for **what each observed resource family is for** and how strong that conclusion currently is.

It is intentionally not a claim that the reverse has discovered every possible byte grammar in the game. The catalog is exhaustive over the **currently observed/named families in project evidence**. New families must be added when direct evidence appears.

## 1. Authority and interpretation rules

Format truth must not be inferred from an extension alone.

Evidence order:

```text
validated magic / structural grammar
 -> direct original-EXE consumer or typed post-load branch
 -> hash-bound corpus structure
 -> contextual/index label
 -> unknown
```

Keep four questions separate:

1. **Identity:** what format/family is this payload?
2. **Purpose:** what subsystem uses it?
3. **Schema:** which byte fields are actually understood?
4. **Product support:** what can current DMC Rengine C++ safely do with it?

A format can have an EXE-confirmed purpose and still have an incomplete binary schema. Conversely, a structural parser can exist without proving complete original-game behavior.

### Evidence status

| Status | Meaning |
|---|---|
| `GAME_VERIFIED` | bounded behavior observed in the original game |
| `EXE_CONFIRMED` | direct canonical executable evidence |
| `DATA_CONFIRMED` | stable binary/corpus evidence |
| `HIGH_CONFIDENCE` | convergent evidence; one promotion boundary remains |
| `CANDIDATE` | evidence-linked interpretation, not yet canonical semantics |
| `RESEARCH_REQUIRED` | identity/label exists; exact role or schema remains open |
| `REJECTED` | superseded/contradicted claim |

### Product support vocabulary

| Support | Meaning |
|---|---|
| `structural` | current C++ has a bounded structural parser/scanner/source adapter |
| `recognized` | current product registry/classification knows the family but has no full parser |
| `runtime-only` | original runtime directly handles it; product integration remains missing/incomplete |
| `research-only` | known from reverse/corpus but not migrated as a product module |
| `capability-only` | executable/media layer can support the family; shipped DMC3 presence is not claimed |

## 2. Containers, namespaces and naming metadata

| Family | Purpose | Evidence | Current product status | Boundary |
|---|---|---|---|---|
| `NBZ` | numbered top-level HD resource volume; classic ZIP-backed materialization surface used by GDSpaces | `EXE_CONFIRMED` + product validation | `structural` source/materializer; bounded STORE overlay authoring | generated overlay equivalence is product behavior, not Capcom packer equivalence |
| `PAC` / `PAC0` | relative-slot resource container; preserves physical slot topology including sparse/empty entries | `EXE_CONFIRMED` + `DATA_CONFIRMED` | `structural`; bounded working-copy/write modes exist | slot number is structural identity, not universal semantic type |
| `PNST` | distinct relative-slot container family; recursively typed-postloaded by original runtime | `EXE_CONFIRMED` + `DATA_CONFIRMED` | `structural` | shares physical envelope with PAC but not semantic slot schema |
| `PACK` | legacy/observed container label | `RESEARCH_REQUIRED` for original DMC3-HD runtime authority | historical/product-side support only | do not treat as original-runtime-confirmed until direct evidence exists |
| `.index` | extraction/display naming manifest and structural metadata layer | `DATA_CONFIRMED` | recognized metadata | not runtime lookup authority and not semantic format authority |
| `.lst` | original list-driven PAC fallback/materialization manifest with `dummy` and nested-list behavior | `EXE_CONFIRMED` bounded | research/runtime path | full grammar, lifetime, recursion/error semantics remain open |
| `GData.afs/`, `GDataX360.afs/` | logical DMC3-HD resource namespace prefixes | `EXE_CONFIRMED` | recognized namespace | **not** evidence of an opaque binary AFS archive in current DMC3-HD path |
| `.ukn` | placeholder/source extension used where extractor lacks semantic identity | `DATA_CONFIRMED` | fallback only | never route by `.ukn` alone; known examples contain HITS/ITM/other payloads |
| `.bin` | generic filename/container leaf extension | contextual only | fallback | semantic purpose must come from bytes/context |
| `PE` / `dmc3.exe` | executable containing runtime code, registries, translation tables and static descriptors | `EXE_CONFIRMED` | `structural` PE inspection | separate canonical analysis build from protected execution build |

## 3. Geometry, models and collision

| Family | Purpose | Evidence | Current product status | Boundary |
|---|---|---|---|---|
| `SCM` | stage/scene mesh geometry and model data | `EXE_CONFIRMED` typed post-load + corpus | `recognized` | full mesh/material/ownership schema still under reconstruction |
| `MOD` | actor/object/model resource family | `EXE_CONFIRMED` typed post-load + corpus | `recognized` | complete model schema and all variants not yet closed |
| `HITS` | spatial collision resource: bounds + 3-D grid -> cell triangle references -> triangle/plane records | `EXE_CONFIRMED`/`DATA_CONFIRMED` | `structural`, collision category | raw flag semantics are not fully decoded; `0x18060001` is not a universal record marker |
| `DCA` | mini-demo/cinematic camera data; recovered two-channel interpretation uses camera eye/world position and look-at target | `HIGH_CONFIDENCE`/research + `DATA_CONFIRMED` structure | `structural` record-boundary scanner | C++ currently exposes 0x10 header + 0x410 records; full field semantics still not product-migrated |

## 4. Texture and render-resource families

| Family | Purpose | Evidence | Current product status | Boundary |
|---|---|---|---|---|
| `DDS` | HD texture representation consumed by tooling/render paths | `DATA_CONFIRMED` + direct signature | `recognized` | DDS compression/packing details are subformats, not new file families |
| `PTX` / texture bundle | texture bundle/container-like payload exposing embedded texture resources | `DATA_CONFIRMED` | `recognized` | runtime-original PTX/TIM2, PAC representation and extracted DDS must remain separate identities |
| `TM2` / `TIM2` | original/legacy texture image family used before/alongside HD translation | `EXE_CONFIRMED` identifier path + research | `research-only` | exact HD conversion/ownership chain remains separate from DDS |
| `EFM` | typed effect-system resource requiring original post-load relative-offset fixup before ready state | `EXE_CONFIRMED` | `runtime-only` | exact field semantics and product module still open |
| `SHW` | dedicated typed render/shadow-family resource requiring original post-load fixup | `EXE_CONFIRMED` typed branch; subsystem meaning `HIGH_CONFIDENCE` | `runtime-only` | exact schema and product module open |
| `SEF` | stage/effect companion resource label | `DATA_CONFIRMED`/project corpus | `research-only` | exact runtime consumer/schema requires reacquisition |
| `EFE` | effect/render companion resource label | `RESEARCH_REQUIRED` | `research-only` | purpose beyond effect-system association not promoted |
| `EFW` | effect/render companion resource label | `RESEARCH_REQUIRED` | `research-only` | exact purpose/schema open |
| `MRP` | observed render/effect-side resource label | `RESEARCH_REQUIRED` | `research-only` | no semantic expansion is canonical yet |
| `C1D` | observed render/effect-side resource label | `RESEARCH_REQUIRED` | `research-only` | exact purpose/schema open |
| `PTZ` | observed texture/render companion label | `RESEARCH_REQUIRED` | `research-only` | exact purpose/schema open |
| `TSC` | observed texture/render/control companion label | `RESEARCH_REQUIRED` | `research-only` | exact purpose/schema open |
| `CLT` | observed texture/render/control companion label | `RESEARCH_REQUIRED` | `research-only` | exact purpose/schema open |

### DDS encodings are not separate resource formats

The following observed tags are DDS/pixel encodings or layout codes, not independent resource families:

```text
DXT1 DXT2 DXT3 DXT4 DXT5
ATI1 ATI2
BC4 BC5
RGBG GRGB YUY2
```

They belong under DDS/texture representation metadata.

## 5. Stage configuration and gameplay-side resource families

The Stage CFG corpus contains a fixed-slot family whose semantic children include lighting, camera, events, positions, items, transforms and camera/demo data. Physical slot number is authoritative; names must not be inferred merely from ordinal position without an independently proven slot map.

Observed corpus summary:

```text
st001cfg.pac: 10 physical slots; observed semantic families include
LIG/CAM/EVE/POS/ITM/STE/DCA/EST

st114cfg.pac: 9 physical slots; observed semantic families include
LIG/CAM/EVE/POS/ITM/STE/DCA
```

| Family | Purpose | Evidence | Current product status | Boundary |
|---|---|---|---|---|
| `CAM` | stage camera definitions/tracks | `DATA_CONFIRMED`/historical editor evidence | `recognized` | complete original consumer and writer contract open |
| `LIG` | stage lighting data family | `DATA_CONFIRMED` | `structural` through lighting registry family | variant relationships remain under reverse |
| `LIG2` | stage lighting record resource; 0x20 header + 0x30 records, 48-record corpus shape | `EXE_CONFIRMED` identifier + `DATA_CONFIRMED` | `structural` | current scanner is structural and field semantics remain open |
| `EVE` | spatial event/trigger **volumes**: polygon corners + height and raw id/type/tag fields | `DATA_CONFIRMED` geometry | `research-only` / historical semantic core | it is not the mission bytecode itself; id/type/tag semantics remain candidate |
| `POS` | stage position/placement/transform data | `HIGH_CONFIDENCE` corpus context | `recognized`/research | exact record schema and original consumer need closure |
| `ITM` | static item placement records: item ID + X/Y/Z + Y rotation | `EXE_CONFIRMED` + `DATA_CONFIRMED`; game-verified item experiments exist | `recognized`; historical editor/core not fully migrated to clean C++ | persistence/behavior comes from EXE/EventTbl, not ITM alone |
| `STE` | scene/effect transform records | `DATA_CONFIRMED`, purpose `HIGH_CONFIDENCE` | `recognized`/research | direct `STE` ASCII hits in the EXE were false positives; no EXE-magic promotion from those hits |
| `EST` | stage dependency/control payload candidate; strong correlation with StageCfg physical slot 9 and enemy-resource demand scanner | `CANDIDATE` with direct runtime slot-9 consumer correlation | `recognized`/research | **do not claim `slot9 == EST` as fact** until an independent raw slot map/header confirms it |
| stage `TXT` | stage script/config text; StageSet tokens and room/door/effect configuration | `EXE_CONFIRMED` parser/token evidence | `structural` | individual token semantics remain partially open |
| `EventTblNN.bin` | mission/event control bytecode: conditions, inventory changes, dynamic spawns and mission flow | `HIGH_CONFIDENCE` project reverse lineage | `research-only` | keep distinct from EVE spatial zones; clean C++ migration remains open |

### ITM under misleading `.ukn`

The following stock fixtures are ITM despite their external `.ukn` names:

```text
st001cfg_006.ukn  size 48   count 1
st010cfg_006.ukn  size 48   count 1
st105cfg_006.ukn  size 64   count 2
st114cfg_006.ukn  size 400  count 19
```

Canonical ITM structure:

```text
+0x00 char[4]  "ITM\0"
+0x04 u16      version/family
+0x06 u16      record count
+0x08..0x0F    preserved raw/reserved
records @ 0x10, stride 0x14:
    +0x00 u32 itemId
    +0x04 f32 X
    +0x08 f32 Y
    +0x0C f32 Z
    +0x10 f32 rotationY
```

This is a canonical example of why extension is not format authority.

## 6. Animation and visibility/control families

| Family | Purpose | Evidence | Current product status | Boundary |
|---|---|---|---|---|
| `MOT`, `MOT2`..`MOT6` | motion/animation resource variants | `DATA_CONFIRMED` corpus + runtime research | `research-only` | exact variant contracts and binding rules remain open |
| `MCV` | motion companion/control/curve family | `CANDIDATE` | `research-only` | exact role relative to MOT requires direct consumer evidence |
| `HID`, `HID2`, `HID3` | hide/visibility-control variants | `HIGH_CONFIDENCE` from resource category/corpus naming | `research-only` | track schema and consumer ABI remain open |

## 7. Audio resource and representation families

HD audio has an important logical-to-physical split.

```text
legacy logical ADX name
 -> basename translation in dmc3.exe
 -> physical OGG descriptor/file
 -> EXE-owned loopStartMs / loopEndMs
```

This translation is runtime behavior, not index equality between two catalogs.

| Family | Purpose | Evidence | Current product status | Boundary |
|---|---|---|---|---|
| `ADX` | legacy logical audio identity requested by game/resource tables | `EXE_CONFIRMED` | research/runtime translation | HD distribution can resolve it to OGG rather than physical ADX |
| `OGG` | physical HD audio stream representation | `EXE_CONFIRMED` | research/consumer support | EXE descriptor owns loop points; do not depend on OGG comments for canonical loop metadata |
| `VAGp` | Sony VAG ADPCM audio payload/stream marker | `EXE_CONFIRMED` identifier | `research-only` | exact DMC3 ownership/container relationship still open |
| `PHD` | sound-bank/header descriptor candidate | `CANDIDATE` | `research-only` | exact fields and linkage open |
| `BD` | sound-bank body/sample-data candidate | `CANDIDATE` | `research-only` | exact fields and linkage open |
| `TSB` | sound table/bank descriptor candidate | `CANDIDATE` | `research-only` | exact fields and linkage open |
| `SPUMAPDT` | SPU sample-map/bank metadata candidate | `CANDIDATE` | `research-only` | exact bank relationship and schema open |

The candidate labels above describe the strongest current subsystem association. They must not be upgraded to exact schema names from acronyms alone.

## 8. Video/media families

DMC3 HD has an EXE-confirmed logical-to-physical video translation:

```text
legacy SFD logical path -> replace extension with .wmv -> media loader
```

| Family | Purpose | Evidence | Status |
|---|---|---|---|
| `SFD` | legacy logical movie identity | `EXE_CONFIRMED` | logical representation |
| `WMV` | physical HD movie representation used after SFD translation | `EXE_CONFIRMED` | shipped/runtime path |
| `PSS` | media/video container capability | capability strings/path evidence | `capability-only` |
| `THP` | media/video container capability | capability strings/path evidence | `capability-only` |
| `PAM` | media/video container capability | capability strings/path evidence | `capability-only` |
| `XMV` | media/video container capability | capability strings/path evidence | `capability-only` |
| `PMF` | media/video container capability | capability strings/path evidence | `capability-only` |
| `AVI` | media/video container capability | capability strings/path evidence | `capability-only` |
| `MPG` | media/video container capability | capability strings/path evidence | `capability-only` |
| `BIK` | media/video container capability | capability strings/path evidence | `capability-only` |
| `MP4` | media/video container capability | capability strings/path evidence | `capability-only` |

`capability-only` explicitly means: **do not claim the DMC3 HD retail corpus ships that family merely because the executable/media middleware can recognize it.**

## 9. Save/persistence families

| Family | Purpose | Evidence | Current product status | Boundary |
|---|---|---|---|---|
| `dmc3.sav` | game-progress/persistent state: global record + ten slot summaries + ten slot payloads with checksums | `DATA_CONFIRMED` + reverse implementation | structural/reverse tooling | bounded canonical size `0x4A30`; gameplay field map continues to expand |
| `options.sav` | configuration/options persistence | existence/subsystem context `HIGH_CONFIDENCE` | `research-only` | exact binary schema remains a research target |

These are separate persistence families and must not share an invented schema.

## 10. Runtime typed-postload authority

Canonical original-runtime post-load dispatch directly recognizes at least:

```text
MOD  -> dedicated in-place fixup
EFM  -> dedicated in-place fixup
SCM  -> dedicated in-place fixup
SHW  -> dedicated in-place fixup
PNST -> recursively visit non-empty children
PAC  -> root/child traversal before typed dispatch where applicable
```

This means “materialized bytes” are not necessarily the final consumer representation. Some formats contain relative offsets which the original runtime converts to in-memory pointers before LoadedResource state 3.

Current product-integration gap: `MOD/SCM/PNST` are represented in the C++ registry, while `EFM` and `SHW` still need explicit clean-product integration despite direct original-runtime evidence.

## 11. Rejected/superseded format claims

### `SCH`

**Status: `REJECTED` as current canonical format identity.**

An older project note described `st001.pac` as containing `PTX, SCM, SCH`. Later corpus reconciliation instead resolves the relevant stage payload surface into texture bundle, SCM, two HITS resources, text, PNST and nested PAC, with misleading `.ukn/.sch` naming possible.

Until an independent raw header and original consumer re-establish `SCH`, it is an acquisition hint only and must not appear as a confirmed format.

### `HITS$`

**Status: `REJECTED` as canonical magic.**

Current C++ parser and integration authority use four-byte `HITS`. The historical five-byte `HITS$` + marker-scanner model is superseded. `0x18060001` is an observed raw flag value, not a universal record delimiter.

## 12. Current unresolved acquisition/reverse frontier

The following named families remain real targets. Their presence in this section is deliberate: unknown purpose is preserved instead of silently dropping the family.

```text
PTZ
PHD
BD
TSB
SPUMAPDT
MRP
C1D
EFE
EFW
MCV
HID/HID2/HID3
TSC
CLT
```

Priority rule for each:

1. acquire one or more provenance-bound retail/corpus payloads;
2. identify magic/header/record grammar without filename assumptions;
3. find direct original-EXE consumer, typed dispatcher or descriptor binding;
4. classify purpose separately from field semantics;
5. add a clean parser only after the structural envelope is evidence-backed;
6. add writer/editing support only after round-trip and consumer validation.

## 13. StageCfg slot-9 / EST boundary

The residual EXE pass found a strong StageCfg relation:

```text
0x1401AF000
  if cfg PAC has >= 10 physical slots
    -> reads offset-table entry +0x2C (physical slot 9)
    -> passes slot-9 payload to 0x1401A9BC0
    -> semantic/dependency scan contributes enemy-resource demand
```

Corpus relation:

```text
st001cfg: 10 slots; EST observed among semantic children
st114cfg:  9 slots; EST absent
```

This is strong enough to record **EST as a stage dependency/control candidate**.

It is **not** sufficient to assert `physical slot 9 == EST` universally. An independent exact slot mapping/raw header is still required before promotion.

## 14. Product integration gaps exposed by the catalog

Current clean C++ coverage is intentionally narrower than the original runtime/corpus universe.

High-value gaps:

- add explicit clean integration descriptors/modules for `EFM` and `SHW`;
- reconcile LIG2 direct identifier evidence with the structural scanner's current probe policy;
- migrate the evidence-backed ITM semantic parser/editor core into clean C++ without reintroducing a private resolver;
- promote DCA semantic camera interpretation only after the clean module can expose the evidence safely;
- continue POS/STE/EST consumer reverse;
- distinguish every `.ukn` by bytes/context rather than extension;
- preserve runtime logical/physical translation for `ADX -> OGG` and `SFD -> WMV`;
- keep capability-only media formats separate from shipped-resource claims.

## 15. Canonical references

Use this catalog together with:

- `docs/gdspaces/classification.md` — current low-level GDSpaces classifier behavior;
- `src/integration/format_registry.cpp` — current clean-product integration maturity;
- `docs/gdspaces/l3-raw-exe-pass-2026-08-26.md` — typed post-load/lifecycle evidence;
- `docs/research/dmc3-vanilla-deep-research-wave-2.md` — cross-build stage/media/runtime research;
- `docs/research/dmc3-vanilla-research-baseline.md` — broader research baseline;
- `docs/stage/dmc3-stage-resource-plan.md` — Stage domain evidence and migration plan;
- `docs/reverse/dmc3-pc-save-pass32-implementation.md` — DMC3 save envelope;
- `docs/gdspaces/l3-residual-format-pass-2026-08-26.md` — residual format census and corrections from the final pass.

When any of those sources conflict, prefer the stronger/newer direct evidence and record the supersession explicitly rather than preserving two incompatible truths.
