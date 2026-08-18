# GDSpaces Resource Key Index — Corrected Normalized Lookup Ownership

**Date:** 2026-08-18  
**Status:** CORRECTIVE IMPLEMENTATION / WHOLE-HEAD CI REQUIRED BEFORE PROMOTION

## Why this correction exists

PR #127 merged a useful normalized-key ambiguity model but placed normalization/query ownership on `ISource::lookup(provider_key, normalization_flags)` and `SourceRegistry::lookup(...)`.

The reconciliation review completed around that merge corrected the architecture: `ISource` must remain the exact source authority for enumeration and byte materialization. Provider-normalized lookup is a **derived index over immutable `ResourceRef` values**, not source behavior.

This change corrects the ownership race without rejecting the useful #127 semantics.

## Canonical ownership

`ISource` owns only:

- source identity/kind;
- exact resource enumeration;
- exact resource read/materialization.

`SourceRegistry` owns only:

- mounting/finding source instances;
- aggregate enumeration;
- routing an exact `ResourceId` read to its source.

`ResourceKeyIndex` owns:

- one explicit `source_id`;
- one normalization profile/flag set;
- a derived normalized-key projection over immutable `ResourceRef` values from that source;
- product-deterministic indexing for lookup and diagnostics;
- all-match ambiguity reporting.

The index never calls or owns an `ISource` and therefore cannot flatten cross-source or cross-volume precedence.

## Build safety and receipt

`ResourceKeyIndexBuildReceipt` records:

- input count;
- indexed identity count;
- invalid `ResourceRef` rejects;
- wrong-source rejects;
- embedded-NUL/C-string-domain rejects;
- empty-normalized-key rejects;
- exact duplicate identity collapse count;
- number of normalized keys with multiple distinct identities.

Only valid resources bound to the requested source enter the index. Exact duplicate copies of one `ResourceId` are not allowed to manufacture false ambiguity.

## Lookup contract

The caller supplies an already-normalized provider key. A query is valid only when it is:

- non-empty;
- C-string compatible;
- idempotent under the index normalization flags.

`ResourceKeyMatchReport` keeps `index_valid` and `key_valid` separate from hit state. It exposes:

- no hit;
- unique hit;
- ambiguous hit.

Source availability is deliberately **not** encoded here because an index is already source-bound. Missing source/index state belongs to the later ordered resolver composition layer.

## Duplicate normalized keys

Issue #100 Pass 22 establishes that the recovered NBZ index keeps all physical central entries, normalizes names, sorts `{normalizedName, entry*}` with CRT `qsort`, and looks up with `bsearch` using a comparator with no semantic secondary tie-break.

Therefore comparator-equal normalized keys have CRT/input-order-dependent behavior, not an evidence-backed semantic winner policy.

GDSpaces consequently preserves every distinct matching `ResourceId` and reports ambiguity. `ResourceKeyIndex` sorts equal keys by stable product identity only so diagnostics/tests are deterministic; that secondary ordering is **not** a claimed original-game tie-break and is never used to choose a winner.

## Provider separation

DMC3 profile policy remains outside generic source ownership:

- archive/NBZ normalization: `0x0E`;
- physical normalization: `0x0C`.

A separate `ResourceKeyIndex` can be built for each source/provider normalization domain.

## Future resolver composition

The DMC3 resolver should compose these layers explicitly:

```text
raw request
 -> ResourceLookupPolicy (basename + 12 attempts)
 -> provider-specific ResourcePathPolicy normalization
 -> ordered source-bound ResourceKeyIndex lookup
 -> exact SourceRegistry/ISource read by selected ResourceId
```

Archive volume precedence remains an ordered resolver concern, not an index concern. For contiguous numbered DMC3 volumes the recovered bootstrap order is highest mounted volume to lowest; equal-key ambiguity inside one source remains visible instead of becoming an enumeration-order winner.

For the physical provider, product filesystem access must additionally prove root containment after normalization and before opening a path. That is product hardening, not a recovered candidate-construction rule.

## Regression

The retained `source_lookup` CTest slot now validates the corrected derived-index architecture:

- two distinct archive identities normalize to one key and both remain visible;
- an exact duplicate `ResourceId` does not create false ambiguity;
- invalid resources do not enter the index;
- wrong-source identities do not enter the index;
- embedded-NUL source paths do not enter the index;
- empty normalized keys do not enter the index;
- noncanonical and embedded-NUL query keys fail closed;
- valid-index/no-hit differs from invalid-index state;
- physical `0x0C` lookup preserves case.

## Boundary

This slice does not implement:

- numbered-volume bootstrap itself;
- cross-volume resolution order;
- the complete twelve-attempt runtime resolver;
- physical filesystem access/root containment implementation;
- `.lst` reconstruction;
- original FileSlot/async/cache/refcount/scene lifecycle.
