# Current Project Status

**Snapshot date:** 2026-08-27  
**Reconciled canonical main:** through merged PR #242 (`f886f27e62ec9a05b6829df7fd074981a06a4b49`)  
**Latest L1/access reconciliation:** #233 merged; PR #238 pending Pocket/member-evidence reconciliation  
**Latest L2 provenance tooling:** #235 merged — process-instance-bound R2B v2  
**Active L2 topology correction:** PR #241 pending — discovery vs successful mounts  
**Latest materialization-completion authority:** #230 + #242 merged  
**Active L3 R1 review:** PR #240 pending  
**Active EXE grey-boundary/RCP audit:** #225 / draft PR #226  
**V/LV architecture:** draft #223, cross-cutting validation only  
**Primary execution program:** GDSpaces L1 final same-lineage acceptance with evidence-driven L2/L3 support  
**Overall status:** L1 INTERNAL PRODUCT PATH CLOSED; L2 STATIC/TOOLING ADVANCED; L3 STATIC SPINE ADVANCED; subsystem remains NOT COMPLETE pending real-retail/protected-process/original-consumer receipts.

## Authority split

- GitHub `main` is canonical implementation truth.
- Reverse claims remain bounded to their recorded artifact/range/scope.
- Synthetic/public CI proves product/tool behavior only.
- A real-device member receipt proves only the exact local snapshot/member/materialization scope encoded in that receipt.
- Original-game equivalence requires original-process evidence.
- GDSpaces owns product resource identity/materialization/authoring; recovered original functions do not move into GDSpaces.
- Canonical analysis executable authority and protected original-execution authority are separate and must not be silently substituted.
- Independent receipts must not be composed by filename alone; final vertical proof requires one reconciled source/request lineage.
- Open PRs #226, #238, #240 and #241 are branch/pending truth only until merged.

## GDSpaces layer model

- **L1 — Resource Materialization:** exact bytes, transform/decompression, nested expansion, bounded authoring, rebuild/repack, reopen/rematerialization.
- **L2 — Resource Resolution:** request, candidates, normalization, provider/volume/source identity, discovery vs successful mount topology, fallback/ambiguity.
- **L3 — Original Runtime/Lifecycle:** FileSlot/async/LoadedResource/typed-ready/claim/reset/release/consumer behavior.
- **V — Validation / Equivalence:** cross-cutting promotion authority; not an execution layer.
- **LV — Live Validation / Original-Process Observation:** V-owned acquisition/observation concept; not L4.
- **RCP — Resource Control Plane:** orthogonal orchestration/control plane proposed by #226; not L4.

Execution follows the dependency-driven [master roadmap](../gdspaces/master-roadmap.md).

## EXE grey-boundary / Resource Control Plane model

The two-pass audit tracked in #225 / draft PR #226 finds **no evidence-based need for L4**.

RCP is an orthogonal orchestration plane covering:

- root request planning/emission;
- dependency planning/emission;
- pending/ready coordination;
- loader claims/retention;
- transition cancellation/quiescence/replacement.

RCP orchestrates L2/L1/L3; it does not replace their ownership or their acceptance gates.

Useful L3 accounting subdomains remain:

- **L3A — Typed Construction / Dependency**;
- **L3B — Ownership / Lifecycle**.

Supporting non-layer planes/tags:

- `TYPE/ID` — descriptor/type and cross-layer identity mappings;
- `RT-IO` — FileSlot/AsyncIO seam;
- `MEM/BACKING` — allocation/backing substrate;
- `BOOTSTRAP` — startup/service substrate;
- `ERROR` — per-owner failure/recovery matrix.

Dependency recursion may be modeled as:

```text
L3A dependency discovery
 -> RCP request emission
 -> L2 selection
 -> L1 materialization
 -> L3A child processing
```

This does not move child request resolution into L3 or materialization into RCP.

### Readiness correction

Raw-EXE authority preserves the distinction:

```text
manager_ready_state3
!= universal family_semantic_ready
!= consumer_effect_observed
```

State3 is a manager/lifecycle readiness boundary, not universal family-semantic success and not a consumer-effect receipt.

Canonical grey-boundary branch docs in #226:

- `../gdspaces/exe-grey-boundary-audit-2026-08-26.md`;
- `../gdspaces/exe-grey-boundary-pass2-2026-08-26.md`;
- `../gdspaces/exe-grey-boundary-roadmap-2026-08-26.md`.

## Materialization completion dependency correction — merged #230/#242

Normal completion ABI:

```text
0x1401B84E0
 -> registers 0x1401B8DC0
 -> one u32 context = record_ptr - 0x140C99D30
 -> valid normal contexts = index * 0x48 for 363 records
```

Normal `0x1401B8DC0` receives no raw transport status pointer, error flag, byte count, FileSlot/ReadRequest handle or child/outstanding-work metadata.

Therefore lower materialization success/failure must already be terminal before normal state2 completion publication, or the queued completion must be suppressed/removed. FIFO insertion order alone is not a proven dependency barrier. No generic original fan-in/outstanding-child counter is evidenced.

The exact-byte reacquisition order is:

