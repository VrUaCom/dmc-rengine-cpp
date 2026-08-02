# Current Blockers

## B-001 — CI result visibility

**Priority:** P0  
**Status:** open

The repository workflow is configured for Windows and Linux, but the connected status API has not yet returned check results for the current head.

Resolution:

- inspect GitHub Actions after the next push;
- fix compiler-specific diagnostics;
- require green build/test checks before the first tagged release.

## B-002 — No serialized evidence packet schema

**Priority:** P0  
**Status:** open

The in-memory Evidence Registry exists, but evidence cannot yet be saved, reviewed, versioned, or exchanged as a stable JSON document.

Resolution:

- define schema version;
- add deterministic JSON serialization without binding core types to a UI;
- validate required IDs, confidence, artifact hashes, and correction links.

## B-003 — Artifact hashing not implemented

**Priority:** P0  
**Status:** open

Historical findings depend on exact executable/resource versions. The C++ repository does not yet calculate or store SHA-256.

Resolution: introduce an artifact identity service and CLI hash command.

## B-004 — PE inspection not implemented

**Priority:** P1  
**Status:** open

Known PE facts are documented but not reproducible by current code.

Resolution: implement read-only PE32+ parsing, bounds checks, sections, entry point, image base, and address conversion with synthetic fixtures.

## B-005 — No container source implementation

**Priority:** P1  
**Status:** open

Only local directories can currently be mounted. NBZ/AFS/PAC/PNST remain migration targets.

Resolution: define parser/source boundaries, begin with synthetic PAC/PNST fixtures, then introduce locally tested read-only volume sources.

## B-006 — Working-copy and writer policy absent in code

**Priority:** P1  
**Status:** open

The no-direct-write rule is documented but not encoded as APIs.

Resolution: immutable source payload, mutable operation journal, validation result, export plan, and rollback manifest.

## B-007 — Legacy evidence is not yet normalized

**Priority:** P1  
**Status:** open

Historical addresses and findings exist in prose and private artifacts, not in machine-readable public evidence packets.

Resolution: migrate one subsystem at a time, starting with PE identity and stage table metadata.

## B-008 — UI technology decision pending

**Priority:** P2  
**Status:** open

Qt 6, Dear ImGui, rendering backend, plugin model, and deployment constraints are not yet selected.

Resolution: defer until core data contracts and read-only inspection are stable; write an ADR comparing viable options.

## B-009 — No public synthetic format corpus

**Priority:** P2  
**Status:** open

Format work requires legally clean malformed/valid fixtures.

Resolution: build generators for minimal PAC, PNST, HITS$, LIG2, DCA, TXT, and PE samples.

## B-010 — No release pipeline

**Priority:** P2  
**Status:** open

There are no signed/versioned binary artifacts or changelog automation.

Resolution: add only after stable CLI behavior and green matrix CI.
