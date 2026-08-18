# DMC3 Resource Lookup Candidate Policy — Reconciled Slice

**Date:** 2026-08-18  
**Status:** IMPLEMENTED / WHOLE-HEAD CI REQUIRED BEFORE PROMOTION

## Purpose

Represent the bounded caller-side candidate sequence recovered for the DMC3 resource VFS path without moving archive-key normalization, mount ownership or original runtime lifecycle into this object.

The executable-side authority is `OpenGameResource` at VA `0x14002FCA0` on the canonical analyzed DMC3 executable. Current reverse evidence establishes that this path strips the incoming request to its basename, constructs six logical namespace candidates, performs one complete archive-provider pass, then one complete physical-provider pass, and uses a `0x400`-byte candidate buffer.

## Exact bounded sequence

Namespace prefixes are tried in this order:

```text
0  GDataX360.afs/
1  GData.afs/
2  Video/
3  afs/sound/
4  SAVEDATA/
5  <empty>
```

The resulting attempt order is:

```text
archive mask 1: prefix 0..5
physical mask 2: prefix 0..5
```

Therefore a complete plan contains twelve ordered attempts. Candidate construction preserves the basename bytes/case at this stage.

The `.afs/` strings above are logical namespace prefixes. They are **not** evidence for, and do not enable, a binary AFS container backend.

## Identity and normalization boundary

This candidate planner deliberately does not normalize archive keys.

Recovered `ResourcePathNormalize` / NBZ lookup behavior is a separate layer. For the evidenced NBZ path, flags `0x0E` later perform lower-ASCII normalization, leading/trailing separator removal, slash canonicalization and repeated-backslash collapse before binary-search lookup.

Keeping these stages separate prevents caller candidate identity from being silently collapsed into provider/backend key identity.

## Buffer boundary

The recovered caller constructs candidates through a bounded `0x400`-byte destination. `ResourceLookupPolicy` therefore requires each materialized candidate to leave room for the terminating NUL.

The product `ResourceLookupPlan` is complete-or-invalid: it does not expose a partial attempt vector as if it were the complete original two-pass sequence when a synthetic oversized basename cannot satisfy the bound. This is a representation safety rule; exact malformed/overflow helper return control flow is not promoted as an original-runtime equivalence claim by this slice.

Real-corpus evidence currently reports source names far below this bound, so this product rule does not alter known real lookup identities.

## Not implemented by this slice

- `ResourcePathNormalize` flags `0x0E`;
- normalized-key sort/binary search;
- normalized duplicate-key behavior;
- contiguous `DMC3-N.nbz` discovery and first-gap stop;
- effective highest-contiguous-volume precedence;
- actual `ResourceMountResolve` source selection;
- `.lst` packed-first loose-container synthesis;
- binary AFS parsing;
- original FileSlot / async I/O / LoadedResource / refcount / scene lifetime.

Those remain separate promotion/reconstruction slices.

## Evidence boundary

This is a bounded representation of the recovered caller-side search order. It is not a claim that GDSpaces reproduces the complete original DMC3 VFS/runtime lifecycle.

Primary live reverse authority: GitHub issue #100 Pass 3 and later reconciliation comments. Broader request-to-unload authority remains issue #55.
