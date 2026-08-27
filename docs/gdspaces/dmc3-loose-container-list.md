# DMC3 `.lst` Loose-Container Reconstruction

**Status:** evidence-backed implementation candidate under active Layer-1 reverse; **L1 is INCOMPLETE / NOT 100%**. Product safety guards are explicitly separate from recovered original behavior.

Canonical executable authority: SHA-256 `e454272ed0fb0247fcbcf300e5d55d7a3e96d50b89b9ffaff81bb978dcbdd082`.

Direct reverse anchors:

- representation selector `0x1401B79E0`;
- size/parser helpers `0x1401B7B90`, `0x1401B7C70`, `0x1401B7D10`, `0x1401B7E60`, `0x1401B7FD0`;
- loose-container materializer `0x1401B85C0`;
- `.lst` extension rewrite helper `0x1401B9390`;
- direct whole-file transfer-extent helper `0x1402EF620`;
- materialization enqueue entry `0x1402EF4D0` and consumer `0x1402EF790`;
- whole-file open/chunk/read spine `0x1400333F0`, `0x1400333C0`, `0x140033500`, `0x14002F930`;
- zeroing allocator `0x140337600` with `memset` thunk `0x140346BEA`;
- top-level materialization/result path `0x1401B84E0`, `0x1401B8CA0`;
- type-3 completion enqueue `0x1402EF580`.

Detailed failure/width reconciliation: `l1-writer-failure-width-reconciliation-2026-08-28.md`.

## Representation precedence

For container-backed resources (`kind16 == 0`) the original runtime prefers the exact packed representation:

```text
exact packed resource exists with positive size
    -> use packed container
else
    -> replace existing extension with lowercase .lst
    -> positive-size .lst exists
         -> synthesize loose container
         -> otherwise fail
```

A path without an extension boundary cannot enter the recovered `.lst` rewrite. `.lst` is therefore an original fallback representation, not an override.

The product API accepts an injected read callback. It does not own a second VFS/resolver and must be called through the existing GDSpaces resource authority.

## Scanner grammar and bounds

Recovered loop/copy bounds:

- total scanner ceiling: `0x1FC0` / 8128 bytes;
- child-token copy bound: `0x100` / 256 bytes;
- normal child text terminates at CR (`0x0D`) or NUL;
- skip/control states return to normal at LF (`0x0A`).

The grammar is CRLF-oriented. LF-only normal resource lines are not proven equivalent. The product implementation therefore fails them closed rather than silently advertising a normalized LF-only file as original-compatible.

Normal-state line classes:

- leading `/` -> skipped/comment line;
- CR/blank line -> skipped;
- leading `#` -> magic directive/control line;
- any other normal line -> declared child slot.

Directive, comment and blank lines do not increment `slotCount`. A normal `dummy` line does.

### Important malformed-input correction

The recovered bounds are **not original clean error enums**:

- `0x1401B7C70` is void; reaching its `0x1FC0` ceiling does not return a dedicated scan-limit error;
- `0x1401B7D10` returns the count accumulated before NUL/ceiling;
- `0x1401B7E60` returns false when the target child is not found before NUL/ceiling;
- at the exact `0x100` token-copy boundary, the original path exits the normal copy/terminator flow and continues toward formatting rather than returning a dedicated token-limit failure.

Therefore product statuses such as `scan_limit_exceeded` and `token_limit_exceeded` are intentional **fail-closed hardening**, not reconstructed original status codes.

## Critical corrections to older shorthand

Three older simplifications are superseded:

1. `hash_comments=true` is wrong. `#` is a magic directive; `/` is the recovered comment/skip marker.
2. `rewrites_entries_to_pac=true` is too broad. Ordinary children are resolved as `baseDirectory + rawToken`. `.pac` rewrite is evidenced specifically for a nested `.lst` sibling-precedence check.
3. “every non-empty child is merely align64(size)” is wrong. A **direct whole-file child uses a 0x800 transfer extent** in the normal safe arithmetic domain. Only the structural header and recursively synthesized complete images use the 0x40 structural alignment.

