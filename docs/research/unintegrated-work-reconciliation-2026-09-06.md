# Unintegrated branch review and current-main implementation plan — 2026-09-06

**Repository:** `VrUaCom/dmc-rengine-cpp`  
**Audit base:** `main@3a3db646c6bf3faf1871efbed31c4e9f4fb32cbe`  
**Canonical analysis executable:** `dmc3.exe` SHA-256 `e454272ed0fb0247fcbcf300e5d55d7a3e96d50b89b9ffaff81bb978dcbdd082`  
**Policy:** `main` is implementation authority. Historical branches are evidence/salvage sources only. No stale branch is to be merged wholesale merely because it contains useful work.

## 1. Executive result

The repository contains a substantial amount of useful work that is still outside current `main`, but the missing work is not one monolithic branch. It falls into six distinct classes:

1. **genuinely missing format readers / contracts** — especially EFM and MOT;
2. **deep reverse/runtime evidence not represented on main** — especially the HITS Pass 8–10 profile layer;
3. **large product architecture that was never promoted** — Stage Ops / Stage Semantic Graph / stage operational workspace;
4. **validation architecture/tooling that was never promoted** — V/LV `ValidationRun` and L3 lifecycle receipt validation;
5. **recovered-game source implementations / recovered source-tree population** — several direct executable reconstructions remain outside main;
6. **historical stacks that look missing by class/file name but are already semantically superseded by newer main implementations** — most of the old texture authoring stack, nested relative-slot reflow and much of #284 MOD hardening.

The correct integration strategy is therefore **semantic decomposition**, not branch consolidation.

## 2. Current-main baseline

Current `main` already contains strong canonical infrastructure that every salvage port must preserve:

- GDSpaces as the only resource resolver/materializer authority;
- canonical resource identity / provenance / naming reconciliation;
- PAC/PNST relative-slot parser and bounded writer/reflow paths;
- nested `RelativeSlotPathReflowWriter`;
- NBZ source/materialization and overlay authoring boundaries;
- DDS/PTX native readers;
- `TextureSlotFramingParser`;
- `TextureSlotPackedReflowWriter` for the evidence-backed size-changing texture subset;
- `TextureSlotRuntimeMaterializationInspector`;
- HITS structural reader;
- DCA, LIG/LIG2, Stage TXT readers;
- SCM, MOD and SHW structural Native Readers;
- current runtime type/family-mask evidence split;
- current process-memory window acquisition with Windows process-creation FILETIME binding;
- current original-resolution observation model;
- current naming and successful-mount topology corrections.

This means many older branches are valuable for **evidence and missing layers**, but unsafe as implementation bases.

## 3. Branch/PR review matrix

### 3.1 PR #291 — current-main AFS/PACK authority split

**Status:** active current-main corrective PR.  
**Action:** **LAND FIRST after exact-head CI review.**

The branch separates:

- `GData.afs` / `GDataX360.afs` logical namespace identity;
- generic binary AFS candidates;
- PACK binary candidates;
- actual expandable containers (`nbz`, `pac`, `pnst`).

The exact-head GitHub Actions run has completed successfully on both Ubuntu and Windows. This correction should be the first stabilization landing before new salvage work so no later branch reintroduces `afs == expandable container` semantics.

### 3.2 PR #280 — mobile-only parser package

**Status:** large mixed salvage source; **DO NOT MERGE WHOLE**.  
**Priority:** P0 for EFM/MOT extraction.

Useful work:

- EFM structural parser and DMC3 EFM contract;
- MOT structural parser and DMC3 MOT contract;
- shared relocated model-shell logic used by EFM/MOD family work;
- animation type metadata plumbing;
- selected naming/index/effect helpers;
- a useful correction that the original PAC walk compares only three magic bytes while PNST retains four-byte identity.

Already superseded or unsafe to port wholesale:

