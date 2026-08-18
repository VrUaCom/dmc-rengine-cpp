# GDSpaces Resource Key Index — Derived Lookup Layer

**Date:** 2026-08-18  
**Status:** IMPLEMENTED / WHOLE-HEAD CI REQUIRED BEFORE PROMOTION

## Purpose

Provide a derived lookup index over existing immutable `ResourceRef` identities without turning `ISource` into a second resolver and without moving provider-specific normalization into source/materialization code.

The intended dependency is:

```text
one exact ISource / mount
        ↓ enumerate()
ResourceRef identities
        ↓
profile/provider key transform
        ↓
source-bound ResourceKeyIndex
        ↓
all matching ResourceRef identities in that source
        ↓
ordered resolver across source/mount indexes
        ↓
selected exact ResourceId
        ↓
SourceRegistry::read(ResourceId)
```

`NbzZipSource::read(ResourceId)` and other source readers therefore remain exact materialization authorities. They do not receive runtime candidate strings or choose among normalized-key aliases.

## Source-bound contract

Every `ResourceKeyIndex` is constructed for one non-empty `source_id` and rejects `ResourceRef` values from any other source.

This is intentional. Cross-volume precedence is a property of the ordered mount/resolver layer, not an equal-key bucket. A global index spanning `DMC3-2.nbz`, `DMC3-1.nbz`, `DMC3-0.nbz` would erase the recovered source-order dimension before resolution even begins.

## Collision contract

The index stores every distinct `ResourceRef` assigned to one key inside its bound source. Lookup returns all matches and exposes only `found / unique / ambiguous` state.

There is intentionally no winner-selection API.

Current DMC3 reverse evidence shows that equal normalized archive keys can enter the original sorted array while the comparator has no secondary tie-break. CRT `qsort`/`bsearch` therefore makes duplicate selection undefined/CRT-dependent rather than a recovered semantic priority.

A future GDSpaces deterministic duplicate policy may exist, but it must be separately named product behavior and must never be presented as original DMC3 winner semantics.

## Product determinism

For stable inspection/testing, resources inside one equal-key bucket are stored in canonical `ResourceId` order. That ordering is product presentation only and carries no original-runtime precedence meaning.

One exact `ResourceId` may occur only once in one index and may not be assigned to two different keys inside the same index instance.

## Normalization boundary

The generic index does not normalize keys. DMC3 provider code supplies the already-derived key:

- archive/NBZ transform: canonical `ResourcePathPolicy` flags `0x0E`;
- physical transform: canonical flags `0x0C`.

This keeps generic identity/index code independent of DMC3 lookup semantics.

## Reconciliation note

Merged PR #127 briefly placed normalized lookup on `ISource`/`SourceRegistry`. That ownership is superseded by this derived-index model. The useful #127 state taxonomy and all-matches rule remain evidence for the future ordered resolver, while exact sources return to enumerate/read/materialization ownership only.

## Not owned here

- six-prefix caller candidate construction;
- numbered-volume mount precedence;
- filesystem/archive discovery;
- byte reads/materialization;
- duplicate winner selection;
- `.lst` reconstruction;
- binary AFS parsing;
- original FileSlot/cache/refcount/lifecycle behavior.