Those historical shorthand fields must not be copied into clean GDSpaces code.

## Magic directive

A directive has structural form:

```text
#XXXX
```

Up to four immediate bytes following `#` are captured as the synthesized four-byte magic. Whitespace is not skipped before capture. If no directive supplies a magic, the synthesized container uses `PAC\0`.

`#PNST` is therefore a valid generic four-byte magic case, not evidence of a special hardcoded PNST directive parser.

## Declared children and sparse slots

The Nth-child helper uses the same scanner grammar. For each normal child line it:

1. captures the raw token;
2. forms `baseDirectory + token`;
3. strips CR termination on the normal bounded path;
4. preserves normal-line order as declared slot order.

The exact lowercase token `dummy` is compared as five bytes. It remains a declared slot but:

- its offset-table entry is `0`;
- it contributes no child payload size;
- no child load is issued.

An arbitrary missing or zero-size ordinary child is not implicitly converted into `dummy`.

## Synthesized binary envelope

The loose representation uses the same slot-access envelope expected by PAC/PNST consumers:

```text
+0x00  magic[4]
+0x04  u32 slotCount
+0x08  u32 relativeOffsets[slotCount]
...    zeroed structural/header padding
        child payload bytes + zeroed transfer/structural slack
```

The header starts child storage at:

```text
align64((slotCount + 2) * 4)
```

Child placement then depends on the child representation authority.

### Direct whole-file child

`0x1402EF620` opens the resource through the whole-file path, obtains the chunk count from `0x1400333C0`, closes the temporary state, then returns `chunkCount << 11`.

In the normal positive/safe domain this is:

```text
ceil(logicalSize / 0x800) * 0x800
```

The planner and writer reserve that **0x800-granular transfer extent** for:

- an ordinary direct child token;
- a positive-size packed sibling `.pac` selected for a nested `.lst` token.

The actual async read writes only the real materialized bytes. The remainder of the reserved transfer extent remains zero because the complete output allocation is zero-initialized before emission.

### Original 32-bit width caveat

The original helper is not mathematically unbounded `ceil()` arithmetic. `0x1400333C0` performs a 32-bit wrapping add of `0x7ff`, signed correction and arithmetic shift; `0x1402EF620` then performs a 32-bit left shift by 11. Large sizes can therefore become negative or zero after wrap.

Examples:

```text
size 0x7FFFF800 -> extent 0x7FFFF800
size 0x7FFFF801 -> extent 0x80000000 (negative as signed 32-bit)
size 0xFFFFF002 -> extent 0
```

GDSpaces intentionally uses checked arithmetic and fails closed instead of reproducing those unsafe wrap semantics.

### Recursively synthesized nested child

When a nested `.lst` has no positive-size packed sibling, the runtime recursively synthesizes its complete child image. The parent then advances by the recursively produced image size under the 0x40 structural alignment rule:

```text
align64(recursivelySynthesizedSize)
```

This distinction is essential: a direct packed `.pac` and a recursively synthesized `.lst` child may contain equivalent logical content but have different placement-extent authority.

## Zero initialization / padding bytes

Fresh canonical-EXE reverse closes the previous padding uncertainty for this runtime-synth path.

The allocation path converges on `0x140337600`, which calls the `memset` thunk at `0x140346BEA` with value `0` across the complete requested allocation size. Therefore:

- header padding is zero;
- direct-child 0x800 transfer slack is zero;
- structural slack after recursively synthesized images is zero.

This is original runtime behavior for the recovered synthesis path, not merely a product determinism policy.

## Nested `.lst` precedence

For a child token ending in lowercase `.lst`:

```text
nested.lst
    -> construct sibling nested.pac
    -> positive-size nested.pac exists:
         load it as a direct whole-file child
         reserve direct whole-file transfer extent
    -> otherwise:
         recursively synthesize nested.lst in place
         reserve align64(completeSynthesizedImageSize)
```

Ordinary non-list children are loaded from their exact directory-relative token path and are not rewritten to `.pac`.

## Enqueue vs byte-producing path

