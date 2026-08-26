# GDSpaces V / LV Validation Roadmap

**Snapshot:** 2026-08-26  
**Architecture:** `validation-equivalence-architecture.md`  
**Base:** `main@c20544cfb7f3ddba69a128a88246550a35eb51c1`

## Goal

Centralize validation/equivalence for L1/L2/L3 under one V authority while keeping LV as the live/original-process evidence-acquisition plane.

This roadmap does not replace L1/L2/L3 roadmaps. It owns the validation gates that those roadmaps depend on.

## V tracks

### V-A — Evidence integrity

Current strengths:

- exact artifact SHA/size binding;
- canonical analysis vs protected execution authority split;
- provenance receipts and child-receipt hash binding;
- fail-closed acquisition tooling;
- non-forgeable promotion direction in L3 trace tooling;
- explicit product-vs-original evidence separation.

Remaining work:

1. define one trusted LV publisher/binder model shared by L2/L3 original-process traces;
2. bind every promoted live trace to observer version/configuration and exact process/module identity;
3. define parent `validation_run_id` and child-receipt hash composition;
4. reject cross-run composition by default;
5. define contradiction/supersession behavior for accepted receipts.

### V-B — Product validation

Keep existing Windows/Ubuntu CI, synthetic tests, parser/writer tests and fail-closed regressions.

Rules:

- exact-head CI proves only the tested branch/product scope;
- synthetic fixtures never satisfy V-D;
- product hardening is not original behavior unless separately evidenced.

### V-C — Real-corpus validation

Required corpus receipts are derived from layer scope.

Current important receipts/gaps include:

- PAC real-corpus parser/materialization coverage;
- PNST real-corpus coverage where still required by declared scope;
- exact retail DMC3 member/central-directory evidence for L2 collision census;
- one exact protected-retail L1 edit/rebuild/rematerialization receipt.

Every V-C receipt must carry exact artifact identity and declared scope.

### LV — live/original-process acquisition

#### LV-L2

Current chain:

```text
#219 runtime RVA acquisition/mapping tooling
 -> real protected-process multi-anchor mapping receipt
 -> #220/#221 original resolver selection observation
```

Required output:

- exact original logical request;
- ordered provider/candidate probes;
- selected provider/source/volume/member;
- exact protected executable identity;
- mapping receipt binding;
- observer integrity markers.

#### LV-L3

Current chain:

```text
#88 remaining static writer/dispatcher/shared-owner closure
 -> trusted lifecycle observer/publisher
 -> #217/#218 V1-V6 live traces
 -> V7 breadth aggregation
```

Required live event vocabulary includes state writes, materialization completion, typed post-load, ready callback, consumer visibility, claims/releases, cancellation/reset and shutdown boundaries.

#### LV-L1

L1 generally obtains exact bytes through product-side artifact-bound acquisition. LV is required where original-process consumption/selection attribution cannot be established from deterministic external effect alone.

Issue #209 remains the immediate original-game L1 acceptance scenario.

### V-D — Original-process equivalence

#### V:L1

Acceptance chain:

```text
exact protected installation
 -> direct-retail acquisition/provenance
 -> supported edit/rebuild/rematerialization
 -> exact authored overlay publication
 -> original game requests/consumes authored identity
 -> deterministic effect or trusted consumer observation
 -> rollback / retail immutability
```

Primary current gate: #209.

#### V:L2

Acceptance chain:

```text
accepted protected-runtime mapping
 -> trusted original resolver observation
 -> exact selected provider/volume/member
 -> comparison with GDSpaces selection
 -> accepted V:L2 receipt
```

Primary current gates: #220/#221 plus real retail `0x0E` collision evidence.

#### V:L3

Acceptance chain:

```text
same selected/materialized identity
 -> original state/lifecycle trace
 -> typed-ready / state-3 consumer visibility
 -> release/transition/cancellation/reset/shutdown breadth
```

Primary current gates: #88 static closure + #217/#218 dynamic program.

### V-E — Cross-layer and breadth acceptance

This is the missing subsystem-level gate.

Required first vertical receipt:

```text
ONE validation_run_id
 -> exact protected EXE
 -> accepted LV mapping/observer authority
 -> [L2] original selected provider/volume/member
 -> [L1] exact materialized bytes
 -> [L1] authored/rebuilt/rematerialized bytes
 -> [L3] original consumer/lifecycle observation
 -> deterministic effect
 -> rollback
```

The parent V validator must prove that all child receipts belong to the same run/resource chain.

After one vertical receipt, broaden across the families/scenarios required by the declared completion scope.

## Immediate implementation order

```text
V-1 canonical architecture + roadmap
 -> V-2 machine status ownership update
 -> V-3 create V parent execution ledger
 -> V-4 define validation_run_id + cross-receipt binding contract
 -> V-5 reconcile #209/#220/#217 as V-owned gates
 -> V-6 trusted LV publisher/binder shared contract
 -> V-7 produce first real LV-L2 mapping/selection receipt
 -> V-8 produce first real V:L1 consumption receipt
 -> V-9 bind same resource into V:L3 consumer/lifecycle receipt
 -> V-10 first cross-layer vertical V receipt
 -> V-11 breadth matrix
 -> V-12 final V audit / subsystem promotion
```

V-1 through V-6 are architecture/infrastructure work and must not be reported as original-process equivalence.

## Required parent-ledger fields

The future V execution ledger/validator must track at minimum:

- validation run ID;
- declared validation scope;
- exact executable authorities;
- runtime mapping authority when used;
- trusted LV observer identity/configuration;
- L1/L2/L3 child receipt hashes;
- logical request + selected identity;
- materialized/authored/rematerialized byte hashes;
- original consumer/lifecycle observation identity;
- effect result;
- rollback result;
- contradictions;
- evidence class V-A/V-B/V-C/V-D/V-E;
- promotion verdict and scope.

## Completion authority

Layer roadmaps may say their implementation/reverse work is internally closed, but only V may promote:

- `original-equivalent`;
- `game-ready-equivalent`;
- `COMPLETE`;
- `100%` when used as a completion claim.

The final project-wide GDS acceptance therefore depends on V, not on three unrelated layer-local PASS flags.
