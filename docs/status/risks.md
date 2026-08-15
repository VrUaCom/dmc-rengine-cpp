# Architecture and Project Risks

**Snapshot date:** 2026-08-15  
**Primary policy:** `completion-and-evidence-policy.md`

## R-001 — Completion inflation

**Severity:** critical

A bounded implemented/tested/EXE-confirmed slice may be described as a complete subsystem.

Examples of invalid promotion:

- green CI -> original-game equivalence;
- compiled recovered C++ -> original Capcom source;
- StageBundle materialization -> vanilla state-3/game-ready object;
- closed wrapper ABI -> whole collision system complete;
- deterministic DMC Rengine writer -> Capcom offline-builder equivalence.

Mitigation:

- mandatory completion policy;
- separate bounded-slice status from subsystem status;
- require representative behavioral ValidationReceipts before equivalence claims;
- keep unresolved lifecycle/ownership/semantic gaps explicit.

## R-002 — Stale evidence/status surfaces

**Severity:** critical

Issue bodies, historical pass documents, `main`, active stacked PRs and Drive research can disagree.

Observed current example: older HITS Pass-8/Pass-9 coordination text still listed top-level `0x14005E7A0/FEC0/601E0` contracts as wholly open after later PR #85 Pass-10 bounded closure/reclassification.

Mitigation:

- maintain one current status snapshot and machine-readable status;
- add supersession/correction notices instead of erasing history;
- update canonical coordination issue bodies when their current-target list changes;
- never treat historical pass documents as latest authority by date-independent reading.

## R-003 — Second resolver / duplicated authority

**Severity:** critical

Stage Ops, ModViz, Semantic Graph, Binary Inspector or another tool may resolve resources independently for convenience.

Mitigation:

- GDSpaces remains the only product resource resolver/materializer;
- consumers receive stable resource identities, payloads, Stage Ops state or typed projections;
- recovered original runtime behavior remains in recovered-game, not product tools.

## R-004 — Ownership collapse

**Severity:** high

Recovered DMC3 runtime code may be moved into GDSpaces/Stage Ops/ModViz, or Semantic Graph may become scene assembly authority.

Mitigation:

- GDSpaces: resource authority;
- Recovered Game Source Tree: original runtime reconstruction;
- Reverse Core: generic evidence/reconstruction infrastructure;
- Stage Ops: product-side scene assembly/operations;
- Semantic Graph: derived semantic/evidence representation;
- ModViz: editor consumer;
- Binary Inspector/EXE Editor: inspection/reconstruction tooling.

## R-005 — Stage identity collapse

**Severity:** high

`st001`, `stNNN` filenames, descriptor rows, numeric selectors and semantic gameplay stages may be conflated.

Mitigation:

- keep resource-set/catalog, numeric selector and semantic gameplay identities separate;
- treat `st001` only as regression/compatibility data;
- never infer gameplay semantics from filename patterns;
- never equate 189 observed descriptors with 189 gameplay stages.

## R-006 — Decompiled-code false confidence

**Severity:** high

Readable recovered C++ can hide incorrect ABI, ownership, lifetime, side effects or incomplete behavior.

Mitigation:

- evidence-linked recovered identities;
- status per unit: direct reconstructed / corpus-pending / executable candidate / research required;
- compile isolation;
- controlled behavioral comparison;
- correction/rejection history;
- no subsystem equivalence claim without ValidationReceipt.

## R-007 — Synthetic-test overclaim

**Severity:** high

Synthetic tests validate implementation contracts but may be mistaken for real-game compatibility.

Mitigation:

- label synthetic CI separately from corpus/game validation;
- require representative real-resource receipts where compatibility is claimed;
- require runtime observations for lifecycle/gameplay equivalence.

## R-008 — Raw-artifact absence hidden by summaries

**Severity:** high

A pass may appear to perform new byte-level reverse while actually relying only on prior summaries/evidence packets.

Mitigation:

- record whether raw canonical `dmc3.exe`/resource bytes were actually mounted;
- distinguish fresh direct re-measurement from project-evidence confirmation;
- require hash-gated raw reacquisition or sanitized disassembly/xref packets for genuinely new byte-level claims.

