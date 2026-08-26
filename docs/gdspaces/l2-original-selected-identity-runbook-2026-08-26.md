# GDSpaces Layer 2 — selected identity content-candidate runbook — 2026-08-26

**Primary gate:** L2 / issue #220 R3.  
**Tooling PR:** #221.  
**Prerequisite:** a **real** protected-process `dmc-rengine.gdspaces-l2-runtime-mapping.v1` packet reconstructed from real #219 child process-window receipts.  
**Status:** candidate tooling in review; trusted original-process selected-identity evidence **not yet acquired**.

## 1. Authority boundary

Two executable roles remain separate:

- canonical instruction reverse authority: `e454272ed0fb0247fcbcf300e5d55d7a3e96d50b89b9ffaff81bb978dcbdd082`, 6,356,432 bytes;
- protected distribution/original execution candidate: `81c7e61983564113b5105e931d9f185accc14e44ae147d27f720c2d50935c7d6`, 6,567,320 bytes.

Canonical analysis addresses are not direct protected-process breakpoint authority. The real R2B mapping packet is the bridge.

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

Therefore R3 v1 covers only the clean path:

- `miss` = clean provider lookup miss;
- `selected` = successfully produced selected resource identity.

Any observed provider/backend/wrapper failure is **unsupported by v1 and fails closed**. Never encode it as `miss` merely to preserve expected precedence order.

## 3. Preflight required for a real run

Before observing selection:

1. verify the on-disk process executable is exact protected SHA/size;
2. build a real R2B mapping from child process-window receipts;
3. enumerate contiguous `DMC3-0..N-1.nbz` with first-gap stop;
4. retain the exact mounted NBZ files for later artifact binding;
5. retain the exact observer/instrumentation executable or package used for the run;
6. record observer id/version;
7. ensure capture can report `trace_complete` and `dropped_event_count`.

Do not use self-declared archive or observer hashes as promotion authority. The binder hashes the artifacts itself.

## 4. Observation content

At mapped `OpenGameResource` entry record without changing control flow:

- PID;
- actual module base;
- exact C-string request;
- observed flags;
- basename derived from the observed request.

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

Any overflow, dropped callback/event, truncated run, or uncertain ordering invalidates the candidate.

## 5. Selected archive identity

An `archive hit` label is insufficient.

For an archive selection record the exact:

- volume index;
- central-directory/member pathname selected by original lookup;
- normalized provider key;
- terminal candidate/attempt identity.

Do not derive the member pathname from GDSpaces or from the request.

The exact `DMC3-N.nbz` SHA/size is not trusted from JSON; the later binder hashes the supplied archive artifact itself.

## 6. Selected physical identity

For a physical selection record a mounted-root-relative identity, never an absolute workstation path.

It must normalize under `0x0C` to the observed provider key. If actual final filesystem identity/casing cannot be observed, do not claim exact ResourceRef parity from candidate text alone.

## 7. Legacy C++ serializer -> explicit candidate

The existing C++ serializer predates the trusted-origin review correction and emits the historical labels:

```text
dmc-rengine.gdspaces-l2-original-selection.v1
original-process-observation
```

Those labels are **legacy content labels, not promotion authority**.

Before binding, normalize them through the fail-closed adapter:

```text
python scripts/reverse/normalize_l2_original_selection_candidate.py \
  --input <legacy-selection.json> \
  --output <selection-candidate.json>
```

The adapter accepts only the exact legacy schema/evidence pair, rejects raw `bytes_hex`, rejects predeclared trust/promotion fields, replaces overclaiming `proves`, and emits:

```text
dmc-rengine.gdspaces-l2-original-selection-candidate.v1
original-process-observation-candidate
promotion_eligible = false
trusted_capture_bound = false
```

A normalized candidate is still self-authored content, not proof of process origin.

## 8. Artifact-backed candidate binder

Bind the normalized candidate to the actual R2B and artifact set:

```text
python scripts/reverse/verify_l2_original_selection_evidence.py \
  --mapping <real-r2b-mapping.json> \
  --mapping-child <open-game-child.json> \
  --mapping-child <type0-child-a.json> \
  --mapping-child <type0-child-b.json> \
  --selection <selection-candidate.json> \
  --observer-artifact <exact-observer-build> \
  --archive-artifact 0=<exact-DMC3-0.nbz> \
  --archive-artifact 1=<exact-DMC3-1.nbz> \
  --output <bound-selection-candidate.json>
```

Repeat `--archive-artifact INDEX=PATH` for **every** mounted contiguous numbered volume. For zero mounted archives, provide none.

The binder independently:

- rebuilds the R2B mapping from child process-window receipts;
- requires exact semantic equality with the supplied mapping packet;
- hashes the exact mapping file;
- verifies same PID/module base;
- hashes the observer artifact and requires equality with declared observer SHA;
- hashes every numbered NBZ and requires exact declared SHA/size;
- validates request/basename, archive topology and recovered order;
- rejects incomplete/lossy traces;
- rejects provider/backend failure under clean-path v1;
- rejects raw `bytes_hex` recursively;
- writes output with no-replace semantics.

Successful output is deliberately:

```text
schema = dmc-rengine.gdspaces-l2-original-selection-bound.v1
status = bound_candidate
promotion_eligible = false
trusted_capture_bound = false
```

It proves artifact/provenance consistency of the **candidate** only. It still does not prove trusted original-process origin.

## 9. Trusted-capture gate still required

A separate trusted publisher/origin mechanism must bind the observation to the actual protected process without relying on an editable JSON boolean or string.

Until that exists and is exercised:

- #220 remains OPEN;
- a `bound_candidate` is not original-process evidence;
- `compare_original_to_product(...)` must not be used to claim original/product parity from a synthetic/self-authored candidate.

## 10. Product comparison after trusted promotion

Only after trusted original-process promotion may the selected identity be compared against a GDSpaces `RuntimeResolutionReport` for the same request and exact source topology.

A bounded `matched` result means only:

```text
same request
+ same selected provider/volume
+ same exact archive member or physical resource identity
```

It does not prove global resolver equivalence.

## 11. Fail-closed conditions

Do not promote if any of these occurs:

- no valid real R2B mapping;
- protected EXE SHA/size differs;
- PID/module base mismatch;
- observer artifact is unavailable or its actual SHA differs;
- any mounted NBZ artifact is unavailable or its actual SHA/size differs;
- archive topology has a gap;
- trace is incomplete or dropped-event count is nonzero;
- prefix/provider/volume order is incomplete or reordered;
- provider/backend/wrapper failure is encoded as `miss`;
- selected archive member is inferred rather than observed;
- selected physical identity is an unresolved candidate/absolute private path;
- raw `bytes_hex` enters public evidence;
- a legacy/self-authored schema attempts to predeclare promotion/trust;
- synthetic fixtures are substituted for protected-process execution.

## 12. Remaining L2 work

Even after one trusted R3 receipt, full Layer 2 still requires:

1. real-retail `0x0E` collision census;
2. enough representative trusted selected-identity receipts for the claimed resolver scope;
3. contradiction/reconciliation audit;
4. final exact-head Windows + Ubuntu validation;
5. explicit final L2 promotion.

Layer 1 materialization and Layer 3 lifecycle remain separate acceptance programs.
