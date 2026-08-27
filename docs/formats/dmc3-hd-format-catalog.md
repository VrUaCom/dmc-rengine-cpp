# DMC3 HD Format and Resource-Purpose Catalog

**Snapshot:** 2026-08-27  
**Reconciliation base:** `main@f886f27e62ec9a05b6829df7fd074981a06a4b49`  
**Canonical analysis EXE SHA-256:** `e454272ed0fb0247fcbcf300e5d55d7a3e96d50b89b9ffaff81bb978dcbdd082`  
**Scope:** all DMC3-HD resource families currently observed/named in the project corpus, canonical executable, current C++ product code or preserved reverse evidence.

This is the canonical human-readable inventory for **what each currently known DMC3-HD resource family is for** and how strong that conclusion is.

Machine-readable companion:

- [`dmc3-hd-format-purpose-registry.json`](dmc3-hd-format-purpose-registry.json)

Latest direct closure evidence:

- [`../research/dmc3-format-purpose-closure-pass-2026-08-27.md`](../research/dmc3-format-purpose-closure-pass-2026-08-27.md)

The registry is exhaustive over the currently observed/named evidence set. That does **not** mean every binary field or writer contract is known.

## 1. Authority and interpretation rules

Evidence order:

```text
validated magic / structural grammar
 -> direct original-EXE consumer or typed post-load branch
 -> hash-bound corpus structure
 -> contextual/index/path label
 -> unknown
```

Keep four independent questions separate:

1. **Identity** — what family is the payload?
2. **Purpose** — which subsystem uses it?
3. **Schema** — which byte fields are understood?
4. **Product support** — what can current clean C++ safely do with it?

A known purpose is not a complete schema. A parser is not proof of complete original-game behavior. A filename extension is never semantic authority by itself.

### Evidence status

| Status | Meaning |
|---|---|
| `GAME_VERIFIED` | bounded behavior observed in the original game |
| `EXE_CONFIRMED` | direct canonical executable evidence |
| `DATA_CONFIRMED` | stable binary/corpus evidence |
| `HIGH_CONFIDENCE` | convergent evidence; one promotion boundary remains |
| `CANDIDATE` | evidence-linked interpretation, not yet canonical semantics |
| `RESEARCH_REQUIRED` | identity/subsystem or label exists; exact purpose/schema remains open |
| `CAPABILITY_ONLY` | media/runtime layer can support it; shipped DMC3 presence is not claimed |
| `REJECTED` | superseded/contradicted claim |

### Product support vocabulary

| Support | Meaning |
|---|---|
| `structural` | bounded current C++ parser/scanner/source adapter exists |
| `recognized` | clean registry/classifier knows the family, but no complete parser |
| `runtime-only` | original runtime directly handles it; clean integration is absent/incomplete |
| `research-only` | known from reverse/corpus, not migrated as a clean product module |
| `capability-only` | executable/media capability only |
| `fallback-only` | metadata/extension fallback, not a semantic parser |

## 2. Containers, namespaces and naming metadata

