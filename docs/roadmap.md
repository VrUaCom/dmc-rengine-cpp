# DMC Rengine Roadmap

**Snapshot:** 2026-09-09  
**Canonical base reviewed:** `main@5b2a541c99442627128ef4d877c99e5ecff16933`  
**Latest reviewed repository promotion:** PR #374 — canonical status reconciliation + machine-status JSON CI guard  
**Latest reverse/model-family promotion:** PR #369 — controlled retail MOD writer receipt + MOD-to-PAC reintegration trust bridge  
**Latest Native Reader module promotion:** PR #288 — evidence-backed SHW Native Reader + cross-registry integration hardening  
**Completion model:** evidence-gated, not percentage-gated.

This roadmap distinguishes four different things that must never be conflated:

- ✅ **PROVEN / CLOSED** — the claim is supported at the required authority level. Original-DMC3 behavior requires a bounded reverse pass against the canonical executable and, where the claim is corpus/runtime-specific, matching corpus or original-process evidence.
- ⚠️ **IMPLEMENTED / PROOF OPEN** — DMC Rengine code exists and may have green tests/CI, but the original-game or real-retail proof required by the claim is still open, or the implementation is intentionally bounded below full production authority.
- ❌ **OPEN** — mandatory implementation, reverse, corpus, runtime or acceptance evidence is still missing.
- ➖ **PRODUCT POLICY / REVERSE N/A** — deliberate DMC Rengine safety/product policy. It is validated by code/tests/CI and is not claimed to reproduce Capcom behavior.

Synthetic tests and public CI prove DMC Rengine behavior only. They do not by themselves prove original-game equivalence.

Detailed proof matrix: [GDSpaces proof roadmap — 2026-09-05](gdspaces/proof-roadmap-2026-09-05.md).

## 1. Canonical executable and reverse authority

- ✅ Canonical `dmc3.exe` identity: SHA-256 `e454272ed0fb0247fcbcf300e5d55d7a3e96d50b89b9ffaff81bb978dcbdd082`, size 6,356,432, PE32+ x86-64, ImageBase `0x140000000`.
- ✅ Numbered DMC3 volume bootstrap / `DMC3-%d.nbz` / first-gap discovery is instruction-backed.
- ✅ `OpenGameResource` bounded direct-call surface, six-prefix candidate policy and archive-then-physical phases are instruction-backed.
- ✅ Archive `0x0E` and physical `0x0C` normalization semantics are instruction-backed.
- ✅ Type-0 physical final-open bounded contract is reverse-backed.
- ✅ LoadedResource central state spine and typed-post-load ordering are reverse-backed in the bounded canonical scope.
- ✅ Exact canonical `e454...` executable bytes are available to recent promoted MOD/model-family static reverse work; this does not automatically close unrelated GDSpaces/L3 raw targets.
- ❌ Full original-runtime behavioral equivalence is not claimed.

## 2. GDSpaces L1 — Resource Materialization

### Product invariants

- ➖ Atomic/no-replace publication — DMC Rengine safety invariant; closed by implementation/tests/CI.
- ➖ Artifact-bound SHA / ByteProvenance — DMC Rengine evidence invariant; closed by implementation/tests/CI.

### Original/runtime-backed materialization facts

- ✅ NBZ numbered-volume/first-gap behavior — reverse-backed.
- ✅ PAC/PNST runtime recognition and recursive materialized-child traversal — reverse-backed in the recovered dispatcher scope.
- ✅ PAC physical slot 0 is traversed; it is not a privileged runtime manifest slot.
- ⚠️ NBZ normal-read implementation (STORE/raw-DEFLATE, indexing/materialization) — product path is implemented and heavily tested, but complete original ZIP stream lifetime/seek/reset/error equivalence remains bounded-open.
- ⚠️ PAC/PNST size-changing and nested writer pipeline — DMC Rengine authoring is implemented; original-game acceptance of authored output is still open. Capcom offline writer equivalence is not required.
- ⚠️ MOD now has a bounded preserve-layout writer and a writer-receipt trust bridge into the existing PAC reintegration path; this narrows one representative authoring path but does not close retail container/NBZ/original-game acceptance.

### L1 acceptance gates

- ❌ Direct-retail resolver-selected provenance receipt from a protected installation.
- ❌ Exact retail representation classification for the selected bytes.
- ❌ Representative real edit + bottom-up rebuild + rematerialization receipt across the selected retail lineage.
- ❌ Proof that the authored higher-numbered NBZ is selected by the original runtime.
- ❌ Original DMC3 consumer-visible effect attributable to the authored bytes.
- ❌ Rollback receipt proving original retail artifacts remain unchanged.
- ❌ Final L1 cross-stack audit.

**L1 COMPLETE:** ❌ NO.

## 3. GDSpaces L2 — Resource Resolution

