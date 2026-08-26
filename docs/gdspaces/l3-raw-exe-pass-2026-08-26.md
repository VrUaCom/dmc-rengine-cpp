# GDSpaces L3 — second raw-EXE pass — 2026-08-26

**Pass class:** L3 static boundary refinement; not a global numbered reverse pass.  
**Canonical target:** `dmc3.exe`  
**Size:** `6,356,432` bytes  
**SHA-256:** `e454272ed0fb0247fcbcf300e5d55d7a3e96d50b89b9ffaff81bb978dcbdd082`  
**Purpose:** refine remaining Layer-3 boundaries after raw canonical EXE reacquisition without reopening already recovered topology/state semantics.

## 1. Acquisition writer `0x1401B84E0` — exact lower-L3 ordering

Direct disassembly establishes the bounded ordering:

```text
record +0x18 <- descriptor/type authority
 -> establish/prepare record +0x28 backing
 -> record +0x20 <- loaded/materialized payload handle/pointer
 -> call 0x1401B8CA0 materialization dispatcher
 -> only on dispatcher success: record +0x04 <- state 1
 -> schedule completion callback 0x1401B8DC0
 -> preserve optional callback/context at record +0x10 where applicable
```

### Promotion

- state `1` is not published before the acquisition/materialization path succeeds;
- L3 lifecycle ownership therefore starts at the success/result boundary above already-selected/materialized representation work, not at raw archive read/decompression itself.

### Still bounded

Exact public semantic names for every descriptor/backing subfield remain evidence-gated.

## 2. `0x1401B8CA0` is the L1/L3 seam

This helper selects/executes representation/materialization paths and returns a boolean result to acquisition.

Canonical classification:

```text
inside 0x1401B8CA0:
    representation/materialization mechanics -> L1 evidence
return success/failure:
    controls publication of LoadedResource state 1 -> L3 boundary
```

Do not classify the entire function by one layer merely because it is called from the L3 acquisition writer.

## 3. Cancellation writer `0x1401B8430` — source-state domain closed

The function walks all 363 records as 121 unrolled groups of three records (`3 * 0x48 = 0xD8` per loop step).

For each state it applies the equivalent of:

```text
if state == 1 or state == 2:
    state = 4
else:
    unchanged
```

It then schedules deferred cleanup callback `0x1401B8F00`.

### Promotion

For this canonical global cancellation writer, the complete bounded source-state domain entering state 4 is:

```text
1 -> 4
2 -> 4
```

States `0`, `3` and `4` are not relabeled state4 by this path.

## 4. Quiescence predicate `0x1401B84B0`

The function scans all 363 records and returns ready/quiescent only when every record is in:

```text
{0, 3}
```

Any state `1`, `2` or `4` makes the predicate false.

Direct static caller census at this pass finds eight direct call sites. The important promoted property is the predicate itself, not semantic naming of every caller.

### Architecture consequence

Cancellation/replacement barriers wait for absence of in-progress/materialized-pending/cancel-pending records; ready state3 records do not block quiescence.

## 5. Completion callback `0x1401B8DC0`

Normal callback behavior recovers the target record from callback context and publishes:

```text
state 1 -> state 2
```

An unusual low-bit-tag/sentinel branch is also present. It is not assigned a gameplay/error meaning in this pass because caller evidence does not yet justify one.

### Promotion

Normal `1 -> 2` completion writer remains strong.

### Freeze

Do not invent semantics for the tagged/sentinel edge.

## 6. State2 finalizer `0x1401B92D0` — exact ready ordering

The function scans all 363 records. For every record in state2:

1. obtain its loaded payload;
2. if the payload is PAC, iterate non-zero child offsets and invoke typed dispatcher `0x1401B9FA0` on children;
3. otherwise invoke `0x1401B9FA0` on the root payload;
4. invoke optional callback from record `+0x10` if present;
5. write record state `3`.