| Family | Purpose | Evidence | Product | Boundary |
|---|---|---|---|---|
| `NBZ` | numbered top-level HD resource volume; ZIP-backed materialization surface | `EXE_CONFIRMED` + product validation | `structural` | overlay writer equivalence is bounded product behavior, not Capcom packer equivalence |
| `PAC` / `PAC0` | relative-slot resource container; preserves sparse/empty/alias slot topology | `EXE_CONFIRMED` + `DATA_CONFIRMED` | `structural` | physical slot is identity; ordinal is not universal semantic type |
| `PNST` | distinct relative-slot container recursively traversed by original runtime | `EXE_CONFIRMED` + `DATA_CONFIRMED` | `structural` | envelope similarity to PAC does not imply the same slot schema |
| `PACK` | legacy/observed container label | `DATA_CONFIRMED`; original runtime authority `RESEARCH_REQUIRED` | `research-only` | do not promote without direct original consumer evidence |
| `.index` | extraction/display naming manifest and child metadata | `DATA_CONFIRMED` | recognized metadata | not runtime lookup authority and not semantic-format authority |
| `.lst` | original loose/list-driven PAC fallback manifest | `EXE_CONFIRMED` bounded | `runtime-only` | full grammar/lifetime/recursion/error semantics remain open |
| `GData.afs/`, `GDataX360.afs/`, `AFS` tokens | logical resource namespace / legacy AFS surface | `EXE_CONFIRMED` | recognized namespace | not proof of an opaque binary AFS backend in the current HD path |
| `.ukn` | placeholder/source extension where extraction lacks semantic identity | `DATA_CONFIRMED` | `fallback-only` | known `.ukn` payloads include HITS and ITM; never route by extension alone |
| `.bin` | generic leaf extension | contextual | `fallback-only` | multiple unrelated resources use it; bytes/context decide identity |
| `PE` / `dmc3.exe` | runtime executable, registries, descriptors and translation tables | `EXE_CONFIRMED` | `structural` | analysis-build and protected execution-build authority stay separate |

## 3. Geometry, models and collision

| Family | Purpose | Evidence | Product | Boundary |
|---|---|---|---|---|
| `SCM` | stage/scene mesh geometry and model data | `EXE_CONFIRMED` typed classifier/fixup + corpus | recognized | full mesh/material/ownership schema open |
| `MOD` | actor/object/model resource | `EXE_CONFIRMED` typed classifier/fixup + corpus | recognized | complete schema/variants/writer contract open |
| `HITS` | spatial collision: bounds + 3-D grid -> cell triangle refs -> triangle/plane records | `EXE_CONFIRMED` + `DATA_CONFIRMED` | `structural` | raw flag semantics incomplete; `0x18060001` is not a universal delimiter |
| `DCA` | mini-demo/cinematic camera data; paired spatial channels correlate with camera eye/world position and look-at target | `DATA_CONFIRMED`, purpose `HIGH_CONFIDENCE` | `structural` | 0x10 header + 0x410 records are structural; exact field semantics open |

## 4. Texture, effect and render families

| Family | Purpose | Evidence | Product | Boundary |
|---|---|---|---|---|
| `DDS` | HD texture representation | `DATA_CONFIRMED` + direct signature | recognized | compression/layout tags are subformats |
| `PTX` | texture bundle/container-like resource exposing texture children | `EXE_CONFIRMED` typed extension branch + corpus | recognized | PTX/TIM2/PAC-bundle/extracted DDS identities remain distinct |
| `TM2` / `TIM2` | original/legacy texture image family | `EXE_CONFIRMED` identifier path + research | research-only | exact HD conversion/ownership chain open |
| `PTZ` | texture-side companion associated with `basic.ptx` | static presence `EXE_CONFIRMED`; purpose `HIGH_CONFIDENCE` | research-only | direct typed consumer/schema still open |
| `EFM` | effect-system typed resource with dedicated post-load fixup | `EXE_CONFIRMED` | runtime-only | exact fields/product module open |
| `SHW` | shadow/render typed companion with dedicated post-load fixup | `EXE_CONFIRMED`; purpose `HIGH_CONFIDENCE` | runtime-only | exact schema open |
| `MRP` | primary model/render/effect-side typed resource-manager companion | identity `EXE_CONFIRMED`; purpose `HIGH_CONFIDENCE` | runtime-only | exact acronym expansion and fields remain open |
| `SEF` | stage/effect companion | `DATA_CONFIRMED`; purpose `HIGH_CONFIDENCE` | research-only | direct consumer/schema requires reacquisition |
| `EFE` | effect-system companion label | `DATA_CONFIRMED`, exact purpose `RESEARCH_REQUIRED` | research-only | no direct original consumer recovered |
| `EFW` | effect-system companion label | `DATA_CONFIRMED`, exact purpose `RESEARCH_REQUIRED` | research-only | no direct original consumer recovered |

