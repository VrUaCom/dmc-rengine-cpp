# GDSpaces L3 R1 current-main reconciliation — 2026-09-02

**Status:** CURRENT-MAIN SEMANTIC PORT / R1 STATIC BOUNDED-CLOSED / CONTRADICTION-GATED  
**Repository base reviewed:** `main@9483663959e5452f9a224c1535445bb5a3b33520`  
**Canonical executable:** `dmc3.exe`  
**SHA-256:** `e454272ed0fb0247fcbcf300e5d55d7a3e96d50b89b9ffaff81bb978dcbdd082`  
**Size:** `6,356,432` bytes  
**ImageBase:** `0x140000000`

## Purpose

This document semantically ports the final R1 writer-census conclusion from historical/open PR #240 onto the current-main evidence model without mechanically merging the stale #240 branch.

The port was reviewed against:

- the current merged LoadedResource lifecycle corpus;
- the 2026-08-31 runtime type-evidence split;
- the 2026-08-31 primary 3D/render family research;
- the 2026-09-01 real MOD/SHW payload bindings;
- the 2026-09-02 layer-boundary/L3 research review;
- stronger but still unmerged L1/L3 terminal-completion seam evidence from #258/#269.

Authority classes remain separate: a stronger open PR is evidence input, not merged truth, until its claims are reconciled on a current-main branch and reviewed.

## Decision

For the exact canonical analysis executable above and the declared `LoadedResource +0x04` state-writer scope:

> **L3-R1 = STATIC BOUNDED-CLOSED / APPROVED / CONTRADICTION-GATED.**

Broad state-writer discovery is no longer an active work item.

R1 is reopened only when later evidence provides a concrete contradictory write with exact LoadedResource record provenance, for example from R2/R3 field ownership, original-process lifecycle traces, a different build/profile, or a newly recovered runtime-computed alias/callback path.

This is not an L3 completion claim. R2–R5 and V1–V7 remain independently open.

## Canonical registry authority

```text
LoadedResource base = 0x140C99D30
record count        = 363
record stride       = 0x48
state field         = record + 0x04
```

Observed group counts and bases remain:

```text
counts = [4,136,60,28,1,128,6]
bases  = [0,4,140,200,228,229,357,363]
```

## Approved state spine

### Startup/static initialization

- `0x140010670` zeroes the full manager region before construction. This is process/static initialization, not an ordinary per-resource lifecycle transition.

### Runtime state 0 initialization

- `0x1401B8380`, including the write at `0x1401B83A5`, initializes each LoadedResource record state to 0.

### State 1

- `0x1401B84E0` publishes acquisition state 1 only after the bounded materialization-dispatch path reports success.

### State 2

- `0x1401B8DC0` publishes normal completion state 2 on the recovered normal even-context path.
- the normal scheduler context is one `u32` registry-relative offset equal to `index * 0x48` for index `0..362`.
- the odd-context branch remains classified as malformed/non-normal-context behavior; no alternate lifecycle semantic is invented.

### State 3

- `0x1401B92D0` performs typed post-load, optional ready callback, then publishes state 3.
- state 3 is consumer-ready lifecycle visibility for the manager path; it is not universal family-semantic success or a consumer-effect receipt.

### State 4

- `0x1401B8430` performs canonical cancellation/invalidation from source states 1 or 2 to state 4.

### State 0 teardown/rollback families

Keep these policies distinct rather than collapsing them into one `release()`:

- acquisition rollback: bounded `1B8DA4`, `1B8E60`, `1B92B8` paths;
- deferred state-4 cleanup: `0x1401B8F00`;
- ordinary owner release: `0x1401B9530`;
- fixed-family indexed release writers around `1B9914`, `1B997B`, `1B99D7`, `1B9A3B`;
- stored group-5 alias release/state0 writers around `1B946D`, `1B96CC`, `1B9B9A`, `1B9C83`;
- forced group reset: `0x1401B9560`;
- forced full reset: `0x1401B95E0`.

Their ordering and success semantics are intentionally not generalized.

## Final contradiction classes reviewed

Historical #240 challenged the proposed closure with the following additional classes and found no new provenance-valid state writer:

