# Contributing to DMC Rengine

DMC Rengine accepts contributions to code, documentation, tests, synthetic fixtures, reverse-engineering evidence, recovered-source reconstruction, visual design and project infrastructure.

## Before contributing

Read:

- `docs/status/completion-and-evidence-policy.md`
- `docs/architecture.md`
- `docs/reverse-engineering-rules.md`
- `docs/legal/clean-room-policy.md`
- `docs/brand/README.md`

Two rules are non-negotiable:

> All product resource resolution goes through GDSpaces.

> A bounded implemented/tested/closed slice must never be promoted into a whole-subsystem `COMPLETE` claim without the applicable evidence and ValidationReceipt gates.

## Current completion context

As of 2026-08-15 DMC Rengine contains substantial working implementation, but **no major end-to-end subsystem is COMPLETE or proven behaviorally equivalent to the original DMC3 runtime**.

This is not a prohibition on finishing bounded tasks. A contribution may legitimately close a parser, ABI, writer rule, recovered function or workflow slice. State that exact scope.

Examples:

- `BOUNDED CLOSED: function wrapper ABI` is valid when evidence closes it;
- `TESTED: Stage Ops edit refresh path` is valid when deterministic CI covers it;
- `COMPLETE: collision subsystem` is invalid while deeper primitive/source/lifecycle/runtime-equivalence gaps remain.

## Architecture ownership

Contributions must preserve these boundaries:

- **GDSpaces** — product resource identity/resolution/materialization/provenance;
- **Recovered Game Source Tree** — reconstructed original DMC3 runtime behavior;
- **Reverse Core** — generic evidence/reconstruction/claim/validation infrastructure;
- **Stage Ops** — product stage/scene assembly and operational state;
- **Stage Semantic Graph** — derived representation/index;
- **ModViz** — editor consumer;
- **Binary Inspector** — byte/structure/ownership/evidence inspection;
- **EXE Editor** — executable/reconstruction editing frontend.

Tool consumption does not transfer semantic ownership of original-game code.

## Stage identity rule

Do not center new Stage work on `st001` or infer Stage identity from `stNNN` filenames.

Keep resource-set/catalog identity, numeric Stage selector identity and semantic/gameplay Stage identity separate. `st001` is a regression/compatibility fixture only.

## Contribution categories

### Engineering

- C++20 libraries/applications;
- CMake and CI;
- parsers/validators;
- tests and synthetic fixtures;
- diagnostics and developer tooling;
- Stage Ops / Binary Inspector / ModViz / editor product slices.

### Reverse engineering / recovered game

- evidence records and packets;
- exact artifact/address/body identities;
- reproducible runtime observations;
- ABI/ownership/lifetime reconstruction;
- recovered C++ units;
- behavioral comparison harnesses;
- correction/rejection/supersession records.

### Documentation and design

- specifications;
- current-authority status docs;
- architecture diagrams;
- tutorials;
- UI concepts/original brand assets;
- translations.

Historical evidence documents should normally receive supersession notes rather than being rewritten to erase what was known at the time.

## Evidence requirements

Use the vocabulary in `docs/status/completion-and-evidence-policy.md`, including:

`HYPOTHESIS`, `CANDIDATE`, `LOW`, `MEDIUM`, `HIGH`, `EXE CONFIRMED`, `DERIVED FROM VERIFIED RUNTIME`, `IMPLEMENTED`, `TESTED`, `BOUNDED CLOSED`, `VALIDATED`, `RESEARCH REQUIRED`, `NOT PROVEN`, `CORRECTED`, `REJECTED`, and the strictly gated `COMPLETE`.

A reverse claim should include enough artifact identity, addresses/offsets, procedure, competing explanations and validation context for another contributor to reproduce or challenge it.

Agent/model consensus is not evidence.

If the raw artifact was not mounted in a pass, do not represent previous Evidence Packets or reports as a fresh direct re-hash/disassembly.

## Recovered-source policy

Recovered C++ is an evidence-backed reconstruction, not automatically original Capcom source.

A recovered unit should link:

- artifact identity;
- address/function/type reconstruction identity;
- ABI and ownership/lifetime assumptions;
- evidence locations;
- unresolved behavior;
- compile/test receipt;
- behavioral ValidationReceipt when equivalence is claimed.

Do not bulk-import speculative decompiler output as canonical recovered source.

## Prohibited repository content

Do not commit:

- Capcom executables or DLLs;
- extracted proprietary game archives/assets;
- copyrighted textures, models, audio, scripts or binary blobs;
- leaked source/confidential material;
- credentials, tokens, personal data or machine-specific secrets;
- large copied decompiler/binary output without clean-room review and provenance.

Use synthetic public fixtures and legally supplied local game files.

## Development workflow

1. Read the current authority/status and check whether an older target has already been superseded or bounded-closed.
2. Open/reference an issue, specification or Reverse Core task claim for non-trivial work.
3. Claim a narrow file/function/type scope to avoid parallel-agent races.
4. Acquire evidence before inventing semantics.
5. Implement the smallest evidence-supported slice.
6. Add/update deterministic tests.
7. Run cross-platform CI or document why a platform/runtime gate is external.
8. Compare against canonical runtime behavior when equivalence is claimed.
9. Update current status/issues and add correction/supersession records where needed.
10. Submit a focused pull request using the repository template.

Generic local validation:

```bash
cmake -S . -B build -DDMC_RENGINE_BUILD_TESTS=ON
cmake --build build --config Release
ctest --test-dir build -C Release --output-on-failure
```

## C++ rules

- C++20, no compiler extensions unless explicitly isolated/justified.
- Prefer value types, RAII, explicit ownership and deterministic behavior.
- Avoid global mutable state.
- Parse untrusted binary data with bounded reads/checked arithmetic.
- Do not hide source resolution inside UI/editor code.
- Public product APIs live under `include/dmc_rengine/`.
- Original-game reconstructed code belongs under the recovered-game tree, not product tools.
- Every new subsystem/slice requires tests and a short architecture/evidence note.
- Unknown semantics stay unknown/raw instead of receiving convenient names.

## PR completion check

Every PR must answer:

- What exact scope is implemented/closed?
- What remains open?
- Is evidence direct, runtime-derived, corpus-derived, or synthetic-test-only?
- Does the PR change a subsystem completion state? If yes, where is the ValidationReceipt and full gate evidence?
- Is the change branch-scoped only, or deliberately promoted to `main`?

A PR that does not satisfy the full completion gate must explicitly avoid a major-subsystem `COMPLETE` claim.
