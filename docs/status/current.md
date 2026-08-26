# Current Project Status

**Snapshot date:** 2026-08-26  
**Canonical implementation base:** `main@eb701b9c523a3ec87f3c73bb8764038f1f2ef8dc`  
**Latest L2 promotions:** PR #215 — type-0 physical-provider reverse/model/controlled receipts; PR #219 — protected-runtime RVA mapping acquisition tooling  
**Latest L3 raw reconciliation:** `l3-boundary-audit-2026-08-26.md`  
**Primary execution program:** GDSpaces L1 final acceptance + evidence-driven L1/L3 handoff and L2 support  
**Overall status:** L1 INTERNAL PRODUCT PATH CLOSED; L2 PHYSICAL-PROVIDER/RUNTIME-MAPPING TOOLING SLICES CLOSED; L3 RAW STATIC BOUNDARY ADVANCED; subsystem remains NOT COMPLETE pending real-retail/original-process receipts.

## Authority split

- GitHub `main` is canonical implementation truth.
- Reverse claims remain bounded to their recorded artifact/range/scope.
- Synthetic/public CI proves product/tool behavior only.
- Original-game equivalence requires original-process evidence.
- GDSpaces owns product resource identity/materialization/authoring; recovered original functions remain Recovered Game Source Tree authority.
- Canonical analysis executable authority and protected original-execution authority are separate and must not be silently substituted.

## GDSpaces layer model

- **L1 — Resource Materialization:** exact selected bytes, read/seek/decompression mechanics, materialized byte buffers/provenance, nested expansion, bounded authoring, rebuild/repack, reopen/rematerialization.
- **L2 — Resource Resolution:** request, candidates, normalization, provider/volume/source identity, fallback/ambiguity.
- **L3 — Original Runtime/Lifecycle:** FileSlot/async request ownership and scheduling, LoadedResource `0/1/2/3/4` lifecycle, typed-ready, claims/cache, cancellation/reset/release/teardown.
- Validation is cross-cutting.
- Stage Ops / Stage Assembly is downstream domain/tooling, not L3.

FileSlot is a boundary subsystem: byte-read mechanics can support L1; request ownership, scheduling, completion, cancellation and close lifetime are L3.

`0x1401B8CA0` is an explicit semantic seam: materialization mechanics are L1-relevant; successful return gates L3 state1 publication.

Execution follows the [master roadmap](../gdspaces/master-roadmap.md), [L1 EXE boundary review](../gdspaces/l1-exe-boundary-review-2026-08-26.md), [L1/L3 handoff pass](../gdspaces/l1-l3-exe-materialization-lifecycle-handoff-pass-2026-08-26.md), and [L3 raw boundary audit](../gdspaces/l3-boundary-audit-2026-08-26.md).

## L1 current state

Canonical L1 implementation includes:

- NBZ classic ZIP bounded indexing/materialization;
- STORE + raw-DEFLATE method 8;
- CRC/size/SHA/ByteProvenance;
- artifact-bound archive/member observations;
- recovered numbered-volume first-gap/runtime-domain behavior;
- resolver-selected direct-retail member acquisition with provenance receipt;
- shared staged atomic/no-replace publication;
- PAC/PNST sparse/empty/alias-preserving parse + expansion;
- size-changing relative-slot reflow;
- nested PAC/PNST slot-path authoring;
- byte-exact untouched sibling preservation;
- immutable verified NBZ copy rebuild;
- deterministic next-contiguous NBZ overlay authoring;
- staged canonical NBZ reopen;
- higher-volume resolver verification;
- protected distribution executable preflight;
- product closure orchestration through exact authored rematerialization.

No known mandatory **internal product-code** blocker remains for the current representative DMC3-HD L1 acceptance scope.

## L1 mandatory remaining work

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

Issue #209 remains the final original-game vertical acceptance gate.

## L1/L3 EXE handoff — reconciled 2026-08-26

Canonical distinctions:

- `0x1400335A0(ticketId,userContext,errorFlag,bytesRead)` = lower whole-file transfer progress/status callback;
- FileSlot byte-read mechanics can support L1, but FileSlot request ownership/scheduling/completion/cancellation/close lifetime are L3;
- `0x1402EF4D0` = resource materialization submission/scheduling wrapper, not proven exact-path resolver/final backend open/raw reader;
- `0x1401B8CA0` = L1/L3 seam: representation/materialization mechanics + success result gating L3 acquisition;
- `0x1402EF580` = L3 scheduler-ring enqueue;
- `0x1402EF790` = L3 scheduler worker/callback execution;
- `0x1402EF460` = pending scheduled-entry clear/rollback, not OS AsyncIO cancellation;
- `0x1401B8DC0` = L3 normal lifecycle completion writer, `state 1 -> 2`, not raw I/O callback and not the terminal L1 boundary;
- `.lst` synchronous temporary acquisition is not proven to be the `0x1402EF920` synchronous-style wrapper;
- a generic child/outstanding-work **fan-in counter is not evidenced**.

