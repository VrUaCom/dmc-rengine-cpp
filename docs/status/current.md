# Current Project Status

**Snapshot date:** 2026-08-27  
**Reconciled canonical main:** through merged PR #242 (`f886f27e62ec9a05b6829df7fd074981a06a4b49`)  
**Latest L1/V access reconciliation:** #233 merged; PR #238 adds Pocket/member-evidence reconciliation and remains pending  
**Latest L2 provenance tooling:** #235 merged — process-instance-bound R2B v2  
**Active L2 topology correction:** PR #241 — discovery vs successful mount topology; pending, not main truth  
**Latest materialization-completion authority:** #230 + #242 merged  
**Active L3 R1 review:** PR #240 — pending, not main truth  
**Primary execution program:** GDSpaces Layer 1 final same-lineage acceptance + evidence-driven L2/L3 support  
**Overall status:** L1 INTERNAL PRODUCT PATH CLOSED; L2 STATIC/TOOLING ADVANCED; L3 STATIC SPINE ADVANCED; subsystem remains NOT COMPLETE pending real-retail/protected-process/original-consumer receipts.

## Authority split

- GitHub `main` is canonical implementation truth.
- Reverse claims remain bounded to their recorded artifact/range/scope.
- Synthetic/public CI proves product/tool behavior only.
- A real-device member receipt proves only the exact local snapshot/member/materialization scope encoded in that receipt.
- Original-game equivalence requires original-process evidence.
- GDSpaces owns product resource identity/materialization/authoring; recovered original functions do not move into GDSpaces.
- Canonical analysis executable authority and protected original-execution authority are separate and must not be silently substituted.
- Independent receipts must not be composed by matching filenames alone; final vertical proof requires one reconciled source/request lineage.
- Open PRs #226, #238, #240 and #241 are branch/pending truth only until merged. Their useful findings may be referenced as pending evidence, but must not be described as current-main implementation authority.

## GDSpaces layer model

- **L1 — Resource Materialization:** exact bytes, transform/decompression, nested expansion, bounded authoring, rebuild/repack, reopen/rematerialization.
- **L2 — Resource Resolution:** request, candidates, normalization, provider/volume/source identity, mount discovery vs successful mount topology, fallback/ambiguity.
- **L3 — Original Runtime/Lifecycle:** FileSlot/async/LoadedResource/typed-ready/claim/reset/release/consumer behavior.
- **V/LV — Validation:** binds selected identity, materialized byte identity and original consumer observation into one acceptance run. V/LV architecture work remains separately gated and must not create an L4.

Execution follows the dependency-driven [master roadmap](../gdspaces/master-roadmap.md).

## Canonical materialization-completion correction

Merged #230/#242 supersede older wording that could be read as a generic original fan-in/outstanding-child counter.

Canonical normal completion ABI:

```text
0x1401B84E0
 -> registers 0x1401B8DC0
 -> one u32 context = record_ptr - 0x140C99D30
 -> valid normal contexts = index * 0x48 for 363 records
```

Normal `0x1401B8DC0` receives no raw transport status pointer, error flag, byte count, FileSlot/ReadRequest handle or child/outstanding-work metadata. Therefore the lower materialization success/error condition must already be resolved before normal `0x1401B8DC0` dispatch, or the queued completion must be suppressed/removed.

**FIFO insertion order alone is not a proven dependency barrier.** If the earlier materialization job can merely submit asynchronous transport and retire, a later completion callback could publish state2 too early. The open seam is therefore **materialization completion ordering / dependency bridge**, not a fabricated generic counter.

Focused exact-byte reacquisition order from #242:

