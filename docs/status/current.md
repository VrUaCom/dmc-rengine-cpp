# Current Project Status

**Snapshot date:** 2026-08-26  
**Canonical implementation base:** `main@f886f27e62ec9a05b6829df7fd074981a06a4b49`  
**Latest materialization-completion documentation promotion:** PR #242  
**Primary execution program:** GDSpaces Layer 1 final acceptance + evidence-driven L2/L3 support  
**Overall status:** L1 INTERNAL PRODUCT PATH CLOSED; L2/L3 STATIC AUTHORITY ADVANCED; subsystem remains NOT COMPLETE pending real-retail/protected-process receipts.

## Authority split

- GitHub `main` is canonical implementation truth.
- Reverse claims remain bounded to their recorded artifact/range/scope.
- Synthetic/public CI proves product/tool behavior only.
- Original-game equivalence requires original-process evidence.
- GDSpaces owns product resource identity/materialization/authoring; recovered original functions remain in the Recovered Game Source Tree/evidence layer.
- Canonical analysis executable authority and protected original-execution authority are separate and must not be silently substituted.

## GDSpaces layer model

- **L1 — Resource Materialization:** exact selected-byte acquisition, transform/decompression, materialized bytes, nested expansion, bounded authoring, rebuild/repack, reopen/rematerialization.
- **L2 — Resource Resolution:** request, candidates, normalization, provider/volume/source identity, fallback/ambiguity.
- **L3 — Original Runtime/Lifecycle:** FileSlot/AsyncIO request ownership, scheduler/callback lifecycle, LoadedResource states, typed post-load, ready visibility, claims/cache, cancellation/reset/release/shutdown.
- Validation is cross-cutting.

`FileSlot` is not classified wholesale: exact byte-read mechanics may support L1, while original request ownership/scheduling/callback lifetime is L3. `0x1401B8CA0` is the explicit L1/L3 materialization-success seam.

Execution follows the dependency-driven [master roadmap](../gdspaces/master-roadmap.md). The current completion-ordering authority is [Materialization Completion Boundary Pass](../gdspaces/materialization-completion-boundary-pass-2026-08-26.md) plus [Pass 2](../gdspaces/materialization-completion-dependency-pass2-2026-08-26.md).

## L1 current state

Canonical L1 implementation includes:

- NBZ classic ZIP bounded indexing/materialization;
- STORE + raw-DEFLATE method 8;
- CRC/size/SHA/ByteProvenance;
- artifact-bound archive/member observations;
- numbered-volume first-gap/runtime-domain behavior;
- resolver-selected direct-retail member acquisition with provenance receipt;
- shared staged atomic/no-replace publication;
- PAC/PNST sparse/empty/alias-preserving parse + expansion;
- size-changing relative-slot reflow;
- root-to-leaf nested PAC/PNST slot-path authoring;
- byte-exact untouched sibling preservation;
- immutable verified NBZ copy rebuild;
- deterministic next-contiguous NBZ overlay authoring;
- staged canonical NBZ reopen and higher-volume resolver verification;
- protected distribution executable preflight;
- product closure orchestration through exact authored rematerialization.

Canonical L1 review: [Final Pre-Level-E Audit](../gdspaces/l1-final-audit-2026-08-25.md).

## L1 mandatory remaining work

No known mandatory **internal implementation** blocker remains for the current representative DMC3-HD L1 acceptance scope.

The remaining completion lineage is evidence execution:

```text
real retail request
 -> exact resolver winner + acquisition receipt
 -> retail representation classification
 -> one supported real edit/rebuild
 -> next-volume overlay + canonical rematerialization receipt
 -> original DMC3 consumer-visible effect
 -> rollback / original retail immutability
 -> final audit
```

Issue #209 remains the final original-game Level-E gate. A crash-free launch alone is insufficient.

Connected Drive evidence can locate the protected distribution `dmc3.exe` and co-located `data/dmc3/dmc3-0.nbz`; the observed NBZ is 960,358,951 bytes. The current connected raw-transfer/materialization path cannot ingest that archive because of the observed 268,435,456-byte ceiling. This is a measured external transport boundary, not evidence that the artifact is absent.

## Materialization completion EXE boundary — reconciled through #242

Merged #228/#230/#242 narrow the remaining cross-layer static seam.

Strong distinctions:

- `0x1400335A0(ticketId,userContext,errorFlag,bytesRead)` is lower whole-file/FileSlot transport completion/status handling;
- normal `0x1401B8DC0` is the higher LoadedResource completion callback that publishes state2;
- normal `0x1401B8DC0` receives exactly one u32 registry-relative context and **does not** receive transport status/error, bytesRead, FileSlot handle or child/outstanding-work metadata;
- `0x1402EF4D0` remains the resource materialization submission/scheduling wrapper; its exact queued job identity and terminal dependency behavior are open;
- `0x1402EF460` remains pending scheduler-entry clear/rollback, not proven OS AsyncIO cancellation.

### Critical correction