### Direct primary typed classifier

The canonical executable now gives a hard type map:

```text
0x1402DB1F0
MOD -> 0
EFM -> 1
SCM -> 2
MRP -> 3
SHW -> 7

0x1402DB3C0
.ptx -> 4
.clt -> 5
.c1d -> 6
```

Dedicated post-load calls observed in the same path:

```text
MOD -> 0x1402FE3B0
EFM -> 0x1402F7A90
SCM -> 0x1403051B0
SHW -> 0x1403204C0
```

`MRP` is therefore a real original-runtime typed family, even though its exact field semantics are still unknown.

### DDS encodings are not separate file families

```text
DXT1 DXT2 DXT3 DXT4 DXT5
ATI1 ATI2
BC4 BC5
RGBG GRGB YUY2
```

These are DDS/pixel encoding or layout metadata.

## 5. Cloth, animation, camera and visibility/control families

The original motion/control manager at `0x1402E01A0` assigns:

```text
.mot -> 0
.mcv -> 1
.cam -> 2
.hid -> 3
.clt -> 4
.tsc -> 5
```

| Family | Purpose | Evidence | Product | Boundary |
|---|---|---|---|---|
| `MOT`, `MOT2`..`MOT6` | motion/animation resources and corpus variants | base `MOT` `EXE_CONFIRMED`; variants `DATA_CONFIRMED` | research-only | exact variant ABI/bindings remain open |
| `MCV` | motion companion/control/curve resource | identity/subsystem `EXE_CONFIRMED`; purpose `HIGH_CONFIDENCE` | runtime-only | exact field semantics and MOT relationship open |
| `CAM` | camera resource/track family | `EXE_CONFIRMED` manager registration + corpus | recognized | full camera schema/writer contract open |
| `HID`, `HID2`, `HID3` | hide/visibility-control family | base `HID` `EXE_CONFIRMED`; variants `DATA_CONFIRMED`; purpose `HIGH_CONFIDENCE` | research-only | track schema/variant ABI open |
| `CLT` | cloth/deformation simulation companion used by primary and motion/control managers | identity `EXE_CONFIRMED`; purpose `HIGH_CONFIDENCE` | runtime-only | exact binary schema open |
| `C1D` | one-dimensional cloth-simulation companion | identity `EXE_CONFIRMED`; purpose `HIGH_CONFIDENCE` | runtime-only | exact binary schema open |
| `TSC` | motion/control-manager companion | identity/subsystem `EXE_CONFIRMED` | runtime-only | exact semantic role/schema `RESEARCH_REQUIRED` |

`CLT`/`C1D` purpose is supported by manager placement plus original `Cloth`, `ClothSim1D...` and `ResetClt...` token families. This is strong subsystem evidence, not permission to invent field names.

## 6. Stage configuration and gameplay-side families

StageCfg corpus summary:

```text
st001cfg.pac: 10 physical slots; observed semantic families include
LIG/CAM/EVE/POS/ITM/STE/DCA/EST

st114cfg.pac: 9 physical slots; observed semantic families include
LIG/CAM/EVE/POS/ITM/STE/DCA
```