- SHW implementation: low-confidence branch model is superseded by the current canonical evidence-backed SHW reader;
- MOD parser: current main now has canonical MOD structural support;
- PTX parser: current main has a later canonical PTX/texture stack;
- classifier/registry edits: based on an older registry state and must not replace current family-mask, naming, AFS/PACK or parser-validation behavior.

Direct current-main inspection confirms that `include/dmc_rengine/formats/efm.hpp` and `include/dmc_rengine/formats/mot.hpp` are absent and code search finds no `EfmParser` / `MotParser`. Those are genuine gaps.

### 3.3 PR #254 — animation/effect/naming reverse pass

**Status:** high-value research/evidence source; **DO NOT MERGE WHOLE**.  
**Priority:** P0 companion to #280 MOT work.

High-value evidence:

- one real 63,440-byte MOT payload with 69 tracks;
- `size == 32 + 8*keys` for all 69 tracks;
- strictly increasing stamps for all 69 tracks;
- measured span equal to the duplicated `650.0f` header span for all tracks;
- six animation kinds from the recovered matcher: `mot`, `mcv`, `cam`, `hid`, `clt`, `tsc`;
- matching is based on `strstr`, explaining accepted numbered names such as `.mot1`…`.mot6`;
- script-side origin for animation names via `Motion`, `Camera`, `Hide` commands;
- `.c1d` identified as `ClothSim1D` text family;
- effect-container stored-name and index-discovery work.

Important reconciliation rule:

The branch wording about an exact global five-tag content census predates the current main split between the three-byte registry probe, four-byte family-mask probe and other format identity paths. Preserve the bounded observations, not the old global simplification.

### 3.4 PR #282 — HITS canonical integration branch

**Status:** active HITS salvage/integration authority; keep as the single HITS branch.  
**Priority:** P0/P1.

Current main already has the basic HITS structural format reader. What is missing from main is the deeper profile/runtime layer represented in #282, including:

- game-agnostic HITS spatial/contact/query helper modules;
- SHA-gated DMC3 query ABI evidence;
- dynamic-category/source topology evidence;
- common contact/surface normal evidence;
- primitive descriptor ownership/type evidence;
- Stage-CFG entry/descriptor collision tables;
- bounded referenced-descriptor census;
- machine-readable Pass 8–10 synthesis.

Current-main code search does not find `hits_query_evidence`, `hits_contact_normal_evidence`, `hits_stage_cfg_collision_tables` or `referenced_descriptor_census`, confirming that this deeper layer is genuinely absent.

Do not replace the current main HITS parser wholesale. Port #282 in bounded profile/runtime slices around the existing reader.

### 3.5 PR #284 — model-family reader hardening

**Status:** mostly superseded verification source.  
**Priority:** P2 audit, not P0 salvage.

The PR contains MOD structural hardening and transform-domain checks, but current main already contains the same canonical MOD parser architecture and the examined parser body includes the branch's key bounds, aggregate-count, stream-span, raw-preservation and skin diagnostic behavior.

Action:

- perform an exact file-level diff of transform-domain implementation/tests;
- port only a demonstrably missing diagnostic or invariant;
- otherwise close #284 as semantically contained by current main.

Do not revive its old SCM/model-family ancestry.

### 3.6 PR #223 — V / LV validation authority

**Status:** genuinely missing architecture and code.  
**Priority:** P1.

Missing from main:

- `ValidationRun` parent receipt;
- explicit validation domains L1/L2/L3/LV;
- evidence classes V-A..V-E;
- same-run / same-resource cross-layer binding;
- child receipt SHA binding;
- rollback binding;
- a promotion gate that prevents manually authored evidence from self-promoting.

Current-main code search finds no `ValidationRun`, so this is not merely a historical class name mismatch.

Port this architecture onto current main before adding further dynamic validation formats. Preserve the key anti-laundering rule: CI, static evidence or manually written JSON can never become original-process equivalence by declaration.

### 3.7 PR #218 — L3 lifecycle trace receipt validator

**Status:** genuinely missing implementation, but should be subordinated to the current V/LV model.  
**Priority:** P1 after #223.

Useful code/contracts:

