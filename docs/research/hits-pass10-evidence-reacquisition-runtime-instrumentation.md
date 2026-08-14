# HITS Raw Reverse Pass 10 — Evidence Reacquisition and Runtime Instrumentation

Date: 2026-08-14  
Canonical executable SHA-256: `e454272ed0fb0247fcbcf300e5d55d7a3e96d50b89b9ffaff81bb978dcbdd082`  
Status: **ACTIVE**

Canonical detailed working authority: Google Drive document `DMC Rengine — HITS Raw Reverse Pass 10 — Evidence Reacquisition and Runtime Instrumentation — 2026-08-14`.

## Why Pass 10 exists

Pass 9 reached static ABI/ownership saturation for the currently materialized canonical evidence. Repeating the same summary/xref corpus no longer yields instruction-level fields for the remaining P0 functions. Pass 10 therefore changes method: reacquire missing instruction windows or gather hash-gated runtime traces without changing collision semantics.

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

If canonical instruction bytes cannot be materialized from project sources, use a guarded observation-only design. Conceptual event envelope:

```cpp
struct HitsQueryTraceEvent {
    uint64_t sequence;
    uint64_t caller_va;
    uint64_t query_va;
    int32_t selected_source;
    uint32_t reject_mask;
    uint32_t dynamic_category;
    uint32_t origin; // instrumentation-level label only
    uint8_t input_snapshot[64];
    uint8_t output_snapshot[64];
    uint8_t changed_byte_mask[64];
    float candidate_metric_before;
    float candidate_metric_after;
    uint32_t result_code;
};
```

The 64-byte snapshots are only a logging-envelope concept. They are **not** evidence that the original result ABI is 64 bytes.

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