1. `0x1402EF4D0` queued materialization job identity/type and inherited load-context consumer;
2. `0x1402EF790` materialization-job persistence/re-poll/terminal retirement;
3. historical `0x1400333E0` status/poll anchor — hypothesis until fresh bytes;
4. historical `0x140033390` terminal release/close anchor — hypothesis until fresh bytes;
5. `0x1400335A0` lower transport success/error state writes;
6. identify what prevents normal `0x1401B8DC0` on incomplete/failed transport;
7. `0x1402EF460` pending scheduler clear/rollback and queued-completion suppression;
8. only then apply the confirmed mechanism to `.lst` child/recursive failure ordering.

Layer ownership is unchanged: FileSlot exact byte-read mechanics may support L1; FileSlot/AsyncIO request ownership/scheduling/callback lifetime is L3; `0x1401B8CA0` is the explicit L1/L3 materialization-success seam; LoadedResource states remain L3.

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
- higher-volume resolver verification for the accepted successful-mounted set;
- protected distribution executable preflight;
- product closure orchestration through exact authored rematerialization.

Canonical L1 references:

- [L1 roadmap](../gdspaces/l1-roadmap.md)
- [Final pre-Level-E audit](../gdspaces/l1-final-audit-2026-08-25.md)
- [Connected retail access reconciliation](../gdspaces/l1-connected-retail-access-reconciliation-2026-08-26.md)
- [Real-device member evidence reconciliation](../gdspaces/l1-real-device-member-evidence-reconciliation-2026-08-26.md)
- [Level-E operator runbook](../gdspaces/l1-level-e-runbook.md)
- [Materialization completion dependency Pass 2](../gdspaces/materialization-completion-dependency-pass2-2026-08-26.md)

## L1 evidence/access correction

Merged #233 establishes that the protected artifacts are locatable in connected Drive evidence:

- protected `dmc3.exe` is locatable;
- executable-relative `data/dmc3/dmc3-0.nbz` is locatable;
- observed `dmc3-0.nbz` size is `960,358,951` bytes.

The full NBZ cannot currently be materialized through the connected Drive/Files execution channel because the observed transfer/materialization ceiling is `268,435,456` bytes. This is a **transport blocker**, not artifact absence.

Pending PR #238 records the Pocket GDS / GDSpace Manager route as an out-of-band real-device evidence seam over the canonical mobile GDSpaces materialization path. Once run against the actual NBZ, `gdspaces.l1.member-acquisition-receipt.v1` can record archive SHA/size, ResourceIdentity, ByteProvenance, exact exported member SHA/size, representation class and producer/core provenance without embedding proprietary bytes.

A Pocket receipt does not by itself prove protected original-process resolver selection or consumption.

## L1 mandatory remaining work

No known mandatory **internal implementation** blocker remains for the current representative DMC3-HD L1 acceptance scope.

The remaining gates are real same-lineage evidence executions:

```text
real selected-source/member lineage
 -> exact member materialization receipt
 -> representation classification
 -> one supported real edit/rebuild
 -> next-volume overlay + canonical rematerialization receipt
 -> original DMC3 consumer-visible effect
 -> rollback / original retail immutability
 -> final V:L1 audit
```

Issue #209 is the mandatory original-game Level-E gate.

`obj\em000.pac` remains a high-value target, but the final proof must bind the exact materialized member to the accepted selected-provider/request lineage. Another representative resource may be used if it provides a stronger deterministic consumer effect.

### Available acquisition routes

1. **Protected-install desktop route:** `extract-dmc3-retail-member` / `verify-dmc3-l1-authoring` preserves resolver-selected retail lineage directly.
2. **Pocket real-device sub-receipt:** useful when the 960 MB NBZ is already local on the phone. It may close the exact member-byte/materialization sub-gate and provide representation-classification evidence, but still needs selected-source/protected authority binding before final promotion.

## Bounded open reverse breadth — not automatic L1 blockers

The following remain real research gaps but only block L1 if the chosen acceptance path depends on them:

- complete `0x140328540` ZIP stream initializer lifetime;
- complete `0x140328FE0` compressed seek/reset/reinflate behavior;
- exhaustive malformed/partial-read original error equivalence;
- dynamic `.lst` allocation/free/error/cycle semantics and real loose-list corpus validation;
- unsupported/evidence-absent binary backends or formats.