- strict bounded lifecycle trace JSON import;
- V1–V6 scenario sequencing;
- LoadedResource state domain `0..4` validation;
- event-loss / overflow / semantic-intrusion rejection;
- cancellation sequencing checks;
- CLI validation path;
- explicit guarantee that manual imports remain non-promotable.

Current main does not contain `L3LifecycleTrace`.

Do not port it first as an independent authority. Rebuild it as a V-owned LV evidence adapter after the shared `ValidationRun` contract is current-main canonical.

### 3.8 PR #236 and PR #232 — L2 process-instance / selected-identity evidence

**Status:** partially integrated; reconcile, do not reimplement from scratch.  
**Priority:** P1/P2.

Current main already includes:

- `ProcessMemoryWindow`;
- exact Windows process creation FILETIME from the same process handle;
- v2 runtime-mapping verification scripts/tests;
- `OriginalResolutionObservation` and protected-execution authority checks.

Therefore the process-instance capture half of #236 is already present.

Potentially missing and requiring exact comparison:

- R3 original-selection v2 candidate/bound receipt chain;
- final trusted publisher/origin seam;
- any #232 structural central-entry identity evidence not already represented in current `OriginalResolutionObservation` or evidence docs.

#232 is evidence/documentation-first and should be treated as a reconciliation source, especially for the corrected archive transition and selected central-entry structural identity.

### 3.9 PR #91 — Stage Ops / Stage Semantic Graph / scene assembly

**Status:** major product architecture genuinely absent from main.  
**Priority:** P1 after the immediate format/runtime dependencies are stable.

The branch implements a coherent product-side flow:

`StageRuntimeLoadReport -> StageOpsIngress -> StageAssemblyWorkspace -> ProjectWorkspace -> StageOperationsSession -> StageDomainWorkspace -> StageRuntimeLinkWorkspace -> Stage Semantic Graph -> ModViz projection`.

Important reusable concepts:

- one resolver/materializer authority remains GDSpaces;
- stage aggregate preserves ResourceIds and ByteProvenance;
- stage revision and stale-derived-state gate;
- WorkingCopy edit/undo/reset coordination;
- retained typed parser results rather than second parsers;
- explicit source byte/revision lineage;
- structural domain projections;
- explicit-only runtime links;
- revision-guarded editor commands.

Current-main code search finds no `StageOperationsSession`, confirming the operational Stage Ops layer is not integrated.

The branch is old and very large. Reconstruct the architecture from a fresh current-main branch; do not merge 172 historical commits.

### 3.10 PR #89 — recovered-game Wave-3 runtime slices

**Status:** high-value recovered-game source; not product-core code.  
**Priority:** P1/P2 depending on consumer.

Direct-reconstructed candidates worth preserving:

- exact Stage descriptor ABI (`0x40`, four `0x10` cells);
- numeric Stage resolver via group-base + selector + Bank A/Bank B;
- SHW post-load pointer fixup helper;
- HD audio translation ABI / ADX→OGG and SFD→WMV boundaries;
- successful scene transition order.

Disassembly-complete/corpus-pending:

- MOD and EFM post-load helpers, including relative-pointer fixups, index-word mutation and generated tri-command output.

The branch's SCM blocker is historical. Current main later recovered the SCM mesh continuation/layout boundary, so any future SCM recovered-postload work must be re-derived against the newer SCM authority rather than copying the old blocked wording.

Current main does not expose `StageNumericResolver`, `shw_postload` or `mod_efm_postload` under the recovered-game source tree, so these remain candidate missing recovered-source slices.

### 3.11 PR #190 — EXE Editor recovered source tree

**Status:** genuinely absent from main.  
**Priority:** P2 after recovered-game authority reconciliation.

The branch adds `RecoveredSourceSymbol` and a 100+ node source tree spanning resource runtime, lifecycle, stage topology, texture runtime and scenes. Current-main search does not find `RecoveredSourceSymbol`.

