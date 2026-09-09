# DMC Rengine Roadmap

**Snapshot:** 2026-09-09  
**Canonical base reviewed:** `main@d8534badbbe52cae1610d624822874431f581fa9`  
**Latest reviewed promotion on current main:** PR #372 — unified MOD/SCM/MOT model-format consolidation and container gates (merged after PR #375)  
**Latest public/status reconciliation:** PR #375 — MOD Writer Gate 1 roadmap/learning reconciliation  
**Latest Native Reader module promotion:** PR #288 — evidence-backed SHW Native Reader + cross-registry integration hardening  
**Completion model:** evidence-gated, not percentage-gated.

This roadmap distinguishes four states:

- ✅ **PROVEN / CLOSED** — supported at the authority required by the claim.
- ⚠️ **IMPLEMENTED / PROOF OPEN** — implementation/research exists, but a stronger retail/original-game or full-production proof remains open.
- ❌ **OPEN** — mandatory implementation, reverse, corpus, runtime or acceptance evidence is still missing.
- ➖ **PRODUCT POLICY / REVERSE N/A** — deliberate DMC Rengine product/safety policy, not claimed as Capcom behavior.

Synthetic tests and CI prove DMC Rengine behavior only. They do not by themselves prove original-game equivalence.

Detailed proof matrix: [GDSpaces proof roadmap — 2026-09-05](gdspaces/proof-roadmap-2026-09-05.md).

## 1. Canonical executable and reverse authority

- ✅ Canonical `dmc3.exe`: SHA-256 `e454272ed0fb0247fcbcf300e5d55d7a3e96d50b89b9ffaff81bb978dcbdd082`, size 6,356,432, PE32+ x86-64, ImageBase `0x140000000`.
- ✅ Numbered `DMC3-%d.nbz` bootstrap / first-gap discovery is instruction-backed.
- ✅ `OpenGameResource` bounded direct-call surface, six-prefix candidate policy and archive-then-physical phases are instruction-backed.
- ✅ Archive `0x0E` and physical `0x0C` normalization are instruction-backed.
- ✅ Type-0 physical final-open bounded contract is reverse-backed.
- ✅ LoadedResource central state spine and typed-post-load ordering are reverse-backed in the bounded canonical scope.
- ✅ Exact canonical executable bytes are available to promoted model-family reverse work.
- ❌ Full original-runtime behavioral equivalence is not claimed.

## 2. GDSpaces L1 — Resource Materialization

### Product / bounded evidence

