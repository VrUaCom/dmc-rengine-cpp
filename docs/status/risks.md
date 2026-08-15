# Architecture and Project Risks

**Snapshot date:** 2026-08-15  
**Synchronization cutoff:** after merge PR #98 and creation of PR #99  
**Primary policy:** `completion-and-evidence-policy.md`

## R-001 — Completion inflation

**Severity:** critical

A bounded implemented/tested/EXE-confirmed slice may be described as a complete subsystem.

Examples of invalid promotion:

- green CI -> original-game equivalence;
- compiled recovered C++ -> original Capcom source;
- StageBundle materialization -> vanilla state-3/game-ready object;
- closed wrapper ABI -> whole collision system complete;
- deterministic DMC Rengine writer -> Capcom offline-builder equivalence;
- merged acquisition primitive -> Reverse Core or decompilation completion;
- structural PAC parser -> full PAC/runtime compatibility.

Mitigation:

- mandatory completion policy;
- separate bounded-slice status from subsystem status;
- require representative behavioral ValidationReceipts before equivalence claims;
- keep unresolved lifecycle/ownership/semantic gaps explicit.

## R-002 — Stale evidence/status surfaces

**Severity:** critical

Issue bodies, historical pass documents, `main`, active stacked PRs and Drive research can disagree within hours.

Observed examples:

- older HITS Pass-8/Pass-9 coordination text listed top-level `0x14005E7A0/B460/FEC0/601E0` questions as wholly open after later PR #85 bounded closure/reclassification;
- the first 2026-08-15 synchronization stopped at HITS Slice 12, then #96/#97 advanced the frontier to validated Slice 15 / active Slice 16;
- PR #98 merged after the first status pass, changing `main` to `25bd70092503cc6ca3be51f05582dcd33af1523d`;
- PR #99 appeared after that pass, reopening resource-status drift around the current-generation PAC parser.

Mitigation:

- record an explicit synchronization cutoff and exact `main` SHA;
- maintain one current status snapshot and machine-readable status;
- add supersession/correction notices instead of erasing history;
- update canonical coordination issue/PR bodies when current-target lists change;
- create a synchronization receipt after material status changes;
- never treat historical pass documents as latest authority by date-independent reading.

## R-003 — Second resolver / duplicated authority

**Severity:** critical

Stage Ops, ModViz, Semantic Graph, Binary Inspector, HITS tooling or another tool may resolve resources independently for convenience.

Mitigation:

- GDSpaces remains the only product resource resolver/materializer;
- PR #99 PAC work feeds the shared `ContainerDocument` path rather than a private HITS parser;
- consumers receive stable resource identities, payloads, Stage Ops state or typed projections;
- recovered original runtime behavior remains in recovered-game, not product tools.

## R-004 — Ownership collapse

**Severity:** high

Recovered DMC3 runtime code may be moved into GDSpaces/Stage Ops/ModViz, or Semantic Graph may become scene assembly authority.

Mitigation:

- GDSpaces: resource authority;
- Recovered Game Source Tree: original runtime reconstruction;
- Reverse Core: generic evidence/reconstruction infrastructure;
- Stage Ops: product-side scene assembly/operations authority;
- Semantic Graph: derived semantic/evidence representation;
- ModViz: editor consumer;
- Binary Inspector/EXE Editor: inspection/reconstruction tooling.

Ownership wording must not use `complete` as a synonym for scope ownership.

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

Current examples include PR #99 PAC structural tests and Slice-15 census-tool regressions: both can be green while real corpus gates remain open.

Mitigation:

- label synthetic CI separately from corpus/game validation;
- require representative real-resource receipts where compatibility is claimed;
- require runtime observations for lifecycle/gameplay equivalence.

## R-008 — Raw-artifact absence hidden by acquisition tooling

**Severity:** high

Merged PR #98 makes exact executable-window acquisition available, but a pass may still appear to perform new byte-level reverse when the legal canonical raw executable is not actually mounted.

A deterministic acquisition receipt proves only artifact/range/byte identity. It does not prove that a discovery window is a full function body or establish ABI/semantics.

