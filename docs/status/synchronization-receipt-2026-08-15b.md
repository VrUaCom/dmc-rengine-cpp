# Synchronization Receipt — 2026-08-15B

## Purpose

Record the second same-day authority synchronization after the first completion/status reconciliation became stale due to new HITS slices, the merge of generic EXE acquisition infrastructure, and new PAC production-path work.

This receipt does not claim subsystem completion. It binds the status surfaces to one explicit evidence/implementation cutoff.

## Synchronization cutoff

- repository: `VrUaCom/dmc-rengine-cpp`;
- merged `main`: `25bd70092503cc6ca3be51f05582dcd33af1523d`;
- `main` change at cutoff: merge PR #98, `Reverse acquisition: add hash-gated executable byte-window extractor`;
- PR #94 branch pre-receipt synchronization head: `c49dfec58f4a80e4319c782767611f4f4e511660`;
- final PR #94 branch head after this receipt is recorded in the PR conversation because the receipt commit itself necessarily changes the branch head.

## Truth-layer rule

1. raw direct artifact/runtime evidence is strongest for the exact artifact/range/observation it covers;
2. sanitized Evidence Packets preserve reproducible evidence authority;
3. merged `main` is merged implementation truth only;
4. exact open PR heads are branch-scoped implementation/research truth only;
5. Google Drive preserves research/current/historical authority only in the explicitly identified scope;
6. bounded closure, green CI, parser success, recovered C++ compilation or acquisition receipts do not imply major-subsystem completion or original-game behavioral equivalence.

## Merged-main delta — PR #98

PR #98 is now merged into `main` at `25bd70092503cc6ca3be51f05582dcd33af1523d`.

Bounded capability:

```text
SourceRegistry
  -> LocalDirectorySource
  -> exact expected SHA-256
  -> PE parse
  -> checked VA/RVA/file-backed mapping
  -> bounded byte window
  -> deterministic acquisition receipt
  -> optional explicit local-only raw hex
```

Receipt authority is artifact/range/byte identity only.

It does **not** prove:

- function name or full body boundary;
- ABI;
- ownership/lifetime;
- gameplay/runtime semantics;
- recovered-source correctness;
- behavioral equivalence.

For HITS Slice 16, known-body windows may use exact existing size/hash evidence. `0x1400594B0` remains discovery/probe territory until direct evidence establishes an exact canonical body size/hash.

## HITS synchronization

### Slice 15 — PR #96

- purpose: referenced Stage-CFG primitive-descriptor census over bounded entry references;
- final evidence/documentation head: `a4be42d5ea73c9e120febd8a9b1b0654d5858dbc`;
- Actions: `31886670409`;
- Ubuntu: SUCCESS;
- Windows: SUCCESS;
- failed jobs: 0;
- status: bounded-complete at implementation/evidence-tooling scope;
- real type-5 presence: NOT CLAIMED;
- real type-5 absence: NOT CLAIMED;
- remaining gate: representative legal real Stage-CFG referenced-descriptor census with resource-set/numeric-stage/source provenance.

Temporary PR #95 was closed unmerged only because its Slice-13 identity collided with already-existing Slice 13/14 numbering. The retained implementation continues as Slice 15 in #96.

### Slice 16 — PR #97

Status: `RESEARCH REQUIRED`.

Current direct evidence:

- `entry+0x01` is a transform selector;
- modern Stage-CFG slots 39/40 = entry/primitive-descriptor tables;
- legacy observed slots 22/23 = entry/primitive-descriptor tables;
- modern slot38 is consumed by `0x1400594B0` and has its own relative-offset structure;
- slot38 is **not proven** to be the C740-style `0x40` transform table;
- C8D0 stack arg5 becomes runtime `+0x20` transform pointer;
- `transform_selector_bounds_available() == false` remains a hard gate.

Current acquisition/reverse targets:

1. full `0x1400594B0` body + callers + slot38 dataflow;
2. modern Stage-CFG route around `0x14009823F`;
3. legacy observed route around `0x1400B6483`;
4. complete C630/C740 caller census classified by manager/source identity;
5. exact producer/base/object/bounds/count/lifecycle of C8D0 stack arg5.

Forbidden without new direct evidence:

- Stage-CFG slot38 transform parser;
- Stage-CFG three-table adapter;
- inference that C740 support proves Stage-CFG uses C740;
- original-runtime transform construction in GDSpaces.

## GDSpaces PAC synchronization — PR #99

PR #99 adds the current-generation production-oriented read-only PAC structural parser.

Promoted structural subset:

```text
+0x00  PAC\0
+0x04  u32 declared slot count
+0x08  u32 slot-offset table[slot_count]
```

Current structural rules:

- slot offset 0 preserves an empty slot;
- populated offsets are absolute PAC byte offsets;
- populated offsets may not point into header/table or outside the PAC span;
- declared slot space is preserved exactly;
- populated extent is inferred from the next greater distinct populated offset or PAC end;
- duplicate non-zero offsets remain separate slot identities sharing one bounded extent;
- no semantic alias meaning is inferred;
- parser fails closed on malformed/truncated/unsafe structure.

Hard semantic boundary:

- no Stage role inference;
- no HITS role inference;
- no global semantic slot numbering;
- no model/texture/audio semantic inference;
- no invented filenames for unnamed slots.

Still open:

- representative legal real-PAC validation across multiple families;
- PNST/NBZ/AFS in the promoted current path;
- recursive expansion policy reconciliation;
- `.lst` behavior;
- source priority;
- write/repack/export;
- issue #3 completion;
- original DMC3 resource-runtime equivalence.

## Stage Ops / executable identity CI readback

PR #91:

- final head: `b0994436457a7ae26e3083a4a13461f50db6e76d`;
- Actions: `31877176748`;
- Ubuntu: 106/106;
- Windows: 106/106;
- interpretation: branch-scoped tested Stage Ops implementation only, not subsystem completion, not merged-main, not vanilla game-ready equivalence.

PR #92:

- Actions: `31877266101`;
- Ubuntu/Windows: success;
- corrected project-evidence tuple: SHA `e454272ed0fb0247fcbcf300e5d55d7a3e96d50b89b9ffaff81bb978dcbdd082`, size `6,356,432`;
- stale pairing `3,735,552` is corrected/rejected for this SHA;
- interpretation: branch-scoped correction remains promotion-pending at this cutoff.

## Google Drive surfaces synchronized

The following current/master documents were updated in place while preserving historical audit material:

- `DMC Rengine — Technical Status — 2026-08-15` — Drive ID `1BzPjrqQ58hnZALwED2Skaf9u4v2cxieenC9oDMifXKE`;
- `DMC Rengine — Full Project Audit — Master — Updated 2026-08-15` — Drive ID `11lI1joYnXI38LNSID1gCRe52ynxkHxrl4xCOEgUtFYc`;
- `DMC Rengine — Architecture Reconciliation Report — Updated 2026-08-15` — Drive ID `1APJ9qXL4t1itXDmEKQHcS1olgmFEx33OEo1SfloA5TA`;
- `DMC Rengine C++ — Повний технічний звіт і звірка GitHub ↔ Google Drive — Updated 2026-08-15` — Drive ID `1UwCr7zmkxKXvXbxseAHBDMvXgeaFmGTI2f43eLywNVI`.

Corrections include:

- E7A0/B460/FEC0/601E0 removed from current P0 discovery wording;
- HITS frontier advanced to Slice 15 validated / Slice 16 active;
- `0x1400594B0` and exact C8D0 transform-source provenance promoted to current reverse target;
- slot38 explicitly marked transform-role unproven;
- final #91/#92 CI receipts recorded;
- merged #98 capability and proof boundary recorded;
- #99 PAC structural scope and real-corpus gate recorded;
- historical `HEAD/no open PRs` text in the old GitHub↔Drive audit explicitly reclassified as historical snapshot rather than current status;
- Stage Ops ownership wording corrected so ownership scope cannot be mistaken for subsystem `COMPLETE`.

## GitHub status surfaces synchronized in PR #94

Updated:

- `docs/status/current.md`;
- `docs/status/canonical-status.json`;
- `docs/status/blockers.md`;
- `docs/status/phase-map.md`;
- `docs/roadmap.md`;
- `docs/status/risks.md`;
- this receipt.

Machine/current status now records:

- exact merged-main SHA at cutoff;
- merged #98 capability;
- #96/#97/#99 active scopes;
- Slice-15/Slice-16 HITS split;
- PAC real-corpus gate;
- raw-artifact availability as distinct from acquisition-tool availability;
- explicit parallel current tracks.

## Current execution tracks after synchronization

Primary project critical path remains:

```text
artifact/evidence integrity
  -> resource lifecycle/factory/cache closure
  -> evidence-backed recovered runtime behavioral receipts
  -> representative Stage Catalog validation
  -> Stage Ops domain/runtime/lifecycle bridge
  -> validated editor/export verticals
  -> progressive recompilation milestones
```

Parallel evidence-heavy tracks:

1. HITS Slice 16 — `0x1400594B0` / slot38 / C8D0 transform-source provenance;
2. HITS Slice 15 data gate — representative real Stage-CFG referenced-descriptor census;
3. GDSpaces PR #99 — representative real-PAC multi-family validation through the canonical container path.

## Completion statement

DMC Rengine has many real implemented/tested/EXE-confirmed/BOUNDED CLOSED/VALIDATED slices.

**No major DMC Rengine end-to-end subsystem is COMPLETE or proven behaviorally equivalent to the original DMC3 runtime at this synchronization cutoff.**
