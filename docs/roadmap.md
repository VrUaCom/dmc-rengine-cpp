# DMC Rengine Roadmap

**Snapshot:** 2026-09-05  
**Canonical base entering current slice:** `main@76841d6f1387b08df40bb65e0083513f9dc7c5bb`  
**Current integration slice:** PR #288 — evidence-backed SHW Native Reader  
**Completion model:** evidence-gated, not percentage-gated.

This roadmap distinguishes four different things that must never be conflated:

- ✅ **PROVEN / CLOSED** — the claim is supported at the required authority level. Original-DMC3 behavior requires a bounded reverse pass against the canonical executable and, where the claim is corpus/runtime-specific, matching corpus or original-process evidence.
- ⚠️ **IMPLEMENTED / PROOF OPEN** — DMC Rengine code exists and may have green tests/CI, but the original-game or real-retail proof required by the claim is still open, or the implementation is still awaiting canonical promotion.
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

### L1 acceptance gates

- ❌ Direct-retail resolver-selected provenance receipt from a protected installation.
- ❌ Exact retail representation classification for the selected bytes.
- ❌ Representative real edit + bottom-up rebuild + rematerialization receipt.
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

Canonical modular Native Reader modules on `main@76841d6...` before the current slice:

- ✅ DDS structural reader.
- ✅ PTX structural reader.
- ✅ HITS reader.
- ✅ DCA reader.
- ✅ LIG2/LIG reader.
- ✅ Stage TXT reader.
- ✅ SCM structural reader.
- ✅ MOD structural reader.
- ✅ PE/EXE reader.

Current SHW slice:

- ⚠️ SHW structural/read-only reader is implemented on PR #288 as `formats.shw-structural-v1`; exact-head Windows + Ubuntu CI, final diff review and canonical promotion remain.
- ✅ The SHW schema used by this reader is backed by the canonical EXE plus hash-bound real payload `slot_0008.shw` (9,488 bytes, SHA-256 `cb392ef2e874addb887d32bc44d409299a32a83a4845afcbdef31698283f2e7e`).
- ✅ The bound layout proves self-contained shadow-hull geometry: triangle topology, exact adjacency, `float4` positions and a per-vertex selector stream.
- ✅ The selector is EXE-confirmed as indexing 0x40-byte transform matrices.
- ❌ SHW matrix-palette ownership/construction remains open.
- ❌ SHW writer authority and universal revision coverage remain open.

Other evidence boundaries:

- ✅ SCM structural reader is backed by dedicated EXE/corpus reverse packets for layout, hierarchy/world transform and runtime flags.
- ✅ MOD structural reader is backed by EXE-family evidence and a hash-bound real payload structural binding.
- ⚠️ DDS/PTX reader behavior is evidence-backed for the promoted DMC3 profiles, but authoring is not promoted by read support.
- ❌ EFM is not yet a canonical Native Reader module.
- ❌ MOT is not yet a canonical Native Reader module.
- ❌ MRP/MCV/CAM/CLT/TSC and other recognized families require evidence-gated structural promotion before reader claims.

Writers:

- ❌ SCM canonical production writer authority.
- ❌ MOD writer authority.
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
3. ⚠️ finish SHW Native Reader PR #288 with exact-head cross-platform CI/review and canonical promotion;
4. ❌ obtain/process a real protected-process R2B mapping packet;
5. ❌ capture trusted selected-provider/member identity without synthesizing probes;
6. ❌ bind selected member to independently materialized SHA/provenance;
7. ❌ bind materialization success to LoadedResource lifecycle observation;
8. ❌ repeat with authored next-volume overlay;
9. ❌ observe deterministic original-game effect and rollback;
10. ❌ run final L1/L2/L3 contradiction audits independently.

The still-open raw scheduler dependency remains a separate L3 proof task. It must not be silently inferred from older labels when the exact canonical executable bytes are unavailable for a fresh pass.

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
