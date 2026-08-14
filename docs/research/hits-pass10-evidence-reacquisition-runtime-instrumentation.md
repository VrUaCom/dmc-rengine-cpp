# HITS Raw Reverse Pass 10 — Evidence Reacquisition and Runtime Instrumentation

Date: 2026-08-14  
Canonical executable SHA-256: `e454272ed0fb0247fcbcf300e5d55d7a3e96d50b89b9ffaff81bb978dcbdd082`  
Status: **ACTIVE — REVERSE / IMPLEMENT / REVIEW / DEBUG LOOP**

Canonical detailed working authority: Google Drive document `DMC Rengine — HITS Raw Reverse Pass 10 — Evidence Reacquisition and Runtime Instrumentation — 2026-08-14`.

The required workflow is also codified in `docs/research/reverse-pass-implementation-review-loop.md` and in Google Drive document `DMC Rengine — Canonical Reverse Pass Implementation Review Debug Loop — 2026-08-14`.

## Why Pass 10 exists

Pass 9 reached static ABI/ownership saturation for the currently materialized canonical evidence. Repeating the same summary/xref corpus no longer yields instruction-level fields for the remaining P0 functions. Pass 10 therefore changes method: reacquire missing instruction windows or gather hash-gated runtime traces without changing collision semantics.

## Required loop for every Pass-10 step

1. acquire evidence;
2. review the direct evidence and assumptions;
3. deepen through callers/callees/writers/readers/ownership/state/corpus;
4. review again and define the exact promotion boundary;
5. implement only the promoted subset;
6. review implementation against evidence;
7. debug/test without weakening evidence gates;
8. consolidate the step into the Pass-10 authority;
9. perform a second independent review/debug cycle;
10. update code, GitHub research/issue/PR documentation, Google Drive Pass document, Mega Synthesis and Preservation Registry as applicable.

No material correction or debug lesson may remain only in chat.

## P0 targets

- `0x14005E7A0`: complete argument ABI, stack locals, static/dynamic temporary results, no-hit initialization, metric comparison, arbitration, equality/tie-break and caller-visible writes;
- `0x14005B460`: body, category-list entry structure, list ownership/lifetime, candidate production and return contract;
- `0x14005FEC0`: exact source-1 segment-query input/output ABI and result propagation;
- `0x1400601E0`: exact in/out layout, fourth component semantics, triangle iteration and accumulation/convergence behavior.

## Static reacquisition order

1. Materialize canonical EXE bytes for the exact SHA.
2. Extract complete disassembly windows for each P0 target.
3. Extract direct caller windows with register/stack setup.
4. Extract direct callee windows for geometry/candidate/result helpers.
5. Record VA/RVA/file-offset and expected-byte anchors.
6. Repeat disassembly independently and compare control-flow/write-set output.

## Required field-offset matrix

For every target function record:

- RCX/RDX/R8/R9 and stack arguments;
- local stack allocation and saved registers;
- every write to caller-owned memory;
- every temporary result-buffer offset and width;
- initialization values/sentinels;
- candidate source/identity;
- comparison metric and branch relation;
- copy/overwrite order;
- return register/value;
- exact caller read-back offsets.

No semantic CollisionResult field name may be assigned before write+read or runtime-delta evidence supports it.

## Runtime instrumentation fallback

If canonical instruction bytes cannot be materialized from project sources, use a guarded observation-only design.

The production-side generic evidence contract is now implemented in:

- `include/dmc_rengine/evidence/runtime_trace.hpp`;
- regression: `tests/runtime_trace_tests.cpp`.

The contract intentionally models instrumentation metadata, not original DMC3 ABI:

- exact target SHA-256;
- function VA;
- expected bytes for a target/hook site;
- tooling capture-window size;
- sequence and trace phase;
- optional caller VA;
- optional selected source/reject mask/dynamic category/result code;
- one or more raw memory snapshots.

`capture_window_bytes` is a tooling parameter only. It is not evidence of any original structure size.

## First implementation / review / debug receipt

### Implementation

Added a generic runtime-trace evidence contract and registered a regression test in CTest.

### Review finding

