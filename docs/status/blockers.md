# Current Blockers

**Snapshot date:** 2026-08-27  
**Canonical base reviewed:** `main@f886f27e62ec9a05b6829df7fd074981a06a4b49`  
**Boundary/status authority:** `../gdspaces/layer-boundary-status-reconciliation-2026-08-27.md`

The canonical Layer-1 execution order is [GDSpaces L1 Roadmap](../gdspaces/l1-roadmap.md). Cross-layer ownership/order is [GDSpaces Master Roadmap](../gdspaces/master-roadmap.md).

## P0 — GDSpaces L1 completion blockers

Layer 1 is **INCOMPLETE / NOT 100%**.

There is now one mandatory internal reverse/equivalence blocker in addition to the real-retail/original-game acceptance gates.

### B-L1-00 — Original materialization terminal dependency

**Status:** MANDATORY STATIC REVERSE OPEN

Current proven chain:

```text
0x1401B8CA0 materialization dispatch
 -> 0x1402EF4D0 job/submission
 -> lower whole-file/FileSlot transport
 -> UNKNOWN exact terminal success/error condition
 -> normal 0x1401B8DC0 eligibility/suppression
 -> state 1 -> 2
```

Normal `0x1401B8DC0` receives only one u32 registry-relative context. It does not receive raw transport error/status, byte count, FileSlot handle, child count or outstanding-work metadata. Therefore transport/materialization success must already be resolved before normal callback dispatch, or the queued completion must be removed/suppressed.

FIFO order alone is insufficient if an earlier materialization job can submit async I/O and retire before transport becomes terminal. No generic fan-in counter is evidenced.

Required closure:

1. recover exact `0x1402EF4D0` queued materialization job identity/type and inherited load-context consumer;
2. identify matching `0x1402EF790` dispatch/persistence/re-poll/retirement behavior;
3. reacquire `0x1400333E0` pending/success/error semantics;
4. reacquire `0x140033390` terminal cleanup/release ordering;
5. bind `0x1400335A0` transport writes into that state;
6. prove what prevents normal `0x1401B8DC0` dispatch on failed/incomplete transfer;
7. recover relevant `0x1402EF460` pending scheduler-entry clear/rollback behavior;
8. apply the confirmed mechanism to `.lst` child/recursive failure ordering where required.

This blocker belongs to L1. L3 cancellation policy may interact with it but does not own the selected-byte terminal mechanism.

### B-L1-01 — Direct-retail representative provenance

**Status:** EXTERNAL REAL-RETAIL RECEIPT REQUIRED

Run canonical direct-retail acquisition against a protected DMC3 installation and preserve:

- protected executable authority;
- observed numbered-volume topology;
- resolver-selected volume/archive/member identity;
- archive SHA/size;
- central-entry metadata;
- materialized SHA/size;
- compression transform and ByteProvenance.

A preferred request such as `obj\em000.pac` is not a predeclared archive-member authority.

### B-L1-02 — Exact retail representation classification

**Status:** EXTERNAL EVIDENCE REQUIRED

Classify exact bytes from B-L1-01. If the representation falls outside current writer authority, stop and open a bounded evidence gate.

### B-L1-03 — Representative real edit/rebuild/rematerialization receipt

**Status:** EXTERNAL VALIDATION REQUIRED

Product code supports top-level/nested PAC/PNST size-changing authoring, next-volume NBZ creation and canonical rematerialization. One real protected-install receipt remains mandatory.

```text
retail-selected member
 -> supported bounded edit
 -> top-level or nested bottom-up rebuild
 -> exact untouched-span validation
 -> next-contiguous NBZ
 -> authored higher-volume winner
 -> exact rebuilt-member rematerialization
 -> exact authored-child verification
```

### B-L1-04 — Original DMC3 Level-E consumption + rollback

**Status:** FINAL EXTERNAL ACCEPTANCE REQUIRED

Canonical tracking: issue #209.

The exact generated overlay must be copied into the protected installation under controlled conditions, consumed through a deterministic original-game path, then removed without changing original retail artifacts.

A crash-free launch alone is insufficient.

### B-L1-05 — Final L1 cross-stack audit

**Status:** OPEN / DEPENDS ON B-L1-00..04

Before `L1 COMPLETE / 100%`:

- B-L1-00 terminal materialization dependency is closed;
- real acquisition provenance exists;
- real representation classification exists;
- real edit/rebuild/rematerialization receipt exists;
- original-game consumer observation exists;
- rollback proves original retail immutability;
- exact-head Windows + Ubuntu CI is green;
- #100, #182, #209, code and canonical docs agree;
- no unresolved contradiction alters the declared L1 scope.