| Family | Purpose | Evidence | Product | Boundary |
|---|---|---|---|---|
| `LIG` | stage lighting data | `DATA_CONFIRMED` | `structural` | full field semantics/variant relation open |
| `LIG2` | stage lighting record resource; 0x20 header + 0x30 records | identifier `EXE_CONFIRMED` + `DATA_CONFIRMED` | `structural` | record field semantics open |
| `EVE` | spatial event/trigger volumes: polygon corners + height + raw id/type/tag | geometry `DATA_CONFIRMED` | research-only | not mission bytecode; id/type/tag semantics open |
| `POS` | stage position/placement/transform companion | corpus purpose `HIGH_CONFIDENCE` | research-only | exact record schema/direct consumer open |
| `ITM` | static item placement: item ID + X/Y/Z + Y rotation | `EXE_CONFIRMED` + `DATA_CONFIRMED`; bounded game experiments exist | recognized | persistence/behavior comes from EXE/EventTbl |
| `STE` | scene/effect transform records | `DATA_CONFIRMED`; purpose `HIGH_CONFIDENCE` | research-only | exact record semantics/direct consumer open; short EXE ASCII hits were false positives |
| `EST` | stage dependency/control payload candidate | `CANDIDATE` with direct StageCfg slot-9 consumer correlation | research-only | do not claim universal `slot9 == EST` without independent raw mapping |
| stage `TXT` | stage script/config text; StageSet/room/door/effect configuration | `EXE_CONFIRMED` parser/token evidence | `structural` | individual token semantics partially open |
| `EventTblNN.bin` | mission/event control bytecode: conditions, inventory changes, dynamic spawns, mission flow | load/scan `EXE_CONFIRMED`, semantics `HIGH_CONFIDENCE` | research-only | full header/opcode/branch semantics open |

### EVE structural boundary

```text
+0x00 char[4] EVE\0
+0x04 u16 family/version (0x0100 observed)
+0x06 u16 count
records @ 0x10, stride 0x80
  +0x20 four vec4 polygon corners
  +0x60 f32 height
  +0x64..0x7F raw tail
```

### ITM under misleading `.ukn`

Confirmed stock fixtures:

```text
st001cfg_006.ukn  size 48   count 1
st010cfg_006.ukn  size 48   count 1
st105cfg_006.ukn  size 64   count 2
st114cfg_006.ukn  size 400  count 19
```

Canonical ITM record:

```text
+0x00 char[4] ITM\0
+0x04 u16 version/family
+0x06 u16 count
+0x08..0x0F preserved raw
records @ 0x10, stride 0x14:
  u32 itemId
  f32 X, Y, Z
  f32 rotationY
```

### StageCfg slot-9 / EST boundary

```text
0x1401AF000
  cfg slotCount >= 10
    -> offset-table +0x2C (physical slot 9)
    -> payload passed to 0x1401A9BC0
    -> dependency/control scan contributes enemy-resource demand
```

`st001cfg` has 10 slots and EST among semantic children; `st114cfg` has 9 and no EST. This supports an EST dependency/control hypothesis but does not prove universal slot identity.

## 7. Audio families

HD audio includes a logical-to-physical translation:

```text
legacy ADX logical identity
 -> basename translation in dmc3.exe
 -> physical OGG stream
 -> EXE-owned loopStartMs / loopEndMs
```

| Family | Purpose | Evidence | Product | Boundary |
|---|---|---|---|---|
| `ADX` | legacy logical audio identity | `EXE_CONFIRMED` | runtime translation | physical HD asset may be OGG |
| `OGG` | physical HD audio stream representation | `EXE_CONFIRMED` | runtime/research | loop metadata authority is EXE descriptor, not comments |
| `VAGp` | Sony VAG ADPCM audio payload/stream marker | identifier `EXE_CONFIRMED`; purpose `HIGH_CONFIDENCE` | research-only | exact DMC3 ownership/container relation open |
| `PHD` | global sound-system bank/header descriptor companion | sound-subsystem identity `EXE_CONFIRMED`; exact role `HIGH_CONFIDENCE` | research-only | exact fields/linkage open |
| `TSB` | global sound-system table/bank companion | sound-subsystem identity `EXE_CONFIRMED`; exact role `HIGH_CONFIDENCE` | research-only | exact fields/linkage open |
| `BD` | global sound-system bank body/sample-data companion | sound-subsystem identity `EXE_CONFIRMED`; exact role `HIGH_CONFIDENCE` | research-only | exact fields/linkage open |
| `SPUMAPDT` / `SpuMap.bin` | SPU sound-memory/sample-bank partition/region-map metadata | `EXE_CONFIRMED` parser | runtime-only | exact region-to-bank/sample linkage open |

