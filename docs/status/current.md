# Current Project Status

**Snapshot date:** 2026-09-09  
**Canonical implementation base reviewed:** `main@cf7bd4d2a57c40e8127aed353ebefe2b4763398a`  
**Latest repository promotion in this reviewed snapshot:** PR #373 — public MOD discovery aligned with Preserve-Layout Writer Gate 1  
**Latest canonical reverse/model-family promotion:** PR #369 — controlled retail MOD writer receipt + MOD-to-PAC reintegration trust bridge  
**Latest canonical Native Reader module promotion:** PR #288 — evidence-backed SHW structural reader + cross-registry integration hardening  
**Latest canonical proof promotion:** PR #287 — proof roadmap + successful-mount topology correction  
**Latest provenance-bound retail MOD evidence:** PR #369 — one controlled retail fixed-layout edit; prerequisite PR #368 proves 38/38 no-op retail byte parity  
**Primary execution program:** proof-gated L2 -> L1 -> L3 vertical acceptance  
**Overall status:** L1/L2/L3 remain incomplete. MOD authoring has advanced from read-only inspection to a bounded preserve-layout writer with retail no-op parity and one controlled retail edit, but full writer authority, retail container reintegration, NBZ/original-game acceptance and the protected-process proof chain remain open.

## Authority split

- GitHub `main` is canonical implementation truth.
- A pull-request branch is branch truth until promoted.
- Reverse claims are bounded to exact artifact/address/range/scope.
- Synthetic/public CI proves product/tool behavior only.
- Original-game equivalence requires canonical-EXE reverse plus original-process evidence where runtime identity/consumption is claimed.
- Retail corpus claims require hash-bound corpus receipts.
- DMC Rengine product-safety policies such as atomic/no-replace publication are not presented as Capcom behavior.
- Canonical analysis executable and protected distribution execution authority are separate builds; canonical VAs/RVAs require independent mapping before protected-process use.
- Discovery/SEO pages are navigation/readability surfaces, never a second technical authority.

See [project roadmap](../roadmap.md) and [proof roadmap](../gdspaces/proof-roadmap-2026-09-05.md).

## Canonical analysis authority

Canonical `dmc3.exe`:

- SHA-256 `e454272ed0fb0247fcbcf300e5d55d7a3e96d50b89b9ffaff81bb978dcbdd082`;
- size 6,356,432;
- PE32+ x86-64;
- ImageBase `0x140000000`.

Protected distribution/original execution candidate remains separately identified as SHA-256 `81c7e61983564113b5105e931d9f185accc14e44ae147d27f720c2d50935c7d6`, size 6,567,320. No global build equivalence is claimed.

## Native Reader current state

The resource-level Native Reader is modular (`NativeReaderModuleRegistry`). PAC/PNST remain container parsers; NBZ remains a source/materialization adapter.

Canonical built-in reader modules on current `main`:

- DDS;
- PTX;
- HITS;
- DCA;
- LIG2/LIG;
- Stage TXT;
- SCM;
- MOD;
- SHW;
- PE/EXE.

SHW is canonical as `formats.shw-structural-v1`, structural/read-only. The promotion is bounded to the canonical EXE plus one hash-bound real payload (`slot_0008.shw`, size 9,488, SHA-256 `cb392ef2e874addb887d32bc44d409299a32a83a4845afcbdef31698283f2e7e`). The reader materializes the confirmed `0x20` header, `0x40` hull records, triangle topology, exact adjacency records, `float4` positions and per-vertex transform selectors. Selector semantics are EXE-confirmed; SHW matrix-palette ownership/construction remains open. One-payload invariants are variant warnings rather than universal hard rejects. No SHW writer or universal revision coverage is claimed.

PR #288 also closed product-integration drift around SHW: `FormatIntegrationRegistry`, `NativeReaderModuleRegistry`, `OpenRouter`, `ToolRegistry`, parser-validation publication and `ResourceAnalyzer` share explicit regression contracts. Parser execution is fail-closed if its canonical workspace completion receipt cannot be published. The exact PR head tree passed Ubuntu + Windows build/test CI before promotion.

