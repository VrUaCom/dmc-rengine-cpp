# DMC3 Runtime Resource Resolver — ResourceKeyIndex Composition

**Status:** current-main implementation slice; promotion pending whole-head review/CI.

This layer composes already-promoted authorities without moving normalized lookup back into exact source ownership:

```text
raw request
  -> ResourceLookupPolicy: basename + 12 ordered attempts
  -> ResourcePathPolicy: provider-specific normalization
  -> VolumeBootstrapPlan: contiguous numbered-volume precedence
  -> source-bound ResourceKeyIndex
  -> ResourceRef / ambiguity / miss
  -> SourceRegistry read by the downstream caller
```

## Ownership

- `ISource`: exact identity, enumeration and read/materialization.
- `SourceRegistry`: mount/find/enumerate/read routing.
- `ResourceKeyIndex`: derived normalized-key buckets for exactly one source.
- `RuntimeResourceResolver`: DMC3 candidate order, provider order and cross-source precedence.

The resolver receives explicit non-owning source/index bindings. A binding is valid only when the index is valid, its `source_id` exactly matches the bound source and its normalization flags match the provider profile (`0x0E` archive, `0x0C` physical). A wrong index profile is configuration failure, never an ordinary miss.

## Recovered ordering contract

The candidate attempt is the outer loop. For an archive attempt, archive precedence is the inner loop. Therefore an earlier prefix hit in a lower-precedence archive beats a later-prefix hit in a higher-precedence archive. Within one candidate, the highest contiguous archive volume is consulted first.

All six archive attempts complete before the six physical attempts. Numbered archive precedence remains the prepend-derived `N..0` order established by the bootstrap policy.

## Ambiguity

Comparator-equal normalized keys inside one source remain ambiguity. The resolver stops at the current highest-precedence source and returns all matching `ResourceRef` identities. It never uses product enumeration order, deterministic diagnostic ordering or a lower-precedence source to manufacture a semantic winner.

## Fail-closed boundaries

- invalid request, including embedded NUL, produces no source probes;
- runtime source/index bindings must exactly cover the contiguous bootstrap mount set;
- every bound source must be mounted in `SourceRegistry`;
- every bound index must match source ID and expected normalization flags;
- provider normalization unexpectedly rejecting a canonical candidate is configuration failure;
- invalid index/key state is configuration failure, not resource miss.

## Evidence boundary

Recovered caller ordering, provider masks, path-normalization profiles, first-gap bootstrap and prepend-derived archive precedence are bounded original-runtime findings. `ResourceKeyIndex`, binding/report structures and this composition API are GDSpaces product architecture.

This slice does not implement `.lst` synthesis, physical-root containment, original FileSlot/async/cache/refcount/LoadedResource lifecycle or Stage Ops assembly. `.afs/` prefixes remain logical namespaces; no binary AFS backend is implied.
