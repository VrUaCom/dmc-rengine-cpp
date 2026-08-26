# GDSpaces EXE Grey-Boundary — Raw Pass 5: Failure, Recovery, and Original Invariants

**Date:** 2026-08-26  
**Tracking:** #225 / PR #226  
**Extends:** raw Pass 3 + Pass 4  
**Canonical artifact:** `dmc3.exe`, 6,356,432 bytes, SHA-256 `e454272ed0fb0247fcbcf300e5d55d7a3e96d50b89b9ffaff81bb978dcbdd082`.

## Purpose

Close the most important static failure/recovery questions exposed by P2-R5/P2-R6:

1. how acquisition failure is propagated above the owner-node loader;
2. whether `plan finished` means `plan successful`;
3. what higher scene/stage control does after a failed plan;
4. whether the 32-node owner pool has a safe exhaustion path;
5. whether owner claims are arithmetically guarded.

This remains static raw-EXE evidence. It is not an original-process timing receipt.

---

# 1. Owner `+0x648` is a plan-level failure latch

Within the recovered owner/resource-control subsystem, byte field:

```text
owner +0x648
```

is explicitly cleared at the beginning of both major plan-start paths:

```text
0x1401AE5B0 -> owner+0x648 = 0
0x1401AEDE0 -> owner+0x648 = 0
```

and set to `1` by multiple plan failures.

## 1.1 Generic `(kind,id)` acquisition failure

`0x1401AE310` selects/allocates an owner node and calls `0x1401B8DF0` for the underlying `LoadedResource`.

If `0x1401B8DF0` returns null:

```text
owner +0x648 = 1
return null
```

This covers stage kinds 0/1/2, demo/message kinds 3/4 and generic kind5/kind6 dispatch through this function.

## 1.2 Paired enemy dependency failure

`0x1401AE8F0` independently sets the same owner latch when one of its direct kind5/kind6 underlying acquisitions fails.

The function has several specialized selector branches, but the common bounded rule remains:

```text
underlying dependency acquisition failed
 -> owner+0x648 = 1
```

## 1.3 The latch is broader than I/O/acquisition failure

Stage control state 3 contains a special current-stage condition around stage id `0x1A5` and helper `0x1401DF7B0`. If that condition fails, the same latch is set:

```text
owner +0x648 = 1
```

Therefore `+0x648` must **not** be named `io_failed` or `resource_open_failed`.

Evidence-safe interpretation:

**owner/resource-plan failure latch**.

It can be raised by underlying acquisition failure and by at least one higher plan-specific condition.

---

# 2. Plan completion and plan success are separate authorities

Function:

```text
0x1401AE210
```

returns true only when:

```text
owner +0x638 == 0
```

It does not inspect `+0x648`.

So this predicate means the outer resource-plan control machine is no longer active; it does **not** prove success.

## 2.1 StageCfg acquisition failure demonstrates the split directly

StageCfg control state 1 in `0x1401AF000` calls:

```text
0x1401AE310(kind=1, current_stage_id)
```

If that returns null, control jumps to `0x1401AF184`:

```text
inner state +0x63C = 0
return true
```

The acquisition helper has already raised `owner+0x648 = 1`.

The outer update routine around `0x1401AEEA0` consumes the inner completion return and clears outer plan state `+0x638` to zero.

Therefore the exact static state can become:

```text
plan_running == false
plan_failure_latch == true
```

This is direct proof that:

```text
plan_finished != plan_success
```

## Validation consequence

Future validation must never promote a run from a `finished`/idle control state alone. A success verdict must bind the independent error/failure authority and the required downstream semantic/consumer evidence.

---

# 3. Two higher scene/control paths explicitly branch on `finished + failed`

Two independent current-raw higher control sites read the same owner failure latch after plan completion.

## 3.1 Control path around `0x140238B1E`

Bounded order:

```text
call 0x1401AE210       // plan inactive/finished?
if not finished: wait
if owner+0x648 == 0: continue normal path
if owner+0x648 != 0:
    call 0x1401AF1C0   // clear current-plan activity markers
    call 0x1401AF6F0   // release inactive + unclaimed owner nodes
    call 0x1401AE5B0   // restart/rebuild the same plan-family with target id
```

