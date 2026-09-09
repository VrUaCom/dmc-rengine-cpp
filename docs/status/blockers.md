# Current Blockers

**Snapshot date:** 2026-09-09  
**Canonical base reviewed:** `main@d8534badbbe52cae1610d624822874431f581fa9`  
**Latest reviewed promotion on current main:** PR #372 — unified MOD/SCM/MOT consolidation and container gates (merged after PR #375)  
**Latest Native Reader module promotion:** PR #288 — evidence-backed SHW Native Reader + cross-registry integration hardening  
**Completion rule:** original-DMC3 claims require the authority appropriate to the claim; synthetic CI never equals original-game equivalence.

The current proof execution order is [DMC Rengine Roadmap](../roadmap.md) plus [GDSpaces Proof Roadmap](../gdspaces/proof-roadmap-2026-09-05.md).

## P0 — GDSpaces L1 completion blockers

### B-L1-01 — Direct-retail representative provenance

**Status:** ❌ TRUSTED ORIGINAL/PROTECTED-PROCESS RECEIPT REQUIRED

Product acquisition tooling and independent retail corpus evidence exist, but they do not prove what the original protected process selected. A valid receipt must bind the protected executable, successful mount topology, resolver-selected provider/volume/member, archive identity, materialized bytes and trusted observer state.

### B-L1-02 — Exact selected representation classification

**Status:** ❌ DEPENDS ON B-L1-01

Classify the exact original-selected bytes before applying any writer authority. Filename, transformed texture data or an unrelated retail corpus is insufficient.

### B-L1-03 — Same-lineage authored rebuild/rematerialization

**Status:** ⚠️ RESOURCE/CONTAINER GATES ADVANCED / ORIGINAL-SELECTED LINEAGE OPEN

PR #372 closes a previously missing **provenance-bound real retail PNST reintegration** gate for a same-size authored MOD child:

- retail parent `m20_s00_012.pac` is byte-classified as PNST with 33 physical slots;
- target MOD is physical slot 23 at offset 129280, size 1888;
- controlled radius edit changes exactly three child bytes;
- parent size remains 346272 bytes;
- slot table and every unrelated parent byte remain unchanged;
- canonical reparse/re-expand returns the exact authored MOD bytes.

PR #372 also proves a **synthetic** MOD -> container -> NBZ overlay -> reopen chain using the existing NBZ writer/source implementation.

What remains for L1 is stronger and same-lineage:

```text
original resolver-selected member
 -> exact representation classification
 -> evidenced bounded edit
 -> bottom-up parent rebuild
 -> authored higher-numbered NBZ
 -> original resolver selects that overlay
 -> exact rebuilt member rematerializes
 -> authored child reaches the original consumer
```

The retail PNST receipt is real evidence, but it is not a trusted original-process resolver-selection receipt and the synthetic NBZ gate is not retail/original-runtime acceptance.

### B-L1-04 — Original DMC3 consumption + rollback

**Status:** ❌ FINAL EXTERNAL ACCEPTANCE REQUIRED

A generated overlay must be SHA-bound, selected by the original runtime, produce an observable effect attributable to authored bytes, and then be removed while original retail artifacts remain byte-identical. A crash-free launch is insufficient.

### B-L1-05 — Final L1 audit

**Status:** ❌ OPEN / DEPENDS ON B-L1-01..04

Requires trusted original selection provenance, exact representation classification, same-lineage authored overlay selection/rematerialization, consumer observation, rollback, exact-head CI and contradiction review.

## Layer 2 blockers

### B-L2-01 — Retail normalized-key collision scope

**Status:** ✅ CLOSED FOR EXACT `dmc3-0.nbz` / ❌ WIDER SCOPE OPEN

Bound receipt remains 4,333 file keys / 4,333 unique / 0 collisions for the exact archived artifact. Additional volumes and cross-volume normalized-key behavior remain open for wider claims.

### B-L2-02 — Discovery vs successful mount topology

**Status:** ✅ CLOSED / PRODUCT CORRECTION PROMOTED BY PR #287

Discovery/registration attempt is not successful linked runtime mount topology. Current product code preserves that distinction.

### B-L2-03 — Real protected-distribution RVA mapping

**Status:** ❌ TOOLING EXISTS / REAL ORIGINAL-PROCESS RECEIPT REQUIRED

Canonical analysis executable and protected distribution execution candidate remain separate authorities. Canonical VAs/RVAs require independent mapping into the exact protected process.

### B-L2-04 — Trusted selected-provider identity

**Status:** ❌ BLOCKED BY REAL R2B + TRUSTED PUBLISHER

A valid R3 promotion requires a trusted, lossless observer bound to the exact process and mounted artifacts. Editable content cannot self-assert original evidence.

### B-L2-05 — Direct-retail original resolver winner

**Status:** ❌ ORIGINAL OBSERVATION REQUIRED

The bound `dmc3-0.nbz` key surface is clean; the actual original resolver winner still needs trusted process evidence.

### B-L2-06 — Final L2 audit

**Status:** ❌ OPEN

## Native Reader / model-format boundaries

### SHW

**Status:** ✅ STRUCTURAL READER CANONICAL / ❌ WRITER OPEN

PR #288 remains the canonical structural/read-only SHW promotion. Matrix-palette ownership, universal revision coverage and writer/original-game authoring acceptance remain open.

### MOD

**Status:** ✅ BOUNDED WRITER + REAL RETAIL PNST REINTEGRATION / ❌ FULL WRITER + RETAIL NBZ + ORIGINAL-GAME ACCEPTANCE OPEN

