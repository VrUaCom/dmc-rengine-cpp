# Current Blockers

**Snapshot date:** 2026-09-11  
**Canonical base reviewed:** `main@d8534badbbe52cae1610d624822874431f581fa9`  
**Latest reviewed promotion:** PR #372 — unified MOD/SCM/MOT consolidation and container gates  
**Native Reader registry authority:** directly verified on current `main`  
**Completion rule:** synthetic CI and registry membership never equal original-game equivalence or unrestricted writer authority.

## P0 — L1 blockers

### B-L1-01 — Trusted original resolver-selected provenance

**Status:** ❌ REQUIRED

Need one protected/original-process receipt binding process/build, successful mount topology, selected provider/volume/member, archive identity, materialized bytes and trusted observer state.

### B-L1-02 — Exact selected representation classification

**Status:** ❌ DEPENDS ON B-L1-01

Do not infer writer authority from filename, transformed texture data or an unrelated retail corpus.

### B-L1-03 — Same-lineage authored rebuild/rematerialization

**Status:** ⚠️ REAL RETAIL PNST GATE CLOSED / ORIGINAL-SELECTED LINEAGE OPEN

PR #372 closes provenance-bound real retail PNST reintegration for an authored MOD child:

- `m20_s00_012.pac` byte-classified as PNST;
- 33 physical slots;
- target MOD physical slot 23, offset 129280, size 1888;
- parent remains 346272 bytes;
- slot table unchanged;
- only three expected authored child bytes change in the full parent image;
- canonical reparse/re-expand returns exact writer output.

PR #372 also proves a synthetic MOD -> container -> NBZ overlay -> reopen chain with the existing NBZ writer/source path.

Still required:

```text
original-selected member
 -> exact representation classification
 -> evidenced bounded edit
 -> parent rebuild
 -> authored higher-numbered NBZ
 -> original resolver selects authored overlay
 -> exact authored member rematerializes
 -> original consumer reaches authored child
```

### B-L1-04 — Original consumer effect + rollback

**Status:** ❌ REQUIRED

Need SHA-bound authored overlay selection, deterministic original-game effect attributable to authored bytes, then rollback with original retail artifacts unchanged.

### B-L1-05 — Final L1 audit

**Status:** ❌ OPEN

## L2 blockers

### B-L2-01 — Wider collision scope

**Status:** ✅ EXACT `dmc3-0.nbz` CLOSED / ❌ WIDER SCOPE OPEN

Exact bound archive: 4,333 file keys / 4,333 unique / 0 collisions. Additional volumes and cross-volume normalized-key behavior remain open.

### B-L2-02 — Discovery vs successful mount topology

**Status:** ✅ CLOSED BY PR #287

### B-L2-03 — Protected-process RVA mapping

**Status:** ❌ REAL R2B RECEIPT REQUIRED

Canonical analysis VAs/RVAs cannot be applied to a different protected distribution build without independent mapping evidence.

### B-L2-04 — Trusted selected-provider identity

**Status:** ❌ REAL R3 RECEIPT REQUIRED

### B-L2-05 — Direct-retail original resolver winner

**Status:** ❌ ORIGINAL OBSERVATION REQUIRED

### B-L2-06 — Final L2 audit

**Status:** ❌ OPEN

## Native Reader registry boundary

Direct inspection of `NativeReaderModuleRegistry` on current `main` confirms canonical module registration for DDS, PTX, HITS, DCA, LIG2/LIG, Stage TXT, SCM, MOD, MOT, SO graph, SO volume, SO link, SHW and PE/EXE.

Therefore older blocker text saying “MOT canonical Native Reader module is open” is obsolete.

Registry membership is not full semantic/writer authority.

- ❌ EFM Native Reader module remains open.
- ❌ MRP/MCV/CAM/CLT/TSC and other recognized families remain evidence-gated.

## MOD boundary

**Status:** ✅ BOUNDED WRITER + REAL RETAIL PNST REINTEGRATION / ❌ FULL WRITER + RETAIL NBZ + ORIGINAL-GAME ACCEPTANCE OPEN

Closed at exact scopes:

- #365 Preserve-Layout Writer Gate 1;
- #368 38/38 provenance-bound retail no-op exact byte parity;
- #369 one provenance-bound controlled retail radius edit + receipt trust bridge + synthetic PAC reintegration;
- #372 provenance-bound real retail PNST reintegration/reopen;
- #372 synthetic MOD -> container -> NBZ overlay -> reopen.

Still open:

- typed-IR-only layout synthesis/reflow;
- transform authoring;
- skin/blend-index authoring;
- source-flag/material/texture-binding authoring;
- texture-companion rewrite/coherence;
- broader preserved-undecoded mutation authority;
- provenance-bound retail NBZ overlay acceptance;
- original `dmc3.exe` no-op/edited MOD acceptance;
- complete current animation/pose ownership;
- any `100% MOD writer` claim.

## SCM boundary

**Status:** ✅ CANONICAL BOUNDED WRITER/REBUILD STACK / ❌ PRODUCTION + ORIGINAL-GAME ACCEPTANCE OPEN

PR #372 closes the selected canonical implementation/corpus scope:

