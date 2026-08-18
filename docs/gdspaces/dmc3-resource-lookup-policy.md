# DMC3 Resource Lookup Candidate Policy — Reconciled Slice

**Date:** 2026-08-18  
**Status:** IMPLEMENTED / WHOLE-HEAD CI REQUIRED BEFORE PROMOTION

`OpenGameResource` at VA `0x14002FCA0` establishes the bounded caller-side lookup sequence represented here.

The request is reduced to its basename after the last `/` or `\\`, then six logical namespace prefixes are tried in exact order:

```text
GDataX360.afs/
GData.afs/
Video/
afs/sound/
SAVEDATA/
<empty>
```

The executable performs one complete provider-mask `1` archive pass across all six candidates, then one provider-mask `2` physical pass across the same six. Candidate construction uses a bounded `0x400`-byte destination.

`ResourceLookupPolicy` therefore produces twelve deterministic attempts and preserves basename case/bytes at this stage. Provider key normalization is intentionally separate: the canonical DMC3 path-normalization layer applies provider-specific transforms only after candidate construction.

The `.afs/` strings are logical namespaces, not evidence for a binary AFS backend.

## Product safety / representation boundary

The recovered caller consumes NUL-terminated paths, while the product API receives `std::string_view`. Requests containing embedded NUL therefore fail closed and do not enter the candidate domain. This prevents a suffix after NUL from becoming a second hidden product identity.

`ResourceLookupPlan::valid()` also binds the stored basename back to `original_request`; a manually modified request cannot retain a stale but superficially consistent attempt list.

The `0x400` destination bound is recovered. The current product representation is complete-or-invalid when any candidate would overflow that destination. Exact original continuation/abort behavior after one oversized candidate is not separately proven and is **not** claimed by this slice.

## Not included

- normalized-key source lookup / archive index search;
- duplicate-normalized-key winner behavior;
- contiguous `DMC3-N.nbz` bootstrap and first-gap stop;
- actual mount/source resolution;
- `.lst` synthesis;
- binary AFS parsing;
- original FileSlot/async/cache/refcount/scene lifecycle.

Primary reverse authority: GitHub issue #100 Pass 3 and later reconciliation. Broader lifecycle authority: issue #55.
