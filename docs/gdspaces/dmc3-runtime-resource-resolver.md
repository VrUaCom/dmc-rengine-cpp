# DMC3 Runtime Resource Resolver — Reconciled Composition

**Status:** implementation prepared; current-main whole-head promotion pending.

This layer composes already-separated authorities rather than reimplementing them:

```text
raw request
  -> ResourceLookupPolicy (basename + 12 ordered attempts)
  -> ResourcePathPolicy (provider-specific normalization)
  -> VolumeBootstrapPlan (archive precedence)
  -> SourceRegistry::lookup (identity-preserving key lookup)
  -> ResourceRef / ambiguity / miss
```

## Ordering authority

The candidate attempt is the outer loop. For an archive attempt, contiguous archive resolution order is the inner loop. Therefore a hit for an earlier prefix in a lower-precedence archive can beat a later-prefix hit in a higher-precedence archive, while within one candidate the highest-precedence archive is consulted first.

All six archive attempts complete before physical attempts begin.

## Fail-closed rules

- invalid request, including embedded NUL, produces no source probes;
- source bindings must exactly cover the contiguous bootstrap mount set;
- every bound source must already be mounted in `SourceRegistry`;
- provider normalization unexpectedly rejecting a canonical candidate is configuration/invariant failure;
- a `SourceLookupReport` with invalid provider key, unavailable expected source, mismatched key/source/flags is configuration/invariant failure, not ordinary miss;
- ambiguity in the current highest-precedence source stops resolution and preserves all matching `ResourceRef`s; no enumeration-order winner is invented.

## Evidence boundary

The recovered caller order, provider masks, path-normalization profiles, first-gap bootstrap and prepend-derived archive precedence are evidence-backed bounded contracts. `SourceRegistry`, reports and the composition data structures are GDSpaces product architecture and are not claimed to reproduce original runtime container/cache objects.

Duplicate normalized-key winner behavior remains unpromoted. `.lst`, FileSlot, async I/O, cache/refcount, loaded-resource state and scene lifecycle remain separate reconstruction tracks.
