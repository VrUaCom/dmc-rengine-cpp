# DMC3 Numbered NBZ Volume Bootstrap — Reconciled Slice

**Date:** 2026-08-18  
**Status:** REVIEWED CORRECTION / WHOLE-HEAD CI REQUIRED BEFORE PROMOTION

`Dmc3ResourceBootstrap` at VA `0x14002E930` derives the executable-relative `\\data\\dmc3\\` root and probes `%sDMC3-%d.nbz` from index zero.

The recovered runtime registers numbered volumes only while the sequence remains contiguous. The first missing `DMC3-N.nbz` terminates probing; later numbered files are not runtime-equivalent mounts for this path.

The physical DMC3 data root is registered first. Numbered archives are then registered ascending, while every mount node is prepended. Thus contiguous `0,1,2` produces archive registration `0 -> 1 -> 2` but effective archive resolution `2 -> 1 -> 0`, followed by the earlier physical root where provider masks permit it.

## Signed filename namespace correction

Second-pass review found that the initial product implementation accepted all `uint32_t` values and even treated `DMC3-4294967295.nbz` as a canonical runtime volume name. That overstates the recovered contract: the executable format string is `%sDMC3-%d.nbz`, so the recovered non-negative numbered-volume namespace is signed decimal.

The corrected product boundary is therefore:

```text
runtime-equivalent index: 0 .. INT32_MAX (2147483647)
outside runtime domain:   INT32_MAX+1 .. UINT32_MAX
```

`VolumeBootstrapPolicy::volume_filename()` returns an empty string for an out-of-domain numeric suffix instead of manufacturing a runtime-equivalent name. `RuntimeArchiveVolume::valid()` rejects such indices.

This is a namespace/evidence correction; it does not claim that a real DMC3 installation can practically contain billions of contiguous archives.

## Product discovery vs runtime mounts

`VolumeBootstrapPolicy::plan(present_indices)` deliberately separates product discovery from recovered mount state.

- in-domain files after the first runtime gap remain in `present_after_first_gap` for diagnostics/navigation;
- numeric `DMC3-N.nbz` discoveries above `INT32_MAX` remain in `present_outside_runtime_index_domain`;
- neither category is promoted as a recovered runtime mount.

This keeps product scanning broad without silently widening the recovered `%d` namespace.

`RuntimeArchiveVolume::resolution_rank` remains explicit: rank `0` is consulted first among archive mounts. Canonical filename validation inside `valid() noexcept` is allocation-free and rejects alternate spellings such as leading-zero indices.

## Regression

The regression now locks:

- `0` and ordinary positive indices;
- `INT32_MAX -> DMC3-2147483647.nbz`;
- `INT32_MAX+1` and `UINT32_MAX` do not produce runtime filenames;
- out-of-domain `RuntimeArchiveVolume` is invalid;
- normal contiguous first-gap semantics and prepend-derived precedence remain unchanged;
- in-domain files after a gap remain diagnostic-only;
- out-of-domain numeric discoveries are separately preserved, deduplicated and sorted;
- malformed plan receipts that place an in-domain index in the out-of-domain list are invalid.

Not included: actual SourceRegistry mounting, six-prefix candidate construction, provider key normalization, `ResourceKeyIndex`, cross-volume runtime resolver composition, `.lst`, FileSlot/async/cache/refcount/scene lifecycle.

Primary reverse authority: GitHub issue #100 Pass 3 and later archive-runtime reconciliation. Broader request-to-unload authority remains issue #55.
