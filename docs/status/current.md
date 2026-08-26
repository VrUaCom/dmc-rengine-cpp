# Current Project Status

**Snapshot date:** 2026-08-26  
**Canonical implementation base:** `main@eb701b9c523a3ec87f3c73bb8764038f1f2ef8dc`  
**Latest L2 promotion:** PR #219 — protected-runtime mapping acquisition tooling  
**Active L2 evidence slice:** PR #221 — selected-identity content-candidate contract  
**Latest L3 static reconciliation:** PR #224 — canonical raw-EXE L3 boundary  
**Active EXE boundary audit:** #225 / PR #226  
**Primary execution program:** GDSpaces final real-retail/original-process acceptance with evidence-driven L2/L3 support  
**Overall status:** L1 INTERNAL PRODUCT PATH CLOSED; L2/L3 STATIC SPINES ADVANCED; subsystem remains NOT COMPLETE pending real-retail/original-process validation.

## Authority split

- GitHub `main` is canonical implementation truth.
- Reverse claims remain bounded to their recorded artifact/range/scope.
- Synthetic/public CI proves product/tool behavior only.
- Original-game equivalence requires original-process evidence.
- GDSpaces owns product resource identity/materialization/authoring; recovered original functions do not move into GDSpaces.
- Canonical analysis executable authority and protected original-execution authority are separate and must not be silently substituted.
- Closed/unmerged historical recovered-source PRs may guide reacquisition targets but do not silently become current-main canonical authority.

## GDSpaces layer model

- **L1 — Resource Materialization:** exact bytes, transform/decompression, nested expansion, bounded authoring, rebuild/repack, reopen/rematerialization.
- **L2 — Resource Resolution:** request, candidates, normalization, provider/volume/source identity, fallback/ambiguity.
- **L3 — Original Runtime/Lifecycle:** FileSlot/async/LoadedResource/typed-ready/claim/reset/release/consumer behavior.
- **V — Validation / Equivalence:** cross-cutting validation/promotion authority, not L4.
- **LV — Live Validation / Original-Process Observation:** V-owned acquisition plane, not L4.

Execution follows the dependency-driven [master roadmap](../gdspaces/master-roadmap.md).

## EXE grey-boundary model

The two-pass audit tracked in #225 / PR #226 finds **no evidence-based need for L4**.

The current architecture additionally recognizes an orthogonal **Resource Control Plane (RCP)**:

- root request planning/emission;
- dependency planning/emission;
- pending/ready coordination;
- loader claims/retention;
- transition cancellation/quiescence/replacement.

RCP orchestrates L2/L1/L3; it does not replace their ownership.

Useful L3 accounting subdomains:

- **L3A — Typed Construction / Dependency**;
- **L3B — Ownership / Lifecycle**.

They remain inside L3.

Supporting non-layer planes/tags:

- `TYPE/ID` — descriptor/type and cross-layer identity mappings;
- `RT-IO` — FileSlot/AsyncIO seam;
- `MEM/BACKING` — allocation/backing substrate;
- `BOOTSTRAP` — startup/service substrate;
- `ERROR` — per-owner failure/recovery matrix.

The runtime may recurse through dependencies:

```text
L3A dependency discovery
 -> RCP request emission
 -> L2 -> L1 -> L3A child processing
```

### Readiness correction

Current raw-EXE authority from #224 proves central unknown/default typed dispatch can no-op while the state2 finalizer still proceeds to callback/state3. Therefore status/evidence must distinguish:

```text
manager_ready_state3
!= universal family_semantic_ready
!= consumer_effect_observed
```

State3 is a strong manager/lifecycle readiness boundary, but not universal family-semantic success.

Canonical grey-boundary docs:

- `../gdspaces/exe-grey-boundary-audit-2026-08-26.md`;
- `../gdspaces/exe-grey-boundary-pass2-2026-08-26.md`;
- `../gdspaces/exe-grey-boundary-roadmap-2026-08-26.md`.

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
- product closure orchestration through exact authored rematerialization.

Canonical L1 review: [Final Pre-Level-E Audit](../gdspaces/l1-final-audit-2026-08-25.md).

## L1 mandatory remaining work

No known mandatory **internal implementation** blocker remains for the current representative DMC3-HD L1 acceptance scope.

The remaining gates require a real protected installation:

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

Issue #209 is the final original-game consumption/rollback gate for the current L1 representative acceptance path.

`obj\em000.pac` remains a high-value target, but the archive/member winner must be observed by the resolver and another representative resource may be used if it provides a stronger deterministic consumer effect.

## Bounded open reverse breadth — not automatic L1 blockers

The following remain real research gaps but only block L1 if the chosen acceptance path depends on them:

- complete `0x140328540` ZIP stream initializer lifetime;
- complete `0x140328FE0` compressed seek/reset/reinflate behavior;
- exhaustive malformed/partial-read original error equivalence;
- dynamic `.lst` allocation/free/error/cycle semantics and real loose-list corpus validation;
- unsupported/evidence-absent binary backends or formats.

Binary AFS and original-runtime PACK remain frozen absent direct evidence. Capcom offline writer equivalence is not an L1 requirement.

## L2 current frontier

The type-0 physical-provider static reverse and controlled product model are closed on `main` through #215/#204. Protected-runtime mapping acquisition tooling is merged through #219.