Canonical milestones now include:

- ✅ reader/reverse promotion stack #305–#323 plus #356/#359/#360/#363;
- ✅ Preserve-Layout Writer Gate 1 (#365);
- ✅ deterministic writer corpus harness (#367);
- ✅ 38/38 provenance-bound retail no-op exact byte parity (#368);
- ✅ one provenance-bound controlled retail bounding-radius edit (#369);
- ✅ writer receipt -> `AuthoredChildImage` trust bridge + synthetic PAC reintegration (#369);
- ✅ provenance-bound real retail PNST reintegration/reopen (#372);
- ✅ synthetic MOD -> container -> NBZ overlay -> reopen (#372).

Still open before a **full production MOD writer** claim:

- layout synthesis/reflow or typed-IR-only rebuild;
- transform authoring;
- skin/blend-index authoring;
- source-flag/material/texture-binding authoring;
- texture-companion rewriting/coherence;
- broader mutation authority for preserved-undecoded fields;
- provenance-bound **retail NBZ** overlay acceptance for the authored MOD lineage;
- original `dmc3.exe` no-op/edited MOD acceptance;
- complete current animation/pose ownership;
- a `100% MOD writer` claim.

### SCM

**Status:** ✅ CANONICAL BOUNDED WRITER/REBUILD STACK / ❌ PRODUCTION + ORIGINAL-GAME ACCEPTANCE OPEN

PR #372 promotes the selected canonical SCM authoring stack:

- `preserve_layout` source-bound same-layout authoring;
- deterministic `canonical_rebuild` typed-IR layout planning;
- typed geometry/normal/UV/texture-slot/alpha/filter/GS-CLAMP/node-transform editing;
- dependent metadata derivation and mandatory canonical output reparse;
- source-bound mutation guards;
- fail-closed canonical reflow on non-zero unmodeled source bytes;
- bounded SCM/texture-companion coherence via `ScmResourceBundleWriter`.

Consolidated no-edit corpus is closed at its explicit scope: 78 paths / 68 unique SHA-256 inputs; 78/78 parse, preserve-layout exact parity, canonical rebuild+reparse and canonical exact no-edit parity.

Still open:

- provenance-bound representative semantic edits across SCM domains;
- provenance-bound retail texture rewrite;
- real-retail size-changing canonical rebuild;
- SCM PAC/PNST/NBZ reintegration;
- original `dmc3.exe` acceptance;
- any `100%` or universal-production claim.

### MOT

**Status:** ✅ CANONICAL PARSER/IR + BOUNDED KEY EVALUATION / ❌ COMPLETE PLAYER PARITY OPEN

PR #372 consolidates MOT onto one modular parser/IR and promotes canonical-EXE-backed recovery for nine-channel binding traversal, signed track start-time offsets, quantization and compression-3 linear/Hermite segment algebra. Three hash-bound real MOT payloads parse through the canonical structural path.

Still open:

- exact segment lookup/cache and duplicate-time behavior;
- flag `0x2` alternate binding path;
- additional compression modes;
- looping/blending/motion selection/full transform composition;
- bit-identical SSE parity where required;
- original-game output comparison;
- built-in Native Reader module promotion if/when that product surface is added.

## Layer 3 blockers

### B-L3-01 — Current-main R1 contradiction-gated closure

**Status:** ⚠️ RESEARCH CONCLUSION EXISTS / CANONICAL PROMOTION OPEN

### B-L3-02 — R2 family/backing ownership

**Status:** ❌ OPEN

### B-L3-03 — Materialization scheduler terminal dependency

**Status:** ❌ FRESH BOUNDED CANONICAL PASS REQUIRED

Fresh raw targets remain `0x1402EF4D0`, `0x1402EF790`, `0x1400333E0`, `0x140033390`, `0x1400335A0`, `0x1402EF460`, with `0x1401B8DC0` as regression anchor. Availability of the canonical executable does not close these address/range/scope-specific targets.

### B-L3-04 — V1–V7 original-process receipts

**Status:** ❌ OPEN

### B-L3-05 — Final L3 audit

**Status:** ❌ OPEN

## Closed foundations — do not reopen absent contradiction

- ➖ atomic/no-replace publication and artifact-bound SHA/ByteProvenance;
- ✅ numbered-volume / first-gap bootstrap;
- ✅ six-prefix `OpenGameResource` bounded direct-call policy;
- ✅ archive `0x0E` / physical `0x0C` normalization;
- ✅ type-0 physical final-open/miss bounded contract;
- ✅ successful-mount topology correction (#287);
- ✅ SHW structural Native Reader integration (#288);
- ✅ PAC/PNST traversal and PAC slot-0 traversal;
- ✅ LoadedResource state1 -> state2 -> typed post-load/callback -> state3 bounded spine;
- ✅ cancellation/quiescence bounded rules;
- ✅ exact `dmc3-0.nbz` zero-collision receipt;
- ✅ `.index` rejected as recovered runtime materialization authority on the canonical path;
- ✅ MOD Gate 1/no-op/controlled edit (#365/#368/#369);
- ✅ MOD real retail PNST reintegration and synthetic NBZ reopen (#372);
- ✅ SCM 68-unique/78-path no-edit writer corpus and canonical selected writer stack (#372);
- ✅ MOT canonical parser/IR structural consolidation and bounded key-evaluation recovery (#372).

## Current access boundary

Trusted protected-process/install observations required for the remaining L1/L2/L3 original-process gates are still unavailable through the connected development evidence. Static/corpus progress must not be upgraded into original-runtime acceptance.
