# Current Blockers

**Snapshot date:** 2026-08-26  
**Canonical base:** `main@453373ff0977fc0aa1f6fab39273cdd9716da6af`  
**L2 candidate tooling:** #221 merged; trusted real capture still open

The canonical Layer-1 execution order is [GDSpaces L1 Roadmap](../gdspaces/l1-roadmap.md). The cross-layer dependency order is [GDSpaces Master Roadmap](../gdspaces/master-roadmap.md). Real-device member-evidence reconciliation is recorded in [L1 real-device member evidence reconciliation](../gdspaces/l1-real-device-member-evidence-reconciliation-2026-08-26.md).

## P0 — GDSpaces L1 completion blockers

There is no known mandatory internal product-code blocker for the current representative DMC3-HD L1 acceptance scope.

The remaining P0 gates are **same-lineage real evidence executions**.

### B-L1-01 — Selected-source + representative member provenance

**Status:** REAL RECEIPT REQUIRED / MEMBER-MATERIALIZATION SUBPATH READY ON POCKET

Canonical protected-install route:

```text
dmc-rengine extract-dmc3-retail-member <exe-dir> <game-request> <output-file>
```

Preserve:

- protected executable authority;
- observed numbered-volume topology;
- resolver-selected provider/volume/archive/member identity;
- archive SHA/size;
- central-entry metadata;
- materialized SHA/size;
- compression transform and ByteProvenance.

Pocket GDS PR #2 adds a real-device `gdspaces.l1.member-acquisition-receipt.v1` path. When the actual NBZ is local on-device, a canonical Export can preserve the archive snapshot SHA/size, ResourceIdentity, ByteProvenance and exact member SHA/size without transferring the 960 MB archive through the connected channel.

A Pocket receipt closes only the **local snapshot/member-materialization sub-gate**. It must still be bound to the accepted selected-provider/request lineage before final L1-C promotion. Filename equality is not sufficient.

`obj\em000.pac` is a high-value request, not a predeclared runtime winner. Another representative request is acceptable if it gives a stronger deterministic authoring/consumer receipt.

### B-L1-02 — Exact retail representation classification

**Status:** REAL RECEIPT REQUIRED / POCKET CLASSIFICATION PATH READY

Classify the exact materialized bytes from B-L1-01. A Pocket receipt may directly record canonical format/container classification and ByteProvenance for the materialized node.

Do not infer retail writer authority from transformed DDS/TM2/runtime evidence or filename alone.

If the representation is outside current supported authoring domains, stop and create a new bounded evidence gate.

### B-L1-03 — Representative real edit/rebuild/rematerialization receipt

**Status:** REAL VALIDATION REQUIRED

Current product code supports top-level and nested PAC/PNST size-changing authoring, next-volume NBZ creation and canonical rematerialization.

The remaining requirement is one real same-lineage receipt:

```text
accepted selected/member identity
 -> exact materialized representation
 -> supported bounded edit
 -> top-level or nested bottom-up rebuild
 -> byte-exact untouched sibling validation
 -> next-contiguous NBZ
 -> canonical resolver higher-volume winner
 -> exact rebuilt-member rematerialization
 -> exact authored-child verification
```

A mobile acquisition PASS alone does not close this blocker.

### B-L1-04 — Original DMC3 Level-E consumption + rollback

**Status:** FINAL EXTERNAL ACCEPTANCE REQUIRED

Canonical tracking: issue #209.

The generated exact overlay must be copied into the protected installation under controlled conditions, its SHA verified, consumed through a deterministic original-game path, then removed without changing original retail artifacts.

A crash-free launch alone is insufficient. A Pocket GDS open/export is also insufficient.

### B-L1-05 — Final L1 cross-stack/V audit

**Status:** OPEN / DEPENDS ON B-L1-01..04

Before `L1 COMPLETE / 100%`:

- protected executable authority exists;
- selected-provider/source/archive/member provenance exists;
- exact materialized member identity exists;
- real representation classification exists;
- real edit/rebuild/rematerialization receipt exists;
- original-game consumer observation exists;
- rollback proves original retail immutability;
- all receipts belong to one reconciled lineage rather than being matched by filename;
- exact-head Windows + Ubuntu CI is green;
- #100, #182, #209, code and current documentation agree;
- no unresolved contradiction alters the declared supported L1 scope;
- V issues the final L1 verdict.

## Layer 2 evidence blockers

These are L2 closure gates. They are not substitutes for the L1 Level-E acceptance sequence.

