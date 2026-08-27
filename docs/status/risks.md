# Architecture and Project Risks

**Snapshot date:** 2026-08-27  
**Canonical base reviewed:** `main@f886f27e62ec9a05b6829df7fd074981a06a4b49`  
**Boundary authority:** `../gdspaces/layer-boundary-status-reconciliation-2026-08-27.md`

## R-001 — Second resolver/materializer regression
**Severity:** critical

A tool or Stage subsystem may reopen paths, parse archives or rediscover resources independently.

**Mitigation:** GDSpaces remains the only product resource resolver/materializer. Stage Ops/ModViz/Binary Inspector consume shared identities and payloads.

## R-002 — False L1 completion from product maturity
**Severity:** critical

Advanced product authoring, green synthetic tests, successful resolver composition or a future successful representative Level-E run may be reported as exhaustive Layer-1 reverse completion.

**Mitigation:** keep three independent statuses: product implementation maturity, original EXE materialization reverse completeness, and real acceptance. Current L1 is `INCOMPLETE / NOT 100%`; original materialization reverse is `NOT EXHAUSTIVE`.

## R-003 — Artifact snapshot split regression
**Severity:** high

Archive index metadata, member bytes and archive SHA may be observed from different physical states.

**Mitigation:** preserve artifact-bound observations and bind provenance to one stable artifact identity.

## R-004 — No-clobber publication regression
**Severity:** high

New authoring/evidence commands may bypass atomic no-replace publication.

**Mitigation:** staged validation + final no-replace publication remains mandatory.

## R-005 — Acquisition mutates measured retail tree
**Severity:** critical

Evidence tooling may change the retail tree whose identity it is measuring.

**Mitigation:** fail closed for measured source trees and keep generated artifacts in explicit output locations.

## R-006 — Closed L2 boundaries accidentally reopened
**Severity:** high

Already bounded request/provider/normalization work may be restarted instead of current evidence gates.

**Mitigation:** keep numbered-volume/bootstrap, request candidates, normalization/index and bounded type-0 provider chain closed unless contradictory evidence appears.

## R-007 — Materializer/repacker authority collapse
**Severity:** high

Original read/materialization behavior may be treated as proof of Capcom offline-writer behavior.

**Mitigation:** separate original reader/materializer evidence, DMC Rengine writers and offline-writer equivalence claims.

## R-008 — Retail representation laundering
**Severity:** critical

Historical/transformed corpus bytes may be treated as pristine retail authority.

**Mitigation:** obtain exact retail provenance first and classify representation before authoring.

## R-009 — Pre-guessed member identity
**Severity:** high

A path may be hard-coded instead of allowing L2 to establish the actual winning member.

**Mitigation:** acquisition begins from the logical game request and records the actual selected identity.

## R-010 — Inferred slot extent treated as intrinsic child EOF
**Severity:** critical

Next-greater-relative-offset ranges may contain padding and are not universal original intrinsic child-size authority.

**Mitigation:** keep layout-preserving patch scope separate from synthesized/reflowed writer scope unless independent intrinsic framing is proven.

## R-011 — Recovered C++ false confidence
**Severity:** high

Readable/compiling recovered code may hide ABI, ownership or lifecycle errors.

**Mitigation:** bind claims to exact artifact/range/scope and preserve contradiction tracking.

## R-012 — Layer ownership bleed between L1/L2/L3
**Severity:** critical

A helper may be classified wholesale by subsystem name/address. Two opposite errors are possible: putting all FileSlot/AsyncIO under L3, or moving all scheduler/completion machinery into L1.

**Mitigation:** classify behavior, not helper:

```text
selection / usable identity -> L2
size / exact byte extent / capacity / transform / materializer result -> L1
request / queue / callback / LoadedResource state lifecycle -> L3
L1 terminal result -> L3 completion eligibility -> cross-layer seam
```

Normal `state1 -> state2` remains L3.

## R-013 — AFS/PACK inference from names/history
**Severity:** high

Logical `.afs/` strings or historical PACK parsers may be promoted as original runtime authority.

**Mitigation:** freeze absent direct evidence.

## R-014 — Stage/domain truth collapse
**Severity:** high

Stage semantic state may be confused with resource-runtime authority.

**Mitigation:** Stage Ops/ModViz remain downstream DOMAIN consumers and cannot define L1/L2/L3 completion.

