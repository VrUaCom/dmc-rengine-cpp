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

The executable performs one complete provider-mask `1` archive pass across all six candidates, then provider-mask `2` physical pass across the same six. Candidate construction uses a bounded `0x400`-byte destination.

`ResourceLookupPolicy` therefore produces twelve deterministic attempts and preserves basename case/bytes at this stage. Provider key normalization is intentionally separate: the now-canonical DMC3 path-normalization layer applies provider-specific transforms only after candidate construction.

The `.afs/` strings are logical namespaces, not evidence for a binary AFS backend.

The product plan is complete-or-invalid for synthetic oversized requests; exact malformed/overflow helper return control flow is not promoted as original-runtime equivalence by this slice.

Not included: normalized-key index/search or duplicate behavior, contiguous `DMC3-N.nbz` bootstrap, actual mount resolution, `.lst`, FileSlot/async/cache/refcount/scene lifecycle.

Primary reverse authority: GitHub issue #100 Pass 3 and later reconciliation. Broader lifecycle authority: issue #55.
