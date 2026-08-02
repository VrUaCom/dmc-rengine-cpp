# DMC Rengine Governance

## Project model

DMC Rengine is an open-source, evidence-driven engineering project. Technical authority comes from maintainership, review responsibility, reproducible evidence, and sustained contribution—not from lore ranks.

## Roles

### Contributors

Submit code, research, tests, documentation, design, or reviews.

### Reviewers

Regular contributors trusted to review defined areas. Reviewers may approve work but do not independently change project policy.

### Maintainers

Own repository health, architecture enforcement, release decisions, security response, and final merge authority.

### Canon Keepers

Maintain the Reverse Canon and Evidence Registry. They ensure claims carry confidence labels and correction history. This is a technical responsibility, not a superior social rank.

## Decision process

Routine changes use pull-request review.

Architecture changes require:

1. a written specification or architecture decision record;
2. explicit non-goals;
3. migration and compatibility impact;
4. tests or a validation plan;
5. maintainer approval.

The following are constitutional rules and require a documented governance decision to change:

- GDSpaces is the only resource access API.
- Editors do not independently resolve NBZ/AFS/PAC/PNST or local game paths.
- Reverse-engineering claims must preserve evidence and confidence.
- Original game files are never committed.
- Writes use working copies, validation, and explicit export policy.
- EXE patches require source identity, source bytes, target bytes, rollback, and tests.

## Consensus and escalation

Prefer consensus. When consensus is unavailable, maintainers decide based on architecture, evidence quality, maintenance cost, legal safety, and project roadmap.

A disputed technical conclusion remains marked uncertain until reproduced. No vote can turn weak evidence into a confirmed fact.

## Releases

Before 1.0, releases may introduce breaking changes. Tagged releases require:

- passing CI;
- updated changelog and status;
- documented known limitations;
- no known proprietary files in artifacts;
- maintainer approval.

## Conflicts of interest

Contributors should disclose financial, employment, or ownership interests that materially affect project decisions.

## Amendments

Governance changes must be proposed in a pull request with rationale and an impact summary.
