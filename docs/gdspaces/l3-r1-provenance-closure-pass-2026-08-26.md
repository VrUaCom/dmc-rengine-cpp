# GDSpaces L3 R1 — provenance closure / contradiction pass — 2026-08-26

## Authority

Canonical analysis executable:

- `dmc3.exe`
- size `6,356,432` bytes
- SHA-256 `e454272ed0fb0247fcbcf300e5d55d7a3e96d50b89b9ffaff81bb978dcbdd082`
- preferred ImageBase `0x140000000`

Primary layer: **[L3] Original Runtime / Resource Lifecycle**.

This pass follows the direct-base, leaf/no-unwind, derived-alias, higher-level-state exclusion and residual writer passes. Its goal is not another textual scan for `+0x04`, but a bounded provenance/contradiction check seeded from exact LoadedResource-producing helpers and the canonical registry.

## Provenance seeds

The reviewed record-producing/record-selecting surface includes:

- `0x1401B8DF0` — group-5 first-free LoadedResource acquisition, returns exact record pointer;
- `0x1401B8EA0` — wrapper that resolves request metadata then tail-calls `1B8DF0`, therefore also returns an exact group-5 record pointer on success;
- fixed-family selectors/wrappers including `1B8D60`, `1B8F50`, `1B8FF0`, `1B90B0`, `1B9160`, `1B9270`;
- canonical registry base `0x140C99D30` and state-base `0x140C99D34`.

The pass requires pointer provenance, not field-number coincidence.

## Helper caller census

The canonical image contains **110** direct call sites to the reviewed record-producing wrapper set, grouped into **70** unwind-bounded caller functions.

For `1B8DF0/1B8EA0`, the dominant observed return pattern is:

```text
call record_producer
 -> RAX = LoadedResource* or null
 -> store RAX into a higher-level object member
 -> later read record.state / payload / descriptor
 -> release through canonical lifecycle helper
```

This is expected ownership propagation and is distinct from higher-level object state stored in the same owner.

## Same-function return-to-write check

Functions that call a record-producing helper and also contain a destination write to dword `[something+0x04]` were reviewed separately.

The only write-bearing caller families are:

1. the already rejected `0x1401C83xx..0x1401C98xx` higher-level orchestration family;
2. `0x14028C4F0`;
3. `0x140299B70`;
4. already-canonical state0 alias/release paths in the `1B` lifecycle cluster.

The first three are already proven to keep exact LoadedResource pointers in separate member slots while their own object/entry `+0x04` is a different state/data field.

**Result:** no new LoadedResource state writer survives the same-function producer/consumer screen.

## One-hop member-alias -> callee writer check

A stronger cross-function check was then applied:

1. collect constant member offsets that actually receive `RAX` from `1B8DF0/1B8EA0` on direct caller paths;
2. in resource-aware callers, identify loads of a qword from one of those member slots into an argument register;
3. follow the immediately invoked callee;
4. taint the incoming argument through register copies inside the callee;
5. report any callee that writes dword `[tainted_pointer+0x04]`.

This produces **48 call-site observations**, but they collapse to exactly **one write-capable callee target**:

```text
0x1401B9530
```

That target is the already-canonical ordinary LoadedResource release helper. Its bounded semantics remain:

```text
record.state == 3
 -> release record+0x28 backing
 -> if release succeeded: record.state = 0
```

**Promotion:** no additional hidden one-hop member-alias state writer is evidenced on this resource-aware call surface.

## Direct helper-return -> callee check

A separate scan checked whether an exact `1B8DF0/1B8EA0` return pointer is immediately copied from `RAX` into an ABI argument register and passed to another callee before being stored.

No direct one-hop `RAX -> argument -> state-writing callee` path was found on the reviewed call sites.

This means the observed ownership pattern is overwhelmingly explicit member storage or local inspection rather than a hidden direct generic state-mutator call.

## Direct-registry symbolic provenance check

For unwind-bounded functions that directly materialize the canonical registry/state base, a simple register provenance pass tracks:

```text
LEA registry/state base
 -> register copies
 -> derived LEA/add arithmetic
 -> direct dword write through a tainted address
 -> calls receiving tainted RCX/RDX/R8/R9
```

The state-write result is consistent with prior passes:

- direct canonical state-base cleanup at `1B8F00` remains visible;
- no new direct-registry-derived callee was found that writes incoming `+0x04` state;
- the leaf/no-unwind `1B8DC0` state2 writer is intentionally outside `.pdata` and is already covered by the dedicated leaf pass.

This check is a bounded contradiction screen, not a general-purpose proof of arbitrary machine-code pointer equivalence.

## Broad member-offset coincidence review

A broader heuristic scan found 31 distinct write sites where a qword is loaded from an offset that is also used somewhere else to store a LoadedResource pointer and the loaded pointer is later written at `+0x04`.

