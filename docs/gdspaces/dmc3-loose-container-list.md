# DMC3 `.lst` Loose-Container Reconstruction

**Status:** current-main implementation candidate under active Layer-1 reverse; canonical EXE mechanics below are evidence-backed, product safety guards are explicitly separate.

Canonical executable authority: SHA-256 `e454272ed0fb0247fcbcf300e5d55d7a3e96d50b89b9ffaff81bb978dcbdd082`.

Direct reverse anchors:

- representation selector `0x1401B79E0`;
- size/parser helpers `0x1401B7B90`, `0x1401B7C70`, `0x1401B7D10`, `0x1401B7E60`, `0x1401B7FD0`;
- loose-container materializer `0x1401B85C0`;
- `.lst` extension rewrite helper `0x1401B9390`;
- direct whole-file transfer-extent helper `0x1402EF620`;
- materialization enqueue entry `0x1402EF4D0` and consumer `0x1402EF790`;
- whole-file open/chunk/read spine `0x1400333F0`, `0x1400333C0`, `0x140033500`, `0x14002F930`;
- zeroing allocator `0x140337600` with `memset` thunk `0x140346BEA`.

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

Recovered bounds:

- total scanner ceiling: `0x1FC0` / 8128 bytes;
- child-token extraction bound: `0x100` / 256 bytes;
- normal child text terminates at CR (`0x0D`) or NUL;
- skip/control states return to normal at LF (`0x0A`).

The grammar is CRLF-oriented. LF-only normal resource lines are not proven equivalent. The product implementation therefore fails them closed rather than silently advertising a normalized LF-only file as original-compatible.

Normal-state line classes:

- leading `/` -> skipped/comment line;
- CR/blank line -> skipped;
- leading `#` -> magic directive/control line;
- any other normal line -> declared child slot.

Directive, comment and blank lines do not increment `slotCount`. A normal `dummy` line does.

## Critical corrections to older shorthand

Three older simplifications are superseded:

1. `hash_comments=true` is wrong. `#` is a magic directive; `/` is the recovered comment/skip marker.
2. `rewrites_entries_to_pac=true` is too broad. Ordinary children are resolved as `baseDirectory + rawToken`. `.pac` rewrite is evidenced specifically for a nested `.lst` sibling-precedence check.
3. "every non-empty child is merely align64(size)" is wrong. Fresh canonical-EXE reverse shows that a **direct whole-file child uses a 0x800 transfer extent**. Only the structural header and recursively synthesized complete images use the 0x40 structural alignment.

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
3. strips CR termination;
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

`0x1402EF620` opens the resource through the whole-file path, obtains `ceil(logicalSize / 0x800)` from `0x1400333C0`, closes the temporary state, and returns:

```text
ceil(logicalSize / 0x800) * 0x800
```

The planner and writer therefore reserve that **0x800-granular transfer extent** for:

- an ordinary direct child token;
- a positive-size packed sibling `.pac` selected for a nested `.lst` token.

The actual async read writes only the real materialized bytes. The remainder of the reserved transfer extent remains zero because the complete output allocation is zero-initialized before emission.

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
         reserve ceil(size/0x800)*0x800
    -> otherwise:
         recursively synthesize nested.lst in place
         reserve align64(completeSynthesizedImageSize)
```

Ordinary non-list children are loaded from their exact directory-relative token path and are not rewritten to `.pac`.

## Enqueue vs byte-producing path

`0x1402EF4D0` is an enqueue function, not the byte-producing body itself. It records the destination pointer/path and a type-2 job in the ring.

The consumer `0x1402EF790` executes the type-2 job through:

```text
0x1400333F0(path)          whole-file open / cached logical size
 -> 0x1400333C0(state)     ceil(size/0x800) chunk count
 -> 0x140033500(...)       async submit
 -> 0x14002F930            seek + repeated reads until request filled, EOF/no-progress, or error
```

That distinction supersedes older documentation that labeled `0x1402EF4D0` itself as the generic materializer.

## Original mechanics vs product hardening

Evidence-backed original mechanics in this slice:

- packed-first top-level selection;
- `.lst` extension fallback;
- scanner/token bounds;
- CRLF-oriented line grammar;
- `/` comments and `#XXXX` magic directives;
- default `PAC\0` magic;
- declared child order/count;
- exact `dummy` sparse slot;
- 0x40-aligned synthesized header;
- direct whole-file child extent `ceil(size/0x800)*0x800`;
- recursively synthesized complete-image extent under 0x40 structural alignment;
- zero initialization of the complete planned image;
- nested `.lst` sibling `.pac` precedence and recursive in-place synthesis.

Product-only safety policy in this implementation:

- ancestry cycle rejection;
- configurable recursion-depth limit;
- configurable total synthesized-output budget;
- explicit host/32-bit offset overflow rejection instead of reproducing unsafe original 32-bit wrap;
- fail-closed treatment of LF-only normal lines and inputs exceeding preserved bounds.

These safety rules must not be quoted as recovered original error semantics until direct evidence closes those branches.

## Open reverse/validation boundaries

- representative real `.lst` corpus receipt is still required;
- exact original recursion-cycle/depth behavior remains unresolved;
- exact malformed/truncated-list and enqueue-failure propagation still require final branch-by-branch reconciliation;
- original planner/writer 32-bit overflow/wrap behavior is observed but product code intentionally fails closed rather than reproducing unsafe wrap;
- controlled game-backed materialization/consumption receipts remain separate from synthetic regression.

`.index` remains a separate external extraction/naming metadata family. It is not the original `.lst` runtime representation mechanism.
