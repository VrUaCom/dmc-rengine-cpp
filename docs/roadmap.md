# DMC Rengine Roadmap

**Snapshot:** 2026-09-11  
**Canonical base reviewed:** `main@d8534badbbe52cae1610d624822874431f581fa9`  
**Latest reviewed promotion on current main:** PR #372 — unified MOD/SCM/MOT consolidation and container gates (merged after PR #375)  
**Latest public/status reconciliation:** PR #375 — MOD Writer Gate 1 roadmap/learning reconciliation  
**Active branch advancement:** `reverse/mod-completion-20260907` — bounded MOT runtime channel path + animated-local EXE acquisition gate  
**Native Reader registry authority:** directly verified on current `main`  
**Completion model:** evidence-gated, not percentage-gated.

- ✅ **PROVEN / CLOSED** — supported at the authority required by the claim.
- ⚠️ **IMPLEMENTED / PROOF OPEN** — implementation/research exists, but stronger retail/original-game/full-production proof remains open.
- ❌ **OPEN** — mandatory work/evidence is missing.
- ➖ **PRODUCT POLICY / REVERSE N/A** — DMC Rengine policy, not claimed as Capcom behavior.

Synthetic tests and CI prove DMC Rengine behavior only. They do not by themselves prove original-game equivalence.

## 1. Canonical executable and reverse authority

- ✅ Canonical `dmc3.exe`: SHA-256 `e454272ed0fb0247fcbcf300e5d55d7a3e96d50b89b9ffaff81bb978dcbdd082`, size 6,356,432, PE32+ x86-64, ImageBase `0x140000000`.
- ✅ `DMC3-%d.nbz` bootstrap/first-gap discovery, bounded `OpenGameResource` policy, archive/physical normalization and bounded LoadedResource spine have canonical static evidence.
- ✅ Exact canonical executable bytes are available to promoted model-family reverse work.
- ❌ Full original-runtime equivalence is not claimed.

## 2. GDSpaces L1 — Resource Materialization