### MOD / model-family promotion stack

The canonical MOD reader remains the structural/semantic inspection authority, and MOD now also has a deliberately bounded **Preserve-Layout Writer Gate 1**. The writer is not a canonical rebuild-from-scratch system: the immutable original serialized image remains physical-layout authority.

Promoted reader/reverse slices include:

- PR #305 — canonical hierarchy/local transforms/world propagation consolidation;
- PR #307 — typed mesh texture slot + legacy GS CLAMP state;
- PR #310 — EXE-backed post-load relocation/topology generation reconstruction;
- PR #312 — typed object alpha/control, source flags and bounds;
- PR #313 — EXE-confirmed model texture companion envelope with TM2-backed payload allocation;
- PR #314 — typed document/header texture-domain mirror and raw runtime header state;
- PR #315 — mesh texture binding validation against companion-authoritative runtime domain;
- PR #316 — inverse-rest ownership and `skinMatrix = inverseRestWorld * currentWorld` palette composition;
- PR #317 — runtime model-texture descriptor ABI with GS TEX0/MIPTBP1 decode;
- PR #318 — EXE-backed object runtime flag/render-parameter projection;
- PR #323 — serialized mesh `+0x0C/+0x38/+0x4C` preservation classification plus direct `/4` blend-index and 5+5+5-bit `/31` weight ABI evidence;
- PR #356 — broader multi-corpus unknown-field closure and preservation contracts;
- PR #359 — source-flag `0x00200000` negative evidence extended through effective-word/material paths without speculative semantic promotion;
- PR #360 — retained serialized-object pointer consumers classified and false SCM provenance rejected;
- PR #363 — MOD branch history consolidated with an explicit no-repeat evidence frontier.

Promoted writer/authoring gates now include:

- PR #365 — Preserve-Layout Writer Gate 1: immutable-source binding, canonical source reparse, explicit authorized-byte spans, output reparse and fixed-size edits only for object bounding center/radius plus existing mesh positions, normals and UVs;
- PR #367 — deterministic multi-file writer corpus runner and machine-readable receipts;
- PR #368 — provenance-bound retail no-op gate: 38/38 MOD files parse, write byte-identically and reopen canonically; 882,736 source bytes, zero modified bytes, zero failures;
- PR #369 — one provenance-bound real retail `em000_021.mod` bounding-radius edit: serialized span `[124,128)`, exactly three changed bytes, unauthorized-byte preservation PASS, disk reread/hash/reopen PASS and independent raw-diff verification PASS;
- PR #369 — `ModAuthoredChildBridge` validates a successful MOD writer receipt before emitting the generic `AuthoredChildImage`; a synthetic PAC regression then reintegrates and reopens the writer output through the existing `NestedRelativeSlotReintegrator`.

This establishes real bounded MOD writer authority, not full MOD authoring authority. Still open are:

- layout synthesis/reflow and rebuild from typed IR alone;
- transform authoring;
- skin/blend-index authoring;
- source-flag/material/texture-binding authoring;
- texture-companion rewriting/coherence;
- provenance-bound retail PAC/PNST reintegration of writer output;
- NBZ overlay acceptance for the MOD authoring chain;
- original `dmc3.exe` acceptance of a no-op rebuilt MOD;
- original `dmc3.exe` acceptance of an edited MOD;
- complete current animation/pose ownership;
- Capcom offline authoring-tool equivalence;
- a `100% MOD writer` claim.

Current reader frontier:

- EFM — reverse evidence exists, but no canonical Native Reader module;
- MOT — research/parser work exists outside the canonical reader set;
- MRP/MCV/CAM/CLT/TSC and other recognized families remain evidence-gated.

## L1 — Resource Materialization

### Closed product capabilities

