# GDSpaces blocked EXE-window acquisition packet

**Scope:** reverse-evidence acquisition support for GDSpaces roadmap gaps.  
**Artifact authority:** canonical DMC3 HD analysis executable only.  
**Canonical SHA-256:** `e454272ed0fb0247fcbcf300e5d55d7a3e96d50b89b9ffaff81bb978dcbdd082`.  
**Reconciled:** 2026-08-26 against the three-pass L1 EXE boundary review.

This runbook groups reusable EXE-window acquisition across several GDSpaces reverse targets. Some targets remain blocked on reacquisition; others are already recovered and are retained only as optional regression/reacquisition anchors.

The packet does **not** promote any semantic claim. It only reacquires exact byte windows through the canonical `dmc-rengine extract-exe-window` command.

## Architecture boundary

```text
canonical analysis dmc3.exe
 -> dmc-rengine extract-exe-window
 -> SHA gate
 -> PE mapping gate
 -> exact VA window receipt
 -> local packet orchestrator
 -> reverse/disassembly work
 -> evidence review
 -> only then semantic promotion
```

The Python packet script is orchestration only. It does not open/map the PE itself and therefore does not create another executable-byte authority.

The protected distribution executable whose known SHA is `81c7...` is not an instruction-level reverse authority and must not be substituted for the analysis build.

## Evidence-lineage guardrails

An acquired packet is bound to the exact plan bytes that produced it:

- the input plan is copied verbatim to `packet.plan.json`;
- SHA-256 of those exact plan bytes is recorded as `plan_sha256` in `packet.receipt.json`;
- the manifest records the plan id/schema, artifact SHA/size and `authority_role`;
- every child receipt is hashed independently and its receipt schema is recorded;
- partial output is removed when any child fails validation; `packet.receipt.json` is written only after every requested window succeeds.

A child receipt is accepted only when all of the following agree with the request and plan authority:

- exact schema `dmc-rengine.exe-byte-window.v1`;
- artifact SHA-256 and artifact size;
- requested VA and size;
- PE `image_base`, derived RVA relationship and file-offset range;
- non-empty section identity;
- canonical window SHA-256.

When `--hex` is requested, the raw hex must be exact lowercase bytes of the requested size and must hash to the reported window SHA. When `--hex` is not requested, a child receipt that unexpectedly contains raw bytes is rejected.

These checks protect provenance and transport integrity only. They do **not** turn a probe window into a semantic reverse claim or an exact function body.

## Validate the plan without proprietary bytes

```text
python scripts/reverse/extract_exe_window_packet.py \
  --plan data/reverse/dmc3-gdspaces-blocked-window-plan.v1.json \
  --validate-plan-only
```

This checks schema, artifact identity shape, authority metadata, duplicate IDs, VA/size ranges, issue/purpose metadata, mode rules and known-body hash requirements. The validation output includes the exact plan SHA-256.

The same plan validation and synthetic packet guardrail tests run in Ubuntu and Windows CI:

```text
python scripts/reverse/test_extract_exe_window_packet.py
```

Synthetic tests validate fail-closed packet behavior without proprietary executable bytes. They are infrastructure tests, not reverse evidence.

## Acquire the packet locally

Build `dmc-rengine`, then run:

```text
python scripts/reverse/extract_exe_window_packet.py \
  --dmc-rengine <path-to-dmc-rengine> \
  --exe <canonical-analysis-dmc3.exe> \
  --expected-sha256 e454272ed0fb0247fcbcf300e5d55d7a3e96d50b89b9ffaff81bb978dcbdd082 \
  --plan data/reverse/dmc3-gdspaces-blocked-window-plan.v1.json \
  --output <private-output-directory>
```

Add `--hex` only for local reverse work that actually needs raw bytes. Raw executable-byte receipts are proprietary local evidence and must not be committed to the public repository.

The output directory is no-replace: if it already exists the run fails rather than overwriting prior evidence. Every successful child receipt is hashed into `packet.receipt.json`, and the exact plan is preserved as `packet.plan.json`.

If any requested window fails SHA/PE/range/schema/raw-byte validation, the packet is not published and the partial output directory is removed.

## Current packet targets

### L1 / materialization fan-in and error handoff — current first priority

The 2026-08-26 three-pass review distinguishes raw transport completion from resource-level materialization completion.

Current first-priority windows are:

- `0x1400333F0` — whole-file selected-resource open/load-state construction;
- `0x140033500` — whole-file caller-owned-destination transfer submission;
- `0x1400335A0` — raw transport/whole-file completion callback and error/status mapping;
- `0x1402EF4D0` — resource materialization submission/scheduling wrapper;
- `0x1402EF580` — scheduler-ring enqueue helper;
- `0x1402EF790` — scheduler worker/callback execution neighborhood;
- `0x1401B84E0` — acquisition constructor, backing/destination setup and scheduling success boundary;
- `0x1401B8CA0` — direct/packed/loose materialization dispatch;
- `0x1401B8DC0` — resource scheduler/materialization completion handoff, normal `state 1 -> 2` publication.

Primary questions:

1. how direct/child submissions aggregate to one parent completion;
2. which counter/state/queue condition permits `0x1401B8DC0`;
3. how transport failure from the `0x1400335A0` layer propagates to the resource scheduler;
4. which failure branches suppress state2 publication;
5. whether partial parent payload remains allocated/live during failed materialization.

