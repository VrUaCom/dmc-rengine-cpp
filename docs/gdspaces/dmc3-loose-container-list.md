# DMC3 `.lst` Loose-Container Reconstruction

**Status:** canonical structural/runtime reconstruction at the evidenced scope; product safety guards remain explicitly separate.  
**Reconciled:** 2026-08-26 against `main@eb701b9c523a3ec87f3c73bb8764038f1f2ef8dc`.

Canonical executable authority: SHA-256 `e454272ed0fb0247fcbcf300e5d55d7a3e96d50b89b9ffaff81bb978dcbdd082`.

Direct reverse anchors:

- representation selector `0x1401B79E0`;
- size/parser helpers `0x1401B7B90`, `0x1401B7C70`, `0x1401B7D10`, `0x1401B7E60`, `0x1401B7FD0`;
- loose-container materializer `0x1401B85C0`;
- `.lst` extension rewrite helper `0x1401B9390`;
- materialization submission/scheduling wrapper `0x1402EF4D0`;
- L1/L3 seam `0x1401B8CA0`.

Cross-layer authority:

- `l1-exe-boundary-review-2026-08-26.md`;
- `l1-l3-exe-materialization-lifecycle-handoff-pass-2026-08-26.md`;
- `l3-boundary-audit-2026-08-26.md`;
- `l3-raw-exe-pass-2026-08-26.md`.

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

The grammar is CRLF-oriented. LF-only normal resource lines are not proven equivalent. The product implementation therefore fails them closed rather than advertising normalized LF-only input as original-compatible.

Normal-state line classes:

- leading `/` -> skipped/comment line;
- CR/blank line -> skipped;
- leading `#` -> magic directive/control line;
- any other normal line -> declared child slot.

Directive, comment and blank lines do not increment `slotCount`. A normal `dummy` line does.

## Critical correction to older Wave-3 shorthand

Two older evidence-model fields remain superseded:

1. `hash_comments=true` is wrong. `#` is a magic directive; `/` is the recovered comment/skip marker.
2. `rewrites_entries_to_pac=true` is too broad. Ordinary children are resolved as `baseDirectory + rawToken`. `.pac` rewrite is evidenced specifically for nested `.lst` sibling precedence.

## Magic directive

A directive has structural form:

```text
#XXXX
```

Up to four immediate bytes following `#` are captured as the synthesized four-byte magic. Whitespace is not skipped before capture. If no directive supplies a magic, the synthesized container uses `PAC\0`.

`#PNST` is therefore a valid generic four-byte magic case, not evidence of a special hardcoded PNST directive parser.

## Declared children and sparse slots

For each normal child line, the child helper:

1. captures the raw token;
2. forms `baseDirectory + token`;
3. strips CR termination;
4. preserves normal-line order as declared slot order.

Exact lowercase `dummy` remains a declared sparse slot:

- offset-table entry `0`;
- no child payload size;
- no child materialization submission.

Missing/zero-size ordinary children are not silently converted into `dummy`.

## Synthesized binary envelope

```text
+0x00  magic[4]
+0x04  u32 slotCount
+0x08  u32 relativeOffsets[slotCount]
...    padding
        child payloads
```

Child storage begins at:

```text
align64((slotCount + 2) * 4)
```

Every non-empty child contribution is rounded to a 64-byte boundary. This is an origin-specific `.lst` synthesis invariant and is not a universal packed PAC/PNST alignment rule.

## Temporary list-text acquisition

Canonical evidence establishes that `.lst` text is acquired **synchronously into aligned temporary storage** before bounded parsing.

A separate synchronous-style wrapper exists around `0x1402EF920`, but no direct caller/callee edge proves that the `.lst` temporary loader is that wrapper.

Therefore:

```text
.lst synchronous temporary load == 0x1402EF920
```

is **not promoted**.

Open byte-side work remains exact allocator/free identity and malformed/failure cleanup.

## Nested `.lst` precedence and child population

For a child token ending in lowercase `.lst`:

```text
nested.lst
    -> construct sibling nested.pac
    -> positive-size nested.pac exists: materialize packed sibling
    -> otherwise: recursively synthesize nested.lst in place
```

Ordinary non-list children are materialized from their exact directory-relative token path and are not rewritten to `.pac`.

After layout planning, `0x1401B85C0` populates the parent destination:

- `dummy` -> no submission;
- ordinary child -> materialization submission to `destination + relativeOffset`;
- existing packed sibling -> same materialization layer;
- nested loose list -> recursive in-place synthesis at `destination + relativeOffset`.

Safe label for `0x1402EF4D0` is **resource materialization submission/scheduling wrapper**. It is not proven to be the final provider open, exact-path resolver or raw file reader.

## L1/L3 completion-ordering boundary

The canonical raw-L3 pass now fixes the ownership cut:

```text
0x1401B85C0 / 0x1401B8CA0 materialization mechanics -> L1 evidence
0x1401B8CA0 success -> 0x1401B84E0 may publish state1 -> L3
0x1401B8DC0 state1 -> state2 -> L3
```

For `.lst`, current evidence does **not** prove an explicit child-count/outstanding-work fan-in counter.

The exact open question is:

> What dependency/order guarantees that L3 acquisition/completion cannot advance while required `.lst` child population is invalid or failed?

Possible mechanisms include synchronous child work, scheduler ordering, nested callbacks, another status/dependency object or an explicit counter. None is promoted without direct evidence.

Open questions:

- exact `0x1402EF4D0` behavior for ordinary/packed child submissions;
- whether child population is synchronous, queued or callback-driven at the relevant seam;
- how one child transport/materialization failure reaches acquisition/cancellation;
- whether partially populated parent bytes remain live on failure;
- temporary-list allocation/free/failure cleanup;
- interaction with higher scheduler rollback if lower transport has already started.

Focused handoff plan: `data/reverse/dmc3-materialization-lifecycle-handoff-plan.v1.json`.

## Original mechanics vs product hardening

Evidence-backed original mechanics in this slice:

- packed-first top-level selection;
- `.lst` extension fallback;
- synchronous aligned temporary list-text acquisition;
- scanner/token bounds;
- CRLF-oriented grammar;
- `/` comments and `#XXXX` magic directives;
- default `PAC\0` magic;
- declared child order/count;
- exact `dummy` sparse slot;
- 64-byte synthesized header/child placement;
- nested `.lst` sibling `.pac` precedence and recursive in-place synthesis;
- ordinary/packed child population through the generic materialization layer.

Product-only safety policy:

- ancestry cycle rejection;
- configurable recursion-depth limit;
- configurable total synthesized-output budget;
- explicit 32-bit offset overflow rejection;
- fail-closed treatment of LF-only normal lines and inputs exceeding preserved bounds.

These safety rules must not be quoted as recovered original error semantics until direct evidence closes those branches.

## Open reverse/validation boundaries

- no raw real `.lst` artifact is currently exposed in the connected retail corpus;
- exact temporary-buffer allocator/free and failure cleanup remain unresolved;
- exact original recursion-cycle/depth behavior remains unresolved;
- exact malformed/truncated-list error propagation remains unresolved;
- exact child-population -> L3 lifecycle completion/error dependency remains unresolved;
- a generic child/outstanding-work fan-in counter is **not evidenced**;
- game-backed exact-build receipts remain separate from synthetic product regression.

`.index` remains separate external extraction/naming metadata. It is not the original `.lst` runtime representation mechanism.
