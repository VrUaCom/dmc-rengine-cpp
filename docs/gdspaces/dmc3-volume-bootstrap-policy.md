# DMC3 Numbered NBZ Volume Bootstrap — Reconciled Slice

**Date:** 2026-08-18  
**Status:** IMPLEMENTED / WHOLE-HEAD CI REQUIRED BEFORE PROMOTION

`Dmc3ResourceBootstrap` at VA `0x14002E930` derives the executable-relative `\\data\\dmc3\\` root and probes `%sDMC3-%d.nbz` from index zero.

The recovered runtime registers numbered volumes only while the sequence remains contiguous. The first missing `DMC3-N.nbz` terminates probing; later numbered files are not runtime-equivalent mounts for this path.

The physical DMC3 data root is registered first. Numbered archives are then registered ascending, while every mount node is prepended. Thus contiguous `0,1,2` produces archive registration `0 -> 1 -> 2` but effective archive resolution `2 -> 1 -> 0`, followed by the earlier physical root where provider masks permit it.

`VolumeBootstrapPolicy::plan(present_indices)` deliberately separates product discovery from recovered mount state. Files observed after the first gap remain in `present_after_first_gap` for diagnostics/navigation but are not promoted as mounted archives.

`RuntimeArchiveVolume::resolution_rank` is explicit: rank `0` is consulted first among archive mounts.

Not included: actual SourceRegistry mounting, six-prefix candidate construction, provider key normalization, normalized-key duplicate policy, `ResourceMountResolve`, `.lst`, FileSlot/async/cache/refcount/scene lifecycle.

Primary reverse authority: GitHub issue #100 Pass 3 and later archive-runtime reconciliation. Broader request-to-unload authority remains issue #55.