- ➖ Atomic/no-replace publication and artifact-bound SHA/ByteProvenance are closed DMC Rengine policies.
- ✅ NBZ numbered-volume/first-gap behavior is reverse-backed.
- ✅ PAC/PNST runtime recognition and recursive child traversal are reverse-backed in the recovered dispatcher scope.
- ✅ PAC physical slot 0 is traversed.
- ⚠️ NBZ STORE/raw-DEFLATE product path is implemented; full original ZIP stream lifetime/seek/reset/error equivalence remains bounded-open.
- ⚠️ PAC/PNST size-changing/nested authoring exists; original-game acceptance remains open.
- ✅ MOD Writer Gate 1 + writer-receipt trust bridge are canonical (#365/#369).
- ✅ Provenance-bound real retail PNST reintegration of a same-size authored MOD child is canonical (#372): physical slot 23 in `m20_s00_012.pac`, parent size preserved, slot table preserved, only the three expected child bytes changed, canonical parent reparse/re-expand returns the exact authored MOD bytes.
- ✅ Synthetic MOD -> container -> next-volume NBZ overlay -> reopen is regression-proven (#372) using the existing NBZ writer/source path.
- ❌ Provenance-bound **retail** NBZ overlay selection/reopen by the original runtime is not proven.

### L1 acceptance gates

- ❌ Direct-retail resolver-selected provenance receipt from a protected/original process.
- ❌ Exact representation classification bound to that selected lineage.
- ⚠️ Resource/container authoring evidence now includes real retail PNST reintegration, but the same-lineage original-selected -> authored overlay -> rematerialized chain remains open.
- ❌ Proof that the authored higher-numbered NBZ is selected by the original runtime.
- ❌ Original DMC3 consumer-visible effect attributable to authored bytes.
- ❌ Rollback receipt proving original retail artifacts remain unchanged.
- ❌ Final L1 cross-stack audit.

**L1 COMPLETE:** ❌ NO.

## 3. GDSpaces L2 — Resource Resolution

- ✅ `OpenGameResource` direct caller census and bounded `flags=1` policy.
- ✅ Mount-list construction and clean higher-numbered precedence are reverse-backed.
- ✅ Archive `0x0E` normalization is reverse-backed.
- ✅ Retail `dmc3-0.nbz` normalized-key census: 4,333 file keys / 4,333 unique / 0 collisions; scope is this archive only.
- ✅ Type-0 physical-provider final-open/miss behavior in the recovered direct-call scope.
- ✅ Discovery != successful mount topology — reverse proof plus product correction promoted by PR #287.
- ❌ Per-volume + cross-volume collision census for a wider resolver scope.
- ❌ Real protected-distribution R2B multi-anchor mapping receipt.
- ❌ Trusted original-process R3 selected-provider identity trace.
- ❌ Direct-retail original resolver winner receipt.
- ❌ Final L2 audit.

**L2 COMPLETE:** ❌ NO.

## 4. GDSpaces L3 — Original Runtime / Lifecycle

Reverse-backed bounded core:

- ✅ materialization success precedes LoadedResource state1;
- ✅ normal completion publishes `1 -> 2` through `0x1401B8DC0`;
- ✅ typed post-load -> optional callback -> state3;
- ✅ cancellation source domain `1|2 -> 4`;
- ✅ quiescence requires all records in `{0,3}`;
- ✅ ordinary release, cancellation cleanup and forced reset have distinct ordering;
- ✅ central typed dispatcher covers recovered MOD/EFM/SCM/SHW paths plus PNST recursion.

Still open:

- ❌ current-main promotion of the final L3-R1 contradiction-gated closure;
- ❌ L3-R2 family/backing ownership;
- ❌ exact materialization scheduler terminal condition;
- ❌ V1–V7 original-process lifecycle receipts;
- ❌ original `dmc3.exe` selection/consumption of authored MOD/SCM chains;
- ❌ final L3 audit.

**L3 COMPLETE:** ❌ NO.

## 5. Native Reader and model-format coverage

Canonical built-in Native Reader modules remain:

- ✅ DDS, PTX, HITS, DCA, LIG2/LIG, Stage TXT, SCM, MOD, SHW and PE/EXE.
- ❌ EFM is not yet a canonical Native Reader module.
- ⚠️ MOT now has a canonical parser/IR and recovered key-evaluation layer (#372), but that does not by itself promote MOT into the built-in Native Reader module set.
- ❌ MRP/MCV/CAM/CLT/TSC and other recognized families remain evidence-gated.

### MOD

- ✅ hierarchy, transforms, texture/GS state, post-load/topology, object/header state, companion binding, inverse-rest skin palette, runtime texture descriptors, preserved serialized state and direct skin ABI are canonical (#305–#323, #356/#359/#360/#363).
- ✅ Preserve-Layout Writer Gate 1 (#365).
- ✅ deterministic writer corpus tooling (#367).
- ✅ provenance-bound 38-file retail no-op exact byte parity (#368): 38/38, 882,736 bytes, zero modified bytes, zero failures.
- ✅ one provenance-bound controlled retail `bounding_radius` edit (#369), exactly three changed bytes in `[124,128)`.
- ✅ writer receipt -> `AuthoredChildImage` trust bridge and synthetic PAC reintegration (#369).
- ✅ provenance-bound real retail PNST reintegration/reopen (#372).
- ✅ synthetic MOD -> PNST/container -> NBZ overlay -> reopen (#372).
- ❌ full MOD writer remains open: layout synthesis/reflow, transform, skin, material/source-flag, texture-companion authoring and broader mutation authority are not promoted.
- ❌ provenance-bound retail NBZ/original-game acceptance remains open.

### SCM

PR #372 promotes one selected canonical SCM authoring stack:

- ✅ `preserve_layout` source-bound same-layout authoring.
- ✅ deterministic `canonical_rebuild` typed-IR layout planning.
- ✅ typed edits for geometry, normals, UV, texture slot, alpha, nearest-filter bit, GS CLAMP REGION_REPEAT and node translation/rotation.
- ✅ dependent metadata derivation and mandatory canonical output reparse.
- ✅ source-bound mutation guards and fail-closed canonical reflow when non-zero unmodeled source bytes exist.
- ✅ `ScmResourceBundleWriter` provides bounded SCM/texture-companion count/index coherence through existing texture framing/reflow code.
- ✅ consolidated no-edit corpus: 78 paths / 68 unique SHA-256 inputs; 78/78 parse, preserve-layout exact parity, canonical rebuild+reparse and canonical no-edit exact parity.
- ❌ real representative semantic-edit receipts across SCM domains remain open.
- ❌ provenance-bound retail texture rewrite, size-changing retail rebuild, PAC/PNST/NBZ reintegration and original-game acceptance remain open.

**SCM production/100% authoring:** ❌ NOT CLAIMED.

### MOT

PR #372 consolidates MOT onto one canonical parser/IR:

- ✅ `MOT\0` marker, aligned header/channel-mask extent, nine-bit channel mask and record-count/popcount relationship.
- ✅ typed compression-2 and compression-3 key payloads and bounded track extents.
- ✅ three hash-bound real MOT payloads parse through the modular parser.
- ✅ canonical-EXE recovery of binding-bit traversal, signed 16-bit track start-time offsets, quantization decode and compression-3 linear/Hermite segment evaluation with slope orientation.
- ⚠️ interpolation helper is algebraic recovery, not bit-identical SSE parity.
- ❌ exact segment lookup/cache/duplicate-time behavior, alternate flag `0x2` binding path, other compression modes, looping/blending/full player composition and original-game output parity remain open.

### SHW / textures

- ✅ SHW structural/read-only Native Reader remains canonical (#288).
- ❌ SHW matrix-palette ownership and writer authority remain open.
- ⚠️ DDS/PTX read behavior is evidence-backed; production texel authoring is not promoted by reader support.

## 6. Current P0 proof track

```text
OpenGameResource(request)
 -> successful mount topology
 -> selected provider/volume/member
 -> exact materialized bytes
 -> PAC/PNST expansion where applicable
 -> typed post-load
 -> LoadedResource state3
 -> deterministic consumer-visible effect
```

Then repeat the same exact lineage with an authored higher-numbered NBZ and rollback.

Execution order:

1. ✅ static resolver/materialization authority retained;
2. ✅ successful-mount topology correction (#287);
3. ✅ SHW Native Reader integration (#288);
4. ✅ MOD Writer Gate 1 + 38-file no-op + controlled retail edit (#365/#368/#369);
5. ✅ MOD provenance-bound retail PNST reintegration and synthetic NBZ reopen gate (#372);
6. ✅ SCM writer/corpus consolidation and MOT parser/key-evaluation consolidation (#372);
7. ⚠️ scheduler raw pass still requires its own bounded canonical analysis;
8. ❌ real protected-process R2B mapping;
9. ❌ trusted selected-provider/member identity;
10. ❌ bind selected identity to exact materialized SHA/provenance;
11. ❌ original-process typed post-load/state3 on the same lineage;
12. ❌ repeat using authored higher-volume overlay;
13. ❌ deterministic original-game effect + rollback;
14. ❌ final independent L1/L2/L3 audits.

## 7. Long-term milestones

1. ❌ GDSpaces L1 accepted with real resolver-selected provenance, authored overlay selection/consumption and rollback.
2. ❌ Narrow L2 closure with trusted selected-provider identity.
3. ❌ Representative L3 lifecycle validation.
4. ❌ Stage Ops game-backed assembly over representative catalog selections.
5. ❌ Stable Stage Semantic Graph and ModViz editing verticals.
6. ❌ First bounded recovered-subsystem behavioral-equivalence receipt.
7. ❌ Progressive recompilation with controlled replacement modules.
8. ❌ Working rebuilt executable milestones without weakening evidence gates.

## 8. Completion rule

A checkbox becomes ✅ only at the authority required by its claim. Reader success, synthetic fixtures, CI green status, a successful rebuild, or a crash-free launch never upgrades a claim beyond its evidence class.