- `preserve_layout`;
- deterministic `canonical_rebuild`;
- typed geometry/normal/UV/texture-slot/alpha/filter/GS-CLAMP/node-transform editing;
- mandatory reparse and mutation guards;
- fail-closed reflow on non-zero unmodeled source bytes;
- bounded texture-companion coherence;
- 78 paths / 68 unique inputs with 78/78 parse, preserve-layout exact parity, canonical rebuild+reparse and canonical exact no-edit parity.

Still open:

- provenance-bound representative semantic edits;
- provenance-bound retail texture rewrite;
- real-retail size-changing rebuild;
- SCM PAC/PNST/NBZ reintegration;
- original-game acceptance;
- universal/100% production claim.

## MOT boundary

**Status:** ✅ CANONICAL NATIVE READER MODULE + PARSER/IR + BOUNDED NORMAL-PATH CHANNEL EVALUATION / ❌ COMPLETE PLAYER PARITY OPEN

MOT already exists in the canonical Native Reader registry as `native_reader_modules::mot()`. PR #372 replaces duplicate structural decoding with the modular parser/IR and promotes bounded canonical-EXE-backed key evaluation. The current `reverse/mod-completion-20260907` branch additionally closes the static compression-3 cached-search path and composes it with exact normal-path T/R/S channel binding plus the MOD motion-group selector.

Closed at current scope:

- `MOT\0` marker and aligned header/channel-mask contract;
- nine-bit channel mask and record/popcount relation;
- compression-2/3 typed key payloads;
- three hash-bound real MOT payloads through the modular parser;
- exact normal-path binding-bit traversal and track ordinal consumption;
- exact Translation/Rotation/Scale channel semantics and CMotionJoint channel bases;
- signed track start-time offsets;
- quantization decode;
- compression-3 forward/backward cached segment search at `0x1402E8C80..0x1402E8E10`;
- endpoint behavior and cache-dependent duplicate-time selection in the recovered static algorithm;
- compression-3 linear/Hermite segment algebra and slope orientation;
- group-aware compression-3 scalar projection through the EXE-confirmed MOD motion-group selector.

Still open:

- execution/differential confirmation of cache lifecycle and bit-identical SSE parity where required;
- header flag `0x2` alternate binding at `0x140310CBF`;
- compression-2 whole-track evaluation parity and other compression modes;
- exact mutable CMotion channel-state ownership;
- T/R/S channel state -> animated local matrix construction around the normal `0x14030E9B0` path;
- looping/blending/motion-selection/scheduler lifecycle;
- original-game output comparison.

The next direct-EXE acquisition surface is tracked by `data/reverse/dmc3-mot-local-matrix-window-plan.v1.json`. Do not promote `joint+0x110` to a final animated-local matrix until that packet closes ownership and write ordering.

## SO boundary

**Status:** ✅ SO GRAPH/VOLUME/LINK REGISTRY MEMBERSHIP / ⚠️ SEMANTIC + WRITER CLAIMS REMAIN EVIDENCE-GATED

SO registry presence is canonical product integration only. Do not convert it into universal SO semantic identity, format parity or writer authority without dedicated evidence.

## SHW boundary

**Status:** ✅ STRUCTURAL READER / ❌ WRITER OPEN

Matrix-palette ownership, universal revision coverage and authored-resource original-game acceptance remain open.

## L3 blockers

### B-L3-01 — Current-main R1 promotion

**Status:** ⚠️ RESEARCH EXISTS / PROMOTION OPEN

### B-L3-02 — Family/backing ownership

**Status:** ❌ OPEN

### B-L3-03 — Scheduler terminal dependency

**Status:** ❌ FRESH BOUNDED PASS REQUIRED

Targets remain `0x1402EF4D0`, `0x1402EF790`, `0x1400333E0`, `0x140033390`, `0x1400335A0`, `0x1402EF460`, with `0x1401B8DC0` as regression anchor.

### B-L3-04 — V1–V7 original-process receipts

**Status:** ❌ OPEN

### B-L3-05 — Final L3 audit

**Status:** ❌ OPEN

## Closed foundations — do not reopen absent contradiction

- atomic/no-replace publication and ByteProvenance product policies;
- numbered-volume/first-gap bootstrap;
- bounded `OpenGameResource` policy;
- archive/physical normalization;
- type-0 physical final-open contract;
- successful-mount topology correction (#287);
- SHW structural reader integration (#288);
- exact `dmc3-0.nbz` zero-collision receipt;
- MOD Gate 1/no-op/controlled edit (#365/#368/#369);
- MOD retail PNST + synthetic NBZ reopen (#372);
- SCM selected writer stack + 68-unique/78-path no-edit corpus (#372);
- MOT canonical registry membership plus parser/IR/key-evaluation consolidation (#372 for the parser/IR and evaluation slice);
- bounded compression-3 cached search + normal T/R/S channel/motion-group scalar bridge on `reverse/mod-completion-20260907`;
- SO graph/volume/link canonical registry membership.

## Current access boundary

Trusted protected-process/install observations required for remaining L1/L2/L3 original-process gates are still missing. Static/corpus/product evidence must not be upgraded into original-runtime acceptance.