Canonical executable system-audio table:

```text
SpuMap.bin
snd_sys.phd
snd_sys.tsb
snd_sys.bd
```

Source-tree-style evidence in the same EXE also names the same four under `..\..\mw\CSE\DATA\...`.

### SPUMAPDT parser

`0x140339D80`:

```text
magic[0..7] == "SPUMAPDT"
count-like u32 @ +0x08
up to 16 u32 region sizes @ +0x10
cumulative regions
cumulative bound <= 0x200000
```

This promotion is stronger than acronym inference: the original executable actually parses this structure.

## 8. Video/media families

Original HD translation:

```text
legacy SFD logical path -> replace extension with .wmv -> media loader
```

| Family | Purpose | Evidence | Status |
|---|---|---|---|
| `SFD` | legacy logical movie identity | `EXE_CONFIRMED` | logical representation |
| `WMV` | physical HD movie representation | `EXE_CONFIRMED` | shipped/runtime path |
| `PSS` | media/container capability | `EXE_CONFIRMED` capability string | `CAPABILITY_ONLY` |
| `THP` | media/container capability | `EXE_CONFIRMED` capability string | `CAPABILITY_ONLY` |
| `PAM` | media/container capability | `EXE_CONFIRMED` capability string | `CAPABILITY_ONLY` |
| `XMV` | media/container capability | `EXE_CONFIRMED` capability string | `CAPABILITY_ONLY` |
| `PMF` | media/container capability | `EXE_CONFIRMED` capability string | `CAPABILITY_ONLY` |
| `AVI` | media/container capability | `EXE_CONFIRMED` capability string | `CAPABILITY_ONLY` |
| `MPG` | media/container capability | `EXE_CONFIRMED` capability string | `CAPABILITY_ONLY` |
| `BIK` | media/container capability | `EXE_CONFIRMED` capability string | `CAPABILITY_ONLY` |
| `MP4` | media/container capability | `EXE_CONFIRMED` capability string | `CAPABILITY_ONLY` |

Capability does not prove a retail DMC3-HD payload exists in that family.

## 9. Save, font and legacy UI sidecars

| Family | Purpose | Evidence | Product | Boundary |
|---|---|---|---|---|
| `dmc3.sav` | game-progress persistent state: global record + slot summaries + slot payloads/checksums | `DATA_CONFIRMED` + reverse implementation | structural/research | bounded size `0x4A30`; gameplay field map continues |
| `options.sav` | configuration/options persistence | subsystem/path `EXE_CONFIRMED`; purpose `HIGH_CONFIDENCE` | research-only | exact binary schema open |
| `FON` | font/glyph definition resource for UI/text | static paths `EXE_CONFIRMED`; purpose `HIGH_CONFIDENCE` | research-only | glyph/atlas schema and exact loader open |
| `ICO` (`icon00/01/02.ico`) | legacy save/icon image sidecar | static table `EXE_CONFIRMED`; purpose `HIGH_CONFIDENCE` | research-only | exact HD runtime use/interpretation open |
| `icon.sys` | legacy save-icon metadata sidecar | static table `EXE_CONFIRMED`; purpose `HIGH_CONFIDENCE` | research-only | exact HD runtime use/schema open |

Canonical font paths include `font\euro28.fon`, `font\china20m.fon`, `font\china20z.fon`.

## 10. Runtime typed-postload authority

Materialized bytes are not always final consumer representation. Original DMC3 code converts relative structures for several families before LoadedResource-ready state.

Known typed paths include:

```text
MOD  -> dedicated in-place fixup
EFM  -> dedicated in-place fixup
SCM  -> dedicated in-place fixup
SHW  -> dedicated in-place fixup
PNST -> recurse non-empty children
PAC  -> root/child traversal before typed dispatch where applicable
```

