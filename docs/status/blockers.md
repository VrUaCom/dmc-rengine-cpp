# Current Blockers

**Snapshot date:** 2026-08-26  
**Canonical base:** `main@f886f27e62ec9a05b6829df7fd074981a06a4b49`  
**Latest materialization-completion documentation promotion:** PR #242

The canonical Layer-1 execution order is [GDSpaces L1 Roadmap](../gdspaces/l1-roadmap.md). The cross-layer dependency order is [GDSpaces Master Roadmap](../gdspaces/master-roadmap.md).

## P0 — GDSpaces L1 completion blockers

There is no known mandatory internal product-code blocker for the current representative DMC3-HD L1 acceptance scope.

The remaining P0 gates are evidence executions.

### B-L1-01 — Direct-retail representative provenance

**Status:** EXTERNAL REAL-RETAIL RECEIPT REQUIRED

Run the canonical direct-retail acquisition path against a protected DMC3 installation and preserve:

- protected executable authority;
- observed numbered-volume topology;
- resolver-selected volume/archive/member identity;
- archive SHA/size;
- central-entry metadata;
- materialized SHA/size;
- compression transform and ByteProvenance.

`obj\em000.pac` is a high-value request, not a predeclared archive member. Another representative request is acceptable if it gives a stronger deterministic authoring/consumer receipt.

Connected Drive evidence can locate the protected distribution executable and co-located `data/dmc3/dmc3-0.nbz`. The observed archive is 960,358,951 bytes. The current connected raw-transfer/materialization ceiling is 268,435,456 bytes, so the whole archive cannot be ingested through that path. This is a measured transport blocker, not artifact absence.

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

## Supporting EXE reverse gaps — bounded, not automatic completion blockers

### B-L1-R1 — Materialization completion ordering / dependency bridge

**Status:** STATIC REVERSE BREADTH OPEN

Merged #228/#230/#242 correct the older generic `fan-in/completion` shorthand. No universal original child/outstanding-work counter is currently evidenced.

Known distinction:

- `0x1400335A0(ticketId,userContext,errorFlag,bytesRead)` = lower transport/whole-file completion/status handling;
- normal `0x1401B8DC0` = LoadedResource completion callback that publishes state2;
- normal `0x1401B8DC0` receives exactly one u32 registry-relative record context and no raw transport status/error, bytesRead, FileSlot handle or child/outstanding-work metadata.

Therefore success/error eligibility must already be resolved before normal B8DC0 dispatch, or that queued completion must be suppressed/removed.

FIFO insertion order alone is insufficient if an earlier materialization job can submit asynchronous I/O and retire before lower transport becomes terminal.

Current exact-byte targets:

```text
0x1402EF4D0 queued materialization job identity/type + load-context consumer
 -> relevant 0x1402EF790 persistence/re-poll/terminal retirement case
 -> historical 0x1400333E0 status/poll anchor
 -> historical 0x140033390 terminal release/close anchor
 -> 0x1400335A0 transport status/error writes
 -> determine what blocks/suppresses normal 0x1401B8DC0 on failed/incomplete transport
 -> 0x1402EF460 pending higher scheduler clear/rollback
 -> .lst child/recursive failure ordering
```

`0x1400333E0` and `0x140033390` remain **reacquisition hypotheses** until fresh canonical bytes confirm their exact roles.

### B-L1-R2 — Transport error -> higher completion suppression

**Status:** STATIC REVERSE BREADTH OPEN

Raw whole-file transfer error/status behavior is substantially recovered, but the exact bridge from transport failure to materialization-job terminal failure and/or queued B8DC0 suppression remains open.

### B-L1-R3 — `.lst` child/recursive failure + temporary cleanup

**Status:** STATIC REVERSE BREADTH OPEN / ACCEPTANCE-ACTIVATED

`.lst` grammar/layout and in-place recursive synthesis are already strong. Open breadth is child submission return handling, recursive failure propagation, temporary allocation/free/failure cleanup and interaction with the direct-resource terminal dependency model.

Do not reopen grammar/layout before the direct-resource mechanism is closed.

### B-L1-R4 — FileSlot / ReadRequest exact error breadth

**Status:** BOUNDED

The central FileSlot/ReadRequest architecture and callback ABI are strong. Partial-read/error/already-running cancellation breadth should be recovered only where a compatibility or acceptance claim requires it.

### B-L1-R5 — ZIP initializer / compressed-seek exact-body breadth

**Status:** BOUNDED / LOWER PRIORITY

`0x140328540` lazy realization and `0x140328FE0` reset+reinflate/discard seek architecture are already strong. Complete exact error/lifetime branches are not the first reverse target unless a claimed boundary requires them.

