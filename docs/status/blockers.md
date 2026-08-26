# Current Blockers

**Snapshot date:** 2026-08-26  
**Canonical base:** `main@c147facb310d32ef084c56ba82d1e4b6b9b1b496`  
**Active L2 evidence slice:** PR #219  
**Canonical L1 EXE review:** [l1-exe-boundary-review-2026-08-26.md](../gdspaces/l1-exe-boundary-review-2026-08-26.md)

The canonical Layer-1 execution order is [GDSpaces L1 Roadmap](../gdspaces/l1-roadmap.md). The cross-layer dependency order is [GDSpaces Master Roadmap](../gdspaces/master-roadmap.md).

## P0 — GDSpaces L1 completion blockers

There is no known mandatory internal product-code blocker for the current representative DMC3-HD L1 acceptance scope.

The remaining P0 gates are evidence executions.

### B-L1-01 — Direct-retail representative provenance

**Status:** EXTERNAL REAL-RETAIL RECEIPT REQUIRED

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

## Supporting L1 reverse gaps — bounded, not automatic completion blockers

These are the current EXE reverse seams found by the three-pass 2026-08-26 review. They become P0 only if the selected acceptance path or claimed compatibility scope depends on them.

### B-L1-R1 — Materialization fan-in / completion

**Status:** STATIC REVERSE BREADTH OPEN

Known distinction:

- `0x1400335A0(ticketId,userContext,errorFlag,bytesRead)` = transport/whole-file completion;
- `0x1401B8DC0` = scheduler/resource materialization completion handoff registered through `0x1402EF580`, normal branch publishes `state 1 -> 2`.

Open exact questions:

- outstanding direct/child submission aggregation;
- parent-completion condition;
- nested `.lst` participation in completion;
- one-child-failure behavior;
- partial destination lifetime on failure;
- exact condition preventing state2 publication.

### B-L1-R2 — Transport error -> resource scheduler/materialization error mapping

**Status:** STATIC REVERSE BREADTH OPEN

Raw transfer error/status behavior is substantially recovered, but the exact bridge from transport failure into resource-level scheduling/completion failure is not fully closed.

### B-L1-R3 — `.lst` temporary allocation/free/failure cleanup

**Status:** STATIC REVERSE BREADTH OPEN

The list text is known to be loaded synchronously into aligned temporary storage before bounded parsing. It is **not** proven to use the synchronous-style wrapper around `0x1402EF920`.

Exact allocator/free identity, malformed/truncated failure cleanup and recursion-failure propagation remain open.

### B-L1-R4 — FileSlot / ReadRequest exact error breadth

**Status:** BOUNDED

The 100×`0x20` FileSlot pool, `ReadRequestV2` architecture and completion ABI are strong. Partial-read/error/cancellation breadth should be recovered only where a compatibility or acceptance claim requires it.

### B-L1-R5 — ZIP initializer / compressed-seek exact-body breadth

**Status:** BOUNDED / LOWER PRIORITY

`0x140328540` lazy realization and `0x140328FE0` reset+reinflate/discard seek architecture are already strong. Complete exact-body/state-error details remain open but are no longer the automatic first reverse target.

## Layer 2 evidence blockers

These are L2 closure gates. They are not substitutes for the L1 Level-E acceptance sequence.

### B-L2-01 — Real-retail `0x0E` collision census

**Status:** EXTERNAL ARTIFACT ACCESS REQUIRED

The exact `dmc3-0.nbz` artifact is approximately 960 MB and cannot currently be transferred through the connected Drive channel. No exact central-directory/member-list derivative is available in the connected corpus.

Required evidence is an exact member-name/central-directory surface cryptographically bound to the retail archive, followed by the canonical `0x0E` normalized-key collision census.

### B-L2-02 — Protected-distribution runtime RVA mapping

**Status:** PRODUCT ACQUISITION SEAM IN PR #219 / REAL ORIGINAL-PROCESS RECEIPT REQUIRED

Authority split:

- canonical analysis executable: SHA-256 `e454272ed0fb0247fcbcf300e5d55d7a3e96d50b89b9ffaff81bb978dcbdd082`, size 6,356,432;
- protected distribution execution candidate: SHA-256 `81c7e61983564113b5105e931d9f185accc14e44ae147d27f720c2d50935c7d6`, size 6,567,320.

Canonical analysis VAs/RVAs must not be applied to the protected process without independent mapping evidence.

PR #219 adds bounded live main-module acquisition by explicit PID + RVA, exact protected-image SHA/size gating, metadata-only receipts and a multi-anchor mapping validator. Promotion still requires a real protected-process packet covering the approved L2 anchors. Synthetic/self-process CI proves tooling behavior only.

### B-L2-03 — Original-process selected-provider identity

**Status:** BLOCKED BY B-L2-02

After bounded runtime mapping is proven, instrument the mapped resolver path and preserve the original-process request, candidate order and exact selected provider/source/member identity.

`preflight-dmc3-game-test` is not this receipt; it validates executable/archive presence and authority only.

### B-L2-04 — Final L2 audit

**Status:** OPEN / DEPENDS ON B-L2-01..03

Layer 2 remains incomplete until real-retail collision evidence, protected-runtime mapping, original-process selected identity, exact-head CI and canonical code/docs agree.

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

## Closed former L2 blocker

Do not reopen absent contradictory direct evidence:

- exact type-0 physical-provider post-`0x0C` Win32 final path/open/miss semantics — static reverse closed by #215/#204;
- product physical native-path model + controlled hit/miss/archive→physical fallback receipts — promoted by #215 with Windows + Ubuntu validation.

## Explicit superseded reverse shorthand

Do not reintroduce:

- `0x1402EF4D0 == packed-file reader/exact-path resolver/final backend open`;
- `0x1401B8DC0 == raw I/O callback`;
- `.lst synchronous temporary load == 0x1402EF920`;
- `FileSlot/AsyncIO == wholly L3` for byte-transport accounting;
- `type-0 physical final-open semantics still open` after #215;
- `0x140328540/0x140328FE0 architecture unknown`.

## Evidence-gated freezes / non-blockers

- Binary AFS is not inferred from `.afs/` logical namespace strings.
- Historical PACK parsing does not establish original DMC3 PACK runtime authority.
- Capcom offline writer equivalence is not required for DMC Rengine L1 product authoring acceptance.
- Stage Ops, ModViz and unrelated HITS semantics do not count as L1 closure.

## Environment blocker

The connected automation environment does not currently expose all exact raw protected-install artifacts required to execute the real L1 receipts, retail DMC3 collision census or protected-process runtime mapping here.

Canonical-analysis static EXE reacquisition is available through the guarded window-packet tooling on current `main`. Supporting reverse should use that authority rather than ad hoc unbound byte windows.

This is an external evidence/access limitation. It must not be hidden by synthetic CI or converted into a weaker completion criterion.
