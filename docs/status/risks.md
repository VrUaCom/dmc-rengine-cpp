# Architecture and Project Risks

**Snapshot:** 2026-09-02  
**Reviewed base:** `main@9483663959e5452f9a224c1535445bb5a3b33520`

## R-001 — Second resolver/materializer/lifecycle authority
**Severity:** critical

A downstream tool may independently reopen paths, rediscover resources, rematerialize bytes or invent lifecycle state.

**Mitigation:** L2 selects, L1 materializes/authors, L3 owns original runtime lifecycle. Stage Ops/ModViz/editors consume these authorities rather than duplicating them.

## R-002 — Layer completion laundering
**Severity:** critical

A mature parser/writer/resolver, green synthetic CI, a preview or a crash-free launch may be reported as layer completion.

**Mitigation:** every layer uses mandatory gate-based completion and a contradiction-free final audit. L1/L2/L3 are all currently incomplete.

## R-003 — Branch truth reported as merged truth
**Severity:** critical

Stronger but open PR research may be described as current-main implementation/evidence.

**Mitigation:** distinguish merged canonical, current reconciliation proposal, stronger-unmerged evidence and implementation candidate. Semantic-port old branches onto current main before promotion.

## R-004 — Discovery conflated with successful runtime mount
**Severity:** critical

Filename discovery/registration attempts may be treated as proof of successful linked provider topology.

**Mitigation:** port the #246 distinction into product code: discovery/attempt plan is separate from explicit successful mount topology; resolver traverses successful mounts only.

## R-005 — Resolver lookup hit conflated with usable provider selection
**Severity:** critical

An archive normalized-key hit may later fail wrapper/backend/open creation and be rewritten as a clean miss.

**Mitigation:** provider/backend failure is terminal/fail-closed for the bounded recovered path; do not continue lower precedence as if the key were absent.

## R-006 — Materialization success boolean treated as exact-all-bytes receipt
**Severity:** critical

Coarse original booleans may hide child enqueue/writer failure, final completion admission failure, short-success or width/wrap behavior.

**Mitigation:** product exactness uses explicit byte/provenance validation. Original-L1 reverse remains open where these semantics affect the claimed scope.

## R-007 — Unsafe original behavior copied into product policy
**Severity:** critical

Original 32-bit wrap, short-success or failure-swallowing behavior may be reproduced for “parity”.

**Mitigation:** preserve original behavior as evidence metadata while keeping product overflow checks, exact-byte receipts and fail-closed publication stricter.

## R-008 — Transport completion conflated with LoadedResource state2
**Severity:** critical

Lower whole-file transport/result callbacks may be labeled as `LoadedResource state 1 -> 2` completion.

**Mitigation:** keep L1 byte/result semantics distinct from L3 normal completion publication at `0x1401B8DC0`.

## R-009 — FIFO-only proof overclaim
**Severity:** high

Queue order alone may be treated as proof of terminal byte completion before state2.

**Mitigation:** use direct persistence/retirement evidence. Stronger static evidence reports pending/retry jobs remain current and status3 retires; dynamic cancellation/concurrency still requires L3 evidence.

## R-010 — Reopening L3-R1 without provenance
**Severity:** high

New numeric `+0x04` writes in unrelated objects may restart broad state-writer hunting.

**Mitigation:** R1 is bounded-closed/contradiction-gated. Reopen only with exact LoadedResource record provenance.

## R-011 — Universal runtime type detector regression
**Severity:** critical

Registry identity, container post-load dispatch, family-mask identity and parser/geometry semantics may collapse into one enum/detector.

**Mitigation:** preserve separate evidence sites:

```text
0x1402DB1F0 registry probe
0x1401B9FA0 container dispatcher
0x1402FD650 family-mask classifier
```

Handler existence, semantic format, geometry capability and writer maturity are independent claims.

## R-012 — Universal LoadedResource refcount overclaim
**Severity:** high

Bounded loader-node claim evidence may be generalized to all resource families.