Current L2 closure gates:

1. **real-retail `0x0E` collision census** — exact DMC3 retail member-name/central-directory evidence still required;
2. **real protected-process RVA mapping packet** — tooling exists, real process evidence required;
3. **original-process selected-provider identity** — #220/#221, trusted capture still required;
4. **final L2 audit** — only after corpus + original-process receipts and exact-head validation agree.

Authority identities:

- canonical analysis executable: `e454272ed0fb0247fcbcf300e5d55d7a3e96d50b89b9ffaff81bb978dcbdd082`, 6,356,432 bytes;
- protected distribution/original execution candidate: `81c7e61983564113b5105e931d9f185accc14e44ae147d27f720c2d50935c7d6`, 6,567,320 bytes.

The protected build is not instruction-reverse authority. Canonical analysis VAs/RVAs cannot be promoted into the protected process without independent mapping evidence.

### Request-ingress grey edge

The canonical direct-call surface contains three direct `OpenGameResource` callers and all pass `flags=1`, but their complete upstream semantic/request-origin ABI is not yet classified.

#225 P2-R1 owns the supporting upstream caller census. This is not currently called L0.

## L3 current frontier

Merged #224 re-established the canonical raw-EXE L3 boundary from the exact analysis executable.

Strong current static authority includes:

- 363 x `0x48` LoadedResource registry and seven groups;
- central state `0 -> 1 -> 2 -> typed post-load -> callback -> 3` ordering;
- cancellation `1|2 -> 4` and quiescence `{0,3}`;
- distinct ordinary/cancel/forced-reset state-zero policies;
- known typed MOD/EFM/SCM/SHW/PNST boundaries;
- central unknown/default typed dispatch as best-effort/no-op;
- loader-node `(kind,id)` claims/release for the bounded gameplay path;
- runtime vs CRT vs process-lifetime distinctions.

Still open:

- alias-aware residual state-writer census;
- family-complete `+0x08/+0x18/+0x20/+0x28` ownership;
- external factory/dependency and SCM edges;
- shared-owner family breadth;
- allocation/profile differences;
- #217 original-process lifecycle receipts.

For the first vertical proof, L3 must provide enough original-process evidence to prove that the authored L1 bytes reached the intended consumer. State3 may be part of that chain but is not a universal family semantic-success verdict.

## EXE boundary / RCP current frontier

Parent #225 work order:

1. P2-R1 upstream request-origin census;
2. P2-R2 current raw StageCfg dependency-preload reacquisition;
3. P2-R3 Type/Descriptor identity xref;
4. P2-R4 factory/resource-set demand edges;
5. P2-R5 ownership hierarchy breadth;
6. P2-R6 manager-ready/family-ready/consumer-effect taxonomy;
7. P2-R7 dependency-aware LV/V integration after the static graph is bounded.

Historical Wave-3 PR #84 is a high-value target source, especially for StageCfg-driven enemy dependency preload and resource-set demand mapping, but remains historical/unmerged evidence until reconfirmed against current canonical authority.

## Current critical path

### L1 vertical acceptance

1. obtain access to a protected DMC3 installation;
2. run direct-retail acquisition and preserve provenance;
3. classify the exact retail representation;
4. perform one supported bounded real edit, top-level or nested;
5. run next-volume authoring + canonical rematerialization closure;
6. execute issue #209 original-game consumption + rollback;
7. run final L1 acceptance audit;
8. allow final completion promotion only through the canonical V authority once #222/#223 integration is promoted.

### L2 closure support

1. obtain cryptographically bound retail DMC3 member-list/central-directory evidence and run the `0x0E` collision census;
2. produce a real protected-process multi-anchor mapping packet through #219 tooling;
3. use only proven mapped anchors to capture original-process resolver selection identity;
4. reconcile code/docs/evidence and run final L2 audit.

### L3 / RCP supporting closure

1. finish narrow L3 static ownership/factory/dependency gaps rather than re-reversing the core state spine;
2. perform #225 P2-R1/P2-R2 where required to bind root/dependency request origins;
3. capture #217 lifecycle traces with exact L1/L2 identity;
4. distinguish manager-ready state from family-semantic/consumer success in validation.

No synthetic-only feature should displace the real evidence sequence unless a real run reveals a concrete missing dependency.

## Environment boundary

The currently connected automation environment does not expose all exact raw protected-install artifacts required for the real-retail/original-process runs. Synthetic CI must not substitute for those receipts.

## Navigation

- [Canonical L1 roadmap](../gdspaces/l1-roadmap.md)
- [Final pre-Level-E L1 audit](../gdspaces/l1-final-audit-2026-08-25.md)
- [Three-layer master roadmap](../gdspaces/master-roadmap.md)
- [EXE grey-boundary Pass 1](../gdspaces/exe-grey-boundary-audit-2026-08-26.md)
- [EXE grey-boundary Pass 2](../gdspaces/exe-grey-boundary-pass2-2026-08-26.md)
- [EXE grey-boundary reverse roadmap](../gdspaces/exe-grey-boundary-roadmap-2026-08-26.md)
- [Blockers](blockers.md)
- [Machine-readable status](canonical-status.json)
- [GDSpaces contract](../gdspaces-contract.md)

No percentage or implementation milestone overrides the gate-based/V-owned completion rule.
