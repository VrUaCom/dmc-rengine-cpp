# Completion and Evidence Policy

**Effective date:** 2026-08-15  
**Scope:** all DMC Rengine code, issues, pull requests, status documents, Google Drive research, recovered-game code, Reverse Core records, and user-facing claims.

## Canonical rule

DMC Rengine currently has many implemented and tested components, but **no major end-to-end subsystem is complete or proven equivalent to the original DMC3 runtime**.

A bounded function, ABI, parser, writer, workflow slice, or branch may be closed/validated without promoting the containing subsystem to `COMPLETE`.

Examples:

- a HITS query wrapper ABI may be closed while collision source ownership, primitive production, geometry helpers, runtime validation, and lifecycle remain open;
- Stage Ops assembly may be implemented on a branch while vanilla game-ready equivalence, complete domain coverage, representative lifecycle validation, and merge promotion remain open;
- recovered C++ may compile and pass synthetic regressions while behavioral equivalence to the canonical executable remains unproven;
- a deterministic DMC Rengine writer may be structurally correct without being equivalent to Capcom's unknown offline builder.

## Required status vocabulary

Use the narrowest truthful status.

- `HYPOTHESIS` — plausible interpretation without sufficient direct evidence.
- `CANDIDATE` — evidence-supported interpretation that still requires reconciliation or validation.
- `EXE CONFIRMED` — directly supported by canonical executable evidence.
- `DERIVED FROM VERIFIED RUNTIME` — mechanically derived from verified runtime observations.
- `IMPLEMENTED` — code exists for the stated product-side scope.
- `TESTED` — the implementation passes the named deterministic tests; tests do not by themselves prove original-game behavior.
- `BOUNDED CLOSED` — a precisely scoped reverse target has sufficient evidence to stop discovery at that boundary.
- `VALIDATED` — the stated bounded behavior has a reproducible validation receipt.
- `PARTIAL` / `OPEN` / `RESEARCH REQUIRED` — meaningful behavior or evidence remains unresolved.
- `NOT PROVEN` — a stronger equivalence/semantic claim is intentionally withheld.
- `CORRECTED` — a previous claim has been replaced by stronger evidence.
- `REJECTED` — evidence contradicts the claim.
- `COMPLETE` — reserved for a fully defined subsystem scope whose required evidence, implementation, integration, runtime validation, lifecycle behavior, receipts, and promotion gates are all satisfied.

## `COMPLETE` gate

A major subsystem may be called `COMPLETE` only when all applicable conditions are true:

1. its scope and ownership boundary are explicitly defined;
2. all required raw/executable/runtime evidence is linked to exact artifact identity;
3. unresolved ABI/format/lifetime semantics inside the claimed scope are either closed or explicitly outside scope;
4. product implementation exists without duplicated authority/resolvers;
5. Windows and Ubuntu CI are green on the exact promoted head;
6. real representative corpus coverage exists where binary/resource compatibility is claimed;
7. original-game behavior claims have controlled behavioral comparison, not only synthetic tests;
8. lifecycle claims cover creation/load/use/reload/transition/release/unload as applicable;
9. a deterministic `ValidationReceipt` exists for the claimed equivalence;
10. the result has been deliberately promoted to the appropriate canonical branch/status authority.

If any applicable item is missing, the subsystem is not `COMPLETE`.

## Current project-wide completion statement

As of 2026-08-15:

- GDSpaces — substantial structural/lookup/materialization implementation exists; full request-to-unload DMC3 runtime equivalence is **NOT COMPLETE**.
- Recovered Game Source Tree — selected executable-backed units compile and execute tests; whole-system behavioral equivalence is **NOT COMPLETE**.
- Reverse Core — evidence/reconstruction direction exists; first universally accepted full subsystem reconstruction/validation gate is **NOT COMPLETE**.
- Stage Ops — substantial assembly/operations implementation exists on active PR #91; full scene-domain + vanilla lifecycle equivalence is **NOT COMPLETE**.
- Stage Semantic Graph — implemented projections exist; it is a derived representation and its complete target coverage is **NOT COMPLETE**.
- ModViz — integration/editing slices exist; complete scene/asset/HUD editor is **NOT COMPLETE**.
- HITS/collision — strong file-format, spatial, writer, source and bounded query evidence exists; whole collision runtime/original-builder/gameplay/lifecycle equivalence is **NOT COMPLETE**.
- Binary Inspector — substantial domain capabilities exist; full native product/editor integration is **NOT COMPLETE**.
- EXE Editor / decompilation / recompilation — selected recovered functions and build-lineage infrastructure exist; full decompilation and behaviorally equivalent rebuilt DMC3 executable are **NOT COMPLETE**.
- Item/HUD/editor flows — bounded implemented slices exist; complete production editor/export/runtime equivalence is **NOT COMPLETE**.

This statement does **not** mean "nothing works". It means no major vertical may be advertised as finished end-to-end.

## Bounded HITS correction — Pass 10

Do not reuse the older Pass-8/Pass-9 statement that the top-level P0 wrapper ABI is still entirely unknown.

Active PR #85 records later Pass-10 evidence that closes/reclassifies the upper wrapper layer, including:

- `0x14005E7A0` combined point-query wrapper ABI/precedence at the stated bounded level;
- `0x14005FEC0` and `0x1400601E0` top-level contracts at the stated bounded level;
- `0x14005B460` reclassified into the separate dynamic-world update pipeline rather than an `E7A0` candidate producer;
- common contact-normal semantics and primitive descriptor ownership as later validated slices.

Those bounded closures **must not** be promoted to "HITS/collision complete". Primitive-specific producer/helper reconstruction, source-2 backing/lifetime, complete gameplay semantics, controlled runtime comparison, and other deeper boundaries remain open.

## Stage identity rule

Never use `st001` as the architectural Stage identity or project completion gate.

Keep these axes separate:

1. `resource_set_id / catalog_entry_id`;
2. `numeric_stage_id`;
3. separately evidenced semantic/gameplay Stage identity.

The current Wave-2 model uses 189 observed descriptors (110 Bank A + 79 Bank B), a separate 193-entry selector space, and a 10-pointer group-base table. The 189 descriptors are not automatically 189 gameplay stages. `st001` is only a regression/compatibility fixture.

## Truth layers and precedence

Different truths must not be collapsed:

- **raw canonical artifact/runtime observation** — strongest reverse evidence for the exact artifact;
- **Evidence Packet / reconciliation record** — sanitized reproducible project evidence;
- **Recovered Game reconstruction** — executable specification backed by evidence, not automatically original source;
- **feature/integration PR head** — branch-scoped implementation truth only;
- **GitHub `main`** — merged product implementation truth, which may lag active research/stacked PRs;
- **Google Drive research documents** — research history and newer reverse material; historical documents remain snapshots and can be superseded.

A green PR does not become `main` truth until merged. A passing synthetic test does not become game behavior evidence. Agent consensus is not evidence.

## Documentation rule

Historical documents are not silently rewritten to pretend their old state never existed. Instead:

- current master/status documents state the latest truth;
- stale claims receive `CORRECTED`, `REJECTED`, or supersession notices;
- historical pass documents remain evidence history;
- issue bodies/checklists must be updated when they become current-authority coordination surfaces;
- no current document may use `complete`, `fully reversed`, `game-ready`, `equivalent`, or `original source` without satisfying the corresponding gate in this policy.