**Mitigation:** R2/R4 must preserve fixed-family groups, dynamic group 5, loader nodes and specialized managers separately.

## R-013 — Naming evidence promoted to identity/write authority
**Severity:** critical

`.index`, embedded aliases, semantic suffixes or display names may retarget materialization/writes.

**Mitigation:** exact ResourceId/physical slot/bytes remain identity/write authority. Extracted ordinal and names are separate sealed evidence domains.

## R-014 — `.index` promoted to runtime manifest
**Severity:** high

Historical extraction metadata may be used as original lookup authority.

**Mitigation:** `.index` remains extraction/naming evidence unless an independent original-runtime consumer is directly proven.

## R-015 — Artifact snapshot split
**Severity:** high

Archive index, member bytes and archive hash may be observed from different file states.

**Mitigation:** provenance-grade receipts bind all observations to one stable artifact identity.

## R-016 — Publication clobber / measured-tree mutation
**Severity:** critical

Generated output may overwrite existing artifacts or alter the retail tree being measured.

**Mitigation:** staged validation + atomic no-replace publication; keep measurement sources immutable and generated outputs separate.

## R-017 — Retail representation laundering
**Severity:** critical

Transformed/historical corpus files may be treated as pristine retail representation authority.

**Mitigation:** acquire exact real selected bytes first, then classify; activate a writer only inside its evidenced representation domain.

## R-018 — Parent extent treated as intrinsic child EOF
**Severity:** critical

Container slot spans may include padding/alignment and be promoted as intrinsic child size.

**Mitigation:** require independent framing/complete-image evidence for intrinsic child extent or keep parent span authority explicitly separate.

## R-019 — L3 validator authority laundering
**Severity:** critical

A structurally valid JSON trace may claim `original_process=true` and become promotion evidence.

**Mitigation:** current-main L3 validator/publisher design must make trusted origin non-forgeable by editable content and bind the active process instance plus exact observer/config identity.

## R-020 — Dynamic cancellation inferred from static normal completion
**Severity:** critical

The narrowed admitted type-2 → type-3 normal path may be extrapolated to in-flight cancellation/concurrency.

**Mitigation:** keep V5 early in dynamic validation; static normal FIFO evidence does not close current-slot cancellation timing.

## R-021 — State3 overpromotion
**Severity:** high

Manager state3 may be reported as universal format-semantic success or observable gameplay consumption.

**Mitigation:** preserve:

```text
manager state3
!= universal family semantic ready
!= consumer-visible effect receipt
```

## R-022 — Original runtime ownership leakage into GDSpaces product core
**Severity:** high

Recovered LoadedResource/FileSlot/cache lifetime code may be moved into generic product materialization modules.

**Mitigation:** original runtime reconstruction remains a recovered-runtime/profile domain. Product GDSpaces consumes bounded contracts rather than cloning original ownership structure blindly.

## R-023 — AFS/PACK authority inference
**Severity:** high

`.afs/` logical namespaces or historical product PACK parsers may be promoted to original binary backend authority.

**Mitigation:** keep binary AFS/PACK evidence-gated until direct original-runtime/retail evidence activates them.

## R-024 — Recovered C++ false confidence
**Severity:** high

Readable/compiling recovered code can hide ABI, field ownership or timing errors.

**Mitigation:** retain exact binary evidence, contradiction tracking and original-vs-reconstruction behavioral receipts.

## R-025 — Historical status drift
**Severity:** high

Old snapshots can continue to advertise obsolete bases, open R1 work or old global type claims.

**Mitigation:** current roadmaps/status files carry 2026-09-02 truth; historical audits remain chronology and must be read through explicit current reconciliation documents.

## R-026 — Public repository contamination
**Severity:** high

Proprietary game bytes or leaked source could be committed.

**Mitigation:** commit only lawful/sanitized evidence metadata, hashes, synthetic fixtures and original project code. Keep proprietary analysis artifacts local where redistribution is not allowed.
