# Current Project Status

**Snapshot date:** 2026-08-26  
**Canonical implementation base:** `main@c147facb310d32ef084c56ba82d1e4b6b9b1b496`  
**Latest L2 promotion:** PR #215 — type-0 physical-provider reverse/model/controlled receipts  
**Active L2 evidence slice:** PR #219 — protected-runtime RVA mapping acquisition  
**Primary execution program:** GDSpaces Layer 1 final acceptance + evidence-driven supporting reverse  
**Overall status:** L1 INTERNAL PRODUCT PATH CLOSED; L2 PHYSICAL-PROVIDER INTERNAL SLICE CLOSED; subsystem remains NOT COMPLETE pending real-retail/original-process receipts.

## Authority split

- GitHub `main` is canonical implementation truth.
- Reverse claims remain bounded to their recorded artifact/range/scope.
- Synthetic/public CI proves product/tool behavior only.
- Original-game equivalence requires original-process evidence.
- GDSpaces owns product resource identity/materialization/authoring; recovered original functions do not move into GDSpaces.
- Canonical analysis executable authority and protected original-execution authority are separate and must not be silently substituted.

## GDSpaces layer model

- **L1 — Resource Materialization:** selected-byte transport, exact span/member acquisition, transform/decompression, materialized bytes, loose/packed representation materialization, nested expansion, bounded authoring, rebuild/repack, reopen/rematerialization and the resource-level materialized-byte handoff through `state 1 -> 2`.
- **L2 — Resource Resolution:** request, candidates, normalization, provider/volume/source identity, fallback/ambiguity.
- **L3 — Original Runtime/Lifecycle:** typed post-load from state2, optional ready callback, `state 2 -> 3`, consumer visibility, claims/cache, cancellation/reset/release/shutdown.
- Validation is cross-cutting.

`FileSlot`/AsyncIO is not wholesale L3: the byte-transport portion belongs to L1; wider pool ownership/lifecycle policy remains L3. Cross-layer functions are classified by behavior.

Execution follows the dependency-driven [master roadmap](../gdspaces/master-roadmap.md). The canonical EXE cut is [L1 EXE Boundary Review](../gdspaces/l1-exe-boundary-review-2026-08-26.md).

## L1 current state

Canonical L1 implementation now includes:

- NBZ classic ZIP bounded indexing/materialization;
- STORE + raw-DEFLATE method 8;
- CRC/size/SHA/ByteProvenance;
- artifact-bound archive/member observations;
- recovered numbered-volume first-gap/runtime-domain behavior;
- resolver-selected direct-retail member acquisition with provenance receipt;
- shared staged atomic/no-replace publication;
- PAC/PNST sparse/empty/alias-preserving parse + expansion;
- size-changing relative-slot reflow;
- root-to-leaf nested PAC/PNST slot-path authoring;
- byte-exact untouched sibling preservation;
- immutable verified NBZ copy rebuild;
- deterministic next-contiguous NBZ overlay authoring;
- staged canonical NBZ reopen;
- higher-volume resolver verification;
- protected distribution executable preflight;
- product closure orchestration through exact authored rematerialization;
- guarded canonical-analysis EXE window acquisition packet for bounded supporting reverse.

Canonical L1 reviews:

- [Final Pre-Level-E Audit](../gdspaces/l1-final-audit-2026-08-25.md)
- [Canonical EXE Boundary Review](../gdspaces/l1-exe-boundary-review-2026-08-26.md)

## L1 mandatory remaining work

No known mandatory **internal implementation** blocker remains for the current representative DMC3-HD acceptance scope.

The remaining completion gates require a real protected installation:

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

Issue #209 is the final original-game Level-E gate.

`obj\em000.pac` remains a high-value target, but the archive/member winner must be observed by the resolver and another representative resource may be used if it provides a stronger deterministic consumer effect.

## L1 EXE boundary — reconciled 2026-08-26

Strong distinctions now canonical:

- `0x1400335A0(ticketId,userContext,errorFlag,bytesRead)` is a transport/whole-file completion callback;
- `0x1401B8DC0` is the higher resource scheduler/materialization completion handoff registered through `0x1402EF580`; normal record context publishes `state 1 -> 2`;
- `0x1401B8DC0` is **not** a raw I/O callback;
- `0x1402EF4D0` is a **resource materialization submission/scheduling wrapper**, not proven exact-path resolver/final backend open/raw reader;
- `.lst` text is synchronously loaded into aligned temporary storage, but is not proven to use the synchronous-style `0x1402EF920` wrapper;
- `0x1401B84E0` is cross-layer: L1 allocation/materialization start plus scheduler/state boundary behavior.

Current highest-value supporting L1 reverse order while Level-E is externally blocked:

```text
materialization fan-in/completion
 -> transport failure -> resource scheduler/materialization failure mapping
 -> .lst temporary allocation/free/failure cleanup
 -> acceptance-activated FileSlot partial-read/cancellation breadth
 -> acceptance-activated ZIP exact-body/error breadth
```