The initial regression fixture paired canonical HITS function VAs with synthetic expected bytes. Although the bytes existed only in a test, that presentation could be misread as an evidence claim about the canonical executable.

### Debug/correction

The fixture was corrected to use a fully synthetic SHA, synthetic VAs and synthetic bytes. Canonical addresses may only be paired with expected-byte anchors after those bytes are actually reacquired from the exact canonical artifact.

This correction is part of the Pass-10 evidence record and must not be silently discarded.

## Second implementation / review / debug receipt — profile-specific HITS runtime evidence

### Evidence promoted

Pass 7B/7C already EXE-confirmed the specialized query-family identities, the six dynamic category bindings, the category-to-static-HITS reject-mask bridge at dispatcher `0x14005B8E0`, the `AL`-observed success paths, the explicit 16-byte in/out correction paths, and the three wrapper source slots. Those facts were safe to promote without inventing the unresolved generic result ABI.

### Implementation

Added DMC3-profile evidence descriptors in:

- `include/dmc_rengine/profiles/dmc3/hits_query_evidence.hpp`;
- regression: `tests/hits_query_evidence_tests.cpp`.

The profile records:

- nine known HITS query-family VAs with unresolved fields kept unresolved;
- `EBE0` and `F070` as confirmed mutable 16-byte in/out paths;
- `EE40` and `60790` only as `AL`-observed success at the preserved caller, without claiming non-mutation;
- dynamic category bindings `0x02/0x05/0x08/0x0B/0x0E/0x11`;
- activation flags `0x1000..0x20000`;
- manager offsets `+0x10..+0x88` in `0x18` steps;
- dispatcher-level static-HITS masks `0x0040/0x0002/0x0010/0x0020/0/0`;
- wrapper source 0/member 3, source 1/member 6, and structurally present external/global source 2 with its selection/backing uncertainty preserved;
- an exact-build guard through the existing `phase12_canonical_target().matches_hash(...)` contract, so these VA descriptors are accepted only for canonical SHA-256 `e454272ed0fb0247fcbcf300e5d55d7a3e96d50b89b9ffaff81bb978dcbdd082` and reject the packed build SHA `81c7e61983564113b5105e931d9f185accc14e44ae147d27f720c2d50935c7d6`.

### Review finding 1 — ownership and correction

The first version placed DMC3-specific function VAs under generic `include/dmc_rengine/hits/`. This violated the project-wide game-profile architecture rule: universal/generic HITS code must not own DMC3 addresses or profile-specific runtime identities.

The implementation was corrected immediately:

- DMC3-specific descriptor moved to `include/dmc_rengine/profiles/dmc3/hits_query_evidence.hpp`;
- generic `include/dmc_rengine/hits/query_evidence.hpp` removed;
- regression updated to consume the DMC3 profile layer.

This correction is canonical and must not be reverted by moving hardcoded DMC3 VAs back into generic HITS core.

### Review finding 2 — dispatcher evidence scope

The first profile field name `static_hits_reject_mask` was too broad: especially for categories `0x0E` and `0x11`, evidence proves only that dispatcher `0x14005B8E0` passes zero in this branch; it does not prove that those categories never participate in filtering elsewhere.

The field was renamed to `dispatcher_static_hits_reject_mask`. Tests assert the dispatcher-level mapping and do not promote zero to a global category property.

### Review finding 3 — build identity gate

Profile namespace alone was insufficient protection because the recovered VAs belong to one exact DMC3 HD executable build. The profile delegates hash matching to the existing canonical known-target contract instead of duplicating target identity. Regression explicitly accepts the canonical SHA, rejects the packed SHA, and rejects malformed hash input.

## Third implementation / review / debug receipt — runtime topology promotion

### Evidence promoted

Phase-4 static saturation and the Pass-7/Pass-9 preserved corpus support a stronger topology layer without resolving the still-open generic result ABI:

- source selector `0x14005EBC0` has exactly four direct callsites:
  - `0x140056832`: select source 1/member 6;
  - `0x14005686E`: restore source 0/member 3;
  - `0x1400568F0`: select source 1/member 6;
  - `0x140056936`: restore source 0/member 3;
