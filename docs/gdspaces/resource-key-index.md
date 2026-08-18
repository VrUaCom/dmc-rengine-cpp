# GDSpaces Resource Key Index — Derived Lookup Layer

**Date:** 2026-08-18  
**Status:** WORK BRANCH / NOT YET PROMOTED

## Purpose

Provide a derived lookup index over existing immutable `ResourceRef` identities without turning `ISource` into a second resolver and without moving provider-specific normalization into source/materialization code.

The intended dependency is:

```text
ISource::enumerate()
        ↓
ResourceRef identities
        ↓
profile/provider key transform
        ↓
ResourceKeyIndex
        ↓
all matching ResourceRef identities
        ↓
selected exact ResourceId
        ↓
SourceRegistry::read(ResourceId)
```

`NbzZipSource::read(ResourceId)` and other source readers therefore remain exact materialization authorities. They do not receive runtime candidate strings or choose among normalized-key aliases.

## Collision contract

`ResourceKeyIndex` stores every distinct `ResourceRef` assigned to one key. Lookup returns all matches and exposes only `found / unique / ambiguous` state.

There is intentionally no winner-selection API.

This is required by current DMC3 reverse evidence: normalized duplicate archive keys are admitted to the original sorted array, comparator equality has no secondary tie-break, and CRT `qsort`/`bsearch` therefore makes duplicate selection undefined/CRT-dependent rather than a recovered semantic priority.

A future GDSpaces deterministic duplicate policy may exist, but it must be a separately named product policy and must never be presented as original DMC3 winner behavior.

## Product determinism

For stable inspection/testing, resources inside one equal-key bucket are stored in canonical `ResourceId` order. That ordering is product presentation only and carries no original-runtime precedence meaning.

One exact `ResourceId` may occur only once in one index and may not be assigned to two different keys inside the same index instance.

## Normalization boundary

The generic index does not normalize keys. DMC3 provider code supplies the already-derived key:

- archive/NBZ transform: canonical `ResourcePathPolicy` flags `0x0E`;
- physical transform: canonical flags `0x0C`.

This keeps generic identity/index code independent of DMC3 lookup semantics.

## Not owned here

- six-prefix caller candidate construction;
- numbered-volume mount precedence;
- filesystem/archive discovery;
- byte reads/materialization;
- duplicate winner selection;
- `.lst` reconstruction;
- binary AFS parsing;
- original FileSlot/cache/refcount/lifecycle behavior.
