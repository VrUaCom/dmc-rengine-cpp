# Current Project Status

**Snapshot date:** 2026-08-27  
**Reconciled canonical main:** through merged PR #242 (`f886f27e62ec9a05b6829df7fd074981a06a4b49`)  
**Merged access/provenance authority:** #233, #235  
**Merged materialization-completion authority:** #230, #242  
**Pending branch truth:** #226 RCP/grey boundary, #238 Pocket evidence reconciliation, #240 L3 R1 final review, #241 successful-mount topology implementation  
**Primary execution program:** GDSpaces L1 final same-lineage acceptance with evidence-driven L2/L3 support  
**Overall status:** L1 INTERNAL PRODUCT PATH CLOSED; L2 STATIC/TOOLING ADVANCED; L3 STATIC SPINE ADVANCED; subsystem remains NOT COMPLETE pending real-retail/protected-process/original-consumer receipts.

## Authority split

- GitHub `main` is canonical implementation truth.
- Reverse claims remain bounded to their exact artifact/range/scope.
- Synthetic/public CI proves product/tool behavior only.
- A real-device member receipt proves only its exact local archive snapshot/member/materialization scope.
- Original-game equivalence requires original-process evidence.
- GDSpaces owns product resource identity/materialization/authoring; recovered original functions do not move into GDSpaces.
- Canonical analysis EXE authority and protected original-execution authority are separate and must not be silently substituted.
- Independent receipts must not be composed by filename alone; final vertical proof requires one reconciled source/request lineage.
- Open PRs #226, #238, #240 and #241 are pending/branch truth only until merged.

## Layer model

- **L1 — Resource Materialization:** exact bytes, transform/decompression, nested expansion, bounded authoring, rebuild/repack, reopen/rematerialization.
- **L2 — Resource Resolution:** request, candidates, normalization, provider/volume/source identity, discovery vs successful mount topology, fallback/ambiguity/failure.
- **L3 — Original Runtime/Lifecycle:** FileSlot/AsyncIO, LoadedResource states, typed-ready, claims/cache, cancellation/reset/release, consumer behavior.
- **V/LV — Validation / live observation:** cross-cutting, not L4.
- **RCP — Resource Control Plane:** draft orthogonal orchestration model from #226, not L4.

Execution follows [master-roadmap.md](../gdspaces/master-roadmap.md).

## Materialization completion correction — merged #230/#242

Canonical normal completion ABI:

```text
0x1401B84E0
 -> registers 0x1401B8DC0
 -> one u32 context = record_ptr - 0x140C99D30
 -> valid contexts = index * 0x48 for 363 records
```

Normal `0x1401B8DC0` receives no raw transport status pointer, error flag, byte count, FileSlot/ReadRequest handle or child/outstanding-work metadata.

Therefore lower materialization success/failure must already be terminal before normal state2 completion publication, or the queued completion must be suppressed/removed. FIFO insertion order alone is not a proven dependency barrier. No generic original fan-in/outstanding-child counter is evidenced.

Focused exact-byte order:

1. `0x1402EF4D0` queued materialization job identity/type/callees/context consumer;
2. relevant materialization case in `0x1402EF790`, persistence/re-poll/terminal retirement;
3. reacquire historical `0x1400333E0` status/poll hypothesis;
4. reacquire historical `0x140033390` terminal cleanup/release hypothesis;
5. `0x1400335A0` lower transport terminal writes;
6. identify what prevents normal `0x1401B8DC0` on incomplete/failed transport;
7. `0x1402EF460` higher-scheduler clear/rollback and queued-completion suppression;
8. only then `.lst` child/recursive failure ordering.

Layer ownership is unchanged: exact byte-read mechanics may support L1; FileSlot/AsyncIO request ownership/scheduling/callback lifetime/cancellation belongs to L3; `0x1401B8CA0` is the L1/L3 materialization-success seam; LoadedResource states remain L3.

## L1 current state

Canonical L1 product implementation includes NBZ STORE/raw-DEFLATE materialization, CRC/size/SHA/ByteProvenance, artifact-bound observations, direct-retail acquisition, atomic/no-replace publication, PAC/PNST recursive sparse/alias-preserving expansion, size-changing/nested authoring, untouched-sibling preservation, immutable NBZ copy rebuild, next-volume overlay authoring, canonical reopen/rematerialization, protected preflight and product closure orchestration.

No known mandatory internal product-code blocker remains for the current representative DMC3-HD L1 acceptance scope.

Merged #233 establishes:

- protected `dmc3.exe` is locatable;
- executable-relative `data/dmc3/dmc3-0.nbz` is locatable;
- observed NBZ size = `960,358,951` bytes;
- connected raw materialization ceiling = `268,435,456` bytes.

This is a transport/access limitation, not archive absence or an L1 parser failure.

Pending #238 reconciles Pocket GDS as an out-of-band exact-member receipt route when the actual NBZ is local on-device. It can prove exact local member/materialization/classification scope, but cannot prove protected original-process resolver selection or consumption by itself.

Remaining L1 chain:

```text
real selected-source/member lineage
 -> exact member materialization receipt
 -> representation classification
 -> one supported real edit/rebuild
 -> next-volume overlay + canonical reopen/rematerialization
 -> original DMC3 consumer-visible effect
 -> rollback / original retail immutability
 -> final V:L1 audit
```