The concept is valuable, but the old static node set must not become a second address/evidence registry. Rebuild the source tree as a projection/index over current canonical evidence and recovered-game modules.

### 3.12 Texture PR stack #155–#180

**Status:** mostly semantically integrated under newer current-main APIs.  
**Priority:** P2 reconciliation only.

Do **not** infer missing capability from missing old class names such as `TextureSlotSizeSerializer` or `TextureSlotReintegrator`.

Current main already has:

- `TextureSlotFramingParser`;
- `TextureSlotPackedReflowWriter` with source DDS SHA gating, DMC3 DDS validation, preserved compression, auxiliary-metadata restrictions, secondary-dimension relation preservation and bundle-span rebuild;
- `TextureSlotRuntimeMaterializationInspector`;
- nested relative-slot reflow.

Remaining audit target:

- compare the dual PTX runtime-entry representation work from #179 (`TM2` versus serialized `gfxTexture`/DDS materialization) against current `TextureSlotRuntimeMaterializationInspector` and PTX reader;
- port only a missing representation distinction/evidence packet if current abstractions do not already encode it.

### 3.13 PR #210 — nested slot-path reflow

**Status:** implementation already present on main.  
**Action:** verify lineage and close as superseded.

Current main has `RelativeSlotPathReflowWriter`, CLI integration and tests. No implementation port is needed.

### 3.14 PR #181 — protected/canonical executable identity

**Status:** HOLD / contradiction review.

Do not use its stronger global same-linked-image wording as executable address authority. Only bounded PE-section observations may be reconsidered after explicit reconciliation with the current separated protected-distribution and canonical-analysis authorities.

### 3.15 PR #226 — Resource Control Plane architecture

**Status:** research/architecture source.

Useful as an orthogonal dependency/control model, but it is not the first missing implementation dependency. Revisit after V/LV and Stage Ops because both provide stronger current ownership boundaries for where RCP responsibilities should live.

### 3.16 PR #50 — mission result policy ABI

**Status:** small isolated recovered-game candidate.

Current-main search does not find `MissionResultPolicyRecordLayout`. Audit it later as a bounded gameplay/recovered-source slice; it should not block the current resource/runtime integration program.

## 4. Research conclusions

### 4.1 The highest-value missing code is not MOD/PTX/SHW

Those areas already have current canonical readers and texture authoring machinery. The biggest genuine Native Reader gaps are:

1. **EFM**;
2. **MOT**;
3. deeper HITS DMC3 runtime/profile evidence;
4. operational Stage Ops layer;
5. V/LV validation layer;
6. selected recovered-game runtime/source-tree slices.

### 4.2 EFM needs two evidence layers, not just the #280 parser

For EFM, combine:

- #280 structural reader / format contract;
- #89 recovered EFM post-load routine;
- current main runtime type/family evidence;
- current main model-family helpers only where the serialized grammar is actually shared.

Do not grant writer authority. If no real EFM corpus is available, expose the reader as structural/read-only with an explicit corpus-pending limitation.

### 4.3 MOT should be promoted from #280 implementation + #254 real corpus/runtime naming evidence

#280 gives the parser implementation. #254 supplies materially stronger data evidence and runtime animation-type matching. The correct canonical MOT slice should combine both rather than choosing one branch.

MOT promotion does **not** automatically promote MCV/CAM/HID/CLT/TSC structural parsers. Those six type identities may be recorded in a profile contract while only MOT receives a Native Reader until each sibling format has independent structure evidence.

### 4.4 HITS needs a profile/runtime layer above the existing main reader

Do not rewrite the structural reader. Add the missing modules around it:

- game-agnostic spatial/contact helpers;
- DMC3 SHA-gated evidence contracts;
- Stage-CFG collision-table views;
- bounded descriptor census;
- later transform-source provenance only when direct evidence closes it.

### 4.5 Stage Ops should be reconstructed after immediate reader/runtime dependencies

Stage Ops is one of the largest pieces of useful non-main work, but rebuilding it before EFM/MOT/HITS reconciliation would force it to target a moving Native Reader/format set. Implement it after the reader and HITS profile surfaces stabilize.

