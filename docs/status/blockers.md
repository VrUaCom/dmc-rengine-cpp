# Current Blockers

**Snapshot date:** 2026-08-26  
**Canonical base:** `main@eb701b9c523a3ec87f3c73bb8764038f1f2ef8dc`  
**Active L2 evidence slice:** PR #221

The canonical Layer-1 execution order is [GDSpaces L1 Roadmap](../gdspaces/l1-roadmap.md). The cross-layer dependency order is [GDSpaces Master Roadmap](../gdspaces/master-roadmap.md).

## P0 — GDSpaces L1 completion blockers

There is no known mandatory internal product-code blocker for the current representative DMC3-HD L1 acceptance scope.

The remaining P0 gates are evidence executions.

### B-L1-01 — Direct-retail representative provenance

**Status:** EXTERNAL REAL-RETAIL RECEIPT REQUIRED

The acquisition and its receipt emitter are library code
(`profiles::dmc3::RetailMemberAcquisition`), not command-only, so the receipt
can be produced by whichever frontend actually reaches a retail installation.
The remaining requirement is the run itself.

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

**Status:** EXTERNAL EVIDENCE REQUIRED

Classify the exact bytes from B-L1-01. Do not infer retail writer authority from transformed DDS/TM2/runtime evidence alone.

If the representation is outside current supported authoring domains, stop and create a new bounded evidence gate.

### B-L1-03 — Representative real edit/rebuild/rematerialization receipt

**Status:** EXTERNAL VALIDATION REQUIRED

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

### B-L1-04 — Original DMC3 Level-E consumption + rollback

**Status:** FINAL EXTERNAL ACCEPTANCE REQUIRED

Canonical tracking: issue #209.

The generated exact overlay must be copied into the protected installation under controlled conditions, its SHA verified, consumed through a deterministic original-game path, then removed without changing original retail artifacts.

A crash-free launch alone is insufficient.

### B-L1-05 — Final L1 cross-stack audit

**Status:** OPEN / DEPENDS ON B-L1-01..04

Before `L1 COMPLETE / 100%`:

- real acquisition provenance exists;
- real representation classification exists;
- real edit/rebuild/rematerialization receipt exists;
- original-game consumer observation exists;
- rollback proves original retail immutability;
- exact-head Windows + Ubuntu CI is green;
- #100, #182, #209, code and current documentation agree;
- no unresolved contradiction alters the declared supported L1 scope.

## Layer 2 evidence blockers

These are L2 closure gates. They are not substitutes for the L1 Level-E acceptance sequence.

### B-L2-01 — Real-retail `0x0E` collision census

**Status:** EXTERNAL ARTIFACT ACCESS REQUIRED

The exact `dmc3-0.nbz` artifact is approximately 960 MB and cannot currently be transferred through the connected Drive channel. No exact central-directory/member-list derivative is available in the connected corpus.

Required evidence is an exact member-name/central-directory surface cryptographically bound to the retail archive, followed by the canonical `0x0E` normalized-key collision census.

### B-L2-02 — Real protected-distribution runtime RVA mapping receipt

**Status:** TOOLING INTEGRATED BY #219 / REAL ORIGINAL-PROCESS RECEIPT REQUIRED

Authority split:

- canonical analysis executable: SHA-256 `e454272ed0fb0247fcbcf300e5d55d7a3e96d50b89b9ffaff81bb978dcbdd082`, size 6,356,432;
- protected distribution execution candidate: SHA-256 `81c7e61983564113b5105e931d9f185accc14e44ae147d27f720c2d50935c7d6`, size 6,567,320.

Canonical analysis VAs/RVAs must not be applied to the protected process without independent mapping evidence.

#219 is already integrated. It provides bounded live main-module acquisition by explicit PID + RVA, exact protected-image SHA/size gating, metadata-only receipts and a multi-anchor mapping validator. The blocker is now execution: produce a real protected-process packet containing `OpenGameResource` plus at least two approved type-0 physical anchors from one process/module session.

Synthetic/self-process CI proves tooling behavior only.

### B-L2-03 — Trusted original-process selected-provider identity

**Status:** CANDIDATE/BINDER TOOLING IN PR #221 / BLOCKED BY REAL B-L2-02 + TRUSTED PUBLISHER

#221 defines a strict selected-identity **content candidate** and artifact-binding pipeline. It intentionally does not turn editable JSON into original-process evidence.

A real R3 promotion still requires:

1. a valid real B-L2-02 mapping packet;
2. a runtime publisher/observer attached to that exact protected process without altering resolver selection semantics;
3. a zero-loss trace (`trace_complete=true`, `dropped_event_count=0`);
4. exact observer artifact SHA binding;
5. exact mounted `DMC3-0..N-1.nbz` artifact SHA/size binding;
6. a trusted origin/capture binding that is not asserted by editable JSON fields.

Fresh canonical EXE review adds a mandatory failure distinction: archive normalized lookup can find an entry and still fail during wrapper/open creation at `0x140328290`. In that case `0x140327430` exits through null/cleanup and does **not** continue to a lower volume as a clean lookup miss. Clean-path R3 v1 therefore supports only `miss` and terminal `selected`; provider/backend failure must fail closed.

`preflight-dmc3-game-test` is not this receipt; it validates executable/archive presence and authority only.

### B-L2-04 — Direct-retail resolver identity receipt

**Status:** BLOCKED BY B-L2-01

A real-retail `ResourceRef`/member winner cannot be promoted until the exact retail member surface is bound and the `0x0E` collision state is known. DMCL/synthetic collision results do not close this gate.

### B-L2-05 — Final L2 audit

**Status:** OPEN / DEPENDS ON B-L2-01..04

Layer 2 remains incomplete until real-retail collision evidence, protected-runtime mapping, trusted original-process selected identity, exact-head CI and canonical code/docs agree.

## Closed former L1 blockers

Do not reopen these absent contradictory direct evidence:

- atomic/no-replace publication — closed by #194;
- artifact-bound archive/member stability — closed by #195;
- direct-retail acquisition implementation — closed by #196;
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

## Closed/integrated former L2 blockers/tooling gaps

Do not reopen absent contradictory direct evidence:

- exact type-0 physical-provider post-`0x0C` Win32 final path/open/miss semantics — static reverse closed by #215/#204;
- product physical native-path model + controlled hit/miss/archive→physical fallback receipts — promoted by #215 with Windows + Ubuntu validation;
- protected-runtime explicit-PID RVA acquisition + bounded multi-anchor mapping tooling — integrated by #219; only the real receipt remains open under B-L2-02.

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

## Environment blocker

The connected automation environment does not currently expose the exact raw protected installation artifacts required to execute the real L1 receipts, retail DMC3 collision census or protected-process R2B/R3 runs here.

This is an external evidence/access limitation. It must not be hidden by synthetic CI or converted into a weaker completion criterion.
