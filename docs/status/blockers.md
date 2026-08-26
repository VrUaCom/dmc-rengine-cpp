# Current Blockers

**Snapshot date:** 2026-08-27  
**Canonical main authority:** through merged PR #242  
**Pending branch truth:** #226, #238, #240, #241

The canonical Layer-1 execution order is [GDSpaces L1 Roadmap](../gdspaces/l1-roadmap.md). The cross-layer dependency order is [GDSpaces Master Roadmap](../gdspaces/master-roadmap.md). The current materialization-success boundary is [Materialization Completion Dependency Pass 2](../gdspaces/materialization-completion-dependency-pass2-2026-08-26.md).

## Cross-layer correction — do not invent a fan-in counter

Merged #230/#242 establish that normal `0x1401B8DC0` receives only one u32 registry-relative record context. It receives no transport status, error flag, byte count, FileSlot/ReadRequest handle or child/outstanding-work metadata.

Therefore lower materialization success/error must be resolved **before** normal `0x1401B8DC0` state2 dispatch, or the queued completion must be suppressed/removed. FIFO insertion order alone is not proven sufficient if the preceding scheduler job can submit asynchronous transport and retire immediately.

This is a **materialization completion ordering / dependency bridge** problem. No generic original outstanding-child counter is evidenced.

Focused exact-byte targets:

1. `0x1402EF4D0` queued materialization job identity/type/callees/context consumer;
2. relevant `0x1402EF790` materialization case, persistence/re-poll/terminal retirement;
3. reacquire historical `0x1400333E0` status/poll hypothesis;
4. reacquire historical `0x140033390` terminal cleanup/release hypothesis;
5. `0x1400335A0` transport success/error terminal writes;
6. identify prevention/suppression of normal `0x1401B8DC0` on incomplete/failed transport;
7. `0x1402EF460` higher-scheduler clear/rollback and queued-completion suppression;
8. only then `.lst` child/recursive failure ordering.

Layer ownership remains unchanged: byte-read mechanics may support L1; FileSlot/AsyncIO request ownership/scheduling/callback lifetime/cancellation belongs to L3; `0x1401B8CA0` is the L1/L3 materialization-success seam.

## P0 — GDSpaces L1 completion blockers

There is no known mandatory internal product-code blocker for the current representative DMC3-HD L1 acceptance scope. Remaining P0 gates are **same-lineage real evidence executions**.

### B-L1-01 — Selected-source + representative member provenance

**Status:** REAL RECEIPT REQUIRED / MEMBER-MATERIALIZATION SUBPATH AVAILABLE OUT-OF-BAND

Canonical protected-install route:

```text
dmc-rengine extract-dmc3-retail-member <exe-dir> <game-request> <output-file>
```

Preserve protected executable authority, actual selected successful source topology, resolver-selected provider/archive/member identity, archive SHA/size, central-entry metadata, materialized SHA/size, compression transform and ByteProvenance.

Merged #233 proves the protected artifacts are locatable. The retail NBZ is observed at `960,358,951` bytes; the connected raw materialization ceiling is `268,435,456` bytes. This is a transport blocker, not artifact absence.

Pocket GDS PR #2 can emit `gdspaces.l1.member-acquisition-receipt.v1` when the actual NBZ is local on-device. Pending #238 reconciles that receipt into this evidence model. A Pocket receipt closes only the exact local snapshot/member-materialization sub-gate and must still be bound to protected selected-source authority before final promotion.

Merged #235 requires the distinction between numbered filename discovery and actual successful mounting. Filename presence is not mount-success evidence. Issue #237 / pending PR #241 owns the product correction.

`obj\em000.pac` is a high-value target, not a mandatory predeclared runtime winner.

### B-L1-02 — Exact retail representation classification

**Status:** REAL RECEIPT REQUIRED

Classify the exact materialized bytes from B-L1-01. Do not infer writer authority from filename or transformed/runtime representations. If the representation is outside current supported authoring domains, stop and create a bounded evidence gate.

### B-L1-03 — Representative real edit/rebuild/rematerialization receipt

**Status:** REAL VALIDATION REQUIRED

Required receipt:

```text
accepted selected/member identity
 -> exact materialized representation
 -> supported bounded edit
 -> top-level or nested bottom-up rebuild
 -> byte-exact untouched sibling validation
 -> next-contiguous NBZ
 -> canonical resolver selected actual successful higher source
 -> exact rebuilt-member rematerialization
 -> exact authored-child verification
```

A mobile acquisition PASS alone does not close this blocker.

### B-L1-04 — Original DMC3 Level-E consumption + rollback

**Status:** FINAL EXTERNAL ACCEPTANCE REQUIRED

Canonical tracking: #209.

The exact generated overlay must be copied into the protected installation under controlled conditions, SHA verified, consumed through a deterministic original-game path, then removed without changing original retail artifacts.

Crash-free launch or Pocket open/export alone is insufficient.

### B-L1-05 — Final L1 cross-stack/V audit

**Status:** OPEN / DEPENDS ON B-L1-01..04

Before `L1 COMPLETE / 100%`:

- exact executable authority;
- selected-provider/archive/member provenance;
- actual successful-mounted topology assumptions evidenced correctly;
- exact materialized member identity;
- real representation classification;
- real edit/rebuild/rematerialization receipt;
- original-game consumer observation;
- rollback proving retail immutability;
- same reconciled lineage across all receipts;
- exact-head Windows + Ubuntu validation;
- #100, #182, #209, code and current docs agree;
- no unresolved contradiction alters the supported scope;
- final V verdict.

