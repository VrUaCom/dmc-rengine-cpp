# GDSpaces L3 R1 — residual state-writer / callback-reference pass — 2026-08-26

## Authority

Canonical analysis executable:

- `dmc3.exe`
- size `6,356,432` bytes
- SHA-256 `e454272ed0fb0247fcbcf300e5d55d7a3e96d50b89b9ffaff81bb978dcbdd082`
- preferred ImageBase `0x140000000`

Primary layer: **[L3] Original Runtime / Resource Lifecycle**.

This pass continues the R1 writer census after the direct-base, leaf/no-unwind, derived-alias and higher-level-state exclusion passes. It targets three remaining static classes:

1. exact immediate `object+0x04 = 1..4` candidates outside the already rejected `0x1401C83xx..0x1401C98xx` family;
2. functions that both reference the canonical registry and perform non-immediate `+0x04 <- register` writes;
3. additional statically addressable registrations of normal completion callback `0x1401B8DC0`.

The promotion rule remains strict: a write is LoadedResource state authority only when its destination is proven to be `0x140C99D30 + index*0x48` or an exact propagated alias.

## Exact-immediate residual census

Whole-image disassembly contains 49 exact immediate writes of `1..4` to a dword at displacement `+0x04`.

After subtracting:

- canonical `1B` lifecycle writers;
- the impossible odd branch in `1B8DC0` already excluded from normal acquisition;
- the 31 exact writes in the separately proven higher-level `1C` orchestration family;

the residual candidate surface is small and can be reviewed by destination provenance.

### `0x14004C5B7 = 1` — REJECTED

Function `0x14004C390` establishes `RBP` from `RSP`:

```text
lea rbp,[rsp-0x110]
sub rsp,0x210
```

The candidate write is therefore into a stack-local descriptor:

```text
[rbp+0x04] = 1
[rbp+0x00] = 6
```

The same function writes many nearby stack-local fields through `rbp-...` and `rbp+...`.

**Result:** stack-local configuration/state, not LoadedResource.

### `0x14008C4B5 = 1` — REJECTED

`0x14008C060` receives its destination object in `R8`, saves it as `RDI`, and initializes a separate small runtime object:

```text
[rdi+0x04] = 1
[rdi+0x0C] = 10.0f
byte [rdi+0x10] = 1
```

The function passes `rdi+0x04` to a separate helper and performs a large dispatch keyed by a small integer input. It has no canonical registry-derived destination provenance.

The byte write at `+0x10` is also incompatible with treating this object as the canonical LoadedResource record whose `+0x10` field participates in the optional callback/context path.

**Result:** separate runtime object, not LoadedResource.

### `0x140242CCF = 1` — REJECTED

The only recovered incoming edge is a tail jump that explicitly sets:

```text
RCX = 0x140C8F250
jmp 0x140242CC0
```

`0x140C8F250` is not within the LoadedResource registry `0x140C99D30 .. +363*0x48`.

The helper initializes its own compact object fields `+0x00/+0x04/+0x08/+0x0C/+0x10/+0x12/+0x14/+0x1C`.

**Result:** exact global-object provenance rejects LoadedResource authority.

### `0x14027A760..0x14027ABC6` family — REJECTED

Candidates include writes of states `2/3/4` to `+0x04`.

Callers repeatedly pass a higher-level subobject at `parent+0x63D0`, for example:

```text
lea rcx,[parent+0x63D0]
call 0x14027A760 / 0x14027A7C0 / 0x14027AA90
```

The same object carries local fields at `+0x08/+0x0C/+0x10/+0x20/+0x30/+0x33` and is used as a gameplay/runtime state machine.

**Result:** higher-level `+0x63D0` subobject state, not LoadedResource.

### `0x1402D60B0 = 1` — REJECTED

Both direct callers pass their large owner object directly as `RCX`.

Inside `0x1402D6090`, the same object is manipulated at offsets through at least `+0x4FA4` and `+0x3ED0` in the same path.

