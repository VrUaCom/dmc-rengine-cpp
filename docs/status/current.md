# Current Project Status

**Snapshot date:** 2026-08-26  
**Reconciled main base:** `main@a90b017ab29171e00174f2a56c719c32241a63f1`  
**Latest L1/V reconciliation:** #228 materialization-completion boundary  
**Latest L2 provenance tooling:** #235 process-instance-bound R2B v2  
**L2 selected-identity candidate tooling:** #221 merged; trusted real capture remains open  
**Primary execution program:** GDSpaces Layer 1 final same-lineage acceptance + evidence-driven L2/L3 support  
**Overall status:** L1 INTERNAL PRODUCT PATH CLOSED; L2 STATIC/TOOLING ADVANCED; subsystem remains NOT COMPLETE pending real-retail/protected-process receipts.

## Authority split

- GitHub `main` is canonical implementation truth.
- Reverse claims remain bounded to their recorded artifact/range/scope.
- Synthetic/public CI proves product/tool behavior only.
- A real-device member receipt proves only the exact local snapshot/member/materialization scope encoded in that receipt.
- Original-game equivalence requires original-process evidence.
- GDSpaces owns product resource identity/materialization/authoring; recovered original functions do not move into GDSpaces.
- Canonical analysis executable authority and protected original-execution authority are separate and must not be silently substituted.
- Independent receipts must not be composed by matching filenames alone; the final vertical proof requires one reconciled source/request lineage.

## GDSpaces layer model

- **L1 — Resource Materialization:** exact bytes, transform/decompression, nested expansion, bounded authoring, rebuild/repack, reopen/rematerialization.
- **L2 — Resource Resolution:** request, candidates, normalization, provider/volume/source identity, mount success/failure, fallback/ambiguity.
- **L3 — Original Runtime/Lifecycle:** FileSlot/async/LoadedResource/typed-ready/claim/reset/release/consumer behavior.
- **V/LV — Validation:** binds selected identity, materialized byte identity and original consumer observation into one acceptance run.

Execution follows the dependency-driven [master roadmap](../gdspaces/master-roadmap.md).

## L1 current state

Canonical L1 implementation includes:

- NBZ classic ZIP bounded indexing/materialization;
- STORE + raw-DEFLATE method 8;
- CRC/size/SHA/ByteProvenance;
- artifact-bound archive/member observations;
- recovered numbered-volume first-gap/runtime-domain behavior for the currently accepted clean product path;
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
- [2026-08-26 real-device member evidence reconciliation](../gdspaces/l1-real-device-member-evidence-reconciliation-2026-08-26.md)
- [Level-E operator runbook](../gdspaces/l1-level-e-runbook.md)

## L1 evidence/access correction — 2026-08-26

The protected artifacts are locatable in connected Drive evidence:

- protected `dmc3.exe` is locatable;
- executable-relative `data/dmc3/dmc3-0.nbz` is locatable;
- observed `dmc3-0.nbz` size is `960,358,951` bytes.

The full NBZ cannot currently be materialized into the connected execution container through Drive/Files because the observed transfer/materialization ceiling is `268,435,456` bytes. This is a **transport blocker**, not artifact absence.

Pocket GDS / GDSpace Manager PR #2 adds an out-of-band real-device evidence seam over the canonical mobile GDSpaces materialization path. Once run against the actual NBZ, `gdspaces.l1.member-acquisition-receipt.v1` can record archive SHA/size, ResourceIdentity, ByteProvenance, exact exported member SHA/size, representation class and producer/core provenance without embedding proprietary bytes.

A Pocket receipt does not by itself prove protected original-process resolver selection or consumption.

## L1 mandatory remaining work

No known mandatory **internal implementation** blocker remains for the current representative DMC3-HD L1 acceptance scope.

The remaining gates are real evidence executions:

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
2. **Pocket real-device sub-receipt:** useful when the 960 MB NBZ is already local on the phone. It can close the exact member-byte/materialization sub-gate and supply L1-D classification evidence, but still needs selected-source/protected authority binding before final promotion.

## Bounded open reverse breadth — not automatic L1 blockers

The following remain real research gaps but only block L1 if the chosen acceptance path depends on them:

- complete `0x140328540` ZIP stream initializer lifetime;
- complete `0x140328FE0` compressed seek/reset/reinflate behavior;
- exhaustive malformed/partial-read original error equivalence;
- dynamic `.lst` allocation/free/error/cycle semantics and real loose-list corpus validation;
- unsupported/evidence-absent binary backends or formats.

Binary AFS and original-runtime PACK remain frozen absent direct evidence. Capcom offline writer equivalence is not an L1 requirement.

## L2 current frontier

Closed/integrated internal L2/tooling slices:

- #215/#204: canonical type-0 physical-provider static reverse, direct native physical product path and controlled receipts;
- #219: first bounded explicit-PID protected-runtime RVA acquisition/mapping tooling;
- #221: selected-provider content-candidate contract, strict normalizer/validator and artifact binder; candidate tooling only, not trusted original-process evidence;
- #235: canonical-artifact-bound **R2B v2** tooling with PID + process-creation FILETIME + module identity from one process instance and seven mandatory bootstrap/resolver anchors;
- #228: materialization-completion dependency boundary reconciliation without moving L2/L3 ownership into L1.

