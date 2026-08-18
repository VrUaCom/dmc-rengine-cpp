# GDSpaces Resource Key Index — Corrected Ownership

**Date:** 2026-08-18  
**Status:** IMPLEMENTED / WHOLE-HEAD CI REQUIRED BEFORE PROMOTION

`ISource` remains the exact resource authority for enumerate/read/materialize. Provider-specific normalized lookup is a derived representation and must not be folded into exact source ownership.

`ResourceKeyIndex` is therefore bound to one `source_id` and stores externally supplied derived keys -> immutable `ResourceRef` identities.

## Contract

- one index belongs to one exact source/mount identity;
- normalization is performed outside the index by profile/provider code;
- one `ResourceId` may be inserted once and may not be reassigned to a second key;
- equal derived keys preserve every distinct physical `ResourceRef`;
- no winner is selected inside the index;
- cross-source precedence is not flattened into one bucket and remains resolver authority;
- deterministic ordering inside a product ambiguity bucket is presentation/testing behavior only.

## Original DMC3 evidence boundary

Pass 22 establishes the original NBZ lookup shape more precisely:

- ZIP central records are normalized with `0x0E`;
- a separate `0x10` lookup array stores normalized-name pointer + central-entry pointer;
- imported CRT `qsort` sorts that array using the recovered bytewise comparator;
- imported CRT `bsearch` returns a comparator-equal element;
- there is no secondary tie-break for equal normalized keys.

Therefore normalized duplicates do **not** have an evidence-backed semantic winner. Any original winner in a colliding archive is an artifact of concrete CRT sorting/search and input arrangement. GDSpaces preserves the conflict instead of canonizing one arbitrary CRT result.

The supplied real NBZ corpus has no normalized collisions, so no product winner policy is required for compatibility with that corpus.

## Ownership correction

The previously promoted `ISource::lookup()` / `SourceRegistry::lookup()` seam is superseded by this derived-index architecture. Useful ambiguity semantics are retained, but exact source authority returns to `enumerate/read/materialize` only.

Resolver composition must query source-bound `ResourceKeyIndex` instances in explicit provider/mount precedence order. `.lst`, FileSlot, cache/refcount and scene lifecycle remain separate layers.
