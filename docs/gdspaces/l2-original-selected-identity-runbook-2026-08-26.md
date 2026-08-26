# GDSpaces Layer 2 — selected identity content-candidate runbook — 2026-08-26

**Primary gate:** L2 / issue #220 R3.  
**Merged candidate/binder tooling:** PR #221.  
**Active process-instance hardening:** issue #229 / PR #236.  
**Prerequisite after #236 promotion:** a **real** protected-process `dmc-rengine.gdspaces-l2-runtime-mapping.v2` packet reconstructed from real v2 process-window receipts belonging to one process instance.  
**Status:** v2 content/binding tooling under exact-head validation; trusted original-process selected-identity evidence **not yet acquired**.

## 1. Authority boundary

Two executable roles remain separate:

- canonical instruction reverse authority: `e454272ed0fb0247fcbcf300e5d55d7a3e96d50b89b9ffaff81bb978dcbdd082`, 6,356,432 bytes;
- protected distribution/original execution candidate: `81c7e61983564113b5105e931d9f185accc14e44ae147d27f720c2d50935c7d6`, 6,567,320 bytes.

Canonical analysis addresses are not direct protected-process breakpoint authority. The real R2B mapping packet is the bounded bridge.

Fresh canonical EXE review is recorded in `l2-exe-reconciliation-2026-08-26.md`.

## 2. Recovered clean-path policy

For the canonical direct-call surface of `OpenGameResource` (`0x14002FCA0`, RVA `0x2FCA0`), all three observed direct callers pass `flags = 1`.

Recovered candidate/provider order:

```text
archive:
  GDataX360.afs/<basename>
  GData.afs/<basename>
  Video/<basename>
  afs/sound/<basename>
  SAVEDATA/<basename>
  <basename>
physical:
  same six candidates in the same order
```

Within every archive candidate, numbered volumes are traversed in effective mounted precedence:

```text
DMC3-(N-1).nbz -> ... -> DMC3-1.nbz -> DMC3-0.nbz
```

where `N` is the first missing contiguous volume index.

Archive normalization uses `0x0E`; physical normalization uses `0x0C`.

### Important EXE-review correction

Archive `normalized lookup hit` is **not automatically equal to selected resource**.

Canonical instructions at `0x1403274BE..0x1403274CD` show:

1. `0x140328160` returns a matched archive entry;
2. `0x140328290` attempts wrapper/open construction;
3. if wrapper/open returns null, `0x140327430` exits through cleanup/null return;
4. it does not continue to a lower volume as if the hit had been a clean lookup miss.

Therefore the clean-path R3 v2 content contract covers only:

- `miss` = clean provider lookup miss;
- `selected` = successfully produced selected resource identity.

Any observed provider/backend/wrapper failure is unsupported by the clean-path contract and fails closed. Never encode it as `miss` merely to preserve expected precedence order.

## 3. Process-instance authority required by v2

`PID + module base + image path` is not sufficient long-lived Windows process identity because PID reuse is possible and a later process may theoretically receive the same module base.

The v2 evidence seam therefore binds one process instance with:

```text
protected executable SHA/size + image path
+ PID
+ Windows process creation FILETIME
+ module base
```

The creation `FILETIME` must be obtained through `GetProcessTimes` from the same opened process HANDLE used for executable-image identity and `ReadProcessMemory`.

Every R2B child receipt must agree on the exact PID + creation FILETIME + module base + image path. A zero creation time, v1 child, or any mismatch fails closed.

Carrying this identity in a candidate is still **not trusted origin**. The final trusted publisher must independently re-query the active process immediately around trusted capture and reject PID reuse / creation-time mismatch instead of trusting editable JSON.

## 4. Preflight required for a real run

Before observing selection:

1. verify the on-disk process executable is exact protected SHA/size;
2. capture v2 process windows from one exact protected process instance;
3. build a real R2B v2 mapping from those child receipts;
4. preserve PID + creation FILETIME + module base + image path from that mapping;
5. enumerate contiguous `DMC3-0..N-1.nbz` with first-gap stop;
6. retain the exact mounted NBZ files for later artifact binding;
7. retain the exact observer/instrumentation executable or package used for the run;
8. record observer id/version;
9. ensure capture can report `trace_complete` and `dropped_event_count`.

Do not use self-declared archive or observer hashes as promotion authority. The binder hashes the artifacts itself.

## 5. Observation content

At mapped `OpenGameResource` entry record without changing control flow:

- PID;
- Windows process creation FILETIME;
- actual module base;
- exact C-string request;
- observed flags;
- basename derived from the observed request.

The PID + creation FILETIME + module base must exactly match the R2B session.

For each actually executed provider operation record:

- `sequence_index`;
- lookup attempt index `0..11`;
- provider class;
- exact candidate;
- normalized provider key;
- archive volume index when archive;
- clean outcome `miss` or `selected`.

Trace integrity must be explicit:

```text
trace_complete = true
dropped_event_count = 0
```

Any overflow, dropped callback/event, truncated run, uncertain ordering, or process-instance mismatch invalidates the candidate.

## 6. Selected archive identity

An `archive hit` label is insufficient.

For an archive selection record the exact:

- volume index;
- central-directory/member pathname selected by original lookup;
- normalized provider key;
- terminal candidate/attempt identity.

Do not derive the member pathname from GDSpaces or from the request.

The exact `DMC3-N.nbz` SHA/size is not trusted from JSON; the later binder hashes the supplied archive artifact itself.

## 7. Selected physical identity

For a physical selection record a mounted-root-relative identity, never an absolute workstation path.

It must normalize under `0x0C` to the observed provider key. If actual final filesystem identity/casing cannot be observed, do not claim exact ResourceRef parity from candidate text alone.

