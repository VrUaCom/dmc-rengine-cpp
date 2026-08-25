# DMC3 Runtime Resource Resolver — Pass 45 correction + 2026-08-25 physical-provider closure

**Status:** corrected implementation slice; Layer-2 promotion still pending corpus/runtime receipts, fresh whole-head CI and re-review.

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

## Physical evidence class — corrected 2026-08-25

The original type-0 physical-provider chain is now recovered directly from the canonical executable. After `0x0C` normalization, `ResourceMountResolve` joins the registered physical root and normalized candidate into a bounded `0x400` path and calls a direct Win32 open helper.

The recovered final open is:

`CreateFileA(path, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, 0, NULL)`.

`ERROR_FILE_NOT_FOUND` and `ERROR_PATH_NOT_FOUND` are ordinary open misses. `ResourcePathExists` uses `FindFirstFileA` / `FindClose`; `ERROR_NO_MORE_FILES`, `ERROR_FILE_NOT_FOUND` and `ERROR_PATH_NOT_FOUND` are ordinary existence-check misses. Other errors are retried. No extra game-side lowercase/qsort/bsearch stage exists between `0x0C` normalization and these Win32 path APIs in the recovered edge.

That closes the previous **static reverse** target, but it does not make the current GDSpaces physical implementation identical to the original mechanism. GDSpaces currently resolves the physical pass through a source-derived `0x0C` `ResourceKeyIndex` built from `ISource::enumerate()`.

Every current product physical probe therefore intentionally remains:

`RuntimeLookupEvidenceClass::product_physical_index`.

This classification now means **“portable product lookup differs mechanically from the recovered original direct Win32 path”**, not **“the original final Win32 path is unknown.”** Relabeling it as original-equivalent before a controlled physical-provider parity receipt would be authority laundering.

The instruction-backed constants and miss classifications are codified in `PhysicalProviderContract`; the direct reverse receipt is `l2-physical-provider-reverse-2026-08-25.md`.

## Recovered ordering contract

The candidate attempt is the outer loop. For an archive attempt, archive precedence is the inner loop. An earlier-prefix hit in a lower-precedence archive therefore beats a later-prefix hit in a higher-precedence archive. Within one candidate the highest contiguous archive volume is consulted first.

All six archive attempts complete before the six physical attempts. Numbered archive precedence remains prepend-derived `N..0`.

A zero-volume bootstrap is valid: no archive probes are produced and the resolver proceeds directly to the physical six-candidate pass. With three archive volumes, a complete miss produces `6 * 3 + 6 = 24` probes.

## Ambiguity

Comparator-equal normalized keys inside the current source remain ambiguity. The resolver returns all distinct `ResourceRef` identities and does not continue into a lower-precedence source to manufacture a winner.

For the archive backend this preserves uncertainty where the original CRT duplicate-key winner has not been proven. For the physical backend it is a product-safe policy of the current source-derived index, not evidence that the original direct `CreateFileA` path performs archive-style normalized-key arbitration.

## Fail-closed boundaries

- embedded-NUL/invalid request fails before source enumeration/probes;
- runtime bindings must exactly cover the contiguous bootstrap archive set;
- duplicate volume bindings and duplicate archive source IDs are invalid;
- archive source IDs may not alias the physical source ID;
- every referenced source must be mounted;
- a mounted source must enumerate resources belonging to itself;
- provider normalization unexpectedly rejecting a canonical candidate is configuration failure;
- no external index pointer/profile can be injected into the resolver anymore.

## Layer-2 targets still open

Static reverse of the exact type-0 final physical open is no longer the blocker. The remaining Layer-2 promotion gates are now:

1. controlled physical-provider parity/model receipt;
2. real-retail `0x0E` normalized-key collision census;
3. direct-retail resolver receipt;
4. explicit `OpenGameResource` caller/fallback census;
5. closure of the separate `OpenGameResource` `0x400` oversized-candidate aftermath;
6. controlled physical/missing/fallback receipts;
7. original-process selected-identity receipt;
8. exact-head Windows + Ubuntu validation and final Layer-2 audit.

This slice still does not implement `.lst` synthesis, original FileSlot/async/cache/refcount/LoadedResource lifecycle or Stage Ops assembly. `.afs/` prefixes remain logical namespaces; no binary AFS backend is implied.
