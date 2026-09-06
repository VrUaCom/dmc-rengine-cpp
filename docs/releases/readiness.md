# DMC Rengine Public Release Readiness

This document defines the minimum gate for publishing a GitHub Release or other user-facing DMC Rengine distribution.

It is intentionally conservative. A release is a trust/distribution surface; it must not upgrade implementation or reverse-engineering claims merely because a tag or downloadable artifact exists.

## Authority

Release notes must be derived from:

1. the exact release commit/tag;
2. [Current Project Status](../status/current.md);
3. relevant evidence/reverse documents;
4. CI/test receipts for the exact candidate;
5. the current changelog.

If those sources disagree, stop and reconcile the conflict before publishing.

## Pre-1.0 rule

DMC Rengine is pre-1.0. Do not publish a cosmetic `1.0` for visibility, SEO, stars or social proof.

A `1.0.0` release requires an explicit project-level acceptance decision and must correspond to a materially complete public contract rather than a marketing milestone.

## Release classes

### Research / documentation milestone

Use when the promoted value is primarily evidence, specifications or documentation.

Requirements:

- exact promoted research scope stated;
- evidence status and artifact identity stated where applicable;
- no binary/tool capability implied unless present in the release commit;
- historical/candidate findings clearly separated from canonical facts.

### Tooling preview

Use when users can build/run a bounded tool or reader, but broad product completeness is not claimed.

Requirements:

- exact supported operation(s) listed;
- unsupported formats/operations listed;
- build/run instructions tested;
- input expectations and legal/proprietary-data boundaries stated;
- failure behavior and destructive-write protections documented where relevant.

### Bounded feature release

Use when one or more capabilities have stable enough contracts for public use.

Requirements:

- capability is promoted in `main`;
- exact-head Windows and Ubuntu CI are green unless a documented platform exception exists;
- relevant parser/writer/round-trip/integration tests are green;
- format/evidence boundary is documented;
- known limitations are explicit;
- user-visible artifact or reproducible build path exists.

## Mandatory release checklist

Before publishing:

- [ ] Candidate commit/tag is exact and immutable for the release process.
- [ ] Windows CI passes on the candidate where supported.
- [ ] Ubuntu CI passes on the candidate where supported.
- [ ] Relevant CTest/integration suites pass.
- [ ] `docs/status/current.md` does not contradict the proposed release notes.
- [ ] `CHANGELOG.md` contains the promoted user-visible changes.
- [ ] Release notes state what is supported.
- [ ] Release notes state what is **not** supported or not proven complete.
- [ ] No historical branch-only capability is presented as canonical.
- [ ] No parser/read support is described as writer/edit support without evidence.
- [ ] No successful synthetic rebuild is described as original-game equivalence without the applicable game-backed receipt.
- [ ] No Capcom offline-writer equivalence is claimed without direct evidence.
- [ ] No proprietary game binary/asset/archive is attached.
- [ ] Users are told when legally obtained local game files are required.
- [ ] Downloadable artifacts, if provided, are produced from the release candidate or a documented reproducible build.
- [ ] Artifact hashes are published when binaries are distributed.
- [ ] Rollback/original-file safety is documented for any modifying workflow.

## Release-note structure

Use this order:

1. **What this release is** — one short bounded description.
2. **What is new** — user-visible promoted capabilities.
3. **Supported scope** — exact formats/tools/operations.
4. **Known limitations** — explicit non-goals and incomplete gates.
5. **Build / install / run** — reproducible commands or artifacts.
6. **Validation** — exact CI/tests/evidence relevant to the release.
7. **Legal / data boundary** — no proprietary game data distributed; local legally obtained inputs where required.
8. **Canonical status** — link to current project status and relevant deep documentation.

## Version naming guidance

Prefer version numbers that reflect product maturity rather than publicity goals.

For pre-1.0 releases:

- patch: corrections and small compatible improvements;
- minor: meaningful new bounded capability or user-facing workflow;
- prerelease suffixes (`-alpha`, `-beta`, `-rc`) only when they communicate a real testing/stability stage.

Do not call a branch snapshot a stable release solely because CI is green.

## Artifact policy

A downloadable binary or package should include or be accompanied by:

- source commit/tag identity;
- platform/architecture;
- build configuration;
- SHA-256;
- required runtime dependencies;
- exact input expectations;
- known unsupported operations.

Generated artifacts must not include Capcom executables, DLLs, extracted archives, textures, models, audio, scripts or other proprietary game data.

## Discovery boundary

Release titles and notes may use clear discovery terms such as `DMC3`, `Devil May Cry 3 HD`, `reverse engineering`, `file-format tooling`, or the exact promoted format when accurate.

Do not keyword-stuff release titles or use a format/game capability that the release does not actually contain.

## Acceptance

A release is ready only when a reviewer can answer all of the following from the release candidate and its documentation:

- What exactly can a user do?
- Which inputs/formats are supported?
- What remains experimental or incomplete?
- What evidence/tests justify the claims?
- Can the build or artifact be reproduced/identified?
- Are proprietary-data and destructive-write risks bounded?

If any answer depends on an unmerged branch, unwritten assumption or stale status document, the release is not ready.
