## Summary

Describe the exact bounded change and why it exists.

## Scope

- [ ] Code
- [ ] Tests
- [ ] Documentation/status
- [ ] Reverse-engineering evidence
- [ ] Recovered Game Source Tree
- [ ] Build/CI
- [ ] Editor/UI

### Exact bounded claim

What is implemented / corrected / rejected / bounded-closed by this PR?

### Explicitly still open

List nearby ABI, semantic, lifecycle, corpus, runtime-equivalence, integration or promotion gaps that this PR does **not** close.

## Completion status

Select the narrowest truthful state for this PR's exact scope:

`HYPOTHESIS | CANDIDATE | EXE CONFIRMED | DERIVED FROM VERIFIED RUNTIME | IMPLEMENTED | TESTED | BOUNDED CLOSED | VALIDATED | RESEARCH REQUIRED | NOT PROVEN | CORRECTED | REJECTED`

- [ ] I am **not** using this PR's green CI, compiled C++, parser/writer, or bounded reverse closure as proof that a major subsystem is `COMPLETE`.
- [ ] If this PR claims a major subsystem is `COMPLETE`, I linked the full completion-gate evidence and deterministic `ValidationReceipt` required by `docs/status/completion-and-evidence-policy.md`.

Major-subsystem completion claim, if any: `none` by default.

## Architecture / ownership

- [ ] Product resources still enter tools only through GDSpaces.
- [ ] No independent container/path/resource resolver was added.
- [ ] Reconstructed original DMC3 runtime code lives in the Recovered Game Source Tree, not GDSpaces/Stage Ops/ModViz.
- [ ] Reverse Core remains generic evidence/reconstruction infrastructure.
- [ ] Stage Ops owns product scene assembly/operations; Semantic Graph is a derived projection; ModViz is a consumer/editor.
- [ ] WorkingCopy / immutable-source boundaries are preserved.
- [ ] EXE changes, if any, use exact artifact identity and guarded expected-byte policy.
- [ ] `st001` is not used as the canonical Stage universe or exit gate.

## Evidence

Evidence level(s):

`raw artifact | EXE confirmed | verified runtime | real corpus | Evidence Packet | reconstruction | synthetic test only | not applicable`

Artifact identity / SHA / build:

Address/range/resource identities:

Evidence packet, Drive document, issue, specification, reconstruction or test references:

### Freshness statement

- [ ] Direct raw artifact/disassembly/runtime evidence used in this PR was actually available for this pass; **or**
- [ ] This PR relies on previously recorded project evidence and does not claim a fresh independent re-hash/re-disassembly.

## Historical correction / supersession

Does this PR correct or supersede an older claim/checklist/pass?

- [ ] No
- [ ] Yes — linked below and marked `CORRECTED` / `REJECTED` / superseded without erasing history.

References:

## Validation

Commands/run IDs:

```text
cmake -S . -B build -DDMC_RENGINE_BUILD_TESTS=ON
cmake --build build --config Release
ctest --test-dir build -C Release --output-on-failure
```

Windows receipt:

Ubuntu receipt:

Real corpus/game/runtime validation, if any:

`ValidationReceipt`, if equivalence is claimed:

## Truth-layer / promotion status

- [ ] Branch-scoped implementation only
- [ ] Evidence/research promotion only
- [ ] Deliberate `main` promotion candidate

Do not describe an active PR as merged `main` truth before merge.

## Legal/content check

- [ ] No proprietary game files, leaked source or large copied binary regions are included.
- [ ] Public fixtures are synthetic/sanitized/authorized.
- [ ] Logs and paths are sanitized.

## Risks and rollback

Describe compatibility risks, unresolved assumptions, evidence limits and how to revert/supersede the change.