1. `0x1402EF4D0` queued materialization job identity/type, callees and inherited context consumer;
2. relevant `0x1402EF790` materialization case and persistence/re-poll/terminal retirement;
3. historical `0x1400333E0` status/poll hypothesis — reacquire fresh bytes;
4. historical `0x140033390` terminal release/cleanup hypothesis — reacquire fresh bytes;
5. `0x1400335A0` lower transport success/error state writes;
6. identify what prevents normal `0x1401B8DC0` on incomplete/failed transport;
7. `0x1402EF460` higher-scheduler clear/rollback and queued-completion suppression;
8. only then `.lst` child/recursive failure ordering.

Layer ownership remains unchanged: exact FileSlot byte-read mechanics may support L1; FileSlot/AsyncIO ownership/scheduling/callback lifetime/cancellation is L3; `0x1401B8CA0` is the L1/L3 materialization-success seam; LoadedResource states remain L3.

## L1 current state

Canonical L1 implementation includes:

- NBZ classic ZIP bounded indexing/materialization;
- STORE + raw-DEFLATE method 8;
- CRC/size/SHA/ByteProvenance;
- artifact-bound archive/member observations;
- numbered-volume discovery/runtime-domain behavior for the accepted clean product path;
- resolver-selected direct-retail member acquisition with provenance receipt;
- shared staged atomic/no-replace publication;
- PAC/PNST sparse/empty/alias-preserving parse + expansion;
- size-changing relative-slot reflow;
- root-to-leaf nested PAC/PNST slot-path authoring;
- byte-exact untouched sibling preservation;
- immutable verified NBZ copy rebuild;
- deterministic next-contiguous NBZ overlay authoring;
- staged canonical NBZ reopen;
- higher-successful-volume resolver verification for represented successful mounts;
- protected distribution executable preflight;
- product closure orchestration through exact authored rematerialization.

No known mandatory internal product-code blocker remains for the current representative DMC3-HD L1 acceptance scope.

Merged #233 establishes that the protected executable and executable-relative `data/dmc3/dmc3-0.nbz` are locatable in connected Drive evidence. The observed NBZ size is `960,358,951` bytes. The connected raw-transfer/materialization ceiling is `268,435,456` bytes, so this is a transport blocker, not artifact absence.

Pending #238 reconciles Pocket GDS as an out-of-band exact-member receipt path when the real NBZ is already local on-device. That receipt can prove exact local archive snapshot/member/materialization/classification scope, but cannot prove protected original-process resolver selection or consumption by itself.

The remaining L1 acceptance chain is:

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

Issue #209 remains mandatory.

## L2 current frontier

Merged canonical slices:

- #215/#204: type-0 physical-provider static reverse and controlled product model;
- #219: legacy explicit-PID runtime mapping tooling;
- #221: selected-provider content-candidate normalizer/validator/artifact binder;
- #235: process-instance-bound R2B v2 tooling with PID + process creation FILETIME + module identity and seven canonical resolver/bootstrap anchors.

#235 also corrects the topology model: numbered filename **discovery** and actual **successful mounting** are separate. The first missing filename bounds discovery only. An existing archive may fail registration and discovery can continue; the successful mounted set may be sparse. Successful archive mounts still prepend, preserving:

```text
higher successful volume -> lower successful volume -> physical
```

Issue #237 tracks the correction and PR #241 is the active clean-product implementation. Until merge, #241 is branch truth only.

Remaining L2 gates:

1. exact archive-SHA-bound retail `0x0E` collision census;
2. real protected-process R2B v2 seven-anchor packet;
3. trusted zero-loss R3 selected-provider identity from that exact process instance;
4. observed actually-successful mount topology and exact archive/member binding;
5. final L2 audit after code/docs/evidence/CI reconciliation.

Fresh canonical EXE review preserves this failure boundary: an archive normalized lookup hit can fail during wrapper/open creation at `0x140328290`, and `0x140327430` then returns through null/cleanup rather than treating the failure as a lower-volume miss. Provider/backend failure is not a clean miss.

Authority identities:

- canonical analysis executable: SHA-256 `e454272ed0fb0247fcbcf300e5d55d7a3e96d50b89b9ffaff81bb978dcbdd082`, 6,356,432 bytes;
- protected distribution/original execution candidate: SHA-256 `81c7e61983564113b5105e931d9f185accc14e44ae147d27f720c2d50935c7d6`, 6,567,320 bytes.

The protected build is not instruction-reverse authority. Canonical analysis VAs/RVAs cannot be promoted into the protected process without independent R2B evidence.

### Request-ingress grey edge

The canonical direct-call surface contains three direct `OpenGameResource` callers and all pass `flags=1`, but their complete upstream semantic/request-origin ABI is not yet classified.

#225 P2-R1 owns the supporting upstream caller census. This remains an RCP/ingress question, not L0.

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

Open L3 breadth includes:

- field/backing ownership beyond the bounded R1 writer census;
- external factory/dependency and SCM edges;
- shared-owner family breadth;
- allocation/profile differences;
- original-process lifecycle/consumer receipts;
- exact terminal-condition dependency mechanism listed under #242.