### B-L2-01 — Real-retail `0x0E` collision census

**Status:** EXACT ARCHIVE-BOUND MEMBER SURFACE REQUIRED

The exact `dmc3-0.nbz` artifact is locatable and its observed size is `960,358,951` bytes, but it cannot currently be transferred through the connected Drive/Files execution channel because of the observed `268,435,456`-byte materialization ceiling.

No complete exact central-directory/member-list derivative is yet canonical in the connected corpus.

Required evidence is an exact member-name/central-directory surface cryptographically bound to the retail archive, followed by the canonical `0x0E` normalized-key collision census.

Pocket GDS may become a useful derivative-producing execution surface only if it exports the **complete required member-name/central-directory surface** and binds that derivative to the archive SHA. A single-member receipt does not close the collision census.

### B-L2-02 — Real protected-distribution runtime RVA mapping receipt

**Status:** TOOLING INTEGRATED BY #219 / REAL ORIGINAL-PROCESS RECEIPT REQUIRED

Authority split:

- canonical analysis executable: SHA-256 `e454272ed0fb0247fcbcf300e5d55d7a3e96d50b89b9ffaff81bb978dcbdd082`, size 6,356,432;
- protected distribution execution candidate: SHA-256 `81c7e61983564113b5105e931d9f185accc14e44ae147d27f720c2d50935c7d6`, size 6,567,320.

Canonical analysis VAs/RVAs must not be applied to the protected process without independent mapping evidence.

#219 is integrated. It provides bounded live main-module acquisition by explicit PID + RVA, exact protected-image SHA/size gating, metadata-only receipts and a multi-anchor mapping validator. The blocker is execution: produce a real protected-process packet containing `OpenGameResource` plus at least two approved type-0 physical anchors from one process/module session.

Synthetic/self-process CI proves tooling behavior only.

### B-L2-03 — Trusted original-process selected-provider identity

**Status:** #221 TOOLING MERGED / REAL R2B + TRUSTED PUBLISHER REQUIRED

#221 defines a strict selected-identity **content candidate**, normalizer, validator and artifact-binding pipeline. Merge does not turn editable/candidate JSON into original-process evidence.

A real R3 promotion still requires:

1. a valid real B-L2-02 mapping packet;
2. a runtime publisher/observer attached to that exact protected process without altering resolver selection semantics;
3. a zero-loss trace (`trace_complete=true`, `dropped_event_count=0`);
4. exact observer artifact SHA binding;
5. exact mounted `DMC3-0..N-1.nbz` artifact SHA/size binding;
6. a trusted origin/capture binding that is not asserted by editable JSON fields.

Fresh canonical EXE review adds a mandatory failure distinction: archive normalized lookup can find an entry and still fail during wrapper/open creation at `0x140328290`. In that case `0x140327430` exits through null/cleanup and does **not** continue to a lower volume as a clean lookup miss. Clean-path R3 therefore supports only `miss` and terminal `selected`; provider/backend failure must fail closed.

`preflight-dmc3-game-test` is not this receipt; it validates executable/archive presence and authority only.

### B-L2-04 — Direct-retail selected identity / member binding

**Status:** REAL EVIDENCE REQUIRED

A final original-process `ResourceRef`/member winner requires trusted R3 capture and exact archive/member binding. The retail `0x0E` collision state must also be known wherever normalized-key uniqueness is part of the identity claim.

A Pocket member receipt can provide exact materialized byte identity **after** a selected member is known; it cannot self-declare the original resolver winner.

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
- protected-runtime explicit-PID RVA acquisition + bounded multi-anchor mapping tooling — integrated by #219; only the real receipt remains open under B-L2-02;
- selected-provider content-candidate/normalizer/validator/artifact-binder tooling — merged by #221; trusted origin/capture remains open under B-L2-03.

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
- The connected 960 MB NBZ transfer ceiling is not a format/parser failure and does not imply the archive is absent.

## Environment blocker — corrected

Connected Drive evidence can locate the protected executable and `data/dmc3/dmc3-0.nbz`. The exact `dmc3-0.nbz` cannot be ingested into this connected execution container because the observed raw transfer/materialization ceiling is `268,435,456` bytes while the archive is `960,358,951` bytes.

Pocket GDS provides a real-device route for exact member materialization where the archive is already local. The protected game machine/process remains required for original-process selected identity, Level-E consumption and rollback evidence that mobile execution cannot prove.

This is an external execution/evidence limitation. It must not be hidden by synthetic CI or converted into a weaker completion criterion.