## 8. C++ serializer -> explicit v2 candidate

The C++ serializer is a metadata/content producer, not a trusted runtime publisher. Under PR #236 it emits:

```text
dmc-rengine.gdspaces-l2-original-selection.v2
original-process-observation
process_creation_filetime = <non-zero uint64>
```

Those labels and fields describe versioned content; they are **not promotion authority**.

Before binding, normalize the content through the fail-closed adapter:

```text
python scripts/reverse/normalize_l2_original_selection_candidate.py \
  --input <selection-v2.json> \
  --output <selection-candidate-v2.json>
```

The adapter accepts only the exact v2 observation schema/evidence pair, requires a non-zero uint64 process creation FILETIME, rejects unknown evidence-surface fields, rejects raw `bytes_hex`, rejects predeclared trust/promotion fields, replaces overclaiming `proves`, and emits:

```text
dmc-rengine.gdspaces-l2-original-selection-candidate.v2
original-process-observation-candidate
promotion_eligible = false
trusted_capture_bound = false
```

A normalized candidate is still self-authored content, not proof of process origin.

Legacy v1 observation/candidate receipts may remain historical artifacts but must not be accepted by the final v2 promotion path.

## 9. Artifact-backed v2 candidate binder

Bind the normalized candidate to the actual R2B packet and artifact set:

```text
python scripts/reverse/verify_l2_original_selection_evidence.py \
  --mapping <real-r2b-v2-mapping.json> \
  --mapping-child <open-game-v2-child.json> \
  --mapping-child <type0-v2-child-a.json> \
  --mapping-child <type0-v2-child-b.json> \
  --selection <selection-candidate-v2.json> \
  --observer-artifact <exact-observer-build> \
  --archive-artifact 0=<exact-DMC3-0.nbz> \
  --archive-artifact 1=<exact-DMC3-1.nbz> \
  --output <bound-selection-candidate-v2.json>
```

Repeat `--archive-artifact INDEX=PATH` for **every** mounted contiguous numbered volume. For zero mounted archives, provide none.

The binder independently:

- accepts only `dmc-rengine.exe-process-window.v2` mapping children;
- rebuilds the R2B v2 mapping from those child receipts;
- requires exact semantic equality with the supplied mapping packet;
- hashes the exact mapping file;
- verifies exact same PID + process creation FILETIME + module base between mapping and selection candidate;
- verifies the protected executable/session authority required by the mapping verifier;
- hashes the observer artifact and requires equality with declared observer SHA;
- hashes every numbered NBZ and requires exact declared SHA/size;
- validates request/basename, archive topology and recovered order;
- rejects incomplete/lossy traces;
- rejects provider/backend failure encoded as a clean miss;
- rejects raw `bytes_hex` recursively;
- writes output with no-replace semantics.

Successful output is deliberately:

```text
schema = dmc-rengine.gdspaces-l2-original-selection-bound.v2
status = bound_candidate
promotion_eligible = false
trusted_capture_bound = false
```

It proves artifact/process-session consistency of the **candidate** within the supplied evidence chain. It still does not prove trusted original-process origin.

## 10. Trusted-capture gate still required

A separate non-forgeable trusted publisher/origin mechanism must bind the observation to the actual protected process without relying on an editable JSON boolean, PID, timestamp, or string supplied by the candidate.

At minimum the trusted publisher must:

1. open/hold the intended protected process;
2. independently query executable image identity and process creation FILETIME from that live handle;
3. compare the active PID + creation FILETIME + module base with the already-proven R2B session;
4. acquire/publish the resolver observation while that same process-instance authority remains valid;
5. re-check or otherwise transactionally guarantee the same process instance around publication;
6. fail closed on process exit/replacement, PID reuse, creation-time mismatch, module drift, lossy trace, or unverifiable origin.

Until that exists and is exercised:

- #220 remains OPEN;
- #229 remains OPEN;
- a `bound_candidate` is not original-process evidence;
- `compare_original_to_product(...)` is content-only and must not be used to claim original/product parity from a synthetic/self-authored candidate.

## 11. Product comparison after trusted promotion

Only after trusted original-process promotion may the selected identity be compared against a GDSpaces `RuntimeResolutionReport` for the same request and exact source topology.

A bounded candidate-content match means only:

```text
same request
+ same selected provider/volume
+ same exact archive member or physical resource identity
```

It does not prove global resolver equivalence.

## 12. Fail-closed conditions

Do not promote if any of these occurs:

- no valid real R2B v2 mapping;
- any mapping child uses a v1/unknown process-window schema;
- protected EXE SHA/size differs;
- PID differs;
- process creation FILETIME is zero, unavailable, or differs;
- module base/image identity differs;
- trusted publisher did not independently verify the active process instance;
- observer artifact is unavailable or its actual SHA differs;
- any mounted NBZ artifact is unavailable or its actual SHA/size differs;
- archive topology has a gap;
- trace is incomplete or dropped-event count is nonzero;
- prefix/provider/volume order is incomplete or reordered;
- provider/backend/wrapper failure is encoded as `miss`;
- selected archive member is inferred rather than observed;
- selected physical identity is an unresolved candidate/absolute private path;
- raw `bytes_hex` enters public evidence;
- a self-authored schema attempts to predeclare promotion/trust;
- synthetic fixtures are substituted for protected-process execution.

## 13. Remaining L2 work

Even after one trusted R3 receipt, full Layer 2 still requires:

1. real-retail `0x0E` collision census;
2. enough representative trusted selected-identity receipts for the claimed resolver scope;
3. contradiction/reconciliation audit;
4. final exact-head Windows + Ubuntu validation;
5. explicit final L2 promotion.

Layer 1 materialization and Layer 3 lifecycle remain separate acceptance programs.