Issue #209 remains mandatory. `obj\em000.pac` is a high-value target, not a mandatory predeclared winner.

## L2 current frontier

Merged slices:

- #215/#204: type-0 physical-provider static reverse + controlled product model;
- #219: legacy bounded runtime mapping tooling;
- #221: selected-provider content-candidate normalizer/validator/artifact binder;
- #235: process-instance-bound R2B v2 tooling and mount-topology correction.

#235 establishes that archive filename discovery and actual successful mounting are separate. First missing filename bounds discovery only. Existing archives may fail registration while discovery continues, so the successful mounted set may be sparse. Successful archive mounts prepend, preserving:

```text
higher successful volume -> lower successful volume -> physical
```

Issue #237 tracks the correction and PR #241 is the active product implementation. Until merge, #241 is branch truth.

Remaining L2 gates:

1. exact archive-SHA-bound retail `0x0E` collision census;
2. real protected-process R2B v2 seven-anchor packet;
3. trusted zero-loss R3 selected-provider capture;
4. exact actually-successful mount topology + archive/member binding;
5. final L2 audit after code/docs/evidence/CI reconciliation.

Fresh canonical EXE review preserves that an archive normalized lookup hit can fail wrapper/open creation at `0x140328290`; that path is terminal null/cleanup, not a clean lower-volume miss.

Authority identities:

- canonical analysis executable: `e454272ed0fb0247fcbcf300e5d55d7a3e96d50b89b9ffaff81bb978dcbdd082`, 6,356,432 bytes;
- protected distribution/original execution candidate: `81c7e61983564113b5105e931d9f185accc14e44ae147d27f720c2d50935c7d6`, 6,567,320 bytes.

Canonical analysis VAs/RVAs must not be applied to the protected process without independent R2B evidence.

## L3 current frontier

The static LoadedResource/typed-ready/lifetime spine is advanced but Layer 3 is not complete.

Current vertical validation boundary:

```text
exact L2 selected identity
 -> exact L1 materialized byte identity
 -> L3 acquisition/state1
 -> lower transport/materialization terminal condition
 -> normal completion/state2
 -> typed-ready/state3
 -> deterministic consumer effect
 -> rollback
```

Pending #240 proposes the exact canonical `LoadedResource +0x04` writer census as `STATIC BOUNDED-CLOSED / APPROVED / CONTRADICTION-GATED`. Until merge this is branch truth only. Even after promotion, R2-R5 and V1-V7 remain open.

The first L1 vertical proof requires enough L3 evidence to prove exact authored bytes reached the intended original consumer. That minimum does not make L3 complete.

## RCP / grey boundary

Draft #226 models RCP as orthogonal orchestration for root/dependency planning, pending/ready coordination, claims and transition control. It does not change L1/L2/L3 ownership and does not create L4.

RCP work may run only where it resolves an active identity/dependency/ownership ambiguity. It must not displace the L1 real-evidence sequence merely to broaden reverse coverage.

Readiness remains:

```text
manager_ready_state3
!= family_semantic_ready
!= consumer_effect_observed
```

## Current critical path

### L1

1. produce exact real member/materialization receipt;
2. bind it to accepted actual selected/successful source lineage;
3. classify exact representation;
4. perform one supported real edit/rebuild;
5. run next-volume publication + canonical reopen/rematerialization;
6. execute #209 original-game consumption + rollback;
7. run final cross-stack/V audit.

### L2 support

1. review/merge or reconcile #241;
2. obtain archive-bound retail member surface and run `0x0E` collision census;
3. run real protected R2B v2 seven-anchor capture;
4. capture trusted zero-loss R3 selected identity;
5. final L2 audit.

### L3 support

1. close #242 exact materialization terminal-condition dependency bytes;
2. review/merge #240 only at bounded R1 scope if no contradiction exists;
3. move to R2 field/backing ownership instead of broad R1 rediscovery;
4. capture original-process lifecycle/consumer evidence bound to exact L1/L2 identity.

No synthetic-only feature should displace the real evidence sequence unless a real run reveals a concrete missing dependency.

## Environment boundary

Connected evidence can locate the protected executable and 960,358,951-byte `dmc3-0.nbz`, but the connected raw transfer path cannot ingest the full archive because of the observed 268,435,456-byte ceiling.

Pocket GDS can materialize members where the archive is already local. The protected game PC/process remains required for original selected identity, real R2B/R3 evidence, #209 consumption/rollback and original lifecycle evidence.

## Navigation

- [L1 roadmap](../gdspaces/l1-roadmap.md)
- [Master roadmap](../gdspaces/master-roadmap.md)
- [Final pre-Level-E L1 audit](../gdspaces/l1-final-audit-2026-08-25.md)
- [Materialization completion dependency Pass 2](../gdspaces/materialization-completion-dependency-pass2-2026-08-26.md)
- [L2 EXE reconciliation](../gdspaces/l2-exe-reconciliation-2026-08-26.md)
- [Blockers](blockers.md)
- [Phase map](phase-map.md)
- [Risks](risks.md)
- [Machine-readable status](canonical-status.json)

No percentage, documentation update, draft architecture or pending PR overrides the gate-based completion rule.
