# DMC3 Numbered NBZ Volume Bootstrap — discovery vs successful mount topology

**Reverse authority:** canonical `dmc3.exe`, SHA-256 `e454272ed0fb0247fcbcf300e5d55d7a3e96d50b89b9ffaff81bb978dcbdd082`, 6,356,432 bytes.  
**Primary code path:** bootstrap `0x14002E930`, physical registration `0x140326D20`, archive registration `0x140326DA0`, mount head `0x140CF3180`.  
**Status:** reverse-backed product correction under issue #237; whole-head Windows + Ubuntu validation required before promotion.

## Recovered bootstrap sequence

`0x14002E930` derives the executable-relative `\\data\\dmc3\\` root and executes this bounded sequence:

```text
physical registration attempt
 -> ascending DMC3-0.nbz, DMC3-1.nbz, ... filename discovery
 -> for each existing filename: archive registration attempt
 -> stop discovery at the first missing DMC3-N.nbz
```

The important correction is that **registration attempt and successful linked mount are not the same fact**.

### Physical registration

Bootstrap calls `0x140326D20` at `0x14002E9A9` with flags `0x0C`.

`0x140326D20` only publishes the type-0 node to global mount head `0x140CF3180` after its initialization succeeds. Bootstrap does not consume or branch on the returned `EAX` value.

Therefore the recovered sequence proves that physical registration is attempted before numbered archives. It does **not** prove that a type-0 physical node exists for every launch.

### Archive discovery and registration

For numbered archives bootstrap formats `%sDMC3-%d.nbz`, starts at index zero, tests filename presence, and stops at the first missing filename.

For every existing filename it calls archive registration `0x140326DA0` at `0x14002E9E8`. Bootstrap does not branch on that registration result either; it increments the index and continues discovery.

`0x140326DA0` prepends a type-1 node only after archive open/index initialization succeeds.

Therefore:

```text
first_missing_index = discovery stop
```

is **not** equivalent to:

```text
all indices below first_missing_index mounted successfully
```

A reverse-valid topology can be:

```text
DMC3-0 exists -> mount succeeds
DMC3-1 exists -> mount fails
DMC3-2 exists -> mount succeeds
DMC3-3 missing -> discovery stops
```

The discovery range is `0..2`, while the linked archive set contains only `0` and `2`.

## Successful mount precedence

Both recovered registration helpers use prepend insertion into `0x140CF3180`.

Successful archive registrations are attempted in ascending discovered-index order. Consequently only the **successful** registrations participate in reverse precedence.

Clean success:

```text
successful registration order: 0 -> 1 -> 2
resolver archive order:         2 -> 1 -> 0
```

Sparse success:

```text
successful registration order: 0 -> 2
resolver archive order:         2 -> 0
```

A discovered-but-failed archive has no linked type-1 node and must not be represented later as an archive lookup miss.

## Product type boundary

The implementation now makes the reverse distinction structural.

### `VolumeBootstrapPlan`

Discovery-only evidence:

- `physical_registration_attempted_before_archives`;
- `archive_discovery_stops_at_first_missing`;
- `first_missing_index`;
- `discovered_archives`;
- `present_after_first_gap` diagnostics;
- `present_outside_runtime_index_domain` diagnostics.

It carries no field named or implying `registered_archives`.

### `RuntimeMountTopology`

Successful linked-node topology:

- `physical_root_mounted`;
- exact successful `mounted_archives`;
- successful registration order;
- prepend-derived `archive_resolution_order`;
- binding back to the discovery stop that produced the attempts.

`VolumeBootstrapPolicy::successful_mount_topology(...)` requires explicit registration outcomes. It does not infer success from filename presence.

No compatibility overload exists that turns a discovery plan into a resolver topology automatically.

## Signed filename namespace

The executable formatter is `%sDMC3-%d.nbz`, so the recovered non-negative numbered-volume namespace is signed decimal:

```text
runtime-equivalent index: 0 .. INT32_MAX (2147483647)
outside runtime domain:   INT32_MAX+1 .. UINT32_MAX
```

`volume_filename()` returns an empty string outside that runtime domain. Product discovery may still preserve larger numeric suffixes as diagnostics, but they are not promoted as recovered numbered-volume identities.

## First-gap diagnostics

Product scanning may discover files that the recovered first-gap loop would never attempt:

- in-domain indices after the first gap remain `present_after_first_gap`;
- numeric suffixes above `INT32_MAX` remain `present_outside_runtime_index_domain`.

Neither list is accepted as successful runtime topology.

## Regression contract

Tests now lock:

- clean all-success precedence `2 -> 1 -> 0`;
- sparse success `0 success / 1 failure / 2 success -> 2 -> 0`;
- physical registration failure is representable independently of archive success;
- duplicate successful-index claims fail closed;
- an index at or beyond `first_missing_index` cannot be claimed as a successful registration;
- after-gap discoveries never become mount attempts automatically;
- signed `%d` domain boundaries remain enforced;
- forged registration/resolution ranks fail structural validation.

## Evidence references

- `l2-exe-reverse-pass-2026-08-26-pass2.md`
- `l2-mount-topology-lifetime-reverse-2026-08-26.md`
- `data/reverse/dmc3-gdspaces-l2-resolver-static-census-2026-08-26.v1.json`
- merged PR #235
- issue #237

## Non-claims

This model does not prove:

- that any particular retail archive mount fails;
- that every retail numbered archive mounts successfully;
- runtime-global immutability of the mount list;
- protected-build address equivalence without R2B evidence;
- trusted selected-provider identity;
- Layer 1, Layer 2, or Layer 3 completion.