`0x1402EF4D0` is an enqueue function, not the byte-producing body itself. It records the destination pointer/path and a type-2 job in a ring.

If the current ring slot already has `state > 0`, `0x1402EF4D0` returns false immediately. Otherwise it publishes `state = 2`, destination/path, advances the producer index modulo capacity and returns true.

The consumer `0x1402EF790` executes the type-2 job through:

```text
0x1400333F0(path)          whole-file open / cached logical size
 -> 0x1400333C0(state)     transfer chunk count
 -> 0x140033500(...)       async submit
 -> 0x14002F930            seek + repeated reads until request filled, EOF/no-progress, or error
```

That distinction supersedes older documentation that labeled `0x1402EF4D0` itself as the generic byte-producing materializer.

## Writer failure propagation — confirmed correction

`0x1401B85C0` does **not** propagate all child dispatch failures.

Recovered call sites include:

```text
0x1401B8C11 -> 0x1402EF4D0   direct child enqueue; return ignored
0x1401B8C2D -> 0x1401B85C0   recursive child writer; return ignored
0x1401B8C53 -> 0x1402EF4D0   direct child enqueue; return ignored
```

Thus the outer loose writer can return success after a child enqueue was rejected by an occupied queue slot or a recursive writer returned failure.

This means:

> `0x1401B85C0 == true` is **not** proof that every child was enqueued or that all exact bytes completed.

`0x1401B8CA0` is also branch-dependent:

- packed branch tail-propagates `0x1402EF4D0` enqueue result;
- loose branch tail-propagates the coarse `0x1401B85C0` result;
- another branch calls `0x1402EF4D0` and then forces `AL=1`.

Finally, `0x1401B84E0` calls type-3 completion enqueue `0x1402EF580` after successful setup but ignores its boolean result. `0x1402EF580` itself can return false when its target ring slot is occupied.

Therefore upstream materialization/writer booleans must not be promoted to exact terminal-byte receipts. The terminal byte/result state must be reconciled through the queued consumer / whole-file status path before the L3 lifecycle publication seam.

## Original mechanics vs product hardening

Evidence-backed original mechanics in this slice:

- packed-first top-level selection;
- `.lst` extension fallback;
- scanner/token loop/copy bounds;
- CRLF-oriented line grammar;
- `/` comments and `#XXXX` magic directives;
- default `PAC\0` magic;
- declared child order/count;
- exact `dummy` sparse slot;
- 0x40-aligned synthesized header;
- direct whole-file 0x800-granular extent in the safe positive arithmetic domain;
- recursively synthesized complete-image extent under 0x40 structural alignment;
- zero initialization of the complete planned image;
- nested `.lst` sibling `.pac` precedence and recursive in-place synthesis;
- 32-bit wrap-prone planner/chunk arithmetic;
- branch-dependent enqueue/failure propagation, including swallowed child/completion enqueue failures.

Product-only safety policy in this implementation:

- ancestry cycle rejection;
- configurable recursion-depth limit;
- configurable total synthesized-output budget;
- explicit host/32-bit offset overflow rejection instead of reproducing unsafe original wrap;
- fail-closed LF-only normal-line handling;
- explicit scan/token limit failure instead of continuing through the original malformed boundary;
- receipt/integrity rules that never launder rejected enqueue or unsafe arithmetic into successful authored-byte authority.

Product hardening must not be mislabeled as original error semantics, and original unsafe behavior must not be copied into product code merely for literal parity.

## Open reverse/validation boundaries

Still open before an exhaustive original L1 claim:

- exact recursive cycle/depth behavior and allocation/free lifetime semantics;
- remaining allocator/backend failure branches not yet classified;
- final L1-terminal -> L3 normal-completion suppression/eligibility reconciliation (`0x1402EF460`, relevant `0x1401B8DC0` context and associated state transitions);
- representative real `.lst` corpus receipt for any real loose-list equivalence claim;
- controlled original-game consumption receipt;
- final contradiction-free L1 audit.

`.index` remains a separate external extraction/naming metadata family. It is not the original `.lst` runtime representation mechanism.
