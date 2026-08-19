# DMC3 Runtime Resource Resolver — Pass 45 correction

**Status:** corrected implementation slice; promotion still pending fresh whole-head CI and re-review.

This layer composes the recovered DMC3 lookup order while preserving the distinction exposed by Pass 45 between the archive backend and the physical backend.

```text
raw request
  -> ResourceLookupPolicy: basename + 12 ordered attempts
  -> ResourcePathPolicy: provider-specific normalization
  -> VolumeBootstrapPlan: contiguous numbered-volume precedence
  -> archive: current mounted source enumeration -> owned local ResourceKeyIndex
  -> physical: current mounted source enumeration -> product 0x0C lookup index
  -> ResourceRef / ambiguity / miss
  -> SourceRegistry read by the downstream caller
```

## Ownership correction

`ISource` remains exact identity/enumeration/read authority and `SourceRegistry` remains mount/routing authority. The resolver no longer accepts caller-owned `const ResourceKeyIndex*` values.

For every resolve call it obtains the exact currently mounted `ISource`, enumerates that source, and derives the lookup index locally. This removes the stale-index/same-ID substitution hole and the dangling raw-pointer lifetime hazard identified in review of the original #136 head.

A source enumeration is configuration-invalid if it emits invalid resources, resources owned by another source ID, C-string-incompatible paths or paths that normalize to an empty key. Duplicate physical identities may still be diagnosed by `ResourceKeyIndex`; comparator-equal distinct identities remain explicit ambiguity.

## Archive evidence class

For the original ZIP/NBZ backend the normalized index is directly recovered behavior. The archive object owns the central-entry list and sorted lookup representation; the recovered layout includes the central-entry list, sorted `{normalizedName, ZipCentralEntry*}` array and count. Index construction/lookup use `0x0E` normalization and qsort/bsearch semantics.

Resolver probes for this path therefore carry:

`RuntimeLookupEvidenceClass::recovered_archive_index`.

The GDSpaces `ResourceKeyIndex` remains a product representation that preserves all comparator-equal identities rather than pretending the original CRT ambiguity is a semantic winner.

## Physical evidence class

The original type-0 physical pass and `0x0C` path normalization are recovered. The exact downstream filename comparison/open semantics of the physical backend are not yet closed: current evidence does not justify claiming that the original backend built an archive-like qsort/bsearch index or that its final comparison is exactly the same as `ResourceKeyIndex` equality.

GDSpaces therefore uses a source-derived `0x0C` product lookup index for the physical pass, but every such probe is explicitly marked:

`RuntimeLookupEvidenceClass::product_physical_index`.

This is a product-safe operational policy, not a claim of full original-equivalent physical filename lookup. Exact type-0 comparison/open behavior remains a reverse target.

## Recovered ordering contract

The candidate attempt is the outer loop. For an archive attempt, archive precedence is the inner loop. An earlier-prefix hit in a lower-precedence archive therefore beats a later-prefix hit in a higher-precedence archive. Within one candidate the highest contiguous archive volume is consulted first.

All six archive attempts complete before the six physical attempts. Numbered archive precedence remains prepend-derived `N..0`.

A zero-volume bootstrap is valid: no archive probes are produced and the resolver proceeds directly to the physical six-candidate pass. With three archive volumes, a complete miss produces `6 * 3 + 6 = 24` probes.

## Ambiguity

Comparator-equal normalized keys inside the current source remain ambiguity. The resolver returns all distinct `ResourceRef` identities and does not continue into a lower-precedence source to manufacture a winner.

## Fail-closed boundaries

- embedded-NUL/invalid request fails before source enumeration/probes;
- runtime bindings must exactly cover the contiguous bootstrap archive set;
- duplicate volume bindings and duplicate archive source IDs are invalid;
- archive source IDs may not alias the physical source ID;
- every referenced source must be mounted;
- a mounted source must enumerate resources belonging to itself;
- provider normalization unexpectedly rejecting a canonical candidate is configuration failure;
- no external index pointer/profile can be injected into the resolver anymore.

## Reverse target still open

Pass 45 leaves one exact lookup subproblem unresolved: the type-0 physical backend after `ResourcePathNormalize(..., 0x0C)` — path construction, final Win32 open/comparison behavior, case behavior and failure semantics. Until that chain is directly recovered, physical lookup receipts must retain the product classification above.

This slice still does not implement `.lst` synthesis, original FileSlot/async/cache/refcount/LoadedResource lifecycle or Stage Ops assembly. `.afs/` prefixes remain logical namespaces; no binary AFS backend is implied.
