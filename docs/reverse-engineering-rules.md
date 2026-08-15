# Reverse Engineering Rules

Read `docs/status/completion-and-evidence-policy.md` before promoting any reverse finding.

## Core rule

Every technical claim must carry a status that describes the **exact bounded claim**, not the apparent maturity of the surrounding subsystem.

Use the narrowest truthful state:

- `HYPOTHESIS` — plausible, insufficient direct evidence;
- `CANDIDATE` — evidence-supported but reconciliation/validation remains;
- `LOW` / `MEDIUM` / `HIGH` — confidence grades when useful;
- `EXE CONFIRMED` — directly supported by the exact canonical executable evidence;
- `DERIVED FROM VERIFIED RUNTIME` — mechanically derived from verified runtime observations;
- `IMPLEMENTED` — code exists for the stated product/reconstruction scope;
- `TESTED` — deterministic tests pass for that scope;
- `BOUNDED CLOSED` — the precisely defined reverse target is sufficiently closed at that boundary;
- `VALIDATED` — a reproducible validation receipt exists for the stated bounded behavior;
- `RESEARCH REQUIRED` / `NOT PROVEN` — stronger behavior/semantics/equivalence are intentionally withheld;
- `CORRECTED` — replaces an older conclusion;
- `REJECTED` — disproven and retained to prevent regression;
- `COMPLETE` — reserved for a major subsystem only after the full applicable completion gate is satisfied.

`IMPLEMENTED`, `TESTED`, green CI, readable C++, or `BOUNDED CLOSED` do **not** imply subsystem `COMPLETE`.

## Evidence hierarchy

Prefer direct evidence in this order where applicable:

1. exact raw artifact bytes / exact hash-gated disassembly;
2. verified runtime observation with trace integrity;
3. reproducible structural/corpus observation;
4. sanitized Evidence Packet/reconciliation record tied to exact artifact identity;
5. evidence-backed reconstruction tested against the above;
6. hypothesis/candidate interpretation.

Agent agreement, model consensus, naming intuition and passing synthetic tests are not substitutes for missing direct evidence.

## Required evidence record

A reverse record should include, as applicable:

- exact artifact role, SHA-256 and size;
- executable/profile/build identity;
- file offset and RVA/VA/address range;
- source byte/disassembly provenance or sanitized hash-gated window identity;
- callers/callees/xrefs;
- argument/return ABI;
- memory reads/writes and structure offsets;
- ownership/lifetime boundary;
- competing interpretations;
- confidence/status;
- reproduction procedure;
- validation result/receipt;
- links to implementation/tests;
- correction/supersession links.

If the raw artifact was not actually mounted in the current pass, do not describe prior project evidence as a fresh independent re-hash/re-disassembly.

## Recovered C++ rule

Readable or compilable C++ is not automatically recovered original behavior.

A reconstructed unit must remain tied to:

- exact artifact evidence;
- reconstruction identity;
- ABI;
- ownership/lifetime assumptions;
- unresolved fields/branches;
- compile receipt;
- controlled behavioral comparison when equivalence is claimed.

Recovered C++ is an evidence-backed executable specification, not leaked/original Capcom source by default.

## Bounded closure rule

Close exactly what the evidence closes.

Example:

```text
combined query wrapper ABI = BOUNDED CLOSED
```

may coexist with:

```text
primitive producer semantics = RESEARCH REQUIRED
source2 lifetime = RESEARCH REQUIRED
collision subsystem = NOT COMPLETE
```

Do not reopen a bounded-closed target merely because an older pass document still lists it as open. Reopen only on contradictory/new direct evidence.

Do not extrapolate a closed wrapper/function into the entire subsystem.

## Historical/supersession rule

Historical pass documents remain immutable evidence history where practical.

When newer evidence conflicts:

- append a supersession/correction notice;
- mark the old claim `CORRECTED` or `REJECTED`;
- update the current canonical issue/status surface;
- preserve the old record for provenance.

The latest current-authority surface wins over stale checklists, but raw evidence wins over all summaries.

## Stage reverse rule

Do not use `st001` or `stNNN` filename templates as canonical Stage identity.

Keep separate:

1. resource-set/catalog identity;
2. numeric Stage selector identity;
3. semantic/gameplay Stage identity only when independently evidenced.

Current Wave-2 evidence uses 189 observed descriptors, 193 selectors and 10 group-base pointers. The 189 descriptors are not automatically 189 gameplay stages. `st001` is regression/compatibility data only.

## Ownership rule

Tool relationships do not define ownership of original game code.

- original DMC3 runtime reconstruction -> Recovered Game Source Tree;
- generic evidence/reconstruction/claim infrastructure -> Reverse Core;
- product resource authority -> GDSpaces;
- product scene assembly/operations -> Stage Ops;
- derived semantic representation -> Stage Semantic Graph;
- editing view -> ModViz;
- byte/structure evidence inspection -> Binary Inspector.

Do not copy recovered original-game runtime code into product tools merely because they consume its behavior.

## Runtime/equivalence rule

A synthetic test verifies the implementation contract represented by the fixture. It does not establish original-game behavior.

Behavioral equivalence claims require controlled comparison against the canonical executable/runtime and a reproducible ValidationReceipt appropriate to the scope.

Lifecycle claims must include relevant creation/load/use/reload/transition/release/unload behavior.

## Patch/edit rule

Executable or resource modification requires, as applicable:

- exact source artifact identity;
- expected source bytes/ranges;
- semantic purpose;
- dependency/conflict checks;
- WorkingCopy/revision boundary;
- rollback data;
- validation procedure;
- copied-output/reintegration policy;
- runtime test where behavior is changed.

Never weaken an evidence or safety gate merely to obtain green CI.

## Public repository rule

Do not commit game executables, proprietary assets, extracted game archive payloads, copyrighted binary blobs, leaked source, credentials, private paths, or user-specific installations.

Use sanitized evidence and synthetic fixtures. Legally obtained game data stays local.
