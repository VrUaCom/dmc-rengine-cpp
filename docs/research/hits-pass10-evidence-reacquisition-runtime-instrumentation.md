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