### 4.6 V/LV is required to stop future evidence drift

The repository has enough static/corpus/CI/runtime tooling that completion claims can otherwise drift between independent receipts. The V/LV parent receipt is the correct place to enforce:

- one resource binding;
- one validation run;
- explicit evidence class;
- rollback binding;
- trusted-origin separation.

This should be canonical before more live L3 validation is added.

## 5. Implementation plan

## Wave 0 — stabilize current main

1. Review PR #291 exact-head diff once more against current `main`.
2. Since both Ubuntu and Windows exact-head jobs are green, merge #291 if no semantic regression is found.
3. Run full current-main CI after merge.
4. Rebase future integration slices on that resulting main, not on this audit branch.
5. Keep this audit document as the branch-salvage map.

**Exit gate:** one clean main with AFS/PACK authority split and cross-platform green CI.

## Wave 1 — EFM Native Reader

Create a fresh current-main branch, e.g. `feature/efm-native-reader-current-main`.

Implement/port:

- EFM format structures from #280;
- only the needed shared relocated-model-shell helpers after comparison with current model-family code;
- DMC3 EFM profile contract;
- structural parser with checked offsets/ranges and raw-byte preservation;
- Native Reader module registration;
- Format Registry `parser_id` and structural/read-only maturity;
- ResourceAnalyzer/OpenRouter integration;
- CMake/tests;
- evidence + format docs.

Then reconcile #89 EFM post-load evidence into `recovered-game`, separately from the product reader.

**Hard gates:** no writer; no fabricated corpus support; no runtime semantic names beyond evidence.

## Wave 2 — MOT + animation type contract

Fresh current-main branch.

Use #280 parser + #254 evidence to implement:

- `formats.mot-structural-v1`;
- track-chain and size/stamp/span validation;
- `AnimationTypeContract` for `mot/mcv/cam/hid/clt/tsc` identity matching;
- parser support only for MOT;
- Native Reader + Format Registry + ResourceAnalyzer integration;
- tests preserving the 69-track corpus invariants as evidence receipts where proprietary bytes are not committed;
- docs that distinguish script naming, animation identity and structural parser support.

**Hard gate:** do not claim MCV/CAM/HID/CLT/TSC parsing from the matcher alone.

## Wave 3 — HITS decomposition from #282

Do not merge `hits` wholesale. Use several current-main PRs:

### H3.1 Shared runtime helpers

Port game-agnostic spatial/contact/query helpers that are not already current-main equivalents.

### H3.2 DMC3 exact-build evidence

Port SHA-gated query/source/category/contact/primitive evidence headers and tests.

### H3.3 Stage-CFG collision tables

Port bounded `0x04` entry / `0x50` descriptor views, modern/legacy slot mappings and reference validation.

### H3.4 Referenced descriptor census

Port `referenced_descriptor_census()` with provenance and invalid-reference reporting.

### H3.5 Transform-source provenance

Research first. Keep `transform_selector_bounds_available == false` until the provider/bounds route is directly proven.

**Exit gate:** all new HITS profile modules are current-main based, evidence levels unchanged, Ubuntu/Windows green.

## Wave 4 — Stage Ops reconstruction

Fresh current-main architecture branch, not #91 rebase.

Port concepts in this order:

1. `StageAssemblyWorkspace` and exact resource membership/provenance;
2. `StageOpsIngress` from already-materialized GDSpaces resources;
3. retained typed parser result lineage (`immutable-source@0`, `working-copy@N` equivalent);
4. `StageOperationsSession` with revision/edit/undo/reset/stale-derived-state gates;
5. structural domain projections using current Native Readers;
6. Stage Semantic Graph projection;
7. ModViz revision-guarded projection/commands;
8. explicit runtime-link bridge only after current recovered-game contracts are reconciled.

Initial Stage Ops format set should use current canonical readers: HITS, DCA, LIG/LIG2, Stage TXT, SCM, MOD, SHW, PTX/DDS, plus EFM/MOT after Waves 1–2 land.