## Layer 2 evidence blockers

L2 is **INCOMPLETE / NOT 100%**. These gates do not substitute for L1.

### B-L2-01 — Real-retail `0x0E` collision census

**Status:** EXTERNAL ARTIFACT ACCESS REQUIRED

Required evidence is an exact retail member-name/central-directory surface cryptographically bound to the archive, followed by the canonical `0x0E` normalized-key collision census.

### B-L2-02 — Real protected runtime RVA mapping receipt

**Status:** TOOLING INTEGRATED / REAL ORIGINAL-PROCESS RECEIPT REQUIRED

Authority split:

- analysis EXE: `e454272ed0fb0247fcbcf300e5d55d7a3e96d50b89b9ffaff81bb978dcbdd082`, size 6,356,432;
- protected execution candidate: `81c7e61983564113b5105e931d9f185accc14e44ae147d27f720c2d50935c7d6`, size 6,567,320.

Canonical analysis VA/RVA mappings require independent protected-process mapping evidence.

### B-L2-03 — Trusted original-process selected-provider identity

**Status:** REAL PROCESS + TRUSTED CAPTURE REQUIRED

Candidate/binder tooling is not original-process authority. Required promotion needs a valid mapping packet, trusted runtime publisher/origin, zero-loss trace and exact observer/NBZ artifact binding.

Provider/backend failure before a usable selected resource exists is L2 failure semantics and must not be rewritten as a clean miss.

### B-L2-04 — Direct-retail resolver identity receipt

**Status:** BLOCKED BY B-L2-01

A real-retail selected identity cannot be promoted before exact collision state is known for the bound retail member surface.

### B-L2-05 — Final L2 audit

**Status:** OPEN

Requires real-retail collision evidence, protected mapping, trusted selected identity, exact-head validation and contradiction-free canonical documentation.

## Layer 3 blockers

L3 is **INCOMPLETE / NOT 100%**.

Canonical L3 starts from completed state2/materialized bytes.

Open L3 gates include:

- residual alias-aware state writer/value-flow census outside already bounded paths;
- family-complete ownership of `+0x08/+0x18/+0x20/+0x28` and other stable fields;
- external typed/factory/dependency failure breadth;
- SCM `mesh +0x28` reconciliation;
- shared-owner coordination breadth;
- cross-build/profile differences;
- protected original-process V1–V7 lifecycle receipts;
- final contradiction-free L3 audit.

The L1 materialization terminal dependency is explicitly **not** an L3 blocker after the 2026-08-27 ownership correction.

## Closed/strong slices not to reopen without contradiction

### L1/product

- atomic/no-replace publication;
- artifact-bound archive/member stability;
- direct-retail acquisition implementation;
- NBZ STORE/raw-DEFLATE product materialization;
- PAC/PNST sparse/empty/alias-preserving parsing;
- recursive PAC/PNST expansion;
- same-level/nested size-changing reflow;
- verified immutable NBZ copy rebuild;
- next-volume STORE overlay authoring;
- ByteProvenance;
- `.lst` core grammar/layout/recursive synthesis.

### L2

- type-0 physical-provider post-`0x0C` static chain at bounded direct-call scope;
- controlled native physical product hit/miss/fallback model;
- protected-runtime RVA acquisition/mapping tooling.

### L3

- registry `363 x 0x48` / seven-group topology;
- state2 typed-finalizer -> optional callback -> state3 ready path;
- cancellation policy `1|2 -> 4`;
- quiescence `{0,3}`;
- state4 cleanup;
- distinct ordinary/group/full release/reset paths;
- representative typed post-load families;
- bounded loader-node claim/release model;
- runtime vs CRT vs process-lifetime distinction.

## Bounded secondary reverse breadth

After B-L1-00, activate only when required by the claimed scope:

- FileSlot/ReadRequest partial-read/cancellation breadth;
- complete ZIP lazy-realization error/lifetime breadth;
- complete compressed-seek/reset error breadth;
- `.lst` temporary allocation/free/malformed failure breadth;
- exhaustive malformed original error equivalence.

## Evidence-gated freezes

- Binary AFS is not inferred from `.afs/` namespace strings.
- Historical PACK parsing does not establish original DMC3 PACK runtime authority.
- Capcom offline writer equivalence is not required for product authoring acceptance.
- Stage Ops/ModViz do not count as L1/L2/L3 completion.

## Environment blocker

The connected environment does not expose every exact protected-install artifact/process required for real retail and original-process gates. This must not be replaced by synthetic CI or weaker completion criteria.