Binary AFS and original-runtime PACK remain frozen absent direct evidence. Capcom offline writer equivalence is not an L1 requirement.

## L2 current frontier

Merged/canonical internal L2/tooling slices:

- #215/#204: canonical type-0 physical-provider static reverse, direct native physical product path and controlled receipts;
- #219: first bounded explicit-PID protected-runtime RVA acquisition/mapping tooling;
- #221: selected-provider content-candidate contract, strict normalizer/validator and artifact binder; candidate tooling only, not trusted original-process evidence;
- #235: canonical-artifact-bound **R2B v2** tooling with PID + process-creation FILETIME + module identity from one process instance and seven mandatory bootstrap/resolver anchors.

#235 also establishes the critical distinction between archive filename **discovery** and actual **successful mounting**. The first missing numbered filename bounds discovery only. An existing archive can fail registration and discovery may continue; the actually successful mount set may therefore be sparse. Successful archive mounts still prepend, so effective archive precedence among successful mounts remains higher successful volume -> lower successful volume -> physical.

Open PR #241 implements this distinction in the clean product topology. Until it merges, it is pending implementation truth, not canonical main.

The remaining L2 closure is split into independent gates:

1. **real-retail `0x0E` collision census** — exact archive-bound member-name/central-directory evidence still required;
2. **real protected-process R2B v2 mapping receipt** — #235 tooling is merged, but no real seven-anchor protected-process packet is canonical yet;
3. **successful-mount topology correction** — PR #241 pending;
4. **R3 trusted selected-provider identity** — #221 tooling is merged, but trusted capture origin, real R2B v2 mapping and zero-loss real trace remain required;
5. **final L2 audit** — only after retail corpus + real R2B/R3 receipts, topology reconciliation and exact-head validation agree.

Fresh canonical EXE review preserves this failure boundary: an archive normalized lookup hit can fail during wrapper/open creation (`0x140328290`), and `0x140327430` then exits through null/cleanup instead of treating that as a lower-volume miss. Provider/backend failure is not a clean miss.

Authority identities:

- canonical analysis executable: `e454272ed0fb0247fcbcf300e5d55d7a3e96d50b89b9ffaff81bb978dcbdd082`, 6,356,432 bytes;
- protected distribution/original execution candidate: `81c7e61983564113b5105e931d9f185accc14e44ae147d27f720c2d50935c7d6`, 6,567,320 bytes.

The protected build is not instruction-reverse authority. Canonical analysis VAs/RVAs cannot be promoted into the protected process without independent R2B evidence.

L2 work may support L1 but must not replace the final L1 acceptance run.

## L3 current frontier

The static LoadedResource/typed-ready/lifetime spine is strong, but Layer 3 is not complete.

Merged #230/#242 make the completion boundary more precise:

```text
exact L2 selected identity
 -> exact L1 materialized byte identity
 -> L3 acquisition/state1
 -> lower transport/materialization terminal condition
 -> normal B8DC0 completion publication/state2
 -> typed-ready/state3
 -> deterministic consumer effect
 -> rollback
```

No generic child/outstanding-work fan-in counter is evidenced. Normal `0x1401B8DC0` cannot itself inspect transport success because its normal ABI only carries the registry-relative u32 context. The pending exact-byte work is to recover the dependency mechanism that makes state2 publication safe.

Pending PR #240 proposes `R1 = STATIC BOUNDED-CLOSED / APPROVED / CONTRADICTION-GATED` for the canonical `LoadedResource +0x04` writer census. This status is **not current-main canonical until #240 merges**. Even if promoted, Layer 3 remains open: R2–R5 and V1–V7 original-process lifecycle receipts remain separate gates.

Broader L3 completion is separate from the minimum original-consumer evidence required for L1 acceptance.

