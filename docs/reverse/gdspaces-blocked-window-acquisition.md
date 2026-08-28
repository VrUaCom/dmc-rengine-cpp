# GDSpaces EXE-window acquisition packet

**Scope:** reusable reverse-evidence acquisition support for GDSpaces roadmap gaps.  
**Artifact authority:** canonical DMC3 HD analysis executable only.  
**Canonical SHA-256:** `e454272ed0fb0247fcbcf300e5d55d7a3e96d50b89b9ffaff81bb978dcbdd082`.  
**Reconciled:** 2026-08-26 against merged #228/#230 and Materialization Completion Dependency Pass 2.

## Authority status

The old blocker “raw canonical analysis `dmc3.exe` is unavailable” is not a subsystem-wide truth. The canonical executable has been reacquired and raw-EXE L3 authority is present on main. However, this session did not expose a fresh canonical raw byte window for the unresolved `0x1402EF4D0` terminal-dependency seam, so Pass 2 remains an acquisition/model-narrowing pass rather than a fresh disassembly claim.

This packet is used for:

- reproducible exact-window reacquisition;
- provenance/receipt guardrails;
- narrow static follow-up on unresolved cross-layer seams;
- regression checks without creating a second PE authority.

It is not itself a semantic proof or original-process receipt.

The protected distribution executable whose known SHA begins `81c7...` is not instruction-level reverse authority and must not be substituted for `e454...`. Static analysis VAs must not be copied into the protected process without independent runtime mapping.

## Architecture boundary

```text
canonical analysis dmc3.exe
 -> dmc-rengine extract-exe-window
 -> SHA gate
 -> PE mapping gate
 -> exact VA window receipt
 -> local packet orchestration
 -> reverse/disassembly
 -> evidence review
 -> only then semantic promotion
```

## Evidence-lineage guardrails

An acquired packet is bound to the exact plan bytes that produced it:

- input plan copied verbatim to `packet.plan.json`;
- SHA-256 of those exact plan bytes recorded as `plan_sha256`;
- manifest records plan id/schema, artifact SHA/size and `authority_role`;
- every child receipt is hashed independently;
- partial output is removed if any child fails validation;
- `packet.receipt.json` is published only after every requested window succeeds.

A child receipt must agree on:

- schema `dmc-rengine.exe-byte-window.v1`;
- artifact SHA-256 and size;
- requested VA and size;
- PE image base / RVA / file-offset relationship;
- non-empty section identity;
- canonical window SHA-256.

Probe acquisition proves only the requested bytes and provenance, not function-body or semantic boundaries.

## Validate / acquire

General plan validation:

```text
python scripts/reverse/extract_exe_window_packet.py \
  --plan data/reverse/dmc3-gdspaces-blocked-window-plan.v1.json \
  --validate-plan-only
```

Focused materialization-completion plan validation:

```text
python scripts/reverse/extract_exe_window_packet.py \
  --plan data/reverse/dmc3-materialization-completion-boundary-plan.v1.json \
  --validate-plan-only
```

Acquire locally:

```text
python scripts/reverse/extract_exe_window_packet.py \
  --dmc-rengine <path-to-dmc-rengine> \
  --exe <canonical-analysis-dmc3.exe> \
  --expected-sha256 e454272ed0fb0247fcbcf300e5d55d7a3e96d50b89b9ffaff81bb978dcbdd082 \
  --plan <selected-plan.json> \
  --output <private-output-directory>
```

Use `--hex` only for private local reverse work. Proprietary executable bytes must not be committed.

## Current first-priority focused packet

Use:

```text
data/reverse/dmc3-materialization-completion-boundary-plan.v1.json
```

Owning reviews:

```text
docs/gdspaces/materialization-completion-boundary-pass-2026-08-26.md
docs/gdspaces/materialization-completion-dependency-pass2-2026-08-26.md
```

This packet addresses one narrow cross-layer question:

```text
L1 byte/materialization support
 -> materialization job/submission
 -> lower FileSlot/whole-file transport
 -> terminal dependency condition
 -> L3 scheduler completion callback
 -> LoadedResource state2
```

Canonical layer ownership is unchanged:

- FileSlot exact byte-read mechanics may support L1;
- FileSlot/AsyncIO request ownership/scheduling/callback lifecycle is L3;
- `0x1401B8CA0` is the explicit L1/L3 seam;
- LoadedResource states remain L3.

## Main correction — no generic fan-in counter

Merged #228 replaces broad `fan-in/completion` shorthand with **materialization completion ordering / dependency bridge**. Current evidence does not establish a generic original child/outstanding-work counter.

Merged #230 proves normal `0x1401B8DC0` receives exactly one u32 registry-relative context. It does not receive raw transport status, error flag, byte count, FileSlot handle or child/outstanding-work metadata.

Therefore the success/error eligibility decision must be resolved before normal `0x1401B8DC0` dispatch, or that queued completion must be suppressed/removed before execution.

FIFO insertion order alone is insufficient if the preceding materialization job can submit async work and retire immediately.

