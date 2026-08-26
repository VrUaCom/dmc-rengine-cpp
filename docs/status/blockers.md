# Current Blockers

**Snapshot date:** 2026-08-26  
**Canonical base:** `main@c20544cfb7f3ddba69a128a88246550a35eb51c1`  
**Unified validation authority:** issue #222 / PR #223  
**Active original-process evidence contracts:** PR #218 (L3), PR #221 (L2)

The canonical execution order is [GDSpaces Master Roadmap](../gdspaces/master-roadmap.md). Validation/equivalence ownership is defined by [V / LV Architecture](../gdspaces/validation-equivalence-architecture.md).

## Validation authority rule

L1/L2/L3 implementation/reverse blockers remain owned by their layers. Validation blockers are classified under V:

- **V-A** provenance/receipt/trusted-origin integrity;
- **V-B** product CI/controlled tests;
- **V-C** exact real-corpus validation;
- **V-D** trusted original-process equivalence;
- **V-E** breadth/cross-layer/subsystem acceptance.

**LV** is V-owned live/original-process evidence acquisition. LV is not L4 and cannot issue a promotion verdict.

No layer-local PASS or percentage bypasses an open mandatory V gate.

## P0 — GDSpaces L1 implementation/evidence blockers

There is no known mandatory internal product-code blocker for the current representative DMC3-HD L1 acceptance scope.

The remaining P0 gates are evidence executions and V acceptance.

### B-L1-01 — Direct-retail representative provenance

**Status:** EXTERNAL REAL-RETAIL RECEIPT REQUIRED / V-C SUPPORT

Run the canonical direct-retail acquisition command against a protected DMC3 installation and preserve:

- protected executable authority;
- observed numbered-volume topology;
- resolver-selected volume/archive/member identity;
- archive SHA/size;
- central-entry metadata;
- materialized SHA/size;
- compression transform and ByteProvenance.

`obj\em000.pac` is a high-value request, not a predeclared archive member. Another representative request is acceptable if it gives a stronger deterministic authoring/consumer receipt.

### B-L1-02 — Exact retail representation classification

**Status:** EXTERNAL EVIDENCE REQUIRED / V-C SUPPORT

Classify the exact bytes from B-L1-01. Do not infer retail writer authority from transformed DDS/TM2/runtime evidence alone.

If the representation is outside current supported authoring domains, stop and create a new bounded evidence gate.

### B-L1-03 — Representative real edit/rebuild/rematerialization receipt

**Status:** EXTERNAL VALIDATION REQUIRED / V-C

Current product code supports top-level and nested PAC/PNST size-changing authoring, next-volume NBZ creation and canonical rematerialization.

The remaining requirement is one real protected-install receipt:

```text
retail-selected member
 -> supported bounded edit
 -> top-level or nested bottom-up rebuild
 -> byte-exact untouched sibling validation
 -> next-contiguous NBZ
 -> canonical resolver higher-volume winner
 -> exact rebuilt-member rematerialization
 -> exact authored-child verification
```

### B-L1-04 — Original DMC3 consumption + rollback

**Status:** FINAL EXTERNAL V:L1 / V-D ACCEPTANCE REQUIRED

Canonical tracking: issue #209, parent V authority #222.

The generated exact overlay must be copied into the protected installation under controlled conditions, its SHA verified, consumed through a deterministic original-game path, then removed without changing original retail artifacts.

If the effect is ambiguous, LV observation is required. Crash-free launch alone is insufficient.

### B-L1-05 — V:L1 verdict + final L1 cross-stack audit

**Status:** OPEN / DEPENDS ON B-L1-01..04

Before `L1 COMPLETE / 100%`:

- real acquisition provenance exists;
- real representation classification exists;
- real edit/rebuild/rematerialization receipt exists;
- original-game consumer observation exists;
- rollback proves original retail immutability;
- exact-head Windows + Ubuntu V-B validation is green;
- V accepts the declared L1 evidence scope;
- #100, #209, #222, code and current documentation agree;
- no unresolved contradiction alters the declared supported L1 scope.

## Layer 2 implementation/evidence blockers

These are L2 closure gates. Their original-process validation is owned by LV/V rather than a separate L2 validation system.

### B-L2-01 — Real-retail `0x0E` collision census

**Status:** EXTERNAL ARTIFACT ACCESS REQUIRED / V-C

The exact `dmc3-0.nbz` artifact is approximately 960 MB and cannot currently be transferred through the connected Drive channel. No exact central-directory/member-list derivative is available in the connected corpus.

Required evidence is an exact member-name/central-directory surface cryptographically bound to the retail archive, followed by the canonical `0x0E` normalized-key collision census.

### B-L2-02 — Protected-distribution runtime RVA mapping

**Status:** LV TOOLING MERGED IN #219 / REAL V-A PROCESS RECEIPT REQUIRED

Authority split:

- canonical analysis executable: SHA-256 `e454272ed0fb0247fcbcf300e5d55d7a3e96d50b89b9ffaff81bb978dcbdd082`, size 6,356,432;
- protected distribution execution candidate: SHA-256 `81c7e61983564113b5105e931d9f185accc14e44ae147d27f720c2d50935c7d6`, size 6,567,320.

Canonical analysis VAs/RVAs must not be applied to the protected process without independent V-accepted mapping evidence.

#219 provides bounded live main-module acquisition by explicit PID + RVA, exact protected-image SHA/size gating, metadata-only receipts and multi-anchor mapping validation. A real protected-process packet is still required.

### B-L2-03 — Original-process selected-provider identity

**Status:** LV/V:L2 CONTRACT ACTIVE IN #220/#221 / REAL OBSERVATION REQUIRED