Exact bounded ordering:

```text
typed post-load
 -> optional ready callback
 -> state 3
```

The direct static caller surface is narrow: two direct call sites in the canonical image. This centralization contrasts with the broad fan-out of ordinary owner release.

## 7. Typed dispatcher `0x1401B9FA0` — central default/failure question narrowed

Direct disassembly establishes:

- null input returns immediately;
- `MOD` dispatches to `0x1402FE3B0`;
- `EFM` dispatches to `0x1402F7A90`;
- `SCM` dispatches to `0x1403051B0`;
- `SHW` dispatches to `0x1403204C0`;
- `PNST` recursively visits non-zero child offsets;
- unrecognized/unknown magic falls through and returns without a promoted error result.

The dispatcher is effectively best-effort/void at this boundary. `0x1401B92D0` does not consume a dispatcher success/failure flag before the optional callback and state3 publication.

### Corrected finding

The older open question:

> Does unknown/default post-load in the central dispatcher leave the record in state2 or move it to cancellation?

is **rejected for this central path**. Unknown/default dispatch itself is a no-op/return and does not block the finalizer from reaching state3.

### Still open

- family-specific failures inside called helpers;
- factory/dependency failures outside `0x1401B9FA0`;
- one odd mechanically observed EF-family comparison/exclusion branch whose semantic role is not promoted;
- SCM `mesh +0x28` contradiction.

## 8. Three distinct state-zero/release policies

### 8.1 Ordinary owner release — `0x1401B9530`

```text
call runtime backing release 0x140337710(record+0x28)
if success:
    state = 0
else:
    preserve current state
```

Direct static caller census: 146 call sites.

### 8.2 Deferred cancellation cleanup — `0x1401B8F00`

For state4 records:

```text
state = 0
 -> call runtime backing release(record+0x28)
```

The state-zero write precedes backing release and the release result is not used to restore the prior state.

### 8.3 Group/full forced reset — `0x1401B9560` / `0x1401B95E0`

For each affected record:

```text
call runtime backing release(record+0x28)
 -> state = 0 unconditionally
```

Group reset uses the group count/base tables; full reset covers all 363 records.

Direct caller census in this pass:

- group reset: 5 direct sites;
- full reset: 2 direct sites.

### Architecture consequence

Do not reconstruct one universal original `ReleaseResource()` semantic. Ordinary release, cancellation cleanup and forced reset deliberately have different ordering/result policy.

## 9. Group-5 dynamic pool `0x1401B8DF0` — capacity invariant

The function:

1. reads group-5 base/count;
2. scans for the first state0 record;
3. writes family-specific selector/index metadata at `+0x08`;
4. calls generic acquisition `0x1401B84E0`;
5. on acquisition failure, invokes backing release and conditionally restores state0 according to that branch.

### New error-path finding

No graceful no-free-record path is recovered before the selected-record write. If the scan exhausts the group, the routine does not show a safe bounded null/no-capacity result at that point.

### Promotion

Group-5 capacity is treated as a hard original runtime invariant by this routine.

### Product rule

This is **not** a safe-product policy. GDSpaces/reconstructed runtime should fail closed or report capacity exhaustion rather than reproduce an unsafe original invariant.

## 10. Registry initialization `0x1401B8380`

Exact bounded ordering across the 363-record array:

```text
record state <- 0
 -> initialize record+0x28 backing baseline via 0x1403376F0
 -> later group-assignment pass writes record+0x00 from group topology tables
```

Backing baseline helper `0x1403376F0`, relative to the `+0x28` subobject, initializes the observed fields:

```text
+0x08 <- 0
+0x10 <- 0
+0x18 <- -1
+0x1C <- 0
```

These offsets are promoted mechanically; stronger semantic field names remain bounded.

## 11. Runtime backing release `0x140337710`

This lower-level backing primitive is shared broadly and therefore is not automatically "L3" outside a LoadedResource lifecycle caller.

