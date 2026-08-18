# DMC3 Numbered NBZ Volume Bootstrap — Reconciled Slice

**Date:** 2026-08-18  
**Status:** IMPLEMENTED ON WORK BRANCH / PROMOTION DEFERRED UNTIL CURRENT VFS SLICES STABILIZE

## Purpose

Represent the bounded numbered-archive bootstrap behavior recovered from the DMC3 executable without making product filesystem discovery itself look like original runtime mount state.

Direct reverse authority identifies `Dmc3ResourceBootstrap` at VA `0x14002E930` as the path that derives the executable-relative `\\data\\dmc3\\` root and probes `%sDMC3-%d.nbz` from index zero.

## Contiguous first-gap rule

The original bootstrap probes:

```text
DMC3-0.nbz
DMC3-1.nbz
DMC3-2.nbz
...
```

and stops at the first missing index.

Therefore a product scan that sees:

```text
DMC3-0.nbz
DMC3-2.nbz
DMC3-3.nbz
```

must not represent volumes 2/3 as original-runtime mounts for this path. They remain useful product discovery evidence and are preserved in `present_after_first_gap`.

## Registration versus resolution order

Recovered mount-list behavior is prepend-based:

1. physical DMC3 data root is registered first;
2. numbered archives are registered in ascending order;
3. each archive node is prepended to the same mount list.

For contiguous volumes `0,1,2`, archive registration is:

```text
0 -> 1 -> 2
```

while effective archive resolution is:

```text
2 -> 1 -> 0 -> earlier physical root
```

`RuntimeArchiveVolume::resolution_rank` is explicit: rank `0` means the archive is consulted first among archive mounts.

## Product/runtime boundary

`VolumeBootstrapPolicy::plan(present_indices)` consumes product-side discovery results. It does not probe the host filesystem itself and does not own GDSpaces sources.

This preserves two distinct facts:

```text
what files the product can see
!=
what the recovered original bootstrap would mount
```

## Not implemented by this slice

- actual `NbzZipSource` registration in `SourceRegistry`;
- `ResourceMountResolve` execution;
- six-prefix caller candidate planning;
- provider key normalization;
- normalized duplicate-key product policy;
- `.lst` reconstruction;
- original mount-node allocation/destruction, FileSlot, async I/O, cache/refcount or scene lifecycle.

## Evidence boundary

This is a bounded representation of the recovered bootstrap/mount ordering. It is not whole VFS or resource-lifecycle equivalence.

Primary live reverse authority: GitHub issue #100 Pass 3 and later archive-runtime reconciliation. Broader request-to-unload authority remains issue #55.