Mitigation:

- record whether raw canonical `dmc3.exe`/resource bytes were actually mounted;
- distinguish fresh direct re-measurement from project-evidence confirmation;
- require exact expected-SHA-gated acquisition or sanitized disassembly/xref packets for genuinely new byte-level claims;
- distinguish known-body verification windows from probe/discovery windows;
- never invent a function size/hash for `0x1400594B0` before direct evidence supplies it.

## R-009 — HITS wrapper closure extrapolated to collision completion

**Severity:** high

Later Pass-10 closure of top-level wrapper contracts may be interpreted as complete collision reconstruction.

Mitigation:

- keep Slice-15 real-corpus census, Slice-16 transform provider/bounds/lifecycle, deeper primitive producers/helpers, source2 backing/lifetime, remaining component semantics and runtime comparison explicitly open;
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

Stale documents may reintroduce `HITS$`, `0x18060001` as a universal record marker, E7A0 metric/tie-break arbitration, runtime `+0x20` as primitive descriptor, or Stage-CFG abstract-inner-blob interpretations.

Mitigation:

- current classifier/format authority uses four-byte `HITS` and header-driven structure;
- raw flag values remain raw unless separately evidenced;
- correction/supersession records remain visible;
- `runtime+0x118` descriptor and `runtime+0x20` transform remain separate;
- Stage-CFG slot39/40 and legacy 22/23 table provenance remains explicit.

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
- PR #92 final branch CI `31877266101` does not imply it is merged to current `main`;
- PR #98 requires an explicit expected SHA for acquisition;
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
- writer/repack claims require evidence and game validation;
- PR #99 remains read-only structural parsing only.

## R-015 — Public repository contamination

**Severity:** high

Proprietary executable/resource/save bytes may be committed.

Mitigation:

- synthetic public fixtures;
- sanitized Evidence Packets;
- legal user-supplied local files only;
- PR #98 raw hex is explicit local-only mode and must not be committed;
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
- run the current three concrete evidence-heavy tracks without broadening them: HITS Slice 16 transform provenance, HITS Slice-15 representative census, PR #99 real-PAC corpus validation;
- use explicit completion gates;
- finish bounded validation loops before broadening UI/features;
- track the project-wide `no major subsystem complete` state until a real exit gate is met.

## R-019 — Full recompilation claim too early

**Severity:** critical

Source/build lineage and EXE acquisition infrastructure may be confused with a behaviorally equivalent rebuilt DMC3 executable.

Mitigation:

- progressive subsystem reconstruction;
- isolated compile;
- behavioral receipt;
- rebinding/replacement boundary;
- composite build validation;
- no full recompilation claim before all required gates are actually satisfied.

## R-020 — PAC slot-number semantic globalization

**Severity:** critical for resource/HITS integration

Matching PAC slot numbers across unrelated PAC resources may be treated as globally equivalent schemas. Slice 13 already has a negative control showing that the same slot numbers can represent different structures in another PAC family.

Mitigation:

- PAC slot index is physical/container-scoped identity only;
- semantic roles require resource/schema evidence;
- PR #99 structural parser must not infer Stage/HITS/model/texture/audio semantics;
- Stage-CFG 39/40 and legacy 22/23 meaning is profile/evidence-specific, not generic PAC behavior;
- representative multi-family corpus validation before broader promotion.

## R-021 — Stage-CFG slot38 transform overclaim

**Severity:** critical for HITS Slice 16

Because slot38 is adjacent to the proven Stage-CFG entry/descriptor tables and because a separate serialized collision sample has a `0x40` transform span, an implementation may incorrectly wire Stage-CFG slot38 as the C740-style transform table.

Current evidence does not support that conclusion. Slot38 is consumed by `0x1400594B0` and has its own relative-offset structure.

Mitigation:

- `transform_selector_bounds_available() == false` remains a hard gate;
- no slot38 transform parser;
- no Stage-CFG three-table adapter;
- recover full `0x1400594B0` body/callers/slot38 dataflow;
- close exact C8D0 stack-arg5 producer/base/object/bounds/count/lifecycle before promotion.
