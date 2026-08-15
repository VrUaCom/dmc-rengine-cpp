# DMC Rengine Governance

## Project model

DMC Rengine is an open-source, evidence-driven engineering/reverse-reconstruction project. Technical authority comes from reproducible evidence, explicit ownership, review responsibility, maintained implementation and validated promotion—not from lore rank, agent consensus or code volume.

The Constitution and `docs/status/completion-and-evidence-policy.md` govern completion/evidence language.

## Roles

### Contributors

Submit code, reverse evidence, recovered-source units, tests, documentation, design or reviews.

### Reviewers

Review defined areas for evidence quality, architecture ownership, correctness, test scope and overclaim. Approval does not convert missing evidence into fact.

### Maintainers

Own repository health, architecture enforcement, merge/promotion decisions, security response and release gates.

### Canon Keepers

Maintain current Reverse Canon, Evidence Registry and supersession history. They ensure claims have exact scope, artifact/evidence identity, status and correction/rejection links.

Canon Keeper or maintainer approval is not itself evidence.

## Decision process

Routine changes use pull-request review.

Architecture/reverse-authority changes require, as applicable:

1. written issue/specification/ADR or Reverse Core claim;
2. exact scope/ownership and non-goals;
3. evidence boundary and artifact identity;
4. migration/supersession impact;
5. tests plus behavioral validation plan where equivalence is claimed;
6. synchronized current status/docs/issues;
7. maintainer approval for canonical promotion.

## Constitutional rules

Changing these requires an explicit governance/Constitution amendment:

- GDSpaces is the only product resource resolver/materializer.
- Reconstructed original DMC3 runtime code belongs in the Recovered Game Source Tree.
- Reverse Core remains generic reverse/evidence/reconstruction/claim infrastructure.
- Stage Ops owns product scene/stage assembly and operational state; Semantic Graph is a derived projection; ModViz is an editor consumer.
- Editors/tools do not independently resolve NBZ/AFS/PAC/PNST/local game paths.
- `st001` is regression/compatibility data only, not the Stage architecture or completion gate.
- resource-set/catalog identity, numeric Stage identity and semantic/gameplay identity remain distinct unless evidence explicitly relates them.
- reverse claims preserve evidence, exact bounded status and correction history.
- agent/model consensus is not evidence.
- green CI does not prove original-game behavioral equivalence.
- bounded closure does not imply major-subsystem completion.
- original game files are never committed.
- writes use immutable sources, WorkingCopies, validation and explicit output/export policy.
- EXE patches require exact target identity, expected bytes/ranges, rollback and appropriate runtime validation.

## Completion governance

A major subsystem may be called `COMPLETE` only when the applicable gate in `docs/status/completion-and-evidence-policy.md` is satisfied and deliberately promoted.

Maintainers/reviewers must reject completion inflation such as:

- compiled recovered C++ -> original/equivalent source;
- green synthetic tests -> game behavior proof;
- parser/writer roundtrip -> original runtime/builder equivalence;
- materialized StageBundle -> vanilla state-3/game-ready Stage;
- bounded wrapper ABI closure -> complete collision subsystem;
- active PR implementation -> merged `main` truth.

As of 2026-08-15 no major DMC Rengine end-to-end subsystem is `COMPLETE` or proven behaviorally equivalent to the original DMC3 runtime.

## Parallel-agent ownership

Parallel work should claim narrow file/function/type/subsystem scopes through the available task/Reverse Core coordination model.

When claims overlap:

1. detect the conflict before duplicate reconstruction proceeds;
2. negotiate/split/supersede ownership;
3. preserve deterministic history;
4. compare evidence, not agent seniority/consensus.

No agent may create a second canonical function/type/resource model solely because it worked independently.

## Consensus and escalation

Prefer consensus for engineering coordination, but disputed technical conclusions remain uncertain until evidence resolves them.

Maintainers decide architecture/maintenance/merge questions; they cannot vote an unsupported technical claim into `EXE CONFIRMED`, `VALIDATED` or `COMPLETE`.

New contradictory direct evidence can reopen a bounded-closed claim. An older stale document alone cannot.

## Historical records / supersession

Historical pass documents remain evidence history. Current coordination/status surfaces must carry later corrections/rejections/closures.

Do not silently rewrite history to hide prior mistakes. Do not allow historical wording to override a later explicit supersession.

## Releases

Before 1.0, releases may be breaking. Tagged releases require:

- passing required CI;
- synchronized changelog/current status;
- documented known limitations/open equivalence gates;
- no proprietary content in artifacts;
- validated output/reintegration policy for shipped modification features;
- maintainer approval.

A release number does not imply original-game runtime equivalence unless explicitly scoped and backed by the required receipts.

## Conflicts of interest

Contributors should disclose financial, employment or ownership interests that materially affect project decisions.

## Amendments

Governance changes require a pull request with rationale, scope, compatibility/migration impact and consequences for Constitution/specifications/status. Constitutional changes also update the Constitution version/date.