The failure branch also resets a bounded set of adjacent higher subsystem objects after restarting the resource plan.

This path corresponds to the other/demo-event style resource-plan family rather than the StageCfg state machine.

## 3.2 Gameplay/Stage control path around `0x14023C421`

This path adds one more gate before inspecting plan failure:

```text
0x1401AE210   // plan inactive
 -> 0x1401B84B0   // all 363 LoadedResource records quiescent in {0,3}
 -> inspect owner+0x648
```

On failure:

```text
0x1401AF1C0   // activity clear
 -> 0x1401AF6F0   // release inactive + unclaimed owner nodes
 -> 0x1401AEDE0   // restart/reinitialize the Stage plan with current target id
```

`0x1401AEDE0` begins by clearing the failure latch and reinitializing the Stage plan state machine (`+0x640 = 1`, `+0x63C = 1` on the fresh-start path).

### Promotion

There is direct current-raw recovery orchestration:

```text
plan reaches inactive state
 -> separate failure latch checked
 -> cleanup stale plan membership/resources
 -> restart the appropriate resource plan
```

The gameplay/Stage variant additionally requires global LoadedResource quiescence before this recovery branch.

This is a concrete RCP recovery contract, not merely a local null-return convention.

---

# 4. Deferred replacement uses cancellation + quiescence before plan restart

Both plan-start functions also handle replacement while another plan is active.

## 4.1 Stage plan start `0x1401AEDE0`

If owner outer plan state `+0x638` is nonzero:

```text
outer plan state = 4
 -> call global LoadedResource cancellation 0x1401B8430
 -> remember deferred mode id in +0x658
 -> remember target id in +0x65C
```

## 4.2 Other plan start `0x1401AE5B0`

The parallel path performs the same pattern but records the other deferred mode value in `+0x658`.

## 4.3 Outer updater `0x1401AEEA0`

For the replacement/cancel state it waits on:

```text
0x1401B84B0
```

until the global 363-record manager is quiescent. It then clears the old owner-list state and dispatches the remembered replacement mode:

```text
+0x658 == 1 -> tail-call 0x1401AEDE0(target +0x65C)
+0x658 == 2 -> tail-call 0x1401AE5B0(target +0x65C)
```

### Consequence

The same RCP has two distinct recovery/replacement mechanisms:

1. **completed-but-failed plan:** higher scene checks `+0x648`, clears/sweeps, then restarts;
2. **replacement while plan active:** mark global unfinished resources cancelled, wait for quiescence, then dispatch deferred replacement mode.

Do not collapse these into one `retry()` semantic.

---

# 5. Owner-node pool capacity 32 is a hard original invariant

The generic owner-node loader `0x1401AE310` scans exactly 32 inline nodes for a free slot when no matching `(kind,id)` node exists.

If all 32 nodes are occupied, the selected node pointer remains null.

The function nevertheless proceeds into descriptor selection / `0x1401B8DF0`, and after that call executes the equivalent of:

```text
selected_node->underlying_resource = acquisition_result
```

There is no recovered safe `no_capacity` return before this dereference.

The paired enemy dependency loader `0x1401AE8F0` contains the same hard-capacity shape for its direct node allocation paths.

### Promotion

For this original owner-node subsystem:

```text
capacity = 32
free node availability is an assumed runtime invariant
```

**No safe capacity-exhaustion behavior is promoted from the EXE.**

### Product rule

DMC Rengine/GDSpaces must not reproduce the unsafe invariant as product behavior. A reconstructed owner-node manager should fail closed with an explicit capacity diagnostic before dereferencing a missing node.

This is analogous in architecture to the separately recovered unsafe original invariant on LoadedResource group-5 exhaustion, but it is a different pool and a different capacity boundary.

---

# 6. Claim counter arithmetic is unchecked in the original owner-node layer

Claim increment:

```text
0x1401AE220
 -> ++dword(node+0x2C)
```

