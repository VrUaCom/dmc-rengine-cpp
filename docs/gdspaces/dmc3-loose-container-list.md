# DMC3 `.lst` Loose-Container Reconstruction

**Status:** current-main clean implementation candidate; canonical EXE mechanics are evidence-backed, product safety guards are explicitly separate.

Canonical executable authority: SHA-256 `e454272ed0fb0247fcbcf300e5d55d7a3e96d50b89b9ffaff81bb978dcbdd082`.

Direct reverse anchors:

- representation selector `0x1401B79E0`;
- size/parser helpers `0x1401B7B90`, `0x1401B7C70`, `0x1401B7D10`, `0x1401B7E60`, `0x1401B7FD0`;
- loose-container materializer `0x1401B85C0`;
- `.lst` extension rewrite helper `0x1401B9390`;
- generic resource materializer `0x1402EF4D0`.

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

## Critical correction to older Wave-3 shorthand

Two older evidence-model fields are superseded:

1. `hash_comments=true` is wrong. `#` is a magic directive; `/` is the recovered comment/skip marker.
2. `rewrites_entries_to_pac=true` is too broad. Ordinary children are resolved as `baseDirectory + rawToken`. `.pac` rewrite is evidenced specifically for a nested `.lst` sibling-precedence check.

Those historical branch fields must not be copied into clean GDSpaces code.

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
...    padding
        child payloads
```

The size planner starts child storage at:

```text
align64((slotCount + 2) * 4)
```

Every non-empty child contribution is rounded to a 64-byte boundary. This is an origin-specific `.lst` synthesis invariant; it must not be promoted as a universal packed PAC/PNST alignment rule.

## Nested `.lst` precedence

For a child token ending in lowercase `.lst`:

```text
nested.lst
    -> construct sibling nested.pac
    -> positive-size nested.pac exists: load packed sibling
    -> otherwise: recursively synthesize nested.lst in place
```

Ordinary non-list children are loaded from their exact directory-relative token path and are not rewritten to `.pac`.

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
- 64-byte synthesized header/child placement;
- nested `.lst` sibling `.pac` precedence and recursive in-place synthesis.

Product-only safety policy in this implementation:

- ancestry cycle rejection;
- configurable recursion-depth limit;
- configurable total synthesized-output budget;
- explicit 32-bit offset overflow rejection;
- fail-closed treatment of LF-only normal lines and inputs exceeding preserved bounds.

These safety rules must not be quoted as recovered original error semantics until direct evidence closes those branches.

## Open reverse/validation boundaries

- no raw real `.lst` artifact is currently exposed in the connected corpus;
- exact original recursion-cycle/depth behavior remains unresolved;
- exact malformed/truncated-list error propagation and temporary-buffer lifetime are not fully present in the preserved direct-disassembly summary;
- game-backed exact-build receipts remain separate from synthetic product regression.

`.index` remains a separate external extraction/naming metadata family. It is not the original `.lst` runtime representation mechanism.