A canonical LoadedResource record is only `0x48` bytes.

**Result:** large owner-object state, not LoadedResource.

### `0x1402DB160 = 3` — REJECTED

`0x1402DB110` receives its object in `RCX/RBP`, then iterates **200** child entries at stride `0x50` beginning around `object+0x48/+0x50` before updating its own `+0x04` state and later clearing it to zero.

This object layout and traversal are incompatible with a single `0x48` LoadedResource record.

**Result:** separate aggregate/runtime object.

### `0x1402F2E1A = 3` / `0x1402F2E48 = 1` — REJECTED

The same `object+0x04` state domain in `0x1402F2DA0` also contains:

```text
object+0x04 = 7
```

Canonical LoadedResource state is bounded to `0..4`.

**Result:** separate state domain, not LoadedResource.

### `0x14031E48B = 4` — REJECTED

The destination comes from `qword [owner+0x58]` and is initialized as a graphics/config-like structure with many non-state constants at nearby offsets, including floats, packed colors/flags and values such as `0x64`, `0x43FA0000`, `0x44898000` in the same `+0x04` location on related branches.

**Result:** descriptor/config object, not LoadedResource.

### `0x14033BAEF = 1` — REJECTED

`0x14033B940` derives `RBX` from a dedicated global table:

```text
base = 0x140D6F4C8
rbx = base + signext(index)*8
```

The candidate is therefore the second dword of an 8-byte table entry, not a LoadedResource record. The function also manipulates independent global synchronization/status variables around `0x140D6F67x..0x140D6F6xx`.

**Result:** global table state, not LoadedResource.

## Exact-immediate residual result

After direct destination/caller review, **no new LoadedResource state1/2/3/4 writer is promoted from the residual exact-immediate class**.

The only true exact-immediate state1/2 writers on the reviewed canonical path remain the already-known central lifecycle authority:

- `0x1401B8569` — acquisition publishes state1 after successful materialization;
- `0x1401B8DD8` — normal completion publishes state2;
- state3 is published by the already-canonical finalizer path;
- state4 is published by the already-canonical cancellation writer.

This does not prove absence of every possible computed/non-immediate writer, but it closes the exact-immediate residual class outside already documented exclusions.

## Direct-registry + non-immediate write census

The next scan groups functions that simultaneously:

1. contain a direct reference to registry base `0x140C99D30`; and
2. contain a non-immediate `DWORD PTR [...+0x04] <- register` write.

After removing already-known `1B` state0 writers and the separately rejected `1C` orchestration family, only three new function clusters remain:

- `0x1400FC8B0`;
- `0x14028C4F0`;
- `0x140299B70`.

All three are rejected as direct LoadedResource state writers.

### `0x1400FC8B0` — REJECTED

The non-immediate writes occur while constructing independent fixed-stride arrays:

```text
[r13-0x04] = source+0x30
[r13+0x00] = source+0x34
[r13+0x04] = source+0x38
[r13+0x08] = 1.0f
r13 += 0x10
```

Another `+0x04 <- register` write occurs in a separate `0x10`-stride loop.

The later registry reference at `0x1400FD9BE` passes the canonical registry to helper `0x1401B7C40`; it does not make the earlier array-element writes LoadedResource state.

**Result:** shared function contains both resource use and unrelated array writes; destination provenance rejects state authority.

### `0x14028C4F0` — REJECTED

This function has a higher-level entry object in `RDI`; its `entry+0x04` is assigned a value returned by `0x140086EF0` and is used in arithmetic/dispatch decisions.

Separately, the owner stores exact LoadedResource pointers in slots such as:

```text
owner + index*8 + 0x310
```

Those pointers are:

- released through `RCX=0x140C99D30; call 0x1401B9530`;
- acquired through `RCX=0x140C99D30; call 0x1401B8EA0`;
- dereferenced separately to read `record+0x04`.

Thus `entry+0x04` and `record+0x04` coexist as distinct fields in the same function.

