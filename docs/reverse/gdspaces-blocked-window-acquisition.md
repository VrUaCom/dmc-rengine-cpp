# GDSpaces EXE-window acquisition packet

**Scope:** reusable reverse-evidence acquisition support for GDSpaces roadmap gaps.  
**Artifact authority:** canonical DMC3 HD analysis executable only.  
**Canonical SHA-256:** `e454272ed0fb0247fcbcf300e5d55d7a3e96d50b89b9ffaff81bb978dcbdd082`.  
**Current main baseline:** `2ed43b438f1bf01638f3e56341e98f6085e5b0fd`.

## 2026-08-26 authority status

The old blocker “raw canonical analysis `dmc3.exe` is unavailable” is closed. The canonical executable was reacquired at exact size 6,356,432 bytes and exact SHA-256 above. The current L3 raw-EXE pass directly revalidates the acquisition/state/finalizer/release boundaries.

This packet remains useful for:

- reproducible exact-window reacquisition;
- provenance/receipt guardrails;
- narrow static follow-up on unresolved cross-layer seams;
- regression checks without creating a second PE authority.

It is not itself a semantic proof or original-process receipt.

Canonical L3 static status remains in:

- `docs/gdspaces/l3-boundary-audit-2026-08-26.md`;
- `docs/gdspaces/l3-raw-exe-pass-2026-08-26.md`;
- issue #88.

Dynamic L3 acceptance remains issue #217.

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

The protected distribution executable whose known SHA begins `81c7...` is not instruction-level reverse authority and must not be substituted for `e454...`. Static analysis VAs must not be copied into the protected process without independent runtime mapping.

## Evidence-lineage guardrails

An acquired packet is bound to the exact plan bytes that produced it:

- the input plan is copied verbatim to `packet.plan.json`;
- SHA-256 of those exact plan bytes is recorded as `plan_sha256`;
- the manifest records plan id/schema, artifact SHA/size and `authority_role`;
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

Owning review:

```text
docs/gdspaces/materialization-completion-boundary-pass-2026-08-26.md
```

This packet addresses one narrow cross-layer question:

```text
L1 byte/materialization support
 -> materialization success/failure bridge
 -> L3 scheduler/request ownership
 -> LoadedResource state1
 -> completion callback
 -> state2
```

Canonical layer ownership is not changed:

- FileSlot byte-read mechanics may support L1;
- FileSlot/AsyncIO request ownership/scheduling/callback lifecycle is L3;
- `0x1401B8CA0` is the explicit L1/L3 seam;
- LoadedResource states are L3.

### Focused targets

- `0x140033500` — caller-owned-destination whole-file transfer submission;
- `0x1400335A0` — lower transport callback;
- `0x14002EA40` — ReadRequest/FileSlot submission seam;
- `0x1402EF460` — pending scheduler-entry clear/rollback reacquisition target;
- `0x1402EF4D0` — materialization submission/scheduling wrapper;
- `0x1402EF580` — scheduler enqueue;
- `0x1402EF790` — scheduler worker;
- `0x1401B8430` — cancellation writer control path;
- `0x1401B84E0` — acquisition/state1 publication ordering;
- `0x1401B85C0` — confirmed loose-container materializer / recursive builder anchor;
- `0x1401B8CA0` — materialization-return seam;
- `0x1401B8DC0` — normal state1→2 completion callback;
- `0x1401B8F00` — deferred state4 cleanup control path.

### Semantic discipline

The focused packet explicitly does **not** assert a generic fan-in counter. The open question is the actual completion ordering/dependency mechanism.

Do not relabel:

- `0x1400335A0` as the LoadedResource state2 callback;
- `0x1402EF460` as OS-level `CancelIo`/AsyncIO cancellation;
- `0x1402EF4D0` as exact-path resolver, final provider open, raw `ReadFile`, sync-only or async-only loader;
- the inherited materialization/load-context parameter as sync/async/priority until direct dataflow proves it.

## Address-authority correction

The pre-#228 general blocked-window plan on main contained the extra-zero staging/materialization address:

```text
0x14002EF4D0
```

Accumulated canonical resource-runtime evidence identifies the relevant materialization wrapper as:

```text
0x1402EF4D0
```

These are different VAs. This change corrects the general plan to `0x1402EF4D0`; the focused plan uses the same canonical target. The extra-zero form remains superseded for this target unless direct canonical bytes prove an independent intended function there.

## L1 ZIP / loose-container support

Already strong architecture should not be restarted without a concrete acceptance dependency:

- `0x140328540` — ZIP/inflater lazy realization;
- `0x140328820` — InflateRead;
- `0x140328F50` — ZipEntryRead;
- `0x140328FE0` — compressed seek/reset/reinflate;
- `0x1401B85C0` — loose-container materialization/recursive synthesis;
- packed-first `.lst` grammar/layout remains already recovered.

Exact malformed/error/lifetime breadth is reacquired only when it changes a claimed compatibility boundary.

The general plan also retains `0x14002DA40` as a `.lst` follow-up anchor. Treat it as an unresolved lower/helper candidate until its direct body/caller role is independently established; it does not replace the confirmed `0x1401B85C0` materializer authority.

## L2 regression / static anchors

- `0x140326D20` — physical mount anchor;
- `0x140327430` — resource mount resolution;
- `0x140327720` — path-existence/final-open context.

These are not the current #204 blocker. Protected-process runtime mapping and original selected-provider observation remain separate dynamic L2 gates.

## L3 regression / remaining-census anchors

Current strong central anchors include:

- `0x1401B8380` — registry initialization;
- `0x1401B8430` — canonical states1/2→4 writer;
- `0x1401B84B0` — quiescence predicate `{0,3}` across all records;
- `0x1401B84E0` — acquisition/state1 publication after materialization success;
- `0x1401B8DC0` — normal state2 completion callback;
- `0x1401B8DF0` — group-5 first-free dynamic pool;
- `0x1401B8F00` — deferred state4 cleanup;
- `0x1401B92D0` — typed post-load -> optional callback -> state3;
- `0x1401B9530` — ordinary owner release;
- `0x1401B9560` / `0x1401B95E0` — group/full reset;
- `0x1401B9FA0` — central typed dispatcher;
- `0x1403051B0` — SCM contradiction follow-up.

The fresh 2026-08-26 raw pass and merged #230 scheduler/context pass already strengthen many of these. Reacquisition should be treated as regression/reproducibility work unless new bytes/callers close an open gate.

## Probe-window rule

Every configured probe size is acquisition coverage only. Never promote `0x400` to a function-body size. Exact-body promotion requires independent boundaries and a body hash.

## After reacquisition

For each focused target:

1. verify artifact, plan and window receipt identity;
2. establish actual function/callee boundaries and xrefs;
3. compare with the latest canonical raw-EXE audit;
4. distinguish direct observation from inference;
5. determine whether evidence belongs to L1 support, L3 lifecycle, or both sides of the seam;
6. update the owning issue only when evidence changes/closes a gate;
7. keep unresolved tails explicitly unresolved.

Priority is now:

```text
materialization completion ordering / dependency bridge
 -> transport-error to materialization/lifecycle error mapping
 -> .lst child failure / temp cleanup when activated
 -> residual L3 state-writer/ownership census
 -> original-process dynamic receipts
```

Do not restart already-bounded ZIP, physical-provider or central L3 state semantics without contradictory evidence.