The clean C++ product registry is intentionally narrower than this original-runtime universe. Documentation must not hide that gap.

## 11. Non-resource/debug strings and synthetic format

The executable string census also contains extension-looking strings which are not promoted as game resource families:

| String family | Classification |
|---|---|
| `HLSL`, `FXH` | shader source/build/debug path evidence; not proof of shipped resource files |
| `PDB` | debug-symbol path |
| `DLL` | runtime dependency/library names |
| `DXT*`, `ATI*`, `BC*`, `RGBG`, `GRGB`, `YUY2` | DDS/pixel encodings/layout tags |

`SLTC` exists in the clean repository as a **synthetic test-only slot container**. It is not original DMC3 evidence and must never be presented as a game-compatible format.

## 12. Rejected/superseded claims

### `SCH`

**`REJECTED` as current canonical format identity.**

An older note described a `st001` payload as `SCH`; later corpus reconciliation instead resolves the relevant surface through texture bundle/SCM/HITS/text/PNST/PAC identities. Until an independent header and original consumer re-establish `SCH`, it is acquisition history only.

### `HITS$`

**`REJECTED` as canonical magic.**

Current authority uses four-byte `HITS`. The old five-byte `HITS$` scanner model is superseded; `0x18060001` is an observed flag value, not a universal record marker.

## 13. Exact-purpose/schema frontier after this closure pass

The subsystem purpose of most named families is now bounded, but these exact contracts remain open:

```text
PTZ       exact consumer/schema
PHD       exact fields and cross-links
TSB       exact fields and cross-links
BD        exact fields and cross-links
EFE       exact consumer/schema
EFW       exact consumer/schema
POS       exact record schema/consumer
STE       exact record schema/consumer
EST       exact header/identity/universal slot map
MRP       exact field semantics
MCV       exact field semantics
HID*      exact track schema/variant ABI
TSC       exact semantic role/schema
CLT/C1D   exact binary schemas
```

This distinction is deliberate: **we have determined the strongest evidence-backed purpose of each currently named family; we have not fabricated exact semantics where the bytes/runtime have not proved them.**

## 14. Product integration gaps exposed by the catalog

High-value clean C++ gaps:

- add explicit descriptors/modules for direct-runtime `EFM`, `SHW`, `MRP`, `CLT`, `C1D`, `MCV`, `HID`, `TSC`;
- preserve `SPUMAPDT` parser evidence before implementing an audio-bank module;
- migrate evidence-backed ITM semantics without introducing a second resource resolver;
- continue POS/STE/EST exact consumer reverse;
- distinguish every `.ukn` by bytes/context;
- preserve ADX -> OGG and SFD -> WMV logical/physical translation;
- keep capability-only media formats separate from shipped-resource claims.

These are integration targets, not permission to add speculative writers.

## 15. Canonical references

- `docs/formats/dmc3-hd-format-purpose-registry.json` — machine-readable format-purpose authority;
- `docs/research/dmc3-format-purpose-closure-pass-2026-08-27.md` — direct EXE closure evidence from this pass;
- `docs/gdspaces/l3-residual-format-pass-2026-08-26.md` — prior residual census and StageCfg corrections;
- `docs/gdspaces/l3-raw-exe-pass-2026-08-26.md` — typed post-load/lifecycle evidence;
- `docs/research/dmc3-vanilla-deep-research-wave-2.md` — stage/media/runtime research;
- `docs/research/dmc3-vanilla-research-baseline.md` — broader research baseline;
- `docs/stage/dmc3-stage-resource-plan.md` — Stage-domain evidence;
- `docs/reverse/dmc3-pc-save-pass32-implementation.md` — save envelope;
- `src/integration/format_registry.cpp` — current clean-product integration maturity.

When sources conflict, prefer stronger/newer direct evidence and record supersession explicitly.