**Result:** non-immediate write is higher-level entry state/data, not LoadedResource.

### `0x140299B70` — REJECTED

This function iterates higher-level entries beginning near `owner+0x62B0`; each entry has its own dispatch/state fields around `-0x04/+0x00/+0x04/+0x08/+0x09/+0x0A`.

The candidate write:

```text
entry+0x04 = owner+0x62A8
```

is distinct from exact LoadedResource pointers stored separately at:

```text
owner + resource_index*8 + 0x68
```

Those pointers are released through `0x1401B9530`, reacquired through `0x1401B8EA0`, and dereferenced independently to read canonical `record+0x04`.

The higher-level entry state domain also uses values such as `5` and `7` in its neighboring dispatch field.

**Result:** exact object/record separation; non-immediate write is not LoadedResource state.

## Direct-registry/non-immediate result

No new LoadedResource state writer is promoted from the direct-registry + register-valued `+0x04` class outside the already-known `1B` state0 writers.

This is important because it rejects the weak inference:

> “a function references the registry and writes some object+4, therefore that write is LoadedResource state.”

The executable repeatedly places higher-level state machines and exact LoadedResource pointers in the same function.

## `0x1401B8DC0` static callback-reference census

The normal completion callback is reviewed for additional static registrations.

### Code references

Whole-image disassembly exposes one code reference that materializes `0x1401B8DC0`:

```text
0x1401B8572: lea rdx,[rip+...] -> 0x1401B8DC0
```

This is the already-proven acquisition registration path in `0x1401B84E0`.

No second direct call/LEA/JMP reference to `0x1401B8DC0` is present in the reviewed disassembly.

### Static pointer-table search

A raw-image search finds:

- zero exact 64-bit absolute pointer literals equal to VA `0x1401B8DC0`;
- zero exact 32-bit RVA literals equal to `0x001B8DC0`.

Therefore no second conventional static pointer-table/rel32-RVA registration is evidenced for this callback.

### Boundary

This closes the **statically addressable exact-reference surface** for normal completion callback registration on the canonical image.

It does not mathematically prove that no runtime-computed pointer could ever equal `0x1401B8DC0`; no such computed registration is evidenced in this pass.

## R1 status after this pass

### Bounded/closed writer classes

The following static classes are now evidence-bounded:

- canonical central state writer spine;
- direct-base immediate writer census;
- exact-immediate leaf/no-unwind class;
- derived/indexed/caller-propagated state0 release/rollback writers;
- large `0x1401C83xx..0x1401C98xx` higher-level state-machine false-positive family;
- residual exact-immediate state1/2/3/4 candidates outside those classes;
- direct-registry + non-immediate `+0x04 <- register` candidate class;
- exact static reference surface for `0x1401B8DC0` registration.

### Still genuinely open before final R1 promotion

1. computed/derived record aliases outside the reviewed lifecycle/orchestration clusters where the function has **no direct registry reference** and the state value is non-immediate;
2. unusual code-generation forms that write state without a literal displacement-4 `mov` pattern;
3. whole-image contradiction sweep against the resulting writer map;
4. any contradictory evidence from dynamic/original-process tracing.

R1 is therefore now a **narrow residual data-flow problem**, not a broad state-writer discovery problem.

## Next reverse step

The next static pass should seed taint/provenance from known record-producing points:

```text
1B8DF0 / fixed-family selectors / 1B8EA0 / known record-returning wrappers
 -> record pointer stored/passed
 -> cross-function alias propagation
 -> any non-central write into alias+0x04
```

Priority is callers outside the already reviewed `1B8xxx..1B9xxx`, `1C83xx..1C98xx`, `28C4F0`, and `299B70` clusters.

If no contradictory writer survives that provenance sweep, R1 can move to final contradiction audit rather than another syntactic scan.

## Completion claim

**R1 is strongly narrowed but not yet declared complete. L3 remains incomplete.**

This pass does not promote L1 retail evidence, L2 trusted selected identity, or L3 V1–V7 dynamic acceptance.