1. whole-manager startup zero vs runtime per-record initialization;
2. runtime bulk-zero/memset candidates;
3. one-hop helpers writing incoming `arg+0x04`;
4. exact record producer -> stored owner alias -> callee mutation;
5. partial-width and non-enum `+0x04` writes;
6. lock/atomic mutation forms;
7. normal completion and cancellation callback registration surfaces;
8. malformed odd completion context;
9. possible LoadedResource state values outside `0..4`.

The strongest rejected false-positive classes include:

- `0x140059390`: independent compact PRNG/hash/random-state transform, not LoadedResource;
- `0x1403097D0`: stack-local `RBP+0x04` write, not LoadedResource;
- higher-level state objects whose own `+0x04` field uses overlapping values while the actual LoadedResource pointer is held in a separate owner field;
- compact byte/word state arrays with separate LoadedResource pointer arrays;
- resource-aware zeroing of unrelated freshly allocated objects/buffers.

No provenance-backed atomic mutation of canonical `record+0x04` is established.

## Callback reference boundary

The bounded static callback-registration review remains compatible with current main:

- `0x1401B8DC0` has the canonical conventional materialization from the acquisition path around `0x1401B8572`;
- `0x1401B8F00` has the canonical conventional materialization from cancellation around `0x1401B8491`;
- no second static exact pointer-table authority was established in the reviewed image.

Dynamic/runtime-computed callback registration remains contradiction-gating evidence if later recovered.

## Compatibility review against newer merged type/family evidence

Newer merged research does not contradict R1 because it changes type/family interpretation rather than the destination provenance of LoadedResource state writes.

Current merged type evidence is explicitly split:

```text
0x1402DB1F0  registry three-byte content probe
0x1401B9FA0  PAC/PNST materialized-child post-load dispatcher
0x1402FD650  four-byte family-mask classifier
```

Merged family research additionally establishes:

- MOD/EFM/SCM as related mesh-bearing model-document families with distinct handlers/layout details;
- SHW as a distinct self-contained shadow-hull mesh family;
- MRP byte-backed runtime identity without a proven normal central post-load handler;
- MCV four-byte family recognition;
- EFW/EFE as dispatcher sentinels without proven normal handlers.

None supplies a new exact `LoadedResource +0x04` writer.

## L1/L3 seam after R1 closure

R1 closure does not move byte-result authority into L3 or state publication into L1.

Current semantic ownership is:

```text
[L1]
selected representation
 -> byte execution / byte-result semantics
 -> terminal materializer result

[L3]
request/scheduler/callback lifetime
 -> normal 0x1401B8DC0 state 1 -> 2 publication
 -> typed post-load / ready lifecycle
```

Stronger #258/#269 static evidence further reports that an admitted type-2 byte job remains current while pending/retrying and must retire before a later admitted type-3 normal completion callback can become current. That evidence is useful for the next lifecycle work but remains recorded separately from current merged implementation truth until reconciled.

## R1 non-claims

R1 closure does **not** prove:

- complete ownership semantics of `+0x08/+0x18/+0x20/+0x28`;
- exhaustive typed/factory/dependency behavior;
- universal family reference counting;
- dynamic cancellation/concurrency timing;
- reset/shutdown timing equivalence;
- cross-build/profile equivalence;
- original-process V1–V7 receipts;
- trusted lifecycle instrumentation;
- L3 COMPLETE / 100%.

## Work-order transition

With R1 bounded-closed, the active static Layer-3 work package becomes:

> **R2 — family-complete field/backing ownership for `+0x08/+0x18/+0x20/+0x28` and stable adjacent fields.**

R2 must preserve family/group distinctions and may reopen R1 only on a concrete contradictory state-write finding.

R3/R4 research may continue opportunistically when it resolves an R2 ownership ambiguity, but must not replace the R2 ownership census.

Dynamic work remains separate: rebuild the lifecycle validation tooling on current main, implement trusted original-process publishing/binding, then capture V1 and V5 before broad transition/shutdown breadth.

## Historical source disposition

Open PR #240 is now treated as a **historical semantic evidence source** for this current-main reconciliation. Its old-base branch should not be mechanically merged into current main. This document is the current proposed R1 authority for review/promotion on the 2026-09-02 reconciliation branch.