Correct labels are mandatory:

- `0x1400335A0` = transport completion;
- `0x1401B8DC0` = resource-level completion handoff;
- `0x1402EF4D0` = materialization submission/scheduling wrapper, not proven exact-path resolver/final open/raw reader.

### L1 / `.lst` temporary storage and recursive child completion

Direct grammar/layout anchors are already strong and should be used to close lifetime/error/fan-in breadth, not to rediscover the grammar:

- `0x1401B79E0` — packed-vs-loose representation selector;
- `0x1401B7B90` — allocation-size planner;
- `0x1401B85C0` — loose-container in-place materializer/recursive builder;
- `0x1401B8CA0` / `0x1402EF4D0` — child submission seam.

Questions:

- temporary aligned list-text allocator/free identity;
- synchronous text acquisition failure cleanup;
- malformed/truncated parse propagation;
- nested recursion failure behavior;
- child fan-in into parent resource completion.

Do **not** equate the `.lst` synchronous temporary loader with the separate synchronous-style wrapper around `0x1402EF920` without a direct caller/callee edge.

### L1 / ZIP support — secondary bounded probes

- `0x140328540` — ZIP/inflater stream initializer;
- `0x140328820` — known InflateRead neighbor/context anchor;
- `0x140328F50` — known ZipEntryRead neighbor/context anchor;
- `0x140328FE0` — compressed seek/reset/reinflate path.

These are no longer the automatic first targets. Lazy realization, STORE-vs-inflate, raw-DEFLATE streaming, reset+replay compressed seek, raw seek and teardown architecture are already strong. Reacquire exact-body/error breadth when a concrete acceptance claim requires it or when it helps resolve the fan-in/error seam.

### L2 / physical-provider regression anchors

- `0x140326D20` — physical mount anchor;
- `0x140327430` — resource mount resolution;
- `0x140327720` — path-existence/final-open caller context.

These three L2 windows are **not blockers for #204 anymore**. The canonical `e454...` executable was reacquired separately on 2026-08-25 and the type-0 post-`0x0C` physical chain was recovered directly, including `FindFirstFileA` / `FindClose` and exact `CreateFileA` final-open flags. They remain only as optional regression/reacquisition anchors.

The current unresolved Layer-2 acquisition target is protected-distribution runtime address mapping before an original-process selected-identity receipt can be trusted. This canonical-analysis packet does not solve that protected-process mapping gate.

### L3 / original runtime lifecycle support

The L3 set remains aligned to the canonical lifecycle roadmap. L3 starts from materialized state2 input for the layer cut; the byte-transport and state1→2 materialization handoff above are L1.

**R1 — state-writer/caller census**

- `0x1401B92D0` — state-2 typed finalizer, ready callback and state `2 -> 3`;
- `0x1401B8430` — canonical unfinished-resource cancellation writer, state `1|2 -> 4`;
- `0x1401B8F00` — deferred cancellation cleanup, state `4 -> 0`;
- `0x1401B9530` — ordinary owner-driven conditional release to state 0;
- `0x1401B9560` / `0x1401B95E0` — group/full reset writers.

`0x1401B84E0`, `0x1401B8CA0` and `0x1401B8DC0` remain shared acquisition/boundary probes in the packet, but their L1 behavior must not be counted as L3 completion.

**R2 — known-field ownership / topology writers**

- `0x1401B8380` — registry initialization;
- fixed-family wrappers `0x1401B8D60`, `0x1401B8F50`, `0x1401B8FF0`, `0x1401B90B0`, `0x1401B9160`, `0x1401B9270`;
- `0x1401B8DF0` — group-5 first-free dynamic pool.

**R3 — typed-dispatch breadth / failure semantics**

- `0x1401B9FA0` — recursive typed post-load dispatcher;
- `0x1403051B0` — SCM post-load layout contradiction follow-up.

Primary ledger: #88 / #55. Dynamic lifecycle receipts remain separate acceptance work.

## Probe-window rule

Every current GDS packet entry is `mode = probe`.

The configured `0x400` size is **coverage for reacquisition**, not an asserted function length. A probe that starts at a known VA must never be relabeled as an exact function body solely because the bytes were acquired successfully.

Exact body promotion requires an independently evidenced boundary and body hash. When those become known, the plan may promote a target to `known-body` with `body_sha256`; the orchestrator then fails closed if the reacquired hash differs.

## After acquisition

Do not merge raw probe bytes or jump directly to C++ behavior.

For each target:

1. verify artifact, exact plan and window receipt identity;
2. disassemble the acquired range locally;
3. establish actual function/callee boundaries and xrefs;
4. distinguish direct observation from inference;
5. update the owning issue with exact range/evidence status;
6. promote only semantics supported by the reacquired bytes;
7. keep unresolved tails explicitly unresolved.

Priority after acquisition is now:

```text
#100 materialization fan-in/completion
 -> #100 transport-to-resource error mapping
 -> #100 .lst temp allocation/free/failure cleanup
 -> acceptance-activated FileSlot/error breadth
 -> acceptance-activated ZIP exact-body breadth
 -> #88 typed-ready/lifecycle static breadth
 -> Level-E original-process instrumentation/receipts
```

This packet reduces evidence-acquisition friction. It does not change the rule that any layer reaches completion only after its real-corpus/original-process acceptance receipts are valid.
