# GDSpaces Layer 2 — mount topology lifetime static reverse — 2026-08-26

**Scope:** canonical DMC3-HD resolver mount-list lifetime.  
**Artifact:** `dmc3.exe`, SHA-256 `e454272ed0fb0247fcbcf300e5d55d7a3e96d50b89b9ffaff81bb978dcbdd082`, 6,356,432 bytes.  
**Status:** STATIC DIRECT-XREF BOUNDARY / NOT A RUNTIME IMMUTABILITY CLAIM.

## 1. Global mount head

Global mount-list head: `0x140CF3180`.

A whole `.text` RIP-relative reference census identifies the bounded direct-code surface around that head.

### Physical prepend writer — `0x140326D20`

The function reads the previous head, stores it at node `+0x50`, then publishes the newly initialized type-0 node as the global head. Its observed direct caller is bootstrap `0x14002E9A9`.

### Archive prepend writer — `0x140326DA0`

Only after archive open/index initialization succeeds, the function stores the previous head at node `+0x50` and publishes the type-1 archive node as the new head. Its observed direct caller is bootstrap `0x14002E9E8`.

### Resolver reader — `0x140327430`

`ResourceMountResolve` reads the global head and traverses `node+0x50`, applying provider-mask filtering and archive/physical lookup semantics.

A separate physical mounted-root existence reader (`0x140326EE0` via `0x1403270B0`) also traverses the same list but is not the runtime resolver selection surface and must not be merged into the R3 receipt merely because it shares topology.

## 2. Discovery and successful mount are separate

Bootstrap `0x14002E930` discovers numbered filenames in ascending order and stops at the first absent `DMC3-N.nbz`.

For every existing filename it calls `0x140326DA0`, but the bootstrap does not consume the mount helper's return value before incrementing `N` and checking the next filename.

Therefore this execution is allowed by the recovered static path:

```text
DMC3-0 exists -> mount succeeds
DMC3-1 exists -> mount fails
DMC3-2 exists -> mount succeeds
DMC3-3 missing -> discovery stops
```

The effective resolver topology is then:

```text
DMC3-2 -> DMC3-0 -> physical root
```

not `2 -> 1 -> 0`.

Consequences:

- `first_missing_archive_volume = 3` proves a discovery stop only;
- a failed archive mount is absent from the mount list;
- it must not be synthesized as a resolver lookup miss;
- successful archive registration order determines actual resolver precedence.

## 3. No direct reset/unlink head writer found

The static direct-reference census did not find another instruction that directly stores a reset/unlinked value to `0x140CF3180`.

Combined with the direct-call census, the recovered canonical direct-code model is:

```text
bootstrap constructs topology
 -> successful registrations prepend nodes
 -> later direct consumers traverse the constructed list
```

No per-request direct registration path was found for the canonical resolver surface.

## 4. Why this is not runtime-global immutability

This static result does not exclude:

- indirect/dynamically computed calls;
- protection/runtime code outside the exact canonical direct-code model;
- corruption/instrumentation changing the list;
- a different protected build having different lifecycle edges.

Use:

> bootstrap-built topology is the recovered canonical direct-code model

not:

> mount topology can never change at runtime.

## 5. Consequence for R2B/R3 v2

The v2 candidate/binder contract separates:

- `first_missing_archive_volume`: numbered filename discovery boundary;
- `archives`: successfully mounted numbered archive artifact subset, sorted ascending for deterministic serialization;
- archive probes: actual provider operations over the successful mounted subset in reverse registration precedence.

Required fail-closed behavior:

- bind topology/selection evidence to one protected executable + PID + process creation FILETIME + module base session;
- do not synthesize mounted archives from the discovery count;
- do not turn a discovered-but-failed mount into a lookup miss;
- if trusted bootstrap/mount observation and trusted resolver traversal disagree, reject rather than repair the trace;
- never reuse a topology snapshot across process instances.

The current candidate remains non-trusted. A future trusted publisher must independently observe/bind the successful mount outcome set for the same live process instance before R3 promotion.

## 6. Non-claims

This checkpoint does not prove:

- runtime-global mount-list immutability;
- protected-build direct-address equivalence;
- that every discovered retail archive mounted;
- trusted original-process selected identity;
- retail normalized-key collision freedom;
- Layer 1 or Layer 3 completion.
