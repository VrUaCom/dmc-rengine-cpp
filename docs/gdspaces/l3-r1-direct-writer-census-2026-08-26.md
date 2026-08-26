# GDSpaces L3 R1 — direct LoadedResource writer census — 2026-08-26

**Target:** canonical `dmc3.exe`  
**Size:** `6,356,432` bytes  
**SHA-256:** `e454272ed0fb0247fcbcf300e5d55d7a3e96d50b89b9ffaff81bb978dcbdd082`  
**Scope:** refine R1 by separating syntactic `+0x04` state-like writes from writes that actually target the LoadedResource registry.

## Method

The pass combined three independent static filters over the whole canonical image:

1. PE x64 unwind/runtime-function boundaries were recovered from `.pdata`/unwind metadata;
2. direct RIP-relative references to LoadedResource registry base `0x140C99D30` were enumerated;
3. immediate dword writes of values `0..4` to object offset `+0x04` were enumerated and assigned to runtime-function boundaries where available.

A function was then treated as a **syntactic candidate only** when it contained both:

- a direct reference to `0x140C99D30`; and
- a state-like immediate write to `+0x04`.

This is intentionally conservative: a syntactic candidate is not promoted until the destination object is proven to be a LoadedResource record.

## Initial syntactic candidate set

Six unwind-bounded functions survived the first filter:

| Function | Candidate write | Initial reason |
|---|---|---|
| `0x1401B84E0–0x1401B85B3` | `+0x04 <- 1` | direct registry reference + state-like write |
| `0x1401C84F6–0x1401C857C` | `+0x04 <- 2` | direct registry reference + state-like write |
| `0x1401C8717–0x1401C879D` | `+0x04 <- 2` | direct registry reference + state-like write |
| `0x1401C8AD5–0x1401C8B5B` | `+0x04 <- 2` | direct registry reference + state-like write |
| `0x1401C9787–0x1401C980D` | `+0x04 <- 2` | direct registry reference + state-like write |
| `0x1401C9830–0x1401C98EA` | `+0x04 <- 2` | direct registry reference + state-like write |

## Destination-object review

### `0x1401B84E0` — true LoadedResource writer

This is the already canonical acquisition function.

Its `+0x04 <- 1` write targets the selected LoadedResource record and occurs only after materialization dispatch succeeds.

**Classification:** true L3 state writer.

### `0x1401C84F6`, `0x1401C8717`, `0x1401C8AD5`, `0x1401C9787` — false positives

These higher-level functions reference the registry only to call canonical registry operations such as:

- ordinary release `0x1401B9530`;
- fixed-record acquisition/reuse helper `0x1401B8EA0`.

Their local `+0x04 <- 2` writes target the higher-level object in `RBX`, not the LoadedResource record passed to the registry helper.

The same functions also maintain unrelated higher-level fields such as local object pointers at offsets around `+0x40/+0x48/+0x58/+0x60/+0x68` and external interface/gameplay state. That object is not the `0x48`-stride LoadedResource record.

**Classification:** reject as LoadedResource state writers; retain only as higher-level L3 consumers/owners where their calls establish ownership relationships.

### `0x1401C9830–0x1401C98EA` — false positive with group5 acquisition

This function directly references the registry to call group5 allocator `0x1401B8DF0` and fixed-record helper `0x1401B8EA0`.

Its `+0x04 <- 2` write targets the higher-level object in `RDI`, not a registry record.

**Classification:** reject as direct LoadedResource state writer.

## Result

The direct-base + immediate-`+0x04` syntactic filter produced five false positives and one true LoadedResource writer.

```text
6 syntactic candidates
 -> 1 true LoadedResource state writer (`0x1401B84E0`, state1)
 -> 5 higher-level-object false positives
```

This materially narrows R1: no new family-specific direct state2 writer was promoted from these candidates.

## Why `0x1401B8DC0` is not contradicted by this result

The normal completion callback `0x1401B8DC0` remains the canonical state2 writer already recovered in the raw pass.

This particular census is intentionally not complete for every true writer because:

- leaf functions may not have their own unwind entry;
- writes can use indexed/base-plus-offset addressing rather than the simple local-object `+0x04` form;
- record pointers can arrive through callers/aliases without a direct RIP-relative reference to `0x140C99D30` in the same function;
- cancellation/reset loops can use different addressing forms and therefore are already classified separately from this syntactic filter.

The census is a **false-positive elimination and narrowing pass**, not an exhaustive proof that no other aliased writer exists.

## R1 status after this census

### Strong / bounded

- canonical acquisition state1 writer `0x1401B84E0`;
- canonical normal completion state2 writer `0x1401B8DC0`;
- canonical state2 finalizer/state3 writer path `0x1401B92D0`;
- canonical `1|2 -> 4` cancellation writer `0x1401B8430`;
- canonical cancellation cleanup/group/full/ordinary state0 policies;
- five direct-base syntactic state2 false positives rejected.

### Remaining R1 work

1. leaf/no-unwind state-like write census;
2. indexed/derived-record-address writes;
3. caller-propagated record-pointer aliases;
4. unusual tagged/sentinel path in the completion region;
5. reconcile any remaining candidate against exact 363-record address/stride provenance before promotion.

## Promotion rule

A future `+0x04` write is not a LoadedResource state writer merely because:

- its immediate value is in `0..4`;
- the same function calls a LoadedResource helper;
- the same function references the registry base.

Promotion requires proof that the write destination is a record in the canonical `0x140C99D30 + index*0x48` registry or an exact alias to such a record.