## R-009 — HITS wrapper closure extrapolated to collision completion

**Severity:** high

Later Pass-10 closure of top-level wrapper contracts may be interpreted as complete collision reconstruction.

Mitigation:

- keep deeper primitive producers/helpers, descriptor writers, source2 backing/lifetime, remaining component semantics and runtime comparison explicitly open;
- preserve `BOUNDED CLOSED` for wrapper targets rather than `COMPLETE` for the subsystem.

## R-010 — HITS original-builder overclaim

**Severity:** high

The deterministic DMC Rengine HITS writer may be presented as Capcom's original builder.

Mitigation:

- call it a DMC Rengine deterministic writer;
- preserve original-builder equivalence as `NOT PROVEN`;
- require real corpus differential evidence and controlled game-runtime validation.

## R-011 — Rejected HITS model regression

**Severity:** high

Stale documents may reintroduce `HITS$` or `0x18060001` as a universal record marker.

Mitigation:

- current classifier/format authority uses four-byte `HITS` and header-driven structure;
- raw flag values remain raw unless separately evidenced;
- correction/supersession records remain visible.

## R-012 — Resource materialization promoted to game-ready

**Severity:** critical

Product bytes/container expansion may be mistaken for original DMC3 state-3 runtime objects.

Mitigation:

- keep materialized vs typed-postload vs factory/consumer/cache/lifetime states distinct;
- `game_ready_equivalent=false` until recovered-runtime evidence and validation close the boundary.

## R-013 — Executable identity drift

**Severity:** high

One SHA may be paired with inconsistent sizes/metadata, or canonical VAs may be applied to a different build.

Mitigation:

- exact SHA-256, size and PE trust boundary;
- PR #92 correction for the stale `3,735,552` size paired with canonical `e454...` SHA;
- fail-closed artifact identity checks;
- future global normalized-SHA immutable-metadata invariant across distinct artifact IDs.

## R-014 — Premature production writes

**Severity:** high

Archive/EXE writes may be added before validation, export, backup, rollback and reintegration semantics stabilize.

Mitigation:

- immutable sources;
- WorkingCopy first;
- guarded expected-byte/hash checks;
- copied-output validation;
- no direct original-file writes by default;
- writer/repack claims require evidence and game validation.

## R-015 — Public repository contamination

**Severity:** high

Proprietary executable/resource/save bytes may be committed.

Mitigation:

- synthetic public fixtures;
- sanitized Evidence Packets;
- legal user-supplied local files only;
- clean-room policy and review.

## R-016 — AI/parallel-agent drift

**Severity:** high

Parallel agents may create duplicate models, race on the same function/type/file, or promote consensus as evidence.

Mitigation:

- Reverse Core task claims/ownership coordination;
- narrow PR scopes;
- one authority per function/type/resource domain;
- agent consensus is never evidence;
- review -> evidence reconciliation -> promotion.

## R-017 — UI-first architecture regression

**Severity:** medium to high

Editor convenience may redefine resource identity, mutable state, scene ownership or write policy.

Mitigation:

- domain/runtime contracts precede GUI work;
- UI consumes Stage Ops/WorkingCopy/Binary Document/reconstruction identities;
- no UI-local resource or scene model.

## R-018 — Scope dispersion

**Severity:** high

Many partially advanced systems can create the appearance of progress while no end-to-end vertical reaches behavioral validation.

Mitigation:

- prioritize evidence-backed vertical receipts;
- use explicit completion gates;
- finish bounded validation loops before broadening UI/features;
- track the project-wide `no major subsystem complete` state until a real exit gate is met.

## R-019 — Full recompilation claim too early

**Severity:** critical

Source/build lineage infrastructure may be confused with a behaviorally equivalent rebuilt DMC3 executable.

Mitigation:

- progressive subsystem reconstruction;
- isolated compile;
- behavioral receipt;
- rebinding/replacement boundary;
- composite build validation;
- no full recompilation claim before all required gates are actually satisfied.