## Layer 2 evidence blockers

These are L2 closure gates. They are not substitutes for the L1 Level-E acceptance sequence.

### B-L2-01 — Real-retail `0x0E` collision census

**Status:** EXTERNAL ARTIFACT ACCESS REQUIRED

The exact `dmc3-0.nbz` is approximately 960 MB and cannot currently be transferred through the connected whole-file path. No exact cryptographically bound central-directory/member-list derivative is available in the connected corpus.

Required evidence is an exact member-name/central-directory surface bound to the retail archive, followed by the canonical `0x0E` normalized-key collision census.

### B-L2-02 — Real protected-distribution runtime RVA mapping receipt

**Status:** TOOLING INTEGRATED / REAL ORIGINAL-PROCESS RECEIPT REQUIRED

Authority split:

- canonical analysis executable: SHA-256 `e454272ed0fb0247fcbcf300e5d55d7a3e96d50b89b9ffaff81bb978dcbdd082`, size 6,356,432;
- protected distribution execution candidate: SHA-256 `81c7e61983564113b5105e931d9f185accc14e44ae147d27f720c2d50935c7d6`, size 6,567,320.

Canonical analysis VAs/RVAs must not be applied to the protected process without independent mapping evidence.

Current tooling can acquire bounded live module windows and validate multi-anchor mapping packets. The blocker is a real protected-process packet from one process/module session.

Synthetic/self-process CI proves tooling behavior only.

### B-L2-03 — Trusted original-process selected-provider identity

**Status:** TOOLING AVAILABLE / BLOCKED BY REAL B-L2-02 + TRUSTED PUBLISHER

A real R3 promotion still requires:

1. a valid real B-L2-02 mapping packet;
2. runtime observation attached to that exact protected process without altering resolver selection semantics;
3. a zero-loss trace;
4. exact observer artifact binding;
5. exact mounted numbered-NBZ artifact binding;
6. trusted origin/capture binding that is not asserted by editable JSON fields.

Provider/backend failure must remain distinct from a clean lookup miss.

### B-L2-04 — Direct-retail resolver identity receipt

**Status:** BLOCKED BY B-L2-01

A real-retail `ResourceRef`/member winner cannot be promoted until the exact retail member surface is bound and the `0x0E` collision state is known.

### B-L2-05 — Final L2 audit

**Status:** OPEN / DEPENDS ON B-L2-01..04

Layer 2 remains incomplete until real-retail collision evidence, protected-runtime mapping, trusted original-process selected identity, exact-head CI and canonical code/docs agree.

## Closed former blockers/tooling gates

Do not reopen absent contradictory direct evidence:

- atomic/no-replace publication — #194;
- artifact-bound archive/member stability — #195;
- direct-retail acquisition implementation — #196;
- raw-DEFLATE artifact-bound regression — #197;
- first-gap retail-read behavior — #198;
- verified immutable NBZ copy rebuild — #199;
- PAC/PNST user-facing size-changing rebuild — #201;
- protected retail product closure orchestration — #208;
- nested PAC/PNST root-to-leaf slot-path authoring — #213;
- type-0 physical-provider post-`0x0C` static reverse/product model — #215/#204;
- protected-runtime acquisition/mapping tooling — #219 and later integrated follow-up tooling;
- merged #230 one-u32 normal B8DC0 callback/context ABI;
- merged #242 materialization completion dependency documentation/packet synchronization.

## Explicit superseded shorthand

Do not reintroduce:

- `fan-in/completion` as proof of a generic original child counter;
- `0x1401B8DC0 == raw I/O callback`;
- FIFO queue order as sufficient completion proof without job persistence/terminal-gate evidence;
- historical `0x1400333E0/0x140033390` helper labels as fresh canonical semantics before reacquisition;
- `0x1402EF460 == OS CancelIo/AsyncIO cancellation`;
- `FileSlot/AsyncIO == wholly L1` or wholly L3 without behavior-level classification.

## Evidence-gated freezes / non-blockers

- Binary AFS is not inferred from `.afs/` logical namespace strings.
- Historical PACK parsing does not establish original DMC3 PACK runtime authority.
- Capcom offline writer equivalence is not required for DMC Rengine L1 product authoring acceptance.
- Stage Ops, ModViz and unrelated HITS semantics do not count as L1 closure.

## Environment blocker

The connected evidence surface can locate the protected distribution executable and retail `dmc3-0.nbz`, but the 960,358,951-byte archive exceeds the observed 268,435,456-byte connected raw-transfer/materialization ceiling.

A range/member export path, local operator extraction or another exact artifact surface is required for the real L1/L2 retail receipts. This external limitation must not be hidden by synthetic CI or converted into a weaker completion criterion.
