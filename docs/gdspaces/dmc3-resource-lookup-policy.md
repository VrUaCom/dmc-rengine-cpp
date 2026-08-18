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

`ResourceLookupPolicy` therefore produces twelve deterministic attempts and preserves basename case/bytes at this stage. Provider key normalization is intentionally separate: the canonical DMC3 path-normalization layer applies provider-specific transforms only after candidate construction.

The `.afs/` strings are logical namespaces, not evidence for a binary AFS backend.

## Product integrity boundaries

The recovered caller consumes C strings, while the product planner accepts `std::string_view`. Embedded NUL input therefore fails closed and cannot create a hidden suffix/key domain.

A `ResourceLookupPlan` is valid only when its stored basename is still derivable from its stored original request and all twelve attempts remain internally consistent. A copied/mutated request cannot keep a stale attempt vector and still pass validation.

The product plan is complete-or-invalid for synthetic oversized requests. The `0x400` destination bound is recovered; exact original continuation behavior after one oversized prefix is not separately proven, so complete-or-invalid handling remains conservative GDSpaces product behavior rather than original-runtime equivalence.

Not included: normalized-key index/search or duplicate behavior, contiguous `DMC3-N.nbz` bootstrap, actual mount resolution, `.lst`, FileSlot/async/cache/refcount/scene lifecycle.

Primary reverse authority: GitHub issue #100 Pass 3 and later reconciliation. Broader lifecycle authority: issue #55.