For the first L1 vertical proof, L3 must provide enough evidence to prove that the exact authored L1 bytes reached the intended original consumer. This minimum does not make L3 complete.

## EXE boundary / RCP current frontier

Parent #225 work order remains:

1. P2-R1 upstream request-origin census;
2. P2-R2 current raw StageCfg dependency-preload reacquisition;
3. P2-R3 Type/Descriptor identity xref;
4. P2-R4 factory/resource-set demand edges;
5. P2-R5 ownership hierarchy breadth;
6. P2-R6 manager-ready/family-ready/consumer-effect taxonomy;
7. P2-R7 dependency-aware LV/V integration after the static graph is bounded and #223 architecture is promoted or explicitly reconciled.

Historical Wave-3 PR #84 is a high-value target source, especially for StageCfg-driven enemy dependency preload and resource-set demand mapping, but remains historical/unmerged evidence until reconfirmed against current canonical authority.

## Bounded open reverse breadth — not automatic L1 blockers

The following remain real research gaps but only block L1 if the chosen acceptance path depends on them:

- complete `0x140328540` ZIP stream initializer lifetime;
- complete `0x140328FE0` compressed seek/reset/reinflate behavior;
- exhaustive malformed/partial-read original error equivalence;
- dynamic `.lst` allocation/free/error/cycle semantics and real loose-list corpus validation;
- unsupported/evidence-absent binary backends or formats.

Binary AFS and original-runtime PACK remain frozen absent direct evidence. Capcom offline writer equivalence is not an L1 requirement.

## Current critical path

### L1 vertical acceptance

1. produce an exact real member/materialization receipt via protected-install route or Pocket real-device route;
2. bind the materialized member to the accepted selected-provider/request and actually-successful source lineage;
3. classify the exact representation;
4. perform one supported bounded real edit, top-level or nested;
5. run next-volume authoring + canonical resolver/reopen/rematerialization closure;
6. execute issue #209 original-game consumption + rollback;
7. run final cross-stack/V audit;
8. mark `L1 = 100% / COMPLETE` only if every mandatory same-lineage receipt is valid.

### L2 closure support

1. obtain cryptographically bound retail DMC3 member-list/central-directory evidence and run the `0x0E` collision census;
2. review/merge #241 without weakening successful-mount semantics;
3. run #235 R2B v2 tooling against the exact protected process and produce all seven same-process-instance anchor receipts;
4. use merged #221 only with a trusted publisher to capture a zero-loss R3 selected-identity trace;
5. bind observer artifact + exact actually-successful mounted numbered NBZ artifacts and selected identity;
6. run final L2 audit.

### L3 / RCP supporting closure

1. retain #242 materialization dependency bridge as the cross-layer acceptance seam;
2. close the exact job/status/poll/retirement/suppression mechanism rather than inventing FIFO/fan-in behavior;
3. review/merge #240 only at its bounded R1 scope if no contradiction exists;
4. continue #225 RCP ingress/dependency/ownership work only where it resolves a concrete dependency;
5. capture original-process lifecycle traces with exact L1/L2 identity;
6. distinguish manager-ready state from family-semantic/consumer success in validation.

No synthetic-only feature should displace the real evidence sequence unless a real run reveals a concrete missing dependency.

## Environment boundary

Connected evidence can locate the protected executable and 960,358,951-byte `dmc3-0.nbz`, but the connected raw transfer path cannot ingest the full archive because of the observed 268,435,456-byte ceiling.

Pocket GDS can execute exact member materialization where the archive is already local on-device. The protected game PC/process remains required for original-process selected identity, R2B v2/R3 evidence, #209 consumption/rollback and original lifecycle evidence that mobile execution cannot prove.

Synthetic CI must not substitute for those receipts.

## Navigation

- [Canonical L1 roadmap](../gdspaces/l1-roadmap.md)
- [Final pre-Level-E L1 audit](../gdspaces/l1-final-audit-2026-08-25.md)
- [Real-device member evidence reconciliation](../gdspaces/l1-real-device-member-evidence-reconciliation-2026-08-26.md)
- [Materialization completion dependency Pass 2](../gdspaces/materialization-completion-dependency-pass2-2026-08-26.md)
- [Three-layer master roadmap](../gdspaces/master-roadmap.md)
- [EXE grey-boundary Pass 1](../gdspaces/exe-grey-boundary-audit-2026-08-26.md)
- [EXE grey-boundary Pass 2](../gdspaces/exe-grey-boundary-pass2-2026-08-26.md)
- [EXE grey-boundary reverse roadmap](../gdspaces/exe-grey-boundary-roadmap-2026-08-26.md)
- [L2 EXE reconciliation checkpoint](../gdspaces/l2-exe-reconciliation-2026-08-26.md)
- [L2 selected-identity runbook](../gdspaces/l2-original-selected-identity-runbook-2026-08-26.md)
- [Blockers](blockers.md)
- [Machine-readable status](canonical-status.json)
- [GDSpaces contract](../gdspaces-contract.md)

No percentage, draft architecture or implementation milestone overrides the gate-based completion rule. RCP/V/LV remain orthogonal control/validation structures and do not create L4 or a completion claim.
