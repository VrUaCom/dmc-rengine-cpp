# GDSpaces Layer 2 — original-process selected identity runbook — 2026-08-26

**Primary gate:** L2 / issue #220 R3.  
**Tooling PR:** #221.  
**Prerequisite:** a **real** protected-process `dmc-rengine.gdspaces-l2-runtime-mapping.v1` packet from #220 R2B.  
**Status:** tooling/contract in review; real original-process receipt **not yet acquired**.

## Authority boundary

Two executable roles remain separate:

- canonical instruction reverse authority: `e454272ed0fb0247fcbcf300e5d55d7a3e96d50b89b9ffaff81bb978dcbdd082`, size `6,356,432`;
- protected distribution / original execution candidate: `81c7e61983564113b5105e931d9f185accc14e44ae147d27f720c2d50935c7d6`, size `6,567,320`.

R3 runs only against the protected execution candidate. Canonical analysis VAs/RVAs are not direct protected-process breakpoint authority. The real R2B mapping packet is the prerequisite bridge.

## Recovered policy that R3 must observe, not assume

For the canonical direct-call surface of `OpenGameResource` (`0x14002FCA0` in the analysis build), all recovered direct callers use `flags = 1`.

That mode is:

```text
request
 -> basename
 -> archive candidate 0: GDataX360.afs/<basename>
 -> archive candidate 1: GData.afs/<basename>
 -> archive candidate 2: Video/<basename>
 -> archive candidate 3: afs/sound/<basename>
 -> archive candidate 4: SAVEDATA/<basename>
 -> archive candidate 5: <basename>
 -> physical candidate 0..5 in the same prefix order
 -> first selected resource or -1
```

For every archive candidate, mounted numbered volumes are consulted in recovered effective precedence:

```text
DMC3-(N-1).nbz -> ... -> DMC3-1.nbz -> DMC3-0.nbz
```

where `N` is the first missing contiguous volume index.

Archive normalization uses `0x0E`; physical normalization uses `0x0C`. The physical provider chain after `0x0C`, including `CreateFileA` flags/miss behavior, is already closed by #215/#204 and must not be re-reversed here.

## Required real acquisition chain

### 1. Preflight the exact process and corpus

Before instrumentation:

- verify on-disk process executable SHA/size is the protected authority above;
- enumerate the contiguous `DMC3-0..N-1.nbz` set with first-gap stop;
- hash every mounted NBZ and record exact size;
- preserve those identities in the R3 observation;
- validate the real R2B mapping packet and record its exact file SHA-256.

A trace without the exact NBZ artifact census cannot promote an archive winner identity.

### 2. Enter through mapped `OpenGameResource`

Use the R2B-mapped protected-process address corresponding to canonical `OpenGameResource` RVA `0x2FCA0`.

At entry record, without modifying control flow:

- PID and actual module base;
- request bytes interpreted as the original C string;
- `flags` value;
- derived basename only after observing the original branch inputs.

For the first R3 receipt, require `flags = 1`. Another flag mode remains a separate evidence slice.

### 3. Observe provider traversal

For every provider operation that actually executes, record an ordered probe:

- monotonically increasing trace `sequence_index`;
- recovered lookup-attempt index `0..11`;
- provider class (`archive` or `physical`);
- exact candidate before provider normalization;
- exact normalized provider key;
- archive volume index for archive probes;
- outcome `miss` or `selected`.

The receipt contract rejects:

- skipped earlier prefixes;
- skipped higher archive volumes;
- a physical probe before the archive phase is exhausted;
- a probe after the first selected result;
- a candidate/provider key inconsistent with the recovered normalizer profile.

### 4. Archive selected identity

A generic `archive hit` is insufficient.

For an archive winner, observe enough of the returned original archive lookup/stream identity to bind the selected result to:

- exact numbered volume index;
- that volume's SHA-256/size from preflight;
- exact physical central-directory/member pathname selected by the runtime lookup;
- normalized provider key used for the lookup.

Do **not** infer the member path from the request or GDSpaces. It must come from the original archive lookup/returned central-entry identity.

The recovered static model establishes normalized sorted index + `qsort`/`bsearch` architecture, but this runbook intentionally does not invent a protected-build helper address for that inner lookup before R2B/runtime acquisition identifies it.

### 5. Physical selected identity

If the original selects the physical provider, record a mounted-root-relative final resource identity. Do not publish an absolute workstation path in the public receipt.

The identity must normalize under `0x0C` to the observed physical provider key. If exact final filesystem casing cannot be observed safely, do not claim exact `ResourceRef` parity from candidate text alone; acquire the final opened-file identity first.

### 6. Terminate at first selected resource

The original selected identity is the terminal R3 event. Any later provider event in the same receipt invalidates the trace.

## Receipt pipeline

The in-process/tooling representation is:

```text
OriginalResolutionObservation
 -> valid()
 -> dmc-rengine.gdspaces-l2-original-selection.v1
```

Then bind it to the **actual mapping packet file**:

```text
python scripts/reverse/verify_l2_original_selection_evidence.py \
  --mapping <real-r2b-mapping.json> \
  --selection <real-original-selection.json> \
  --output <bound-selection-evidence.json>
```

The validator independently hashes the supplied mapping packet and requires:

- `dmc-rengine.gdspaces-l2-runtime-mapping.v1` / `bounded_match`;
- protected executable SHA/size;
- `OpenGameResource` plus at least two type-0 mapping anchors;
- identical PID and module base between mapping and selection receipts;
- exact mapping-file SHA equals the SHA recorded by the selection receipt;
- recovered provider/candidate/volume order;
- terminal selected identity consistency;
- metadata-only inputs with no `bytes_hex`.

Successful output schema:

```text
dmc-rengine.gdspaces-l2-original-selection-bound.v1
```

A successful validator run proves only a **mapping-bound selected identity for that exact request/process/corpus session**.

## Product comparison

`compare_original_to_product(...)` compares the real selected identity against one GDSpaces `RuntimeResolutionReport` using the exact `RuntimeSourceBindings` topology.

A `matched` result means only:

```text
same bounded request
+ same selected provider/volume source identity
+ same exact selected archive member or physical ResourceRef identity
```

It does not relabel product `RuntimeResolutionProbe` objects as original evidence and does not prove global resolver equivalence.

## Fail-closed / non-promotion conditions

Do not promote R3 if any of the following occurs:

- no valid real R2B mapping packet;
- executable SHA/size differs;
- mapping and selection PID/module base differ;
- observer build identity is absent;
- NBZ artifact census is incomplete or unhashed;
- candidate or provider order is incomplete;
- a higher archive volume is skipped;
- selected archive member identity is inferred rather than observed;
- selected physical identity is only an absolute/private path or unresolved candidate guess;
- instrumentation changes the branch/provider result being claimed;
- trace overflow/drop is possible without a detectable invalid-run marker;
- synthetic tests are substituted for original-process execution.

## What remains after a valid R3 receipt

A valid R3 receipt closes the protected original selected-identity gate for its bounded request only. Full L2 still requires:

1. cryptographically bound real-retail `0x0E` collision census;
2. enough representative original/process/product receipts to cover the declared L2 scope;
3. final contradiction audit and exact-head Windows + Ubuntu validation;
4. explicit final L2 promotion.

Layer 1 materialization and Layer 3 lifecycle remain separate acceptance programs.