Claim decrement:

```text
0x1401AF6A0
 -> --dword(node+0x2C)
```

Neither function contains an arithmetic guard at the write site.

Therefore:

- increment at `0xFFFFFFFF` would wrap;
- decrement at zero would underflow;
- the recovered runtime relies on balanced caller ownership rather than enforcing it locally.

The generic sweep later tests only:

```text
claim == 0
```

so an underflowed value is not equivalent to zero and can retain a node incorrectly until another family-specific policy overwrites it.

### Product rule

Recovered evidence should preserve this original behavior, but product ownership code should detect claim overflow/underflow and fail closed rather than intentionally reproducing unchecked arithmetic.

---

# 7. Failure taxonomy after raw Pass 5

The static executable now supports at least four distinct failure/invariant classes in this RCP/L3B slice.

| Class | Raw behavior | Layer/plane ownership |
|---|---|---|
| underlying resource acquisition returns null | raise owner plan failure latch `+0x648` | L3/RCP coordination |
| higher plan-specific condition fails | same plan failure latch | RCP/higher plan logic |
| 32-node owner pool exhausted | no recovered safe path; original hard invariant | owner-node L3B invariant |
| claim counter imbalance | unchecked u32 arithmetic | owner-node L3B invariant |

These are separate from:

- L2 provider/backend failure semantics;
- central typed-dispatch unknown/default no-op behavior;
- family-specific semantic construction failures not yet recovered.

Do not launder one failure class into another.

---

# 8. P2-R6 readiness/success model is now four-dimensional

After Passes 3–5, a dependency-bearing resource path must distinguish at least:

```text
1. materialization/acquisition success
2. manager_ready_state3
3. plan/control success (failure latch clear)
4. family_semantic_ready / consumer effect
```

Example Stage path:

```text
StageCfg acquisition succeeds
 -> manager-ready cfg
 -> semantic cfg dependency scan
 -> child dependency acquisitions
 -> plan reaches inactive/finished
 -> failure latch must remain clear
 -> stage resources claimed/consumed
 -> downstream consumer effect
```

A failure at one dimension cannot be inferred solely from another dimension.

## V/LV implication

A future trusted receipt for a dependency-bearing vertical must carry explicit observations for the dimensions required by that family. At minimum it must not treat any of these as synonyms:

```text
request returned
plan finished
state3 reached
family semantic processing completed
consumer effect observed
```

---

# 9. Updated P2-R5 / P2-R6 status

## P2-R5 ownership/error invariants

Core gameplay owner-node model is now strongly bounded with:

- 32-node capacity;
- active/current-plan membership;
- explicit claims;
- generic inactive+zero-claim sweep;
- kind5 bulk pin/unpin;
- kind6 forced family release;
- explicit stage claim cycle;
- **unsafe no-free-node original invariant**;
- **unchecked claim arithmetic original invariant**.

Remaining breadth is no longer a missing generic ownership model; it is additional consumer/family/profile coverage.

## P2-R6 failure/recovery

Strongly bounded now:

- acquisition null -> plan failure latch;
- plan finished != plan success;
- two higher control paths consume that latch;
- failed plans perform activity clear + sweep + plan restart;
- active-plan replacement uses cancellation + quiescence + deferred mode restart;
- StageCfg/enemy family semantic boundaries from Pass 4.

Still open:

- exact family-specific failure semantics after enemy object factory entry;
- exact kind6 derived-processing failure behavior;
- demo/message consumer failure semantics;
- MOD/EFM/SCM/SHW post-load helper failures;
- dynamic original-process timing/retry receipts.

---

# 10. Pass-5 conclusion

The current raw EXE does not expose a simple boolean resource pipeline. It exposes independent authorities for:

```text
resource acquisition
manager readiness
plan activity/completion
plan failure
family semantic processing
consumer ownership/effect
```

and a concrete recovery controller that can cancel, wait for quiescence, clear/sweep owner nodes and restart a plan.

This further strengthens the conclusion that the missing architecture is **RCP + family-qualified L3 ownership/readiness**, not a new top-level L4.