- two exact temporary source-1 paths are therefore preserved:
  - `0x140056832 -> 0x1400601E0 -> 0x14005686E`;
  - `0x1400568F0 -> 0x14005FEC0 -> 0x140056936`;
- query-family direct caller census is preserved as `1/51/1/1/1/1/1/5/1` for `E460/E7A0/EBE0/EE40/F070/FD10/FEC0/601E0/60790`;
- runtime-helper static call census is preserved for initializer, teardown, binder, selector, broadphase collector, cell-list resolver, record resolver, plane evaluator and normal classifier;
- two final static xref scans produced the same result, so no additional direct source selector, HITS owner, initializer/teardown or raw-record resolver is promoted from the current static corpus.

This is topology evidence only. It does not close source-2 selection/lifetime, `E7A0` arbitration, `B460` candidate production, `FEC0` output ABI or `601E0` accumulation semantics.

### Implementation

The DMC3 profile evidence API now exposes SHA-gated lookup for:

- exact direct selector callsites;
- the two temporary source-1 query paths;
- runtime helper roles/VAs/static caller counts;
- query-family caller counts and the existing dynamic/source evidence.

Packed build SHA `81c7...c7d6` receives no profile evidence through these lookup APIs.

### Second-review finding 4 — `namespace detail` was not access control

The first Slice-3 correction moved backing evidence tables into `namespace detail` and then described them as non-bypassable. That statement was too strong: C++ namespaces do not provide access control, so external code could still name `hits_evidence::detail::k_*` and bypass the intended SHA-gated lookup surface.

Correction commit: `4845120fbe9a3362d4040a0e37e32387411411d7`.

The backing tables now live as `private inline static constexpr` members of `detail::EvidenceStore`. Public lookups delegate to `EvidenceStore` methods that require the exact executable SHA. The intent is not to hide addresses from source code; it is to prevent the library API from exposing an ungated raw-table path that could accidentally apply canonical-build VAs to another executable.

### P0 evidence boundary after Slice 3

The preserved corpus is sufficient to promote caller counts, mask routing, source switching and helper topology. It is not sufficient to fabricate the missing instruction-level body of `0x14005E7A0`.

For `0x14005E7A0`, still `RESEARCH REQUIRED`:

- full prologue and exact RCX/RDX/R8/R9/stack ABI;
- local stack frame and temporary result offsets;
- no-hit sentinel initialization;
- complete static candidate write-set;
- complete dynamic candidate write-set for categories `0x0E/0x11`;
- the metric used to compare candidates;
- branch relation and equality/tie-break behavior;
- copy/overwrite order into caller-visible output;
- exact caller read-back offsets.

The known 51-caller mask census and static/dynamic split constrain this future reverse step but do not substitute for the missing instruction window.

## Safety gates

- exact canonical SHA required;
- expected bytes required for every hook site;
- no hooks on unknown/custom EXEs;
- reversible patch/trampoline only;
- logging must not change arguments, selected source, masks, geometry, result data or branch outcomes;
- preserve event ordering to reconstruct static/dynamic arbitration;
- runtime observations stay `DERIVED FROM VERIFIED RUNTIME` until trace integrity and controlled game validation.

## Control experiment matrix

- source0 generic query with mask 0;
- source0 mask `0x0008` family;
- source0 mask `0x0010` family;
- source1 `0x14005FEC0` path;
- source1 `0x1400601E0` path;
- dynamic categories `0x02/0x05/0x08/0x0B/0x0E/0x11` where reachable;
- hit/no-hit pairs through the same caller;
- static-only vs dynamic-present cases;
- equal or near-equal candidate cases if controllable, to expose tie-break behavior.

## Pass-9 handoff

Pass 9 already established stage-local source ownership, source2 structural HitsRuntime compatibility, specialized query register ABI, dynamic category/mask routing, correction direction and the static evidence boundary. Pass 10 owns evidence reacquisition and runtime proof for unresolved original-game P0 ABI.

## Explicit non-goals

- no guessed original `CollisionResult` layout;
- no invented gameplay names for dynamic categories or source 1/source 2;
- no product `ContactResult` promoted as Capcom ABI;
- no GDSpaces ownership of recovered collision runtime;
- no proprietary EXE/PAC/HITS bytes committed.
