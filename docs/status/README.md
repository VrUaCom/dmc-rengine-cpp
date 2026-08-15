# Status System

The status directory separates **implementation**, **reverse evidence**, **branch-scoped validation**, and **project completion**. These are not interchangeable.

Canonical files:

- `completion-and-evidence-policy.md` — mandatory completion/evidence vocabulary and promotion gate;
- `current.md` — human-readable current project snapshot;
- `canonical-status.json` — machine-readable current snapshot;
- `phase-map.md` — phase/gate map without implying whole-subsystem completion;
- `blockers.md` — unresolved dependencies and validation gaps;
- `risks.md` — architecture/project risk register;
- `weekly/` — dated repository-backed reports and historical receipts.

## Non-negotiable status rule

DMC Rengine may contain implemented, tested, EXE-confirmed, validated, or bounded-closed slices while the containing subsystem remains incomplete.

As of 2026-08-15 **no major end-to-end subsystem is `COMPLETE` or proven equivalent to the original DMC3 runtime**. Do not translate green CI, a compiled recovered unit, a closed function ABI, or a structural writer into a whole-subsystem completion claim.

Read `completion-and-evidence-policy.md` before changing any status language.

## Truth layers

- raw canonical executable/runtime observations are reverse evidence for the exact artifact;
- Evidence Packets/reconciliation records are sanitized evidence authority;
- recovered-game code is evidence-backed reconstruction, not automatically original Capcom source;
- an active PR is branch-scoped implementation truth only;
- GitHub `main` is merged product implementation truth and may lag active stacked work;
- Google Drive contains research history and newer reverse material; older documents may be superseded rather than rewritten.

## Update rules

Update status when:

- an implemented component enters a branch/build or is promoted to `main`;
- tests or CI change confidence;
- reverse evidence closes, corrects, rejects, or reclassifies a bounded target;
- a blocker opens or closes;
- an architecture decision changes ownership or scope;
- representative game/corpus validation produces a new receipt;
- a historical document becomes stale enough to require a supersession notice.

Never use `complete`, `fully reversed`, `game-ready`, `equivalent`, or `original source` for a major subsystem unless the canonical completion gate is satisfied.