Offset equality alone is not provenance. The suspicious non-canonical families were reviewed:

### `0x140047000`

The owner allocates its own **0x10-byte** heap object through `0x140345510`, stores that newly allocated pointer at `owner+0x20`, then writes its own fields:

```text
qword +0x00 = 7
DWORD +0x04 = input
qword +0x08 = buffer pointer
```

The pointer did not originate from LoadedResource acquisition.

### `0x1402F7D60` / `0x1402FE6A0` / `0x1402F9BB0`

These are resource-format/post-load style structure builders. Their apparent `+0x10` aliases are **stack locals** or pointers derived from format arrays with strides such as `0x50` and `0x1A0`.

The `+0x04` writes copy mesh/object descriptor words from resource structures. They are not LoadedResource lifecycle state.

### `0x140303460`

Its `rbp+0x10` is a stack-local pointer that is repeatedly replaced with resource-internal structures/factory results. The function also traverses `0x380`-stride records and writes format/runtime structure fields.

No `1B8DF0/1B8EA0` provenance exists for that local pointer.

### `0x14031B2D0` and related `owner+0x58` consumers

The pointed object is demonstrably not LoadedResource: related functions write values such as floats, `0x64`, `0x43FA0000`, `0x44898000` and `0x80000000` into its `+0x04` field.

### `0x14034195D`

This is a separate bitstream/decompression/parser state machine. Its `r12+0x20` pointer participates in bit-buffer/status handling and unrelated state transitions, not LoadedResource ownership.

**Result:** the broad offset-coincidence set yields no new LoadedResource writer.

## Whole-image displacement-4 accounting

Whole-image disassembly contains **325** dword destination writes whose memory operand uses displacement `+0x04`, spread across **198** apparent function/leaf contexts.

This number is deliberately not treated as “198 possible LoadedResource writers”. Most are unrelated object fields.

The R1 program has now applied the following evidence filters across that surface:

1. exact immediate state `0..4` whole-image census;
2. unwind/direct-registry destination review;
3. leaf/no-unwind exact-immediate review;
4. canonical registry/index arithmetic and raw seven-group table binding;
5. caller-propagated/stored record alias review;
6. direct-registry + non-immediate register-valued write review;
7. higher-level orchestration family separation;
8. exact static callback-registration review;
9. helper-return/member-slot provenance review;
10. one-hop resource-aware member-alias -> callee state-writer review;
11. direct-registry symbolic contradiction screen.

No additional state1/2/3/4 authority survives these filters beyond the already-canonical central lifecycle writers, and no additional state0 authority survives beyond the documented rollback/release/reset writer classes.

## Canonical writer map after the R1 passes

### State 1

- acquisition publication in `0x1401B84E0` after materialization success.

### State 2

- normal completion callback `0x1401B8DC0` on the recovered normal registration domain.

### State 3

- state-2 finalizer `0x1401B92D0`, after typed post-load and optional callback.

### State 4

- canonical cancellation writer `0x1401B8430`, source states only `1|2`.

### State 0

Distinct evidenced policies remain separate:

- acquisition rollback: `1B8DA4`, `1B8E60`, `1B92B8`;
- deferred state4 cleanup: `1B8F00`;
- ordinary owner release: `1B9530` / state write around `1B9546`;
- fixed-family indexed release: `1B9914`, `1B997B`, `1B99D7`, `1B9A3B`;
- stored group5 aliases: `1B946D`, `1B96CC`, `1B9B9A`, `1B9C83`;
- forced group/full reset: `1B95B9`, `1B9609`.

## R1 promotion decision

For the **canonical analysis image and current declared writer scope**, the mandatory static writer census is now sufficiently saturated to move out of broad discovery.

**R1 status: STATIC BOUNDED-CLOSED / contradiction-gated.**

Meaning:

- do not run another generic `+0x04` grep or reopen known writer classes;
- reopen R1 only if a later field-owner pass, typed/factory pass, dynamic original-process trace or cross-build profile supplies a contradictory state write with exact record provenance;
- runtime V1–V7 remains independent and open;
- this is not a claim that arbitrary runtime-computed code pointers are mathematically impossible.

## Next Layer-3 reverse target

Proceed to **R2 family-complete field/backing ownership**:

```text
+0x08 selector/index metadata
+0x18 descriptor/type authority
+0x20 payload
+0x28 backing/owned subobject
+ stable adjacent fields
```

The next pass should start from exact writes in the fixed-family wrappers and `1B84E0`, then build writer/reader/caller ownership by family. `+0x28` must distinguish runtime backing release `0x140337710` from CRT backing destruction `0x140337440`.

## Layer completion

**R1 static census is bounded-closed. Layer 3 remains NOT COMPLETE.**

R2–R5 plus original-process V1–V7 remain mandatory before L3 completion.