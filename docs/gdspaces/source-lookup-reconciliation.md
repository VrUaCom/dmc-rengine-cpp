# GDSpaces Normalized Source Lookup — Duplicate-Key Reconciliation

**Date:** 2026-08-18  
**Status:** IMPLEMENTED / WHOLE-HEAD CI REQUIRED BEFORE PROMOTION

## Purpose

Promote the useful product boundary from historical PR #70 after reconciling it with the newer DMC3 archive evidence through issue #100 Pass 22 and the current path-normalization/candidate-plan stack.

This slice does **not** implement the final DMC3 mount resolver. It establishes the source-level lookup result required by that resolver.

## Recovered archive evidence

The recovered DMC3 ZIP/NBZ backend:

- normalizes archive names and requests before comparison;
- keeps all physical central-directory entries;
- stores `{normalizedName, entry*}` lookup records;
- sorts them with CRT `qsort`;
- searches with CRT `bsearch`;
- uses bytewise NUL-terminated comparison;
- has no recovered semantic secondary ordering for comparator-equal normalized keys.

Therefore two physical archive records that normalize to the same key do **not** have an evidence-backed deterministic semantic winner. A concrete result from the original CRT implementation would be an implementation/input-order artifact, not a resource-priority rule that GDSpaces should invent.

## Product contract

`ISource::lookup(provider_key, normalization_flags)` is a correctness fallback over source enumeration. It returns **every** source identity whose logical path normalizes to the already-normalized provider key.

`SourceRegistry::lookup(...)` returns `SourceLookupReport` with separate states:

- `key_valid` — caller supplied a non-empty, NUL-free key that is idempotent under the declared normalization flags;
- `source_available` — requested mounted source exists;
- `found()` — one or more matching physical identities exist;
- `unique()` — exactly one identity matches;
- `ambiguous()` — multiple physical identities normalize to the same key.

The registry does not collapse `ambiguous()` into a winner.

## Key-domain integrity

The API deliberately accepts an **already-normalized provider key**. A raw/noncanonical key is not treated as an ordinary resource miss.

For example, under DMC3 archive flags `0x0E`:

```text
Room/ST001.PAC        -> invalid lookup-key domain
room\st001.pac       -> canonical provider key
```

This prevents a caller bug between candidate construction and provider normalization from being silently misreported as `not found`.

Embedded NUL is rejected because the recovered runtime consumes NUL-terminated strings while product APIs use `std::string_view`; accepting hidden suffix bytes would create a second identity domain not present in the recovered comparator.

## DMC3 provider separation

The caller remains responsible for selecting the correct provider normalization before source lookup:

- archive/NBZ: `0x0E`;
- physical: `0x0C`.

The generic source layer does not hard-code DMC3 case rules.

## Algorithm boundary

The default `ISource` implementation is intentionally a correctness fallback and may enumerate/normalize linearly. This is **not** claimed to reproduce the original NBZ internal `qsort`/`bsearch` algorithm.

A later archive-index optimization may override the source lookup with a prebuilt normalized index, provided it preserves the same product result contract: all equal identities remain observable and ambiguity remains explicit.

## Regression

The synthetic regression locks:

- missing source vs absent key distinction;
- unique lookup;
- two physical identities collapsing to one archive key and remaining both visible;
- `ambiguous()` rather than winner selection;
- noncanonical raw key rejection;
- embedded-NUL key rejection;
- physical-provider case preservation under `0x0C`.

## Boundary

Not included here:

- DMC3 mount-list traversal;
- contiguous `DMC3-N.nbz` discovery and first-gap stop;
- highest-volume-first archive precedence;
- archive-pass then physical-pass resolution across the twelve lookup attempts;
- physical-root containment/security checks;
- `.lst` reconstruction;
- FileSlot/async/cache/refcount/scene lifecycle.

Those remain separate resolver/lifecycle slices. Original CRT duplicate ambiguity and deterministic GDSpaces product handling must remain separately labeled in future validation receipts.