Do not copy #91's old executable-size/status edits or historical SCM blocker verbatim.

## Wave 5 — V/LV and L3 validation

### V5.1 Shared ValidationRun

Port/rebuild #223 onto current main:

- V-A..V-E;
- L1/L2/L3/LV domains;
- same-run/same-resource binding;
- child hash + rollback binding;
- manual submissions non-promotable.

### V5.2 L3 lifecycle trace

Rebuild #218 as a V-owned adapter:

- strict event schema;
- V1–V6 sequence validation;
- state `0..4` rules;
- loss/overflow/intrusion fail-closed checks;
- no trusted-origin boolean in editable JSON.

### V5.3 L2 selected identity reconciliation

Compare #236/#232 to current main and port only missing R3 candidate/bound receipt lineage. Reuse current process creation FILETIME acquisition and current `OriginalResolutionObservation` instead of duplicating them.

### V5.4 Trusted publisher

Only after the above contracts are stable, implement a process-bound trusted publisher/binder that independently checks the current process instance around capture.

## Wave 6 — Recovered Game Source Tree

Reconcile #89 and #190 in bounded domains:

1. Stage descriptor + numeric resolver;
2. EFM/MOD/SHW post-load helpers after current format authorities are checked;
3. media translation;
4. scene transition spine;
5. stage dependencies only after their unresolved binary edges are closed;
6. EXE Editor recovered source-tree projection.

The EXE Editor tree must reference canonical evidence/recovered modules rather than become a second hard-coded truth registry.

## Wave 7 — residual salvage / closeout

- exact-diff #284 transform-domain hardening; port only missing pieces;
- reconcile effect/index helpers from #254/#280 against current naming system;
- audit #50 mission-result ABI as a small recovered-game slice;
- perform PTX dual-representation reconciliation against current runtime materialization inspector;
- verify #210 is fully contained and close it;
- close old texture PRs whose capabilities are demonstrably covered by current main;
- keep #181 on HOLD until contradiction review;
- revisit #226 RCP only after Stage Ops + V/LV ownership is stable;
- rerun the full open-PR/branch inventory and leave only active research blockers or current-main integration PRs.

## 6. Required review discipline for every salvage slice

Every new integration PR must satisfy all of the following:

1. branch from the latest `main`;
2. list source PR(s)/commit(s) being salvaged;
3. show an exact semantic diff against current-main equivalents;
4. preserve evidence status; never upgrade confidence because code compiles;
5. keep unknown bytes/fields explicit;
6. no second resolver, parser authority or resource identity system;
7. add canonical registry/Native Reader integration only where parser authority exists;
8. run Ubuntu + Windows exact-head CI;
9. add/update machine-readable evidence when executable/corpus claims are promoted;
10. close the stale source PR only after the current-main landing demonstrably covers its useful delta.

## 7. Recommended execution order

```text
#291 stabilization
  -> EFM
  -> MOT + animation contract
  -> HITS deep profile/runtime slices
  -> Stage Ops
  -> ValidationRun V/LV
  -> L3 lifecycle + L2 trusted selection
  -> recovered-game runtime/source tree
  -> residual texture/MOD/effect/mission salvage
  -> final stale-branch closure audit
```

This order minimizes duplicate work because each later layer consumes authorities stabilized earlier.

## 8. Completion definition for this integration program

This branch-reconciliation program is complete only when:

- every high-value historical branch has an explicit disposition (`integrated`, `evidence-only`, `superseded`, `hold`, or `research-required`);
- no current-main capability relies on an undocumented mobile/private parser fork;
- all promoted Native Readers are registered and tested in canonical C++20;
- Stage Ops uses only canonical GDSpaces and canonical parser results;
- validation/equivalence claims route through one V/LV authority;
- recovered-game source modules and EXE Editor source-tree projections point to current evidence;
- stale PRs no longer act as competing implementation truth;
- current main remains cross-platform green.
