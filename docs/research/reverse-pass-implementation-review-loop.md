# Reverse Pass Implementation / Review / Debug Loop

Status: canonical workflow for HITS/EXE reverse work unless a stricter project-specific gate overrides it.

## Purpose

A reverse-engineering pass is not complete when a plausible interpretation is found. Each pass must carry evidence from raw/recovered observations through review, deepening, implementation of only the promoted subset, debugging, consolidation, and a second review/debug cycle. Documentation is a first-class output and must be updated together with code.

## Required pass loop

### 1. Evidence acquisition

- identify exact target artifact/hash/profile;
- collect raw bytes, instruction windows, xrefs, call sites, runtime traces, corpus/resource evidence, or previously preserved evidence;
- record provenance and negative searches;
- do not infer missing bytes or ABI fields.

### 2. Step review

For every reverse step:

- re-read the direct evidence;
- identify assumptions;
- search for contradictory callers/callees/writers/readers;
- separate raw observation from interpretation;
- label confidence/evidence status;
- preserve rejected interpretations rather than silently deleting them.

### 3. Deepening

Before promotion, expand the step laterally and vertically:

- callers and callees;
- register/stack ABI;
- field writers/readers;
- ownership/lifetime;
- state transitions;
- alternative code paths;
- resource/corpus cross-checks;
- negative xref/search sweep;
- interaction with neighboring runtime systems.

Repeat review + deepening until the step stabilizes or reaches an explicit evidence boundary.

### 4. Promotion gate

A finding may be implemented only when the implementation can be stated without inventing unresolved semantics.

Allowed implementation outcomes:

- exact reconstructed behavior;
- evidence-safe structural model;
- validation/instrumentation support;
- explicit `RESEARCH REQUIRED` placeholder only when it cannot accidentally masquerade as original semantics.

Do not promote a convenient product abstraction as original-game ABI.

### 5. Implementation

Implement the promoted subset in the correct ownership layer.

For HITS:

- `formats::hits` owns file format/parser/writer/spatial reconstruction;
- evidence/tooling owns trace/evidence capture contracts;
- original DMC3 runtime ABI belongs in `recovered-game` when evidence is strong enough;
- GDSpaces owns resource identity/resolution/provenance/bytes, not original collision behavior;
- HITS Editor / ModViz / Stage Ops consume those contracts and do not create parallel loaders or semantics.

### 6. Implementation review

Review the code against the evidence, not only against style expectations:

- every semantic name must be supported;
- unknown bits/fields remain preserved;
- address/hash/profile gates match the evidence;
- no accidental architecture ownership drift;
- no compatibility fallback reintroduces rejected reverse assumptions;
- tests must assert evidence boundaries as well as happy paths.

### 7. Debug / validation cycle

Run the strongest available validation:

- compile on supported toolchains;
- unit/regression tests;
- malformed/negative tests;
- deterministic/round-trip tests where applicable;
- corpus comparison;
- runtime instrumentation;
- controlled game validation;
- restart/save/load validation where persistence matters.

A failing validation creates a new reverse/debug step, not an excuse to weaken the test.

### 8. Pass consolidation

Consolidate the full pass into one authority packet containing:

- evidence inventory;
- function/field/caller matrices;
- promoted findings;
- corrections and rejected interpretations;
- implementation diff/commit/PR receipt;
- test/debug receipt;
- unresolved boundaries;
- next-pass targets.

The consolidated pass supersedes fragmented working notes as current authority while preserving historical receipts.

### 9. Second review + debug

After consolidation, perform another independent review:

- compare consolidated claims back to direct evidence;
- compare docs to actual code;
- compare tests to both code and evidence;
- search for stale documentation/planning references;
- rerun relevant CI/debug checks;
- correct any drift before declaring the pass closed.

### 10. Documentation synchronization

Documentation updates are mandatory, not optional cleanup.

For active HITS reverse work update, as applicable:

- Google Drive detailed pass document;
- HITS Reverse Core Mega Synthesis;
- HITS Canonical Preservation Registry;
- GitHub research document;
- issue #25 current authority/planning state;
- PR body/receipts;
- code comments/evidence manifests/tests when they encode the contract.

No material finding, correction, implementation decision, failure/debug lesson, or open gate may remain only in chat.

Google Drive canonical mirror: `DMC Rengine — Canonical Reverse Pass Implementation Review Debug Loop — 2026-08-14`, document ID `1ubJpKLOjI7o1KH-BUK36md8hOGBqZ6uVd1xP2fVHcSE`.

## Closure rule

A pass closes only when one of these is true:

1. the scoped behavior is reconstructed, implemented, reviewed, debugged, consolidated, re-reviewed and documented; or
2. the available evidence is statically saturated and the exact missing evidence required for progress is documented, after which the next pass changes method (for example evidence reacquisition or runtime instrumentation).

Repeatedly summarizing the same evidence without new information is not a new pass.