- ✅ `OpenGameResource` direct caller census and bounded `flags=1` policy.
- ✅ Mount-list construction and clean higher-numbered precedence are reverse-backed.
- ✅ Archive `0x0E` normalization algorithm is reverse-backed.
- ✅ Retail `dmc3-0.nbz` normalized-key census: 4,333 file keys / 4,333 unique / 0 collisions; scope is this archive only.
- ✅ Type-0 physical-provider final-open/miss behavior in the recovered direct-call scope.
- ✅ Discovery != successful mount topology — reverse proof plus current product correction were promoted by PR #287. Discovery is discovery-only; only explicitly successful linked providers enter resolver topology.
- ❌ Per-volume + cross-volume collision census for any resolver scope wider than the bound `dmc3-0.nbz` artifact.
- ❌ Real protected-distribution R2B multi-anchor mapping receipt.
- ❌ Trusted original-process R3 selected-provider identity trace.
- ❌ Direct-retail original resolver winner receipt.
- ❌ Final L2 audit.

**L2 COMPLETE:** ❌ NO.

## 4. GDSpaces L3 — Original Runtime / Lifecycle

Reverse-backed bounded core:

- ✅ LoadedResource acquisition publishes state 1 only after the materialization dispatcher succeeds.
- ✅ Normal completion publishes state `1 -> 2` through `0x1401B8DC0`.
- ✅ State2 finalizer order: typed post-load -> optional callback -> state3.
- ✅ Canonical cancellation writer source domain `1|2 -> 4`.
- ✅ Quiescence requires every record to be in `{0,3}`.
- ✅ Ordinary release, cancellation cleanup and forced reset have distinct state-zero/backing-release ordering.
- ✅ Central typed dispatcher recognizes recovered MOD/EFM/SCM/SHW paths and recursively walks PNST in the bounded path.
- ✅ MOD family-specific post-load/runtime evidence advanced through PRs #310/#318/#323/#356/#359/#360, but this does not close the global LoadedResource lifecycle.

Still open:

- ❌ Canonical promotion of the final L3-R1 contradiction-gated closure onto current `main`.
- ❌ L3-R2 family/backing ownership closure for `+0x08/+0x10/+0x18/+0x20/+0x28` and stable adjacent fields.
- ❌ Exact terminal materialization scheduler condition that prevents normal state2 publication on failed/incomplete transport.
- ❌ V1 initial-load original-process receipt.
- ❌ V2 room/stage transition receipt.
- ❌ V3 restart/reload receipt.
- ❌ V4 return-to-menu/full-reset receipt.
- ❌ V5 in-flight cancellation receipt.
- ❌ V6 shutdown receipt.
- ❌ V7 family/build breadth.
- ❌ Final L3 audit.

**L3 COMPLETE:** ❌ NO.

## 5. Native Reader / format coverage

Canonical modular Native Reader modules on current `main`:

- ✅ DDS structural reader.
- ✅ PTX structural reader.
- ✅ HITS reader.
- ✅ DCA reader.
- ✅ LIG2/LIG reader.
- ✅ Stage TXT reader.
- ✅ SCM structural reader.
- ✅ MOD structural reader.
- ✅ SHW structural/read-only reader.
- ✅ PE/EXE reader.

SHW canonical state after PR #288:

- ✅ `formats.shw-structural-v1` is promoted to `main` as a structural/read-only Native Reader.
- ✅ The schema is backed by the canonical EXE plus hash-bound real payload `slot_0008.shw` (9,488 bytes, SHA-256 `cb392ef2e874addb887d32bc44d409299a32a83a4845afcbdef31698283f2e7e`).
- ✅ The bound layout proves self-contained shadow-hull geometry: triangle topology, exact adjacency, `float4` positions and a per-vertex selector stream.
- ✅ The selector is EXE-confirmed as indexing 0x40-byte transform matrices.
- ✅ Format Registry, Native Reader Registry, OpenRouter, ToolRegistry and workspace parser-validation routing are mechanically cross-checked by regressions.
- ✅ `ResourceAnalyzer` fails closed if the parser runs but its canonical completion receipt cannot be published.
- ✅ Exact PR-head tree passed Ubuntu + Windows build/test CI before promotion; the promoted commit contains the same tree.
- ❌ SHW matrix-palette ownership/construction remains open.
- ❌ SHW writer authority and universal revision coverage remain open.

MOD canonical read/reverse state after the 2026-09-09 promotion stack:

- ✅ hierarchy, serialized local transforms and world propagation are canonical (#305);
- ✅ typed texture slot / legacy GS CLAMP state is canonical (#307);
- ✅ EXE-backed post-load relocation and topology generation is reconstructed read/runtime analysis (#310);
- ✅ object core/header/texture companion and companion-authoritative binding contracts are promoted (#312-#315);
- ✅ inverse-rest ownership and skin-palette composition are promoted (#316);
- ✅ runtime texture descriptor and object-runtime projections are promoted (#317/#318);
- ✅ mesh `+0x0C/+0x38/+0x4C` are explicitly preserved-undecoded and blend-index `/4` plus packed 5-bit `/31` skin-weight ABI are EXE-backed (#323);
- ✅ multi-corpus unknown-field closures, preservation contracts and no-repeat branch consolidation are promoted (#356/#359/#360/#363).

MOD writer/authoring state:

- ✅ Preserve-Layout Writer Gate 1 is canonical (#365): immutable-source binding, source reparse, authorized-byte-span enforcement, output reparse and fixed-size authoring for object bounds plus existing position/normal/UV streams.
- ✅ Deterministic multi-file writer corpus tooling is canonical (#367).
- ✅ Provenance-bound retail no-op parity is closed for the explicit 38-file corpus (#368): 38/38 parse, preserve-layout write, exact byte equality and canonical reopen across 882,736 bytes, with zero modified bytes and zero failures.
- ✅ One provenance-bound real retail controlled edit is closed (#369): `em000_021.mod` object bounding radius, exactly three changed bytes inside serialized span `[124,128)`, with unauthorized-byte preservation, disk hash/reread/reopen and independent raw-diff verification.
- ✅ A MOD writer receipt can be promoted through `ModAuthoredChildBridge` into the existing generic `AuthoredChildImage`, and synthetic PAC reintegration/reopen is regression-proven (#369).
- ❌ Full MOD writer authority is not closed: no layout synthesis/reflow or rebuild-from-typed-IR authority; transform/skin/material/texture-companion authoring remain open; provenance-bound retail PAC/PNST reintegration, NBZ authoring acceptance and original `dmc3.exe` authored-MOD acceptance remain open.

Other evidence boundaries:

- ✅ SCM structural reader is backed by dedicated EXE/corpus reverse packets for layout, hierarchy/world transform and runtime flags.
- ✅ MOD structural reader is backed by EXE-family evidence and hash-bound retail corpus work.
- ⚠️ DDS/PTX reader behavior is evidence-backed for the promoted DMC3 profiles, but authoring is not promoted by read support.
- ❌ EFM is not yet a canonical Native Reader module.
- ❌ MOT is not yet a canonical Native Reader module.
- ❌ MRP/MCV/CAM/CLT/TSC and other recognized families require evidence-gated structural promotion before reader claims.

Writers:

- ❌ SCM canonical production writer authority.
- ⚠️ MOD bounded preserve-layout Writer Gate 1 is proven; **full production MOD writer authority remains open**.
- ❌ SHW writer authority.
- ❌ DDS/PTX texel/production authoring authority through Native Reader.
- ❌ Edited model/texture/shadow original-game acceptance receipts.

## 6. Current P0 proof track

The highest-value cross-layer proof remains one same-lineage vertical chain:

```text
OpenGameResource(request)
 -> discovered archives
 -> successful mount topology
 -> actual selected DMC3-N/provider/member
 -> exact materialized bytes
 -> PAC/PNST expansion when applicable
 -> typed resource post-load
 -> LoadedResource state3
 -> deterministic consumer-visible effect
```

For authoring acceptance the same chain must then be repeated with an exact DMC Rengine-authored higher-numbered NBZ and rollback.

Execution order:

1. ✅ preserve existing static reverse authority for bootstrap, mount-list construction, resolver masks and normalization;
2. ✅ successful-mount topology product correction promoted by PR #287;
3. ✅ SHW Native Reader + registry/routing/parser-validation integration hardening promoted by PR #288;
4. ✅ MOD Preserve-Layout Writer Gate 1, explicit 38-file retail no-op parity and one controlled real-retail edit are promoted by PRs #365/#368/#369; these do not replace the same-lineage L1/L2/L3 acceptance chain;
5. ⚠️ perform the still-open scheduler raw pass against the exact canonical executable; current byte availability removes the old access excuse but does not itself close the scheduler claim;
6. ❌ obtain/process a real protected-process R2B mapping packet;
7. ❌ capture trusted selected-provider/member identity without synthesizing probes;
8. ❌ bind selected member to independently materialized SHA/provenance;
9. ❌ bind materialization success to LoadedResource lifecycle observation;
10. ❌ repeat with authored next-volume overlay;
11. ❌ observe deterministic original-game effect and rollback;
12. ❌ run final L1/L2/L3 contradiction audits independently.

## 7. Long-term milestones

1. ❌ GDSpaces L1 accepted with real retail provenance, authored rebuild/reopen and original-game consumption receipt.
2. ❌ Narrow L2 closure with trusted selected-provider identity.
3. ❌ Representative L3 lifecycle validation across load/reload/transition/cancellation/release.
4. ❌ Stage Ops game-backed assembly over representative catalog selections.
5. ❌ Stable Stage Semantic Graph and ModViz editing verticals.
6. ❌ First bounded recovered-subsystem behavioral-equivalence receipt.
7. ❌ Progressive recompilation with controlled replacement modules.
8. ❌ Working rebuilt executable milestones without weakening evidence gates.

## 8. Completion rule

A checkbox becomes ✅ only at the authority required by its claim:

- DMC Rengine product policy: implementation + regression tests + exact-head CI;
- original DMC3 static behavior: canonical-EXE reverse with exact addresses/ranges and contradiction review;
- retail corpus claim: hash-bound real corpus receipt using the recovered algorithm;
- protected/original-process claim: trusted runtime receipt bound to the exact process/build/artifacts;
- authoring acceptance: generated artifact identity + original-game selection/consumption + rollback.

Readable pseudocode, parser success, synthetic fixtures, CI green status or a crash-free launch alone cannot promote original-game equivalence.