## Bounded open reverse breadth — not automatic L1 blockers

The following remain real research gaps but only block L1 if the chosen acceptance path depends on them:

- materialization fan-in/completion and one-child-failure aggregation before state2 publication;
- transport-error to resource-scheduler/materialization error mapping;
- `.lst` temporary allocation/free/failure cleanup and malformed/recursion breadth;
- FileSlot/ReadRequest partial-read/error/cancellation breadth where required;
- complete `0x140328540` ZIP stream initializer exact-body/error breadth;
- complete `0x140328FE0` compressed seek/reset exact-body/error breadth;
- exhaustive malformed original error equivalence;
- unsupported/evidence-absent binary backends or formats.

The exact type-0 physical-provider post-`0x0C` final-open contract is no longer in this list: #215 recovered and integrated that bounded L2 slice and added controlled product/parity receipts.

Binary AFS and original-runtime PACK remain frozen absent direct evidence. Capcom offline writer equivalence is not an L1 requirement.

## L2 current frontier

The type-0 physical-provider static reverse and controlled product model are closed on `main` through #215/#204. Current L2 closure is split into independent evidence gates:

1. **real-retail `0x0E` collision census** — externally blocked until an exact DMC3 retail central-directory/member-list surface is available;
2. **protected-distribution runtime RVA mapping** — PR #219 implements explicit-PID, RVA-based, SHA/size-gated live acquisition plus bounded multi-anchor receipt validation; real original-process packet still required;
3. **original-process selected-provider identity** — blocked by a valid runtime mapping packet;
4. **final L2 audit** — only after retail corpus + original-process receipts and exact-head validation agree.

Authority identities:

- canonical analysis executable: `e454272ed0fb0247fcbcf300e5d55d7a3e96d50b89b9ffaff81bb978dcbdd082`, 6,356,432 bytes;
- protected distribution/original execution candidate: `81c7e61983564113b5105e931d9f185accc14e44ae147d27f720c2d50935c7d6`, 6,567,320 bytes.

The protected build is not instruction-reverse authority. Canonical analysis VAs/RVAs cannot be promoted into the protected process without independent mapping evidence.

L2 work may support L1 but must not replace the final L1 acceptance run.

## L3 current frontier

The static LoadedResource/typed-ready/lifetime spine is strong. L3 begins from materialized state2 for the canonical layer cut, while FileSlot transport used to produce those bytes remains an L1 transport dependency.

Exact writer ownership/ordering and broader dynamic lifecycle receipts remain open. For the first vertical proof, L3 only needs enough original-process observation to prove that the authored L1 bytes reached a deterministic consumer. Broader L3 completion is separate.

## Current critical path

### L1 vertical acceptance

1. obtain access to a protected DMC3 installation;
2. run direct-retail acquisition and preserve provenance;
3. classify the exact retail representation;
4. perform one supported bounded real edit, top-level or nested;
5. run next-volume authoring + canonical rematerialization closure;
6. execute issue #209 original-game consumption + rollback;
7. run final L1 acceptance audit;
8. mark `L1 = 100% / COMPLETE` only if every mandatory receipt is valid.

### Supporting static L1 reverse while external acceptance is unavailable

1. reacquire the materialization submission/scheduler neighborhood through the guarded EXE packet;
2. close fan-in/completion semantics to the extent supported by exact evidence;
3. map raw transport failure into resource-level scheduling/completion behavior;
4. close `.lst` temporary allocation/free/failure cleanup where evidence permits;
5. do not reopen already strong ZIP/FileSlot architecture without contradictory evidence.

### L2 closure support

1. obtain cryptographically bound retail DMC3 member-list/central-directory evidence and run the `0x0E` collision census;
2. run PR #219 tooling against the exact protected process and produce a multi-anchor bounded mapping packet;
3. use only proven mapped anchors to capture original-process resolver selection identity;
4. reconcile code/docs/evidence and run final L2 audit.

No synthetic-only feature should displace the real evidence sequence unless a real run reveals a concrete missing dependency.

## Environment boundary

The currently connected automation environment does not expose all exact raw protected-install artifacts required for the real-retail/original-process runs. Synthetic CI must not substitute for those receipts.

Canonical-analysis EXE static reacquisition is now supported by the guarded packet tooling on `main`; use that path rather than ad hoc unbound byte windows.

## Navigation

- [Canonical L1 roadmap](../gdspaces/l1-roadmap.md)
- [Final pre-Level-E L1 audit](../gdspaces/l1-final-audit-2026-08-25.md)
- [Canonical L1 EXE boundary review](../gdspaces/l1-exe-boundary-review-2026-08-26.md)
- [Three-layer master roadmap](../gdspaces/master-roadmap.md)
- [Blockers](blockers.md)
- [Machine-readable status](canonical-status.json)
- [GDSpaces contract](../gdspaces-contract.md)

No percentage or implementation milestone overrides the gate-based completion rule.
