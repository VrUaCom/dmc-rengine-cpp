# HITS Pass 10 — Slice 16: Stage-CFG Transform-Source Provenance

**Date:** 2026-08-15  
**Canonical target SHA-256:** `e454272ed0fb0247fcbcf300e5d55d7a3e96d50b89b9ffaff81bb978dcbdd082`  
**Status:** ACTIVE REVERSE / EVIDENCE REACQUISITION REQUIRED

## Purpose

Slices 9, 13 and 14 prove that dynamic-collision entry byte `+0x01` is a **transform selector**, while transform ownership is separate from primitive-descriptor ownership.

What is already closed:

```text
entry +0x02 u16
  -> manager +0x110 descriptor table
  -> descriptor_index * 0x50
  -> C8D0 arg3
  -> runtime +0x118 primitive descriptor
```

and independently:

```text
transform source
  -> C8D0 stack arg5
  -> runtime +0x20 transform pointer
  -> CC530 runtime matrix construction
```

The remaining Stage-CFG question is exact provenance of the transform source indexed or selected by raw `entry+0x01`.

Do not infer the answer from PAC slot adjacency.

## Existing direct evidence

### Builder family

Slice 9 confirms two builder variants that resolve the same primitive descriptor but differ in transform-source resolution.

#### `0x14005C630..0x14005C731`

Body SHA-256:

`97dbb8f5e6cace93530a30c936796a35fca80235467a3a0885f71c8593990d1a`

Confirmed bounded behavior:

- reads raw selector from `entry+0x01`;
- resolves an optional transform indirectly through an external source selected by that byte;
- then uses the selected source object's `+0x110` pointer as the transform source supplied to C8D0.

Canonical reconstruction terminology: **indirect-transform builder**.

The external source identity, count/bounds and Stage-CFG relation are not closed by this statement alone.

#### `0x14005C740..0x14005C83D`

Body SHA-256:

`0778fa7ecce7855712b1d0bd5cb8ef5b32998e1d4629ee0d5d35e951318e06b6`

Confirmed bounded behavior:

- reads the same raw selector from `entry+0x01`;
- indexes an external/direct transform table with exact `0x40` stride;
- supplies the selected transform to C8D0.

Canonical reconstruction terminology: **inline/direct-table transform builder**.

This proves that a valid serialized collision source can use a `0x40` transform table. It does not prove that every Stage-CFG path uses this builder/table.

### Runtime bridge — `0x14005C8D0`

Body SHA-256:

`f779db92f9fee9d1492ef7208eb9950784d782e51542dd65d551ecdf6b950bfe`

Confirmed ABI:

- arg2 / RDX -> runtime `+0x110` entry/source pointer;
- arg3 / R8 -> runtime `+0x118` primitive descriptor pointer;
- arg4 / R9 -> runtime object;
- stack arg5 -> runtime `+0x20` transform pointer.

Therefore the exact producer of C8D0 stack arg5 is the decisive transform-provenance boundary.

### Stage-CFG serialized source

Slice 12 confirms the current Stage-CFG resource path and collision entry/descriptor tables:

- resource kind 1 -> `room\\stXXXcfg.pac`;
- modern observed: slot 39 entry table, slot 40 primitive descriptor table;
- legacy observed: slot 22 entry table, slot 23 primitive descriptor table.

Representative C260 setup callsites already preserved:

- modern Stage-CFG: `0x14009823F`;
- legacy observed Stage-CFG: `0x1400B6483`.

These callsites prove manager entry/descriptor source setup at their bounded scope. They do not by themselves identify the later transform provider.

### Stage-CFG slot 38 correction

A deeper Slice-13 review proves Stage-CFG slot 38 is consumed by `0x1400594B0` and begins with its own `u16` relative-offset structure.

Therefore:

**REJECT:** `Stage-CFG slot 38 == C740 0x40 transform table` from slot adjacency alone.

Current status:

- slot 38 = related collision/source block;
- exact semantic role and relation to transform generation/resolution = **RESEARCH REQUIRED**;
- slot 39/40 entry/descriptor interpretation remains independently valid.

## Evidence acquisition targets

The next reverse step must reacquire or materialize enough instruction/caller evidence to connect the Stage-CFG setup path to one of the transform builder contracts.

### Target A — `0x1400594B0`

Acquire:

- complete function body from prologue through return;
- all callers;
- RCX/RDX/R8/R9 and stack argument setup at each relevant caller;
- every read from the Stage-CFG slot-38 payload;
- every pointer/offset produced from the payload's `u16` relative-offset structure;
- every write to object/manager fields;
- any count, end pointer, sentinel or ownership/lifetime field associated with the produced objects;
- callees reached from the produced pointers.

Questions:

1. Does 594B0 construct a transform registry/table, an object registry, auxiliary collision metadata, or something else?
2. Does any output of 594B0 become the external source used by C630 or C740?
3. Is the `u16` relative-offset structure persistent serialized data or an intermediate construction format?
4. Which object owns the result and when is it released/reset?

No semantic name is promoted until these links are direct.

### Target B — modern Stage-CFG setup around `0x14009823F`

Acquire a bounded caller/callee window sufficient to trace:

```text
room\\stXXXcfg.pac
  -> slot 38 / slot 39 / slot 40 consumers
  -> C260 manager entry/descriptor setup
  -> manager/object population
  -> C630 or C740 builder call(s)
  -> C8D0 stack arg5 transform
```

Required observations:

- owning object/manager identity;
- exact slot-38 pointer destination;
- exact slot-39/40 pointer destination already related to C260;
- builder selected downstream (`C630`, `C740`, both, or another wrapper);
- source/base pointer used with `entry+0x01`;
- count/bounds for that source if present;
- lifetime/rebuild point.

### Target C — legacy Stage-CFG setup around `0x1400B6483`

Repeat the same bounded trace for the legacy observed `22/23` entry/descriptor generation.

Do not assume modern and legacy generations share the same transform provider merely because their entry ABI is the same.

### Target D — C630/C740 caller census and Stage-CFG classification

For every direct caller of `0x14005C630` and `0x14005C740` in the canonical build, record:

- caller VA;
- manager/context identity;
- source resource/object class where recoverable;
- whether the manager's `+0x108/+0x110` came from a Stage-CFG C260 setup path;
- transform-source argument/base;
- selector bounds/count source;
- whether C8D0 stack arg5 is null or non-null.

The goal is not a global gameplay name. The goal is to partition **which builder contract the Stage-CFG paths actually use**.

## Promotion gates

### Gate 1 — builder-route closure

Promote only when direct caller/dataflow evidence proves modern and/or legacy Stage-CFG path reaches C630, C740, or another exact transform-resolution route.

### Gate 2 — transform-source identity

Promote a transform provider only when the exact base/object/table reaching C8D0 stack arg5 is traced from an evidenced owner/source.

### Gate 3 — bounds/count

`Stage-CFG View::transform_selector_bounds_available()` must remain `false` until an exact count/bounds contract for the Stage-CFG provider is proven.

It is not enough to observe a selector value that happens to be small.

### Gate 4 — serialized adapter

A Stage-CFG three-span adapter may only be introduced if Stage-CFG transform bytes are independently identified as an exact bounded span with proven element stride/count.

Do not reuse the `em000.pac` slot-38 `0x40` rule by analogy.

### Gate 5 — lifecycle

If the transform source is constructed rather than directly serialized, preserve:

- creator/initializer;
- owner field;
- reuse/cache behavior;
- reset/unload behavior;
- consumer handoff.

This belongs in recovered runtime evidence, not GDSpaces resource ownership.

## Rejected shortcuts

- `slot 38` is adjacent to 39/40, therefore it is the transform table — **REJECTED**;
- matching PAC slot numbers imply matching schemas — **REJECTED by Slice 13 negative control**;
- raw `entry+0x01` is primitive type — **REJECTED**;
- C740 supports a `0x40` transform table, therefore Stage-CFG must use C740 — **NOT PROVEN**;
- C630/C740 builder naming implies gameplay semantics — **NOT PROVEN**;
- a synthetic transform-selector test proves real Stage-CFG bounds — **REJECTED**.

## Artifact/reacquisition boundary

Current connected project sources preserve body hashes and bounded findings for C630/C740/C8D0, but do not currently expose enough raw instruction/caller evidence to close the Stage-CFG transform provider or the complete body of `0x1400594B0`.

Do not fabricate the missing chain.

Acceptable next evidence inputs:

1. canonical EXE bytes or verified byte-identical derivative window for the target ranges;
2. sanitized disassembly windows with artifact/hash provenance;
3. exact-SHA observation-only runtime trace showing the Stage-CFG setup/builder/C8D0 pointer chain;
4. preserved private raw Stage-CFG data plus sanitized hashes/offset/count reports when data-side proof is applicable.

## Observation plan if runtime tracing is required

Minimum trace points:

- entry/exit of `0x1400594B0`;
- representative Stage-CFG C260 setup callsite path;
- C630 and C740 entry when reached from that owner/manager;
- C8D0 entry.

For each C8D0 event preserve:

- caller VA;
- manager/owner identity;
- entry pointer and raw four entry bytes;
- descriptor pointer/index/type;
- transform pointer stack arg5;
- source/base object or table that produced the transform pointer;
- selector value;
- provider count/bounds when directly available.

Raw proprietary payload bytes remain private; public evidence stores only hashes, offsets, scalar observations and bounded derived semantics.

## Implementation boundary

Slice 16 is a reverse/evidence slice first.

No production code change is justified yet for Stage-CFG transform resolution. In particular:

- keep `transform_selector_bounds_available() == false`;
- do not add a Stage-CFG slot-38 transform parser;
- do not add a three-table Stage-CFG view;
- do not move transform/runtime construction into GDSpaces.

Once the provider/count contract is closed, implement only the confirmed subset and repeat the standard review/debug/CI/Drive synchronization loop.

## Current success criterion

Slice 16 reaches `BOUNDED CLOSED` only when the Stage-CFG `entry+0x01` selector has a direct evidence-backed path to the exact transform provider consumed by C8D0, including sufficient bounds/count information to state whether selector validation is possible.

Until then: **RESEARCH REQUIRED**.