Focused next static packet: `data/reverse/dmc3-materialization-lifecycle-handoff-plan.v1.json`.

Highest-value exact-byte questions:

```text
0x1402EF4D0 exact body/callees + load-context consumer
 -> exact L1 byte-materialization -> L3 request/scheduler handoff
 -> success-side completion ordering/dependency mechanism
 -> scheduler rollback matching + already-running transport interaction
 -> lower transfer failure -> L3 acquisition/cancellation mapping
 -> .lst child failure/completion + temporary-buffer cleanup
```

These are bounded supporting reverse gaps, not automatic L1 completion blockers.

## L2 current frontier

The type-0 physical-provider static reverse/product model are closed through #215/#204. Protected-runtime RVA mapping acquisition/validation tooling is promoted through #219.

Current L2 closure now requires real evidence:

1. real-retail `0x0E` normalized-key collision census;
2. real protected-process multi-anchor RVA mapping receipt using merged #219 tooling;
3. original-process selected-provider identity after valid mapping;
4. final L2 audit.

Authority identities:

- canonical analysis executable: `e454272ed0fb0247fcbcf300e5d55d7a3e96d50b89b9ffaff81bb978dcbdd082`, 6,356,432 bytes;
- protected distribution/original execution candidate: `81c7e61983564113b5105e931d9f185accc14e44ae147d27f720c2d50935c7d6`, 6,567,320 bytes.

Canonical analysis VAs/RVAs cannot be promoted into the protected process without independent mapping evidence.

## L3 current frontier

Canonical raw authority: [L3 boundary audit — 2026-08-26](../gdspaces/l3-boundary-audit-2026-08-26.md).

Strong/bounded static authority now includes:

- 363×`0x48` LoadedResource registry and seven groups;
- acquisition/state `0 -> 1`;
- normal completion/state `1 -> 2`;
- typed post-load / optional callback / state `2 -> 3`;
- cancellation `1|2 -> 4 -> 0`;
- ordinary/group/full release distinctions;
- representative typed families and loader-node claim model;
- runtime vs CRT vs process-lifetime teardown distinction.

Still mandatory for L3: whole-image alias-aware writer/caller census, family-complete field ownership, typed/factory breadth/failure edges, SCM contradiction, owner breadth, cross-build/profile differences and original-process V1–V7 lifecycle receipts.

## Current critical path

### L1 vertical acceptance

1. obtain access to a protected DMC3 installation;
2. run direct-retail acquisition and preserve provenance;
3. classify exact retail representation;
4. perform one supported bounded real edit;
5. run next-volume authoring + canonical rematerialization closure;
6. execute #209 original-game consumption + rollback;
7. run final L1 acceptance audit;
8. mark `L1 = 100% / COMPLETE` only if every mandatory receipt is valid.

### Supporting static handoff reverse

1. reacquire `data/reverse/dmc3-materialization-lifecycle-handoff-plan.v1.json` against exact `e454...`;
2. close the exact `0x1402EF4D0` body/callees/load-context consumer;
3. identify the byte-materialization -> lifecycle ownership handoff;
4. close success-side scheduler completion ordering without presupposing a fan-in counter;
5. close scheduler rollback/transport-failure interaction where evidence permits;
6. close `.lst` child failure + temporary-buffer cleanup if needed by acceptance.

### L2 closure support

1. obtain cryptographically bound retail member-list/central-directory evidence and run the `0x0E` census;
2. use merged #219 tooling to produce a real protected-process multi-anchor mapping packet;
3. capture original-process selected identity only after mapping is proven;
4. reconcile and run final L2 audit.

## Environment boundary

The connected automation environment does not expose all exact raw protected-install artifacts required for real-retail/original-process acceptance. Synthetic CI must not substitute for those receipts.

Guarded canonical-analysis EXE window tooling is available, but a fresh raw canonical `e454...` blob was not exposed through the connected file surface during the first part of the new handoff pass. Therefore the pass records evidence reconciliation plus a focused next packet, not a fresh-disassembly claim.

## Navigation

- [Canonical L1 roadmap](../gdspaces/l1-roadmap.md)
- [Final pre-Level-E L1 audit](../gdspaces/l1-final-audit-2026-08-25.md)
- [L1 EXE boundary review](../gdspaces/l1-exe-boundary-review-2026-08-26.md)
- [L1/L3 materialization-lifecycle handoff pass](../gdspaces/l1-l3-exe-materialization-lifecycle-handoff-pass-2026-08-26.md)
- [L3 raw boundary audit](../gdspaces/l3-boundary-audit-2026-08-26.md)
- [Three-layer master roadmap](../gdspaces/master-roadmap.md)
- [Blockers](blockers.md)
- [Machine-readable status](canonical-status.json)

No percentage or implementation milestone overrides the gate-based completion rule.