## R-015 — Active branch truth reported as main truth
**Severity:** high

Open PR findings may be described as canonical implementation before merge.

**Mitigation:** active PRs remain branch truth until merged; status records exact main SHA.

## R-016 — Historical status drift
**Severity:** high

Older audits may preserve superseded completion labels or layer ownership.

**Mitigation:** current canonical docs point to the 2026-08-27 reconciliation; historical findings remain evidence with supersession notes.

## R-017 — Public repository contamination
**Severity:** high

Proprietary game bytes or leaked source may be committed.

**Mitigation:** public-safe fixtures and sanitized receipts only.

## R-018 — Premature retail mutation
**Severity:** high

Safe authoring may evolve into implicit mutation of originals.

**Mitigation:** WorkingCopy/generated outputs, guarded publication and rollback; retail artifacts immutable by default.

## R-019 — Lower transport result conflated with LoadedResource completion
**Severity:** critical

`0x1400335A0` lower transfer result may be treated as equivalent to normal `0x1401B8DC0` state2 publication.

**Mitigation:** keep them distinct across the seam. Byte/result semantics around `0x1400335A0` are L1-relevant; `0x1401B8DC0` scheduler callback/state publication is L3. The dependency between them remains open.

## R-020 — Generic fan-in counter overclaim
**Severity:** critical

“fan-in/completion” may be read as proof of a universal outstanding-child counter.

**Mitigation:** no generic fan-in counter is promoted without direct evidence.

## R-021 — FIFO-only completion proof
**Severity:** critical

Queue insertion order may be treated as enough to prove L1 work is terminal before L3 state2 publication.

**Mitigation:** require evidence of completion-aware persistence, callback-driven terminal state, another gate, synchronous behavior or equivalent fail-closed mechanism.

## R-022 — Historical helper-role laundering
**Severity:** high

Historical roles for `0x1400333E0` / `0x140033390` may be treated as fresh canonical semantics.

**Mitigation:** keep them reacquisition anchors until exact canonical bytes confirm them.

## R-023 — `0x1402EF460` mislabeled as OS cancellation
**Severity:** high

Pending scheduler-entry clear/rollback may be described as `CancelIo` without evidence.

**Mitigation:** preserve the bounded label and classify its concrete target/action only after reverse.

## R-024 — Scheduler seam displaces direct L1 byte reverse
**Severity:** critical

The completion bridge may become the sole focus while materialized-size, final-chunk, capacity, `.lst` padding/planner and partial-read semantics remain unresolved.

**Mitigation:** PR #244 order is canonical for the next L1 pass: close direct byte-exactness first, then reconcile the L1-terminal/L3-completion seam.

## R-025 — Product zero-fill promoted as original padding equivalence
**Severity:** high

DMC Rengine may intentionally zero synthesized alignment gaps, but original runtime allocation/write behavior is not yet proven to do the same.

**Mitigation:** recover allocation initialization and `.lst` writer behavior before claiming byte-equivalent original padding.

## R-026 — Rounded transfer overruns logical byte authority
**Severity:** critical

`ceil(totalBytes/0x800)` may request more bytes than the logical resource size. Without the exact clamp, destination capacity and terminal semantics, exhaustive byte correctness cannot be claimed.

**Mitigation:** close final-chunk/EOF/short-read behavior and relation between logical size, requested bytes and allocated capacity.

## R-027 — Size sentinel/zero semantics assumed
**Severity:** high

Logical size, missing/error sentinel and valid zero-size resource may be conflated.

**Mitigation:** recover `0x14002F9F0 -> 0x140048E20` exact ABI/domain before promotion.

## R-028 — `.lst` planner/writer mismatch hidden by structural grammar
**Severity:** high

Known grammar/layout may mask divergence between planned capacity and actual child writing/failure handling.

**Mitigation:** reconcile `0x1401B7FD0` with `0x1401B85C0`, including recursive size, child failures, cleanup and partial-image state.

## R-029 — `internal product path closed` used as completion proxy
**Severity:** critical

A bounded implementation statement may be interpreted as `L1 COMPLETE` or “only external receipts remain.”

**Mitigation:** current allowed labels are `L1 INCOMPLETE / NOT 100%`, `product implementation advanced`, and `original EXE materialization reverse NOT EXHAUSTIVE`.