- ➖ Atomic/no-replace publication and artifact-bound SHA/ByteProvenance are closed product policies.
- ✅ PAC/PNST traversal, PAC slot 0 traversal and numbered NBZ behavior are reverse-backed in bounded scopes.
- ⚠️ NBZ STORE/raw-DEFLATE product path is implemented; full original stream/error equivalence remains open.
- ⚠️ PAC/PNST size-changing/nested authoring exists; original-game acceptance remains open.
- ✅ MOD Writer Gate 1 + writer receipt bridge are canonical (#365/#369).
- ✅ Provenance-bound real retail PNST reintegration of an authored MOD child is canonical (#372): target physical slot 23 in `m20_s00_012.pac`, unchanged parent size/slot table, only three expected child bytes changed, exact authored MOD recovered after reparse/re-expand.
- ✅ Synthetic MOD -> container -> next-volume NBZ overlay -> reopen is regression-proven (#372).
- ❌ Provenance-bound retail NBZ selection/consumption by the original runtime remains open.

### L1 acceptance gates

- ❌ trusted original resolver-selected provenance;
- ❌ exact representation classification for that selected lineage;
- ⚠️ retail PNST reintegration is proven independently, but the same-lineage original-selected -> authored-overlay -> rematerialized chain remains open;
- ❌ original runtime selects authored higher-numbered NBZ;
- ❌ deterministic consumer-visible effect;
- ❌ rollback proving retail immutability;
- ❌ final L1 audit.

**L1 COMPLETE:** ❌ NO.

## 3. GDSpaces L2 — Resource Resolution

- ✅ bounded resolver/mount behavior and successful-mount topology correction (#287) are canonical.
- ✅ exact `dmc3-0.nbz` normalized-key census is 4,333 / 4,333 / 0 collisions for that archive only.
- ❌ wider per-volume/cross-volume collision census;
- ❌ protected-process R2B mapping;
- ❌ trusted R3 selected-provider/member identity;
- ❌ direct-retail original resolver winner;
- ❌ final L2 audit.

**L2 COMPLETE:** ❌ NO.

## 4. GDSpaces L3 — Original Runtime / Lifecycle

- ✅ materialization success -> state1 -> state2 -> typed post-load/callback -> state3 bounded spine;
- ✅ bounded cancellation/quiescence/release ordering;
- ✅ central dispatcher coverage for recovered MOD/EFM/SCM/SHW + PNST recursion.
- ❌ current-main L3-R1 promotion;
- ❌ L3-R2 family/backing ownership;
- ❌ exact scheduler terminal dependency;
- ❌ V1–V7 original-process lifecycle receipts;
- ❌ original `dmc3.exe` authored-resource selection/consumption;
- ❌ final L3 audit.

**L3 COMPLETE:** ❌ NO.

## 5. Native Reader / format coverage

Direct inspection of `NativeReaderModuleRegistry` on `main@d8534bad…` confirms canonical registry membership for:

- ✅ DDS;
- ✅ PTX;
- ✅ HITS;
- ✅ DCA;
- ✅ LIG2/LIG;
- ✅ Stage TXT;
- ✅ SCM;
- ✅ MOD;
- ✅ MOT;
- ✅ SO graph;
- ✅ SO volume;
- ✅ SO link;
- ✅ SHW;
- ✅ PE/EXE.

This corrects older roadmap/status snapshots that omitted MOT and the SO module family. Registry presence is product integration only; it does not imply full semantics or writer authority.

- ❌ EFM is not yet a canonical Native Reader module.
- ❌ MRP/MCV/CAM/CLT/TSC and other recognized families remain evidence-gated.

### MOD

- ✅ canonical read/reverse stack: hierarchy/transforms, texture/GS state, post-load topology, object/header state, companion binding, inverse-rest palette, runtime descriptors, preserved unknowns and skin ABI (#305–#323, #356/#359/#360/#363).
- ✅ Preserve-Layout Writer Gate 1 (#365).
- ✅ deterministic writer corpus tooling (#367).
- ✅ provenance-bound 38-file no-op exact byte parity (#368): 38/38, 882,736 bytes, zero modified bytes/failures.
- ✅ one provenance-bound controlled retail radius edit (#369), exactly three changed bytes in `[124,128)`.
- ✅ writer receipt bridge + synthetic PAC reintegration (#369).
- ✅ provenance-bound real retail PNST reintegration/reopen (#372).
- ✅ synthetic MOD -> container -> NBZ overlay -> reopen (#372).
- ❌ full MOD writer remains open: typed-IR-only layout synthesis/reflow, transform, skin, material/source-flag, texture-companion authoring and broader mutation authority.
- ❌ provenance-bound retail NBZ/original-game acceptance remains open.

### SCM

PR #372 promotes one selected canonical bounded authoring stack:

- ✅ `preserve_layout` same-layout authoring;
- ✅ deterministic `canonical_rebuild` typed-IR layout planning;
- ✅ typed geometry/normal/UV/texture-slot/alpha/filter/GS-CLAMP/node-transform edits;
- ✅ dependent metadata derivation and mandatory canonical output reparse;
- ✅ source-bound mutation guards and fail-closed reflow on non-zero unmodeled source bytes;
- ✅ bounded texture-companion coherence through `ScmResourceBundleWriter`;
- ✅ 78 paths / 68 unique inputs: 78/78 parse, preserve-layout exact parity, canonical rebuild+reparse and canonical no-edit exact parity.
- ❌ provenance-bound representative semantic edits, retail texture rewrite, real-retail size-changing rebuild, SCM PAC/PNST/NBZ reintegration and original-game acceptance remain open.

### MOT

- ✅ MOT is a canonical `NativeReaderModuleRegistry` member (`native_reader_modules::mot()`).
- ✅ PR #372 consolidates its structural implementation onto one modular parser/IR.
- ✅ `MOT\0`, aligned header/mask contract, nine-bit channel mask, record/popcount relation and compression-2/3 typed keys are canonical.
- ✅ three hash-bound real MOT payloads parse through the modular path.
- ✅ canonical-EXE recovery covers exact normal-path binding traversal, serialized track ordinal consumption, signed start-time offsets, quantization and exact T/R/S semantic channel mapping/CMotionJoint channel bases.
- ✅ compression-3 cached forward/backward segment search, endpoints and cache-dependent duplicate-time selection are statically recovered from `0x1402E8C80..0x1402E8E10`.
- ✅ compression-3 linear/Hermite segment algebra and slope orientation are recovered and composed into a bounded scalar evaluator.
- ✅ branch-level analysis composes `MOT track -> semantic T/R/S channel -> EXE-confirmed MOD motion group` without renumbering excluded track ordinals.
- ⚠️ interpolation/search recovery is semantic/static authority, not bit-identical SSE or complete runtime cache-lifecycle parity.
- ❌ header flag `0x2` alternate binding, compression-2 whole-track parity/other modes, exact mutable CMotion channel-state ownership, T/R/S -> animated local matrix construction, looping/blending/player scheduler behavior and original-game parity remain open.
- ⚠️ next direct-EXE gate is encoded in `data/reverse/dmc3-mot-local-matrix-window-plan.v1.json`; `joint+0x110` is not promoted to a final animated-local matrix until fresh canonical bytes close ownership/write ordering.

### SO / SHW / textures

- ✅ SO graph/volume/link are canonical registry members; this is not universal SO writer/semantic authority.
- ✅ SHW structural/read-only module remains canonical (#288).
- ❌ SHW palette ownership/writer authority remain open.
- ⚠️ DDS/PTX read behavior is evidence-backed; production texel authoring is not promoted by read support.

## 6. Current P0 proof track

```text
OpenGameResource(request)
 -> successful mount topology
 -> selected provider/volume/member
 -> exact materialized bytes
 -> PAC/PNST expansion
 -> typed post-load
 -> LoadedResource state3
 -> deterministic consumer-visible effect
 -> repeat with authored higher-numbered NBZ
 -> rollback
```

Execution frontier:

1. ✅ resolver/materialization static authority retained;
2. ✅ mount-topology correction (#287);
3. ✅ MOD Gate 1/no-op/controlled edit (#365/#368/#369);
4. ✅ MOD real retail PNST + synthetic NBZ reopen (#372);
5. ✅ SCM writer/corpus and MOT parser/key-evaluation consolidation (#372);
6. ❌ protected-process mapping/selected identity;
7. ❌ same-lineage original-process authored overlay selection/rematerialization;
8. ❌ deterministic original consumer effect + rollback;
9. ❌ final independent L1/L2/L3 audits.

## 7. Completion rule

A claim is complete only at its required authority level. Reader success, registry membership, synthetic fixtures, CI green state, successful rebuild or crash-free launch never upgrade a claim beyond its evidence class.