- NBZ classic ZIP indexing/materialization;
- STORE + raw-DEFLATE product path;
- CRC/size/SHA/ByteProvenance;
- artifact-bound archive/member observations;
- atomic/no-replace publication;
- PAC/PNST sparse/empty/alias-preserving parse + recursive expansion;
- size-changing relative-slot reflow;
- nested root-to-leaf PAC/PNST slot-path authoring;
- immutable NBZ copy rebuild;
- next-contiguous STORE NBZ overlay authoring;
- canonical reopen/rematerialization checks;
- direct-retail acquisition tooling and protected-build preflight.

### Reverse-backed bounded facts

- `DMC3-%d.nbz` bootstrap and first-gap discovery;
- PAC/PNST typed traversal in the recovered post-load path;
- PAC physical slot 0 is traversed;
- materialization-dispatch success gates LoadedResource state1;
- packed-vs-`.lst` representation selection is original-runtime authority; external `.index` is not recovered as runtime materialization authority on this path.

### Mandatory remaining L1 receipts

1. real protected-install resolver-selected provenance;
2. exact selected retail representation classification;
3. one provenance-bound retail container reintegration/reopen receipt for a supported authored child chain;
4. original runtime selects the authored higher-numbered overlay;
5. deterministic consumer-visible effect attributable to authored bytes;
6. rollback proving original retail immutability;
7. final L1 audit.

The synthetic MOD-in-PAC regression from PR #369 strengthens the product reintegration path but does not satisfy the provenance-bound retail or original-runtime gates above.

**L1 COMPLETE = NO.**

## L2 — Resource Resolution

### Closed/reverse-backed