Bounded behavior:

- if backing owner/pool pointer at subobject `+0x08` is absent, normalize empty fields and return false/zero;
- if present, release/normalize the owned block, decrement observed owner/pool usage state, invoke lower cleanup, reset the backing fields and return true/non-zero.

Direct static caller census: 178 call sites.

### Boundary consequence

L3 classification depends on caller/record/state contract. A lower generic allocator/backing function cannot be assigned wholesale to L3 by address.

## 12. CRT backing destructor `0x140337440`

This function is distinct from runtime release `0x140337710`.

Record destructor `0x1401B79D0` advances to `record +0x28` and enters this backing destructor. The global manager/static destruction path therefore does not equal ordinary runtime release/reset.

This reconfirms the three lifetime layers:

1. runtime owner/reset lifetime;
2. CRT/static object destruction;
3. process-lifetime infrastructure with no recovered normal explicit teardown.

## 13. Call-graph shape of L3

Representative direct-call counts from the canonical image:

| Function | Role | Direct callers |
|---|---|---:|
| `0x1401B8430` | global cancel marker | 2 |
| `0x1401B84B0` | quiescence predicate | 8 |
| `0x1401B84E0` | generic acquisition | 7 |
| `0x1401B92D0` | state2 finalizer | 2 |
| `0x1401B9530` | ordinary owner release | 146 |
| `0x1401B9560` | group reset | 5 |
| `0x1401B95E0` | full reset | 2 |
| `0x1401B9FA0` | typed dispatcher/recursion | 3 direct static sites |
| `0x1401AE220` | loader claim++ | 11 |
| `0x1401AF6A0` | loader claim-- | 6 |
| `0x1401AF6F0` | zero-claim sweep | 6 |
| `0x140337710` | generic runtime backing release | 178 |

### Promoted architectural conclusion

**Layer 3 is not a contiguous VA region of `dmc3.exe`.**

It is a semantic/lifetime contract spanning central registry functions and many owner call sites. Broad lower-level helpers must be classified by caller context; narrow central functions define state boundaries but do not contain every L3 owner.

## 14. Higher-level cancellation linkage

A higher-level loader transition path around the recovered cancellation callers:

- changes its own loader/transition state;
- calls global LoadedResource cancellation `0x1401B8430`;
- later consults global quiescence `0x1401B84B0` before progressing cleanup/replacement.

This directly connects the global `1|2 -> 4` writer and `{0,3}` quiescence predicate to higher-level transition coordination without collapsing those higher-level state variables into the LoadedResource state machine.

## 15. What this pass closes vs leaves open

### Promoted / bounded stronger

- exact pre-state1 acquisition ordering;
- explicit L1/L3 success seam at `0x1401B8CA0`;
- complete source-state domain of canonical `1|2 -> 4` writer;
- exact global quiescence predicate `{0,3}`;
- typed-postload -> callback -> state3 order;
- central dispatcher unknown/default no-op behavior;
- distinction among ordinary/cancel/reset state-zero policies;
- group-5 hard capacity invariant;
- `+0x28` runtime-release vs CRT-destructor distinction;
- non-contiguous semantic nature of the L3 EXE boundary.

### Still open

1. alias-aware whole-image census of every possible state writer, especially unusual/tagged paths;
2. family-complete ownership of all `+0x08/+0x18/+0x20/+0x28` uses and stable unknown fields;
3. external factory/dependency failures not represented by central best-effort dispatcher;
4. SCM `mesh +0x28` reconciliation;
5. shared-owner family breadth beyond bounded loader-node cases;
6. dynamic original-process V1–V7 receipts;
7. profile/build differences.

## Acceptance statement

This is a **static raw-EXE promotion pass**, not Layer-3 completion.

No dynamic transition timing, protected-process identity mapping, original-process consumption or Level-E receipt is claimed by static disassembly alone.