Do **not** describe the open seam as a proven generic `fan-in counter`.

The safe label is:

> **materialization completion ordering / dependency bridge**

Success/error eligibility must be resolved before normal `0x1401B8DC0` dispatch, or that queued completion must be suppressed/removed before execution.

FIFO insertion order alone is insufficient if an earlier materialization job can submit asynchronous transport and retire before the transport becomes terminal.

### Current focused raw order

```text
0x1402EF4D0 queued job identity/type + inherited load-context consumer
 -> relevant 0x1402EF790 materialization-job persistence/re-poll/retirement
 -> historical 0x1400333E0 status/poll anchor
 -> historical 0x140033390 terminal release/close anchor
 -> 0x1400335A0 transport status/error writes
 -> determine what blocks/suppresses normal 0x1401B8DC0 on failed/incomplete transport
 -> 0x1402EF460 queued higher-work clear/rollback
 -> .lst child/recursive failure ordering after the direct-resource mechanism is closed
```

`0x1400333E0` and `0x140033390` remain **reacquisition hypotheses** until fresh canonical `e454...` bytes confirm their exact roles.

This supporting reverse does not change L1/L2/L3 completion criteria.

## Bounded open reverse breadth — not automatic L1 blockers

The following remain real research gaps but only block L1 if the chosen acceptance path depends on them:

- materialization terminal dependency / transport-error-to-completion-suppression mapping;
- `.lst` child/recursive failure and temporary cleanup behavior when real loose-list acceptance is activated;
- complete `0x140328540` ZIP stream initializer lifetime/error breadth when required;
- complete `0x140328FE0` compressed seek/reset/reinflate error breadth when required;
- exhaustive malformed/partial-read original error equivalence;
- unsupported/evidence-absent binary backends or formats.

Binary AFS and original-runtime PACK remain frozen absent direct evidence. Capcom offline writer equivalence is not an L1 requirement.

## L2 current frontier

Closed/integrated internal L2 slices include the type-0 static provider chain, protected-runtime RVA acquisition/mapping tooling and selected-identity content-candidate/binder infrastructure. Real L2 closure still requires:

1. exact retail `0x0E` collision evidence;
2. a real protected-process bounded mapping receipt;
3. trusted original-process selected-provider identity;
4. final L2 audit after exact-head validation and evidence reconciliation.

Authority identities:

- canonical analysis executable: `e454272ed0fb0247fcbcf300e5d55d7a3e96d50b89b9ffaff81bb978dcbdd082`, 6,356,432 bytes;
- protected distribution/original execution candidate: `81c7e61983564113b5105e931d9f185accc14e44ae147d27f720c2d50935c7d6`, 6,567,320 bytes.

The protected build is not instruction-reverse authority. Canonical analysis VAs/RVAs cannot be promoted into the protected process without independent mapping evidence.

## L3 current frontier

The central static LoadedResource spine, scheduler callback ABI, cancellation/quiescence/release distinctions and representative typed post-load families are strong.

The completion-ordering seam above is L1-supporting evidence but lifecycle ownership remains L3. Current remaining L3 work is residual static writer/ownership breadth plus original-process lifecycle receipts. For the first L1 vertical proof, L3 only needs enough original-process observation to prove that the authored bytes reached a deterministic consumer.

## Current critical path

### L1 vertical acceptance

1. export/acquire one real retail selected member with exact provenance;
2. classify its exact retail representation;
3. perform one supported bounded real edit, top-level or nested;
4. run next-volume authoring + canonical rematerialization closure;
5. execute #209 original-game consumption + rollback;
6. run final L1 acceptance audit;
7. mark `L1 = 100% / COMPLETE` only if every mandatory receipt is valid.

### Supporting static EXE reverse while Level-E is externally blocked

1. close `0x1402EF4D0` queued materialization-job identity/type;
2. close the corresponding `0x1402EF790` persistence/re-poll/retirement case;
3. reacquire the `0x1400333E0/0x140033390` terminal cluster;
4. bind `0x1400335A0` transport status/error writes into that terminal model;
5. close queued-completion suppression/rollback through `0x1402EF460`;
6. only then generalize to `.lst` child/recursive failure ordering.

No synthetic-only feature should displace the real evidence sequence unless a real run reveals a concrete missing dependency.

## Navigation

- [Canonical L1 roadmap](../gdspaces/l1-roadmap.md)
- [Final pre-Level-E L1 audit](../gdspaces/l1-final-audit-2026-08-25.md)
- [Materialization completion boundary](../gdspaces/materialization-completion-boundary-pass-2026-08-26.md)
- [Materialization completion Pass 2](../gdspaces/materialization-completion-dependency-pass2-2026-08-26.md)
- [Three-layer master roadmap](../gdspaces/master-roadmap.md)
- [Blockers](blockers.md)
- [Machine-readable status](canonical-status.json)
- [GDSpaces contract](../gdspaces-contract.md)

No percentage or implementation milestone overrides the gate-based completion rule.