## Pass-2 focused targets

### Terminal whole-file cluster

The focused plan adds one primary window beginning at `0x140033390`, intentionally covering the historical Pass-90 cluster:

- `0x140033390` — historical load-state release/close anchor;
- `0x1400333E0` — historical status/poll anchor;
- `0x140033500` — caller-owned-destination transfer submit;
- `0x1400335A0` — lower transport completion/status write.

`0x140033390` and `0x1400333E0` remain **reacquisition hypotheses** until fresh canonical bytes confirm their exact roles.

### Higher materialization/scheduler cluster

- `0x1402EF4D0` — materialization submission/scheduling wrapper; primary target for exact queued job identity/type, callees, inherited load-context consumption and failure return;
- `0x1402EF790` — identify materialization-job dispatch case, persistence/re-poll behavior and terminal retirement condition;
- `0x1402EF580` — merged #230 enqueue ABI regression/context anchor;
- `0x1402EF460` — pending scheduler-entry clear/rollback; determine queued-completion suppression behavior without relabeling OS cancellation;
- `0x1401B84E0` — regression anchor for materialization-success -> state1 -> one-u32 completion registration;
- `0x1401B8DC0` — regression anchor for normal state2 publication with no transport metadata;
- `0x1401B8430` / `0x1401B8F00` — cancellation/cleanup comparison path.

### `.lst` follow-up

- `0x1401B85C0` remains the confirmed loose-container in-place materializer/recursive builder.
- Do not reopen grammar/layout.
- Apply the direct-resource terminal dependency model to child/recursive failure only after the direct-resource mechanism is closed.

## Falsifiable dependency models

The raw pass should discriminate among:

1. **persistent polling scheduler job** — job remains live/re-dispatchable until lower transfer terminal state;
2. **callback-driven terminal state** — `0x1400335A0` writes status observed by the higher job before retirement;
3. **separate scheduler gate** — later completion record cannot dispatch until another shared dependency changes;
4. **synchronous completion before `0x1402EF4D0` success** — allowed as a hypothesis until the body is closed.

A generic counter is not privileged above these alternatives.

## Revised raw-pass order

```text
1. 0x1402EF4D0 queued job identity/type + load-context consumer
2. 0x1402EF790 materialization-job persistence/re-poll/retirement
3. 0x1400333E0 pending/success/error status domain
4. 0x140033390 terminal cleanup/release point
5. 0x1400335A0 bind lower transport writes into terminal state
6. determine what blocks/suppresses normal 0x1401B8DC0 on failed/incomplete transport
7. 0x1402EF460 queued higher-work suppression/rollback
8. .lst child/recursive failure ordering
```

## Semantic discipline

Do not relabel:

- `0x1400335A0` as the LoadedResource state2 callback;
- `0x1402EF460` as OS-level `CancelIo`/AsyncIO cancellation;
- `0x1402EF4D0` as exact-path resolver, final provider open, raw `ReadFile`, sync-only or async-only loader;
- `0x1400333E0` or `0x140033390` historical helper roles as fresh canonical facts before reacquisition;
- the inherited materialization/load-context parameter as sync/async/priority without direct dataflow;
- queue insertion order alone as the proven dependency barrier.

## Address-authority correction

The old extra-zero staging/materialization address:

```text
0x14002EF4D0
```

is superseded for this target. Canonical accumulated authority identifies:

```text
0x1402EF4D0
```

The general and focused plans use the canonical target.

## Already strong architecture — do not restart broadly

- `0x140328540` ZIP/inflater lazy realization;
- `0x140328820` InflateRead;
- `0x140328F50` ZipEntryRead;
- `0x140328FE0` compressed seek/reset/reinflate architecture;
- `0x1401B85C0` loose-container materialization/recursive synthesis;
- packed-first `.lst` grammar/layout;
- central LoadedResource state/finalizer/cancellation/quiescence structure;
- merged #230 normal completion callback ABI.

Exact malformed/error/lifetime breadth is reacquired only when it changes a claimed compatibility boundary.

The general plan retains `0x14002DA40` as an unresolved `.lst` lower/helper candidate. It does not replace `0x1401B85C0` authority.

## Probe-window rule

Every configured probe size is acquisition coverage only. Never promote `0x400` to a function-body size. Exact-body promotion requires independent boundaries and a body hash.

## After reacquisition

For each target:

1. verify artifact, plan and window receipt identity;
2. establish actual function/callee boundaries and xrefs;
3. compare with current raw-EXE authority;
4. distinguish direct observation from inference;
5. classify evidence as L1 support, L3 lifecycle, or cross-layer seam evidence;
6. update the owning issue only when evidence changes/closes a gate;
7. keep unresolved tails explicit.

Current priority:

```text
materialization terminal dependency state machine
 -> transport-error to completion-suppression mapping
 -> .lst child failure / temp cleanup when activated
 -> residual L3 ownership/value-flow census
 -> original-process dynamic receipts
```