Fresh #235 reverse also corrected an important topology assumption: archive **filename discovery** and **successful mount topology** are distinct. Original bootstrap can continue discovery after an existing archive fails mount initialization. Clean success still yields higher successful volume -> lower successful volume -> physical, but a sparse successful-mounted set is possible. Product correction is tracked separately in open issue #237 and must not be laundered into an L1 completion claim.

The remaining L2 closure is split into independent real-evidence gates:

1. **real-retail `0x0E` collision census** — exact archive-bound member-name/central-directory evidence is still required; recovered qsort/bsearch uses normalized-string-only comparison with no equal-key secondary tie-break;
2. **real protected-process R2B v2 mapping receipt** — #235 tooling is integrated, but no real protected-process seven-anchor packet is canonical yet;
3. **successful-mount topology correction** — #237 remains open so product resolver state does not infer successful mounts from mere pre-gap filename presence in failure cases;
4. **R3 trusted selected-provider identity** — #221 tooling is merged, but trusted capture origin, real R2B v2 mapping and zero-loss real trace remain required;
5. **final L2 audit** — only after retail corpus + real R2B/R3 receipts, topology reconciliation and exact-head validation agree.

Fresh canonical EXE review also preserves this failure boundary: an archive normalized lookup hit can fail during wrapper/open creation (`0x140328290`), and `0x140327430` then exits through null/cleanup instead of treating that as a lower-volume miss. Clean-path R3 accepts only `miss -> selected`; provider/backend failure is fail-closed.

Authority identities:

- canonical analysis executable: `e454272ed0fb0247fcbcf300e5d55d7a3e96d50b89b9ffaff81bb978dcbdd082`, 6,356,432 bytes;
- protected distribution/original execution candidate: `81c7e61983564113b5105e931d9f185accc14e44ae147d27f720c2d50935c7d6`, 6,567,320 bytes.

The protected build is not instruction-reverse authority. Canonical analysis VAs/RVAs cannot be promoted into the protected process without independent R2B evidence.

L2 work may support L1 but must not replace the final L1 acceptance run.

## L3 current frontier

The static LoadedResource/typed-ready/lifetime spine is strong. Exact writer ownership/ordering and broader dynamic lifecycle receipts remain open.

Current boundary for the first vertical proof is:

```text
exact L2 selected identity
 -> exact L1 materialized byte identity
 -> L3 acquisition/state1
 -> completion/state2
 -> typed-ready/state3
 -> deterministic consumer effect
 -> rollback
```

No generic child/outstanding-work fan-in counter is currently evidenced and none should be invented to simplify instrumentation.

Broader L3 completion is separate from the minimum original-consumer evidence required for L1 acceptance.

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
2. close product topology correction #237 without weakening clean-path precedence;
3. run #235 R2B v2 tooling against the exact protected process and produce all seven same-process-instance anchor receipts;
4. use merged #221 only with a trusted publisher to capture a zero-loss R3 selected-identity trace;
5. bind observer artifact + exact successfully mounted numbered NBZ artifacts and preserve selected identity without treating provider/backend failure as a miss;
6. compare product resolution only after trusted origin is established, keeping product and original evidence classes separate;
7. reconcile code/docs/evidence and run final L2 audit.

No synthetic-only feature should displace the real evidence sequence unless a real run reveals a concrete missing dependency.

## Environment boundary

Connected artifact discovery is no longer the primary L1 blocker: the protected EXE and 960,358,951-byte `dmc3-0.nbz` are locatable. The connected raw-transfer path cannot ingest that NBZ because of the observed 268,435,456-byte ceiling.

Pocket GDS can execute member materialization where the archive is already local on-device. The protected game PC/process is still required for selected-source/original-process evidence that cannot be established by the mobile receipt and for #209 consumption/rollback.

Synthetic CI must not substitute for those receipts.

## Navigation

- [Canonical L1 roadmap](../gdspaces/l1-roadmap.md)
- [Final pre-Level-E L1 audit](../gdspaces/l1-final-audit-2026-08-25.md)
- [Real-device member evidence reconciliation](../gdspaces/l1-real-device-member-evidence-reconciliation-2026-08-26.md)
- [Level-E operator runbook](../gdspaces/l1-level-e-runbook.md)
- [Three-layer master roadmap](../gdspaces/master-roadmap.md)
- [L2 EXE reconciliation checkpoint](../gdspaces/l2-exe-reconciliation-2026-08-26.md)
- [L2 selected-identity runbook](../gdspaces/l2-original-selected-identity-runbook-2026-08-26.md)
- [Blockers](blockers.md)
- [Machine-readable status](canonical-status.json)
- [GDSpaces contract](../gdspaces-contract.md)

No percentage or implementation milestone overrides the gate-based completion rule.
