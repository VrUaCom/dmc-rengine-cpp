# GDSpaces L1 — Byte-Exactness Reverse Gap Pass

**Pass date:** 2026-08-27  
**Base:** `main@f886f27e62ec9a05b6829df7fd074981a06a4b49`  
**Canonical analysis executable:** `dmc3.exe`, 6,356,432 bytes, SHA-256 `e454272ed0fb0247fcbcf300e5d55d7a3e96d50b89b9ffaff81bb978dcbdd082`  
**Primary layer:** L1 — Resource Materialization  
**Supporting layers:** L2 only where final selected-byte identity is required; L3 only where ownership/scheduling explains whether an L1 byte write is terminal  
**Primary ledger:** #100

## 1. Verdict

L1 is **not exhaustively reversed**.

The current product acceptance path may be implementation-ready for a representative Level-E run, but that does not mean every original L1 byte-materialization behavior is instruction-level closed. Pass 55 correctly narrowed the completion question and moved LoadedResource state publication into L3. This pass therefore stops using `state1 -> state2` as the L1 endpoint.

Canonical cut for this audit:

```text
L2 selected provider/member identity
 -> L1 materialized-size authority
 -> L1 destination-capacity/allocation requirements
 -> L1 selected byte/span acquisition
 -> L1 STORE / InflateRead transform
 -> L1 exact caller-owned destination bytes
 -> L1 packed representation OR .lst synthesized representation
 -> L1 materialization result returned through 0x1401B8CA0
 ===== END L1 =====
 -> L3 state/scheduler/completion/typed-ready lifecycle
```

`0x1401B8CA0` remains the explicit L1/L3 seam. The unresolved dependency between lower transport completion and `0x1401B8DC0` is real, but its scheduler/state publication mechanism is primarily L3. It must not hide remaining byte-exactness questions inside L1 itself.

## 2. What is already strong and should not be restarted generically

The following L1 architecture is already direct-EXE / exact-artifact backed at bounded scope:

- ZIP central/member construction and normalized archive lookup;
- method 0 direct member path and raw-DEFLATE inflater path;
- `ZipEntryRead 0x140328F50` and `InflateRead 0x140328820`;
- compressed reset+replay seek and raw seek architecture;
- whole-file caller-owned destination transfer family `0x1400333F0 / 0x1400333C0 / 0x140033500 / 0x1400335A0` at the already-recorded structural level;
- PAC/PNST relative-slot physical envelope, sparse slot identity and recursive container traversal;
- `.lst` packed-first fallback, grammar, `dummy`, `#XXXX`, 64-byte synthesized placement and nested packed-first recursion;
- `0x1401B8CA0` as direct/packed/loose materialization dispatcher and L1/L3 seam.

This pass does **not** reopen those conclusions without contradictory bytes.

## 3. Newly explicit L1 reverse gaps

### G1 — Rounded transfer request versus exact logical byte extent

Canonical whole-file evidence says:

```text
chunkCount = ceil(totalBytes / 0x800)
requestedBytes = chunkCount << 11
```

That can make `requestedBytes > totalBytes` for non-0x800-aligned resources.

Open instruction-level questions:

- where the lower backend clamps the final request to logical EOF/materialized size;
- whether the clamp is guaranteed for both physical and ZIP-backed FileSlots;
- whether a no-error short read is terminal success, retry/progress, or failure;
- whether `loadedBytes` must equal `totalBytes`, merely reach/exceed it, or is interpreted through another terminal status;
- whether the destination capacity is rounded to cover the transfer request or only the logical resource size;
- whether bytes in the rounded tail may be written, left untouched or are impossible to reach because the backend clamps first.

This is a direct L1 correctness question: exact materialized bytes and write bounds cannot be called exhaustively reversed until the final rounded chunk is closed.

### G2 — Materialized-size authority and zero-size semantics

Known whole-file path:

```text
0x1400333F0
 -> OpenGameResource
 -> FileSlot
 -> 0x14002F9F0
 -> 0x140048E20
 -> totalBytes
```

The current general blocked-window plan does not contain dedicated reacquisition windows for `0x14002F9F0` or `0x140048E20`.

Open questions:

- exact physical-file size return semantics;
- exact ZIP member size selected for STORE and compressed members;
- whether compressed archive size or logical uncompressed size can ever leak upward at this seam;
- signed/unsigned width and error sentinel domain;
- valid zero-length resource behavior;
- distinction between `missing`, `size query failure`, `zero-size valid resource`, and `.lst` representation checks that explicitly require positive size.

The current evidence strongly suggests logical/materialized size, but the exact helper ABI/error/zero domain is not yet an exhaustive L1 proof.

### G3 — Size planning, integer width and allocation capacity

`0x1401B7B90` computes required backing size. Direct resources are rounded to 64 bytes. Container TypeInfo may use the `.lst` required-size path.

The current general blocked-window plan has no dedicated `0x1401B7B90` window.

Open questions:

- exact integer width of every intermediate size/align operation;
- overflow/wrap behavior near alignment boundaries;
- zero-size direct resource allocation behavior;
- whether the rounded backing capacity is initialized, partially initialized or left allocator-defined before materialization;
- exact relation between required logical bytes, rounded capacity and any 0x800 transfer granularity;
- exact allocation-failure return propagation back to `0x1401B8CA0`/caller.

This is L1 because destination capacity and initial byte state can affect write safety and synthesized-byte determinism.

### G4 — `.lst` planner/materializer equivalence

`0x1401B7FD0` plans the synthesized container size and `0x1401B85C0` writes the representation in-place.

The materializer already has a general acquisition window, but the size planner does not.

Required closure:

- prove both functions use the same child-size source and packed-sibling decision on every branch;
- exact missing/zero-size ordinary-child behavior;
- exact nested `.lst` recursive size propagation;
- integer/align overflow behavior;
- whether header/table is committed before all child submissions succeed;
- first-failure versus continue-after-failure behavior;
- return value propagation from ordinary child `0x1402EF4D0` calls;
- partial parent byte-image state on failure;
- exact temporary-list-buffer release on every exit.

The grammar/layout can remain HIGH while these byte-production/error questions stay open.

### G5 — Synthesized padding byte contents

The synthesized *placement* rule is known: header and child starts are 64-byte aligned. That does not, by itself, prove the original contents of all alignment gaps.

Current product reconstruction can intentionally zero padding, but exhaustive original equivalence requires the direct allocation/write evidence that explains whether original gap bytes are:

- zeroed by backing allocation;
- explicitly memset/cleared by the synthesizer;
- copied from a prebuilt aligned header buffer;
- or otherwise unspecified/unwritten.

Do not promote product zero-fill policy to original byte-equivalence without closing this writer/allocation path.

### G6 — Exact direct-materialization ingress behind `0x1402EF4D0`

Safe current label remains `resource materialization submission/scheduling wrapper`.

Still open:

- exact queued job type/body;
- whether its byte-producing job reaches the known `0x1400333F0` whole-file path, another FileSlot opener, or multiple modes;
- exact consumer of inherited materialization/load-context parameter;
- whether full ResourceTypeInfo / `.lst` child paths are transformed before OpenGameResource basename policy;
- exact failure return contract before L3 state publication.

Lookup/provider selection inside this chain can be L2, but the exact byte-producing ingress and its success contract are required to close L1 materialization.

### G7 — Partial read / transform terminal semantics

ZIP internal readers are strong, but the composition into the outer whole-file loader is not exhaustively closed for all terminal cases.

Questions that remain L1-relevant:

- direct STORE short read versus EOF semantics when caller asks beyond logical end;
- InflateRead returning fewer produced bytes than requested without `-1`;
- terminal stream-end before the outer logical target is reached;
- whether outer transport treats partial produced bytes as progress and resubmits;
- exact no-progress handling;
- whether malformed/truncated compressed data can expose a partially written destination as successful materialization.

