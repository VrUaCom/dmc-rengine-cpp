# GDSpaces Layer 2 — mount topology lifetime static reverse — 2026-08-26

**Scope:** canonical DMC3-HD resolver mount-list lifetime.  
**Artifact:** `dmc3.exe`, SHA-256 `e454272ed0fb0247fcbcf300e5d55d7a3e96d50b89b9ffaff81bb978dcbdd082`, 6,356,432 bytes.  
**Status:** STATIC DIRECT-XREF BOUNDARY / NOT A RUNTIME IMMUTABILITY CLAIM.

## 1. Global mount head

Global mount-list head:

```text
0x140CF3180
```

A whole `.text` RIP-relative reference census found the direct code references that access this global head.

### Direct prepend writer — physical registration

`0x140326D20` reads the previous head and writes a newly initialized type-0 node as the new head around:

```text
0x140326D77  read old head
0x140326D88  node+0x50 = old head
0x140326D8C  global head = node
```

Direct caller census for `0x140326D20` finds only bootstrap call `0x14002E9A9`.

### Direct prepend writer — archive registration

`0x140326DA0` prepends a successfully initialized type-1 archive node around:

```text
0x140326E61  read old head
0x140326E6D  node+0x50 = old head
0x140326E71  global head = node
```

Direct caller census for `0x140326DA0` finds only bootstrap call `0x14002E9E8`.

### Direct reader — physical mounted-root existence search

Function `0x140326EE0` reads the global head at `0x140326F01`, traverses node `+0x50`, considers type-0 nodes, builds bounded root-relative paths and tests them through `0x140327720`.

It is reached through wrapper `0x1403270B0`, whose observed direct caller is `0x14002E889`.

This path is distinct from `ResourceMountResolve` and must not be merged into the runtime selection receipt merely because both traverse the same mount list.

### Direct reader — resource resolver

`ResourceMountResolve 0x140327430` reads the global head at `0x14032747B` and traverses `node+0x50` while applying provider-mask filtering and archive/physical lookup semantics.

## 2. No direct reset/unlink head writer found

The static direct-reference census did not find another instruction that directly stores a reset/unlinked value to `0x140CF3180`.

Combined with the direct-call census above, the canonical direct surface supports this bounded model:

```text
bootstrap constructs mount list
 -> successful registrations prepend nodes
 -> later direct consumers traverse the constructed list
```

No per-request direct registration path was found for the canonical resolver surface.

## 3. Why this is not an immutability claim

This static result does **not** prove that runtime mutation is globally impossible.

It does not exclude:

- an indirect call reached through a dynamically computed target;
- external/protection/runtime code not represented by an exact direct-call or direct-RIP-reference pattern;
- corruption or instrumentation changing the list;
- a different protected build containing a different lifecycle path.

No exact absolute-qword function-pointer references to the core resolver functions were found in the canonical image, but that is still not proof of complete indirect-CFG absence.

Therefore use the wording:

> bootstrap-built topology is the recovered canonical direct-code model

not:

> mount topology can never change at runtime.

## 4. Consequence for trusted R3 observation

A trusted publisher may use bootstrap mount outcome capture as the expected topology for the same **process instance**, but selection evidence must still be based on actually observed provider operations.

Required fail-closed behavior:

- bind topology/selection evidence to one PID + process creation FILETIME + module base + executable authority;
- do not synthesize mounted archive probes solely from `first_missing_archive_volume`;
- if observed provider traversal disagrees with the bootstrap-derived successful mount set, reject the receipt instead of repairing the sequence;
- do not treat a discovered-but-failed archive mount as a lookup miss;
- do not reuse a topology snapshot across process launches.

This makes #229 process-instance identity part of the mount-topology provenance chain, not merely an address-mapping detail.

## 5. Non-claims

This checkpoint does not prove:

- runtime-global mount-list immutability;
- protected-build direct-address equivalence;
- that every discovered retail archive mounted;
- trusted original-process selected identity;
- retail normalized-key collision freedom;
- Layer 1 or Layer 3 completion.