## Draft EXE control-plane / V-LV architecture status

PR #226 remains draft branch truth for the Resource Control Plane / grey-boundary model. It must preserve the canonical three-layer model: RCP is an orthogonal orchestration/control plane, not L4. PR #223 likewise keeps V/LV as validation/observation architecture rather than a new execution layer. Neither draft may alter L1/L2/L3 completion criteria without separate promotion.

## Current critical path

### L1 vertical acceptance

1. produce an exact real member/materialization receipt via the protected-install route or Pocket real-device route;
2. bind the materialized member to the accepted selected-provider/request lineage;
3. classify the exact representation;
4. perform one supported bounded real edit, top-level or nested;
5. run next-volume authoring + canonical resolver/reopen/rematerialization closure;
6. execute issue #209 original-game consumption + rollback;
7. run final cross-stack/V audit;
8. mark `L1 = 100% / COMPLETE` only if every mandatory same-lineage receipt is valid.

### L2 closure support

1. obtain cryptographically bound retail DMC3 member-list/central-directory evidence and run the `0x0E` collision census;
2. merge/reconcile #241 after review and exact-head CI without weakening successful-mount precedence;
3. run #235 R2B v2 tooling against the exact protected process and produce all seven same-process-instance anchor receipts;
4. use merged #221 only with a trusted publisher to capture a zero-loss R3 selected-identity trace;
5. bind observer artifact + exact actually-successful mounted numbered NBZ artifacts and preserve selected identity without treating provider/backend failure as a miss;
6. reconcile code/docs/evidence and run final L2 audit.

### L3 supporting closure

1. retain #242 materialization dependency bridge as the cross-layer acceptance seam;
2. merge/reconcile #240 only if review finds no contradictory canonical writer evidence;
3. proceed to R2 field/backing ownership after R1 promotion rather than reopening broad `+0x04` discovery without concrete contradiction;
4. capture #217/V1–V7 lifecycle traces with exact L1/L2 identity;
5. distinguish manager-ready state from family-semantic/consumer success in validation.

No synthetic-only feature should displace the real evidence sequence unless a real run reveals a concrete missing dependency.

## Environment boundary

Connected artifact discovery is no longer the primary L1 blocker: the protected EXE and 960,358,951-byte `dmc3-0.nbz` are locatable. The connected raw-transfer path cannot ingest that NBZ because of the observed 268,435,456-byte ceiling.

Pocket GDS can execute member materialization where the archive is already local on-device. The protected game PC/process is still required for selected-source/original-process evidence, R2B v2/R3 evidence, #209 consumption/rollback and lifecycle traces that mobile execution cannot prove.

Synthetic CI must not substitute for those receipts.

## Navigation

- [Canonical L1 roadmap](../gdspaces/l1-roadmap.md)
- [Final pre-Level-E L1 audit](../gdspaces/l1-final-audit-2026-08-25.md)
- [Connected retail access reconciliation](../gdspaces/l1-connected-retail-access-reconciliation-2026-08-26.md)
- [Real-device member evidence reconciliation](../gdspaces/l1-real-device-member-evidence-reconciliation-2026-08-26.md)
- [Level-E operator runbook](../gdspaces/l1-level-e-runbook.md)
- [Materialization completion dependency Pass 2](../gdspaces/materialization-completion-dependency-pass2-2026-08-26.md)
- [Three-layer master roadmap](../gdspaces/master-roadmap.md)
- [L2 EXE reconciliation checkpoint](../gdspaces/l2-exe-reconciliation-2026-08-26.md)
- [L2 selected-identity runbook](../gdspaces/l2-original-selected-identity-runbook-2026-08-26.md)
- [Blockers](blockers.md)
- [Machine-readable status](canonical-status.json)
- [GDSpaces contract](../gdspaces-contract.md)

No percentage or implementation milestone overrides the gate-based completion rule. No L1/L2/L3 COMPLETE claim is made by documentation synchronization alone.