After bounded runtime mapping is proven, instrument the mapped resolver path and preserve the original-process request, candidate order and exact selected provider/source/member identity.

`preflight-dmc3-game-test` is not this receipt; it validates executable/archive presence and authority only.

### B-L2-04 — V:L2 verdict + final L2 audit

**Status:** OPEN / DEPENDS ON B-L2-01..03

Layer 2 remains incomplete until real-retail collision evidence, protected-runtime mapping, original-process selected identity, original-vs-GDSpaces comparison, exact-head V-B validation and V acceptance agree.

## Layer 3 validation blockers

### B-L3-01 — Remaining static writer/dispatcher/shared-owner closure

**Status:** OPEN / issue #88

Do not restart already bounded registry/state/lifecycle structure absent contradictory evidence. Remaining mandatory static work is limited to exact writer ownership/order, field ownership, typed-dispatch/failure breadth and shared-owner family behavior required by the declared L3 scope.

### B-L3-02 — Trusted LV-L3 observer/publisher

**Status:** OPEN / V-A + LV INFRASTRUCTURE

PR #218 defines a fail-closed lifecycle trace contract but intentionally does not provide trusted runtime origin. A shared LV publisher/binder must emit or bind sanitized original-process observations without allowing editable JSON to manufacture authority.

### B-L3-03 — Original-process lifecycle receipts

**Status:** OPEN / V:L3 / V-D

Canonical tracking: issue #217 / PR #218.

Required scenarios include initial load, transition, restart/reload, cancellation, full reset, shutdown and sufficient family/build breadth.

### B-L3-04 — V:L3 / V-E verdict + final L3 audit

**Status:** OPEN

L3 is not COMPLETE until mandatory static gaps are closed or explicitly demoted with evidence, required original-process receipts are V-accepted and breadth requirements are satisfied.

## Unified V blockers

Canonical parent: issue #222.

### B-V-01 — Parent V receipt/binding contract

**Status:** FIRST IN-MEMORY CONTRACT ACTIVE IN PR #223

The first code slice binds child receipts by:

- one `validation_run_id`;
- one `resource_binding_sha256`;
- exact original-execution artifact identity;
- child receipt hashes;
- rollback receipt when required;
- mandatory L1+L2+L3+LV composition for cross-layer vertical candidates.

Still open: serialized import/export, exact schema and trusted origin binding.

### B-V-02 — Non-forgeable trusted LV binder/publisher

**Status:** OPEN

PR #223 deliberately defines the binder authority as non-constructible and exposes no promotion operation. A later slice must introduce trusted publishing/binding without creating an editable/manual promotion path.

### B-V-03 — First same-run/same-resource vertical receipt

**Status:** OPEN / V-D

Required chain:

```text
ONE validation_run_id
 -> exact protected executable
 -> accepted LV mapping/observer authority
 -> L2 original selected provider/volume/member
 -> L1 exact materialized/authored/rematerialized bytes
 -> L3 original consumer/lifecycle observation
 -> deterministic effect where required
 -> rollback/cleanup
 -> V verdict
```

Receipts from unrelated runs cannot be combined by name alone.

### B-V-04 — Breadth matrix / final V audit

**Status:** OPEN / V-E

One representative vertical receipt is not subsystem completion. V-E must define and satisfy the required resource families, transitions, ownership cases and build/profile breadth for the final declared GDSpaces equivalence scope.

## Closed former L1 blockers

Do not reopen these absent contradictory direct evidence:

- atomic/no-replace publication — #194;
- artifact-bound archive/member stability — #195;
- direct-retail acquisition implementation — #196;
- raw-DEFLATE artifact-bound regression — #197;
- first-gap retail-read behavior — #198;
- verified immutable NBZ copy rebuild — #199;
- PAC/PNST user-facing size-changing rebuild — #201;
- protected retail product closure orchestration — #208;
- nested PAC/PNST root-to-leaf slot-path authoring — #213;
- NBZ STORE/raw-DEFLATE product materialization;
- PAC/PNST sparse/empty/alias-preserving parsing;
- recursive PAC/PNST expansion;
- ByteProvenance;
- next-volume STORE overlay generation and resolver selection composition.

## Closed former L2 blockers

Do not reopen absent contradictory direct evidence:

- exact type-0 physical-provider post-`0x0C` Win32 final path/open/miss semantics — #215/#204;
- product physical native-path model + controlled hit/miss/archive→physical fallback receipts — #215;
- protected-runtime RVA mapping acquisition/tooling seam — #219 (tooling only; real receipt remains B-L2-02).

## Bounded reverse gaps — not automatic L1 blockers

These become P0 only if the chosen real acceptance path depends on them:

- complete ZIP stream initializer `0x140328540` body/lifetime;
- complete compressed seek/reset/reinflate `0x140328FE0` behavior;
- exhaustive malformed/partial-read original error equivalence;
- dynamic `.lst` allocation/free/error/cycle semantics;
- real `.lst` corpus validation when claiming real loose-list consumption.

## Evidence-gated freezes / non-blockers

- Binary AFS is not inferred from `.afs/` logical namespace strings.
- Historical PACK parsing does not establish original DMC3 PACK runtime authority.
- Capcom offline writer equivalence is not required for DMC Rengine L1 product authoring acceptance.
- Stage Ops, ModViz and unrelated HITS semantics do not count as L1 closure.
- LV is not L4 and does not own original game functions.

## Environment blocker

The connected automation environment does not currently expose the exact raw protected installation artifacts required to execute the real retail/original-process receipts.

This is an external evidence/access limitation. It must not be hidden by synthetic CI or converted into a weaker V acceptance criterion.