- `OpenGameResource` bounded direct caller census;
- six-prefix archive-then-physical request policy for the recovered direct-call surface;
- executable-relative `data\\dmc3\\` root and numbered first-gap discovery;
- successful archive registrations prepend to the mount list;
- clean higher-numbered archive precedence;
- archive `0x0E` / physical `0x0C` normalization;
- terminal archive-wrapper failure distinction;
- type-0 physical final-open/miss bounded contract;
- protected-runtime mapping tooling (#219).

### Retail collision evidence

Bound retail `dmc3-0.nbz`:

```text
files-only          : 4333 keys / 4333 unique / 0 collisions
all central entries : 4334 keys / 4334 unique / 0 collisions
```

Receipt: `data/reverse/dmc3-nbz-archive-key-census-20260903.json`.

This closes collision freedom only for that exact archive. Wider resolver scope still requires per-volume and cross-volume census.

### Successful-mount topology correction

PR #287 is promoted to `main`. The product model reflects the reverse-backed distinction:

```text
filename discovery / registration attempt
!=
successful linked runtime mount topology
```

Canonical product behavior keeps discovery evidence separate from explicitly successful providers; sparse successful archive registration is representable; the resolver traverses only successful topology; discovered-but-failed archives are absent rather than manufactured as misses; failed physical registration produces no physical probe. Product receipts still do not claim original-process mount topology.

### Remaining L2 frontier

- per-volume + cross-volume collision census for any wider resolver scope;
- real protected-process R2B multi-anchor mapping receipt;
- trusted R3 selected-provider/member identity;
- direct-retail original resolver winner receipt;
- final L2 audit.

**L2 COMPLETE = NO.**

## L3 — Original Runtime / Lifecycle

Reverse-backed bounded core includes:

- 363-record LoadedResource registry topology;
- acquisition/materialization success -> state1;
- normal completion `1 -> 2`;
- typed post-load -> optional callback -> state3;
- global cancellation `1|2 -> 4`;
- quiescence requires all records in `{0,3}`;
- distinct ordinary release, cancellation cleanup and forced reset policies;
- central typed dispatcher paths for MOD/EFM/SCM/SHW plus PNST recursion.

The promoted MOD post-load/runtime and authoring evidence strengthens one family-specific part of this spine but does not establish original-runtime acceptance of authored MOD bytes and does not close global lifecycle proof.

Remaining L3 work:

- promote/reconcile final R1 contradiction-gated writer census onto current main;
- close R2 family/backing ownership;
- close the exact materialization scheduler terminal condition preventing failed/incomplete transport from reaching normal state2 publication;
- capture V1–V7 original-process lifecycle receipts;
- validate original `dmc3.exe` selection/consumption of the bounded MOD authoring chain before promoting game-acceptance claims;
- final L3 audit.

**L3 COMPLETE = NO.**

## Current P0 proof track

```text
OpenGameResource(request)
 -> discovered volumes
 -> successful mount topology
 -> selected provider/volume/member
 -> exact materialized bytes
 -> PAC/PNST expansion where applicable
 -> typed post-load
 -> LoadedResource state3
 -> deterministic consumer-visible effect
```

Then repeat the same lineage with an authored next-volume NBZ and rollback.

Immediate order:

1. ✅ roadmap/status proof model and successful-mount topology are canonical;
2. ✅ SHW structural Native Reader and cross-registry routing/validation hardening are canonical;
3. ✅ MOD Preserve-Layout Writer Gate 1, 38/38 provenance-bound no-op retail parity, one controlled retail fixed-layout edit and a synthetic PAC reintegration trust bridge are canonical;
4. ⚠️ exact canonical EXE bytes are available to recent reverse work, but unrelated GDSpaces/L3 raw targets still require their own fresh bounded passes;
5. ❌ execute protected-process R2B mapping;
6. ❌ capture trusted R3 selected identity;
7. ❌ bind selected identity to exact independently materialized bytes;
8. ❌ obtain provenance-bound retail container reintegration for the bounded authored MOD child;
9. ❌ observe typed post-load/state3 for the same lineage;
10. ❌ repeat using authored higher-volume overlay;
11. ❌ record deterministic consumer effect + rollback;
12. ❌ run independent final L1/L2/L3 audits.

## Current evidence-access boundary

The old 2026-09-05 statement that raw canonical `e454...` executable bytes were unavailable is no longer current. The exact canonical executable has been supplied and used for promoted MOD/model-family reverse work.

That availability is **not** a blanket proof upgrade. Each unrelated GDSpaces/L3 raw target still requires its own fresh address/range/scope-bounded analysis before any claim changes, and protected-process/original-consumption gates remain mandatory where runtime identity or acceptance is claimed.

## Discovery/publication state

The discovery surface is production-active and remains separate from technical authority:

- GitHub Pages is enabled and production HTTP acceptance is green;
- the manifest-driven surface contains **104 controlled Pages URLs** across **63 format/resource families**;
- canonical URLs, `index,follow`, sitemap ↔ `site-index.json` parity, breadcrumbs/`BreadcrumbList`, Open Graph/Twitter metadata and the approved 1280x640 social raster are production-validated;
- automated IndexNow notification runs only after successful Pages deployment/acceptance; the 104-URL batch has a production **HTTP 200 accepted** receipt;
- Google Search Console is intentionally not a project dependency;
- the reviewed repository Topics payload remains unapplied because the connected GitHub path does not expose a Topics/settings write action (#294);
- the specialist external reverse-engineering index contribution remains prepared but not submitted because the connected GitHub path does not expose fork creation (#300);
- the latest independent public-search recheck still observed no DMC Rengine Pages result; IndexNow acceptance is not treated as proof of indexing or ranking.

Discovery work does not alter L1/L2/L3 completion.

## Navigation

- [Project roadmap](../roadmap.md)
- [Proof roadmap](../gdspaces/proof-roadmap-2026-09-05.md)
- [Canonical L1 roadmap](../gdspaces/l1-roadmap.md)
- [Three-layer master roadmap](../gdspaces/master-roadmap.md)
- [Blockers](blockers.md)
- [Machine-readable status](canonical-status.json)

No percentage, green synthetic suite, parser success, discovery metadata or crash-free launch overrides the gate-based completion rule.
