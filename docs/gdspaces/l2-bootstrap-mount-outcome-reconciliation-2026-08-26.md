# GDSpaces Layer 2 — bootstrap mount-outcome reconciliation — 2026-08-26

**Scope:** DMC3 Layer 2 bootstrap/provider topology only.  
**Canonical analysis artifact:** `dmc3.exe`, 6,356,432 bytes, SHA-256 `e454272ed0fb0247fcbcf300e5d55d7a3e96d50b89b9ffaff81bb978dcbdd082`.  
**Status:** instruction-backed correction to product topology semantics; not a retail runtime receipt.

This checkpoint extends the merged pass-2 resolver reverse. It does not retract the recovered call order. It corrects the stronger assumption that a bootstrap registration **attempt** proves a successful mount.

## 1. Physical registration has a real success/failure result

`RegisterPhysicalMount 0x140326D20`:

- allocates a `0x58`-byte node;
- writes type `0` and supplied flags (`0x0C` in bootstrap);
- duplicates the physical root string;
- prepends to global mount head `0x140CF3180`;
- returns `EAX = 1` only after the prepend succeeds.

Observed `EAX = 0` exits occur before insertion when node allocation fails or the root-string duplication/allocation fails.

Bootstrap calls this helper at `0x14002E9A9`. The next instruction sequence immediately initializes numbered archive discovery. There is no test/compare/branch on the helper return.

Therefore static authority is:

> the physical registration **attempt occurs before** numbered archive attempts.

It is not:

> the physical root is guaranteed to have mounted successfully.

## 2. Archive registration also has a real success/failure result

`RegisterArchiveMount 0x140326DA0` returns `EAX = 1` only after the initialized type-1 node is prepended at `0x140326E61..0x140326E71`.

Bounded return-0 gates before list insertion include:

1. mount-node allocation failure;
2. low-level archive file-open failure;
3. archive structure/init failure after `0x140328320`;
4. normalized search-index build failure after `0x140327CC0`;
5. archive path duplication/allocation failure.

Bootstrap calls the helper at `0x14002E9E8`, then increments the numeric index and formats/checks the next filename without testing `EAX`.

Therefore an existing/discovered numbered filename can fail registration while later numbered filenames continue to be discovered and attempted.

## 3. Three distinct surfaces are required

The recovered bootstrap semantics require three separate concepts:

```text
A. bootstrap attempts
   physical registration attempt first
   numbered archive attempts in ascending discovery order

B. filename discovery
   DMC3-0, DMC3-1, ... until first missing filename

C. successful mount topology
   only registration attempts returning success enter 0x140CF3180
```

A and B are not C.

Clean all-success execution still constructs:

```text
highest discovered archive
 -> ...
 -> DMC3-0
 -> physical
```

because successful registrations prepend.

A failure can instead produce a sparse topology. Example:

```text
physical: success
DMC3-0: success
DMC3-1: registration failure
DMC3-2: success
DMC3-3: missing
```

Effective successful provider list:

```text
DMC3-2 -> DMC3-0 -> physical
```

No `DMC3-1` lookup miss should be fabricated: no type-1 node for volume 1 exists in the resolver list.

Physical failure is also representable. If physical registration fails but archives succeed, the second provider phase traverses no type-0 node rather than manufacturing physical misses from the attempted root path.

## 4. Product correction

Previous `VolumeBootstrapPlan` terminology (`physical_root_registered_before_archives`, `registered_archives`) conflated attempts/discovery with successful original mounts.

PR #239 corrects this by separating:

- `VolumeBootstrapPlan` — filename discovery + bootstrap attempt order only;
- `RuntimeMountTopology` — explicit successful physical/archive mounts and their prepend-derived resolution order;
- `RuntimeSourceBindings` — exact source bindings for the successful topology only;
- `RuntimeResourceResolver` — traverses only successful mounts.

`VolumeBootstrapPolicy::all_success_topology()` remains available only as an explicitly named product/test clean-path convenience. It is not evidence inferred from filename presence.

## 5. Evidence consequences

- `first_missing_index` proves only the contiguous filename-discovery range.
- A trusted R3 publisher must observe registration outcomes/provider operations; it must not synthesize mounted providers from first-missing discovery.
- `OriginalResolutionObservation.v1` remains a non-promotional clean-contiguous candidate schema and is not silently widened by this correction.
- Sparse/failure-aware original-process evidence belongs in the future trusted R3 v2 path.

## 6. Non-claims

This checkpoint does not prove that any specific retail DMC3 archive or physical registration actually fails. It proves only that the original code has distinct failure-capable registration helpers whose return values bootstrap ignores.

It does not prove retail `0x0E` collision freedom, a real protected-process R2B v2 receipt, trusted selected identity, global build equivalence, or Layer 2 completion.