## Layer 2 blockers

### B-L2-01 — Real-retail `0x0E` collision census

**Status:** EXACT ARCHIVE-BOUND MEMBER SURFACE REQUIRED

The recovered archive index comparator has no equal-key secondary tie-break. Required evidence is an exact archive-SHA-bound complete member-name/central-directory surface followed by the canonical `0x0E` normalized-key collision census. A single-member receipt is insufficient.

### B-L2-02 — Real protected-distribution R2B v2 mapping receipt

**Status:** #235 V2 TOOLING MERGED / REAL ORIGINAL-PROCESS RECEIPT REQUIRED

Authority split:

- canonical analysis executable: SHA-256 `e454272ed0fb0247fcbcf300e5d55d7a3e96d50b89b9ffaff81bb978dcbdd082`, size 6,356,432;
- protected distribution candidate: SHA-256 `81c7e61983564113b5105e931d9f185accc14e44ae147d27f720c2d50935c7d6`, size 6,567,320.

A promotable real R2B v2 packet must bind one process instance using exact PID, non-zero OS process creation FILETIME, module identity, the independently read canonical EXE and all seven mandatory `0x40` anchors: `0x2FCA0`, `0x326D20`, `0x326DA0`, `0x327430`, `0x327800`, `0x328160`, `0x328290`.

Legacy v1 is not promotion authority. The blocker is real execution.

### B-L2-03 — Trusted original-process selected-provider identity

**Status:** #221 TOOLING MERGED / REAL R2B V2 + TRUSTED PUBLISHER REQUIRED

A real R3 promotion requires valid B-L2-02, a trusted publisher/observer attached to that exact process instance, zero-loss trace, exact observer artifact binding, exact actually-successful mounted archive SHA/size binding and exact selected provider/archive/member identity.

Provider/backend failure must fail closed. Archive lookup hit + wrapper/open failure at `0x140328290` is not a lower-volume clean miss.

### B-L2-04 — Successful-mount topology reconciliation

**Status:** ISSUE #237 OPEN / PR #241 PENDING

`VolumeBootstrapPlan` discovery/attempt order and `RuntimeMountTopology` actual successful mounts must remain separate. The successful set may be sparse. Successful archive registrations prepend, preserving higher successful volume -> lower successful volume -> physical precedence.

Until #241 merges, that implementation is branch truth. Final evidence must still observe the original successful mounted set; product topology cannot self-prove original runtime outcome.

### B-L2-05 — Final L2 audit

**Status:** OPEN

Layer 2 remains incomplete until real-retail collision evidence, protected R2B v2, trusted R3, successful-mount topology reconciliation, exact-head CI and canonical code/docs agree.

## Layer 3 blockers relevant to cross-stack acceptance

### B-L3-01 — Materialization terminal-condition dependency bridge

**Status:** STATIC MECHANISM NOT YET EXACT-BYTE CLOSED

Normal `0x1401B8DC0` cannot decide transport success from its callback ABI. Recover the job/status/retirement/suppression mechanism listed above. Do not replace it with a guessed counter or FIFO-only story.

### B-L3-02 — R1 writer census promotion

**Status:** PR #240 PENDING

#240 proposes the exact canonical `LoadedResource +0x04` writer census as `STATIC BOUNDED-CLOSED / APPROVED / CONTRADICTION-GATED`. Until merge it is branch truth. If promoted, broad R1 discovery should stop absent concrete contradictory provenance; R2 field/backing ownership becomes next.

### B-L3-03 — Original-process lifecycle/consumer evidence

**Status:** OPEN

R2-R5 and V1-V7 remain open. L1 requires only bounded L3 evidence sufficient to prove exact authored bytes reached the intended original consumer; this does not make L3 complete.

## Closed former L1 blockers

Do not reopen absent contradictory direct evidence:

- atomic/no-replace publication — #194;
- artifact-bound archive/member stability — #195;
- direct-retail acquisition implementation — #196;
- raw-DEFLATE artifact-bound regression — #197;
- clean first-gap retail-read behavior — #198 at its declared clean scope;
- verified immutable NBZ copy rebuild — #199;
- PAC/PNST size-changing rebuild — #201;
- protected retail product closure orchestration — #208;
- nested PAC/PNST root-to-leaf authoring — #213;
- NBZ STORE/raw-DEFLATE materialization;
- PAC/PNST sparse/alias-preserving parsing and recursive expansion;
- ByteProvenance and next-volume clean-path composition.

## Evidence-gated freezes / non-blockers

- Binary AFS is not inferred from `.afs/` strings.
- Historical PACK parsing does not establish original DMC3 PACK authority.
- Capcom offline writer equivalence is not required for L1 acceptance.
- Stage Ops/ModViz do not count as L1 closure.
- Connected 960 MB transfer failure is not a parser failure or archive absence.
- Draft #226/#223 RCP/V-LV work does not create L4 or alter completion accounting.

## Environment boundary

Connected Drive evidence can locate protected `dmc3.exe` and `data/dmc3/dmc3-0.nbz`; the full archive cannot enter the connected execution container because `960,358,951 > 268,435,456` bytes.

Pocket GDS is an out-of-band exact-member materialization route where the archive is already local. The protected PC/process remains required for original selected identity, R2B/R3, #209 consumption/rollback and lifecycle evidence.

This external evidence limitation must not be hidden by synthetic CI or converted into a weaker completion criterion.
