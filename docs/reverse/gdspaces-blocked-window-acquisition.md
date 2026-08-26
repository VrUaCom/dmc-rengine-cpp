# GDSpaces EXE-window acquisition packet

**Scope:** reusable reverse-evidence acquisition support for GDSpaces roadmap gaps.  
**Artifact authority:** canonical DMC3 HD analysis executable only.  
**Canonical SHA-256:** `e454272ed0fb0247fcbcf300e5d55d7a3e96d50b89b9ffaff81bb978dcbdd082`.

## 2026-08-26 status correction

The old Layer-3 blocker **"raw canonical analysis `dmc3.exe` is unavailable" is CLOSED**.

The canonical executable was reacquired and re-identified at exact size `6,356,432` bytes and exact SHA-256 `e454272e...d082`. Layer-3 state-writer, typed-dispatch, release/reset and lifecycle-boundary targets were re-reviewed directly from those bytes.

Therefore this packet is now primarily:

- reproducible exact-window acquisition infrastructure;
- provenance/receipt guardrail tooling;
- regression/reacquisition support for static reverse targets;
- a convenient way to re-extract bounded windows without creating another PE authority.

It is **not the current Layer-3 semantic blocker**. Current L3 static status lives in:

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
 -> local packet orchestrator
 -> reverse/disassembly work
 -> evidence review
 -> only then semantic promotion
```

The Python packet script is orchestration only. It does not create another executable-byte authority.

The protected distribution executable whose known SHA is `81c7...` is not instruction-level reverse authority and must not be substituted for the analysis build. Conversely, static analysis VAs from `e454...` must not be copied into the protected process without independent runtime mapping.

## Evidence-lineage guardrails

An acquired packet is bound to the exact plan bytes that produced it:

- the input plan is copied verbatim to `packet.plan.json`;
- SHA-256 of those exact plan bytes is recorded as `plan_sha256` in `packet.receipt.json`;
- the manifest records plan id/schema, artifact SHA/size and `authority_role`;
- every child receipt is hashed independently and its receipt schema is recorded;
- partial output is removed when any child fails validation;
- `packet.receipt.json` is published only after every requested window succeeds.

A child receipt is accepted only when all of the following agree with the request and plan authority:

- exact schema `dmc-rengine.exe-byte-window.v1`;
- artifact SHA-256 and artifact size;
- requested VA and size;
- PE image base, derived RVA relationship and file-offset range;
- non-empty section identity;
- canonical window SHA-256.

When `--hex` is requested, the raw hex must be exact lowercase bytes of the requested size and must hash to the reported window SHA. When `--hex` is not requested, a child receipt that unexpectedly contains raw bytes is rejected.

These checks protect provenance and transport integrity only. They do not turn a probe into a semantic claim or an exact function body.

## Validate the plan without proprietary bytes

```text
python scripts/reverse/extract_exe_window_packet.py \
  --plan data/reverse/dmc3-gdspaces-blocked-window-plan.v1.json \
  --validate-plan-only
```

Synthetic packet guardrail tests remain CI-safe infrastructure tests:

```text
python scripts/reverse/test_extract_exe_window_packet.py
```

They are not reverse evidence.

## Acquire/reacquire the packet locally

```text
python scripts/reverse/extract_exe_window_packet.py \
  --dmc-rengine <path-to-dmc-rengine> \
  --exe <canonical-analysis-dmc3.exe> \
  --expected-sha256 e454272ed0fb0247fcbcf300e5d55d7a3e96d50b89b9ffaff81bb978dcbdd082 \
  --plan data/reverse/dmc3-gdspaces-blocked-window-plan.v1.json \
  --output <private-output-directory>
```

Add `--hex` only for local reverse work that actually needs raw bytes. Raw executable-byte receipts are proprietary local evidence and must not be committed to the public repository.

The output directory is no-replace. Any failed SHA/PE/range/schema/raw-byte validation prevents packet publication and removes partial packet output.

## Current target classification

### L2 regression / static anchors

- `0x140326D20` — physical mount anchor;
- `0x140327430` — resource mount resolution;
- `0x140327720` — path-existence/final-open caller context.

These are no longer the unresolved #204 static blocker. Protected-distribution runtime mapping and selected-provider observation remain separate L2 dynamic gates.

### L1 ZIP / loose-container support

- `0x140328540` — ZIP/inflater stream initializer;
- `0x140328820` — InflateRead anchor;
- `0x140328F50` — ZipEntryRead anchor;
- `0x140328FE0` — compressed seek/reset/reinflate path;
- `0x14002DA40` — `.lst` synthesis/builder follow-up;
- `0x14002EF4D0` — shared staging/materialization helper.

These remain available when a specific L1 compatibility edge requires direct reacquisition.

### L3 regression / remaining-census anchors

- `0x1401B8380` — registry initialization;
- `0x1401B8430` — canonical `1|2 -> 4` writer;
- `0x1401B84B0` — global quiescence predicate;
- `0x1401B84E0` — acquisition and state1 publication;
- `0x1401B8CA0` — L1/L3 materialization-success seam;
- `0x1401B8DC0` — normal state2 completion callback;
- `0x1401B8DF0` — group-5 first-free dynamic pool;
- `0x1401B8F00` — deferred state4 cleanup;
- `0x1401B92D0` — state2 finalizer / callback / state3;
- `0x1401B9530` — ordinary owner release;
- `0x1401B9560` / `0x1401B95E0` — group/full reset;
- `0x1401B9FA0` — central typed post-load dispatcher;
- `0x1403051B0` — SCM contradiction follow-up.

The 2026-08-26 raw pass already strengthens several of these semantics. Reacquisition of the same windows should now be treated as regression/reproducibility evidence unless contradictory bytes or new caller evidence appear.

## Important updated L3 facts

The current static authority includes:

- acquisition publishes state1 only after materialization dispatch succeeds;
- canonical cancellation changes only states1/2 to state4;
- quiescence requires every record to be state0 or state3;
- finalizer order is typed post-load -> optional callback -> state3;
- central unknown/default typed dispatch is best-effort/no-op and does not return failure to block state3;
- ordinary release, cancellation cleanup and group/full reset have different state0/release ordering;
- group-5 no-free-record behavior is a hard original invariant, not a safe product policy;
- L3 is a semantic/lifetime contract, not one contiguous EXE VA range.

## Probe-window rule

A configured probe size is coverage for reacquisition, not an asserted function length. A probe that starts at a known VA must never be relabeled as an exact function body solely because the bytes were acquired successfully.

Exact body promotion requires independently evidenced boundaries and body hashes.

## After reacquisition

For any target being re-opened:

1. verify artifact and packet identity;
2. establish actual function/callee boundaries and xrefs;
3. compare against the latest canonical raw-EXE audit before creating a new claim;
4. distinguish direct observation from inference;
5. update the owning issue only when evidence changes or closes a current gate;
6. keep unresolved tails explicitly unresolved.

Do not use this acquisition packet as a reason to restart already-bounded L3 core semantics.