Product hardening may be stricter than original behavior; the two policies must remain distinct.

### G8 — Generic packed child extent is not original intrinsic child length

This is not a missing parser fact; it is a hard reverse boundary that must remain visible in L1 authoring.

The runtime proves relative slot starts, not a universal packed child-size field. Current `next-greater-distinct-offset` extraction extent is a product policy and can include packed padding. Therefore original L1 reverse is not complete enough to claim a universal size-changing retail PAC/PNST packer model.

Two safe scopes remain distinct:

1. layout-preserving byte patch against original packed bytes;
2. runtime-synthesized relative-slot image using `.lst` layout evidence when exact intrinsic child bytes are independently known.

## 4. Acquisition-plan audit result

Current main's general EXE packet plan already targets the Pass-55 completion bridge, `0x1401B85C0`, `0x1401B8CA0`, FileSlot submission and ZIP anchors.

However, the plan currently has **no dedicated windows** for these byte-exactness anchors:

- `0x14002F9F0` — FileSlot/resource size wrapper;
- `0x140048E20` — lower size helper;
- `0x1401B79E0` — packed-vs-list positive-size/representation checks;
- `0x1401B7B90` — direct/container required-size planner;
- `0x1401B7FD0` — `.lst` synthesized required-size planner;
- allocation/backing context needed to distinguish logical size, capacity and initialization policy.

This omission is a concrete reason not to call the L1 EXE reverse exhaustive.

A focused packet is added in `data/reverse/dmc3-l1-byte-exactness-gap-plan.v1.json` rather than broadening unrelated L3/ZIP windows.

## 5. Focused raw pass order

When exact `e454...` bytes are available, use this order:

1. `0x14002F9F0` + `0x140048E20`: close logical/materialized size and error/zero domain.
2. `0x140033390..0x1400335A0`: close `333C0/333E0/333500/3335A0` final-chunk, short-read, progress and terminal success semantics.
3. lower FileSlot/backend read clamp around the actual physical/ZIP read dispatch used by this whole-file path.
4. `0x1401B7B90`: close rounded backing capacity, integer width and allocation-failure propagation.
5. allocation/backing helper callees reached by `0x1401B84E0`: determine initialization/zero/padding state only from direct call flow.
6. `0x1401B79E0` + `0x1401B7FD0`: close representation positive-size tests and `.lst` size planning.
7. `0x1401B85C0`: prove planner/writer equivalence, child return/failure propagation and alignment-gap contents.
8. `0x1402EF4D0`: connect the byte-producing job to the known whole-file/FileSlot path and close inherited load-context consumption.
9. Only after direct-byte success semantics are exact, reconcile the L3 completion dependency bridge from Pass 55.

This ordering deliberately prevents L3 scheduler work from displacing unresolved L1 byte semantics.

## 6. Acceptance and completion consequence

This pass does **not** invalidate the current product authoring/Level-E readiness claim at its bounded supported scope.

It changes the reverse-completeness statement:

- `L1 product path implementation-ready for representative acceptance` may remain true.
- `L1 original EXE materialization exhaustively reversed` is false today.
- `L1 COMPLETE` still requires the real-retail / original-game receipts defined by the canonical acceptance program.

Static reverse completion and product acceptance are related but not identical gates.

## 7. Non-claims

This pass does not claim:

- fresh raw disassembly of the new windows;
- a newly discovered allocator ABI;
- that original `.lst` padding is nonzero or zero;
- that zero-byte resources are valid or invalid globally;
- that `0x1402EF4D0` is OpenGameResource;
- that the Pass-55 L3 completion bridge is already solved;
- that generic packed child extents are intrinsic file sizes;
- any L1/L2/L3 COMPLETE status.

**Status:** gap hunt bounded-complete. It identifies a distinct L1 byte-exactness frontier that must be reacquired before an exhaustive original-materialization claim is allowed.