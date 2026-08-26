# GDSpaces EXE Grey-Boundary — Raw Pass 4: Ownership + Family Readiness

**Date:** 2026-08-26  
**Tracking:** #225 / PR #226  
**Extends:** `exe-rcp-raw-pass3-2026-08-26.md`  
**Canonical artifact:** `dmc3.exe`, 6,356,432 bytes, SHA-256 `e454272ed0fb0247fcbcf300e5d55d7a3e96d50b89b9ffaff81bb978dcbdd082`.

## Purpose

Continue the raw-EXE program into:

- P2-R3 remaining loader-kind identity breadth;
- P2-R5 owner-local retention/release hierarchy;
- P2-R6 family-specific post-manager-ready behavior.

The pass does not equate static disassembly with original-process timing evidence and does not move gameplay/factory ownership into GDSpaces.

---

# 1. P2-R3 — loader kinds 0..6 are now bounded on the current raw surface

Generic owner-node loader:

```text
0x1401AE310
```

uses a seven-way jump table for `kind = 0..6` and converts `(kind,id)` into a concrete descriptor/resource cell before entering `0x1401B8DF0`.

## 1.1 Kinds 0/1/2 — Stage descriptor roles

The exact cases select one Stage descriptor row and apply:

```text
kind 0 -> +0x00
kind 1 -> +0x10
kind 2 -> +0x20
```

With the canonical Stage cell ABI this is:

```text
0 = stage script
1 = stage cfg
2 = stage effect
```

Stage sound remains a separate descriptor `+0x30` path around `0x1401B985D` and is not represented as another owner-node kind here.

## 1.2 Kind 3 — demo/event resource

Kind 3 uses one of two descriptor-pointer banks:

```text
id < 300  -> [0x1405B3F50 + id*8]
id >= 300 -> [0x1405B4EB0 + (id-300)*8]
```

Current raw descriptors resolve to paths such as:

```text
demo\m99_s00\m99_s00.pac
demo\m01_s00\m01_s00.pac
demo\m01_c00\m01_c00.pac
...
```

The second bank includes movie-related demo descriptors such as:

```text
demo\m99_s00\m99_s00_movie.pac
```

Therefore the bounded kind-3 role is **demo/event resource PAC** on this loader surface.

## 1.3 Kind 4 — localized message cell associated with demo/event identity

Kind 4 begins from the same demo/event descriptor bank but applies:

```text
descriptor + 0x18 + language_index * 0x10
```

The language index is read through the existing language selector path before acquisition.

The selected cells resolve directly to localized message resources such as:

```text
message\japanese\m99_s00_msg_jpn.txt
message\english\m99_s00_msg_eng.txt
message\french\m99_s00_msg_frn.txt
message\german\m99_s00_msg_ger.txt
message\italian\m99_s00_msg_ita.txt
message\spanish\m99_s00_msg_spa.txt
message\chinese\m99_s00_msg_chn.txt
message\chinese\m99_s00_msg_chs.txt
```

Therefore kind 4 is a **language-selected localized message resource** on this loader surface.

## 1.4 Kinds 5/6 — enemy dependency resource sets

Already re-confirmed in raw Pass 3:

```text
5 = enemy object/model PAC resource-set selector
6 = enemy sound PAC resource-set selector
```

## 1.5 P2-R3 promotion

Current raw loader-kind table:

| kind | bounded resource-domain role |
|---:|---|
| 0 | stage script |
| 1 | stage cfg |
| 2 | stage effect |
| 3 | demo/event PAC |
| 4 | localized message resource selected by language |
| 5 | enemy object/model PAC |
| 6 | enemy sound PAC |

**P2-R3 loader-kind identity breadth for this owner-node surface: CLOSED.**

Broader TypeInfo/family-object reverse mappings outside this surface remain separate identity breadth.

---

# 2. Owner-local node structure has two independent retention dimensions

The recovered owner-local cache/pool contains 32 nodes of stride `0x30`.

Current raw behavior mechanically identifies the following node fields at the bounded scope:

```text
+0x18 underlying LoadedResource pointer
+0x20 active/needed-by-current-plan byte
+0x24 kind
+0x28 id/selector
+0x2C claim counter
```

Other list/link fields remain mechanically present but are not semantically renamed beyond their observed linked-list use.

## 2.1 Request/reuse marks activity independently of claims

`0x1401AE310`:

- searches existing nodes by `(kind,id)`;
- reuses an existing node where possible;
- sets node `+0x20 = 1` on reuse/new request;
- creates/acquires underlying `LoadedResource` when required.

The paired enemy loader `0x1401AE8F0` follows the same active-node model for kind5/kind6.

## 2.2 Explicit claims are separate

Claim increment:

```text
0x1401AE220
 -> locate matching ready-list node by (kind,id)
 -> ++node+0x2C
```

Claim decrement:

```text
0x1401AF6A0
 -> locate matching node by (kind,id)
 -> --node+0x2C
```

No universal `LoadedResource.refCount` is created by these functions. The counter belongs to this higher owner-node layer.

## 2.3 Plan-membership/activity reset

Function:

```text
0x1401AF1C0
```

walks all 32 nodes and, for every node with an underlying resource and nonzero `+0x20`, clears:

```text
node +0x20 = 0
```

It does **not** clear `+0x2C` claims.

This separates:

```text
needed by current request/dependency plan
```

from:

```text
claimed by a higher consumer/owner
```

## 2.4 Zero-claim sweep requires both dimensions to be clear

Function:

```text
0x1401AF6F0
```

walks all 32 inline nodes. A node is eligible for underlying release only when:

```text
underlying LoadedResource != null
claim counter +0x2C == 0
active byte +0x20 == 0
```

For kinds `0..6`, it calls ordinary LoadedResource owner release `0x1401B9530`, clears the node's underlying resource pointer/claim state and removes the node from the owner list.

### P2-R5 consequence

The owner model is not one reference counter. At minimum it is:

```text
current-plan membership/activity
 + explicit consumer claims
 + family-specific override policy
```

---

# 3. Raw transition/control sites prove activity-reset -> sweep ordering

Multiple higher scene/stage control paths directly execute:

```text
0x1401AF1C0  // clear active/current-plan markers
 -> 0x1401AF6F0  // sweep nodes whose claims are also zero
```

Representative current-raw sites include the higher control paths around:

```text
0x140238B3C -> 0x140238B49
0x14023C45B -> 0x14023C468
```

The first path then emits/rebuilds the next resource plan through a subsequent owner-loader call.

This proves a concrete replacement pattern:

```text
old plan membership off
 -> preserve claimed nodes
 -> release unclaimed inactive nodes
 -> emit/rebuild new plan
```

It is a direct RCP/L3B coordination edge rather than generic allocator behavior.

---

# 4. Stage consumer claims and release are explicit for kinds 0/1/2

A gameplay setup path around `0x14023C93E` adds explicit owner claims through `0x1401AE220` for current Stage resource nodes before consuming their payloads.

A corresponding teardown path around `0x14023B2C0` performs:

```text
claim-- kind 0, current stage id
claim-- kind 1, current stage id
claim-- kind 2, current stage id
 -> kind5 bulk claim reset
 -> zero-claim/inactive sweep
```

The exact higher semantic role of every caller is not inferred beyond the observed scene/gameplay context, but the ownership sequence itself is direct.

This further rejects a universal path-cache lifetime model.

---

# 5. Kind5 enemy-object nodes have explicit bulk pin/unpin policy

Two functions modify the **claim counter**, not the activity byte, for every live kind5 node.

## 5.1 `0x1401AF790` — bulk kind5 claim reset

For every live kind5 node in the 32-node pool:

```text
node +0x2C = 0
```

A teardown path executes this after decrementing current Stage kind0/1/2 claims and before the generic zero-claim sweep.

## 5.2 `0x1401AF800` — bulk kind5 baseline claim

For every live kind5 node:

```text
node +0x2C = 1
```

A gameplay setup path invokes this after clearing current-plan activity markers and before subsequent scene/gameplay consumer setup.

### Promotion

Enemy object resources use a family-specific **bulk pin/unpin claim policy** on top of the generic node activity + claim model.

Do not reconstruct this as ordinary per-resource reference-count increments/decrements alone.

---

# 6. Kind6 enemy-sound nodes have a different forced-release policy

Function:

```text
0x1401AF620
```

walks the owner list and for nodes with:

```text
kind == 6
```

performs direct underlying release through `0x1401B9530`, clears the node's underlying resource/claim state and removes the node from the list.

This path does not implement the generic `inactive && claim==0` sweep test.

### Promotion

Enemy sound ownership is **not symmetric** with enemy object ownership:

```text
kind5 -> bulk claim pin/unpin + generic sweep
kind6 -> family-specific bulk release path
```

This closes a major P2-R5 family exception.

---

# 7. P2-R6 — StageCfg itself proves manager-ready is not semantic completion

Raw Pass 3 showed:

```text
kind1 StageCfg LoadedResource reaches usable manager state
 -> higher control extracts cfg payload/subview
 -> 0x1401A9BC0 scans semantic cfg commands
 -> dependency graph is emitted
```

Therefore even for StageCfg:

```text
LoadedResource manager-ready
 != dependency semantics already executed
```

The semantic dependency phase is downstream of the generic resource lifecycle boundary.

---

# 8. P2-R6 — enemy object has an explicit manager-resource -> gameplay factory edge

Gameplay object construction path around `0x1401A4680` performs:

```text
external enemy identity
 -> resource_set_selector = second dword of 0x1405A29A0 record
 -> lookup owner node (kind=5, selector) through 0x1401AE1D0
 -> obtain node +0x18 underlying LoadedResource
 -> pass that live resource authority into enemy factory 0x1401AC6D0
```

The factory independently uses the **first dword** of the same external-enemy mapping record as its constructor/class selector.

So both identity axes rejoin at construction time:

```text
external enemy identity
  -> factory selector ---------------------\
                                           -> enemy gameplay construction
  -> resource-set selector -> kind5 load --/
```

### Promotion

For enemy object resources:

```text
manager-ready LoadedResource
 -> family/gameplay factory construction
 -> consumer object
```

is an explicit stronger semantic edge.

`manager_ready_state3` alone must not be reported as equivalent to successful enemy object construction.

---

# 9. P2-R6 — enemy sound has post-manager-ready processing before release

A higher gameplay setup sequence around `0x14023C7A2` performs:

```text
higher owner/resource-plan completion gate
 -> 0x1401AF1F0
 -> repeatedly drive 0x1401AF4C0 until its family-processing condition completes
 -> 0x1401AF620 bulk release kind6 resources
```

## 9.1 `0x1401AF1F0`

This function enumerates kind6 owner nodes, extracts their underlying payload/subviews and feeds them into a separate derived processing/output subsystem. The bounded behavior is mechanically clear even though this pass does not assign semantic names to every lower helper in that subsystem.

## 9.2 `0x1401AF4C0`

This function advances per-item processing over the collected kind6-derived inputs and returns completion only after the observed sequence has progressed through the bounded set.

## 9.3 Release after derived processing

Only after that higher processing completes does the caller invoke `0x1401AF620` to release the kind6 resource nodes.

### Promotion

For enemy sound resources the current raw sequence is:

```text
manager-owned loaded sound resource
 -> family-specific derived processing
 -> derived consumer/system state
 -> release original kind6 LoadedResource ownership
```

Again, state3/manager readiness is not the final family semantic boundary.

---

# 10. Enemy object vs enemy sound lifecycle is deliberately asymmetric

Both families are emitted from the same StageCfg resource-set selector, but their post-load ownership diverges:

```text
StageCfg selector
  |
  +-> kind5 object resource
  |      -> retained/pinned through family claim policy
  |      -> supplied to gameplay factory construction
  |      -> later unpinned/swept
  |
  +-> kind6 sound resource
         -> family-specific derived processing
         -> bulk resource-node release
```

This is strong direct evidence for the RCP + family-qualified L3B model.

It would be incorrect to implement or document one universal dependency-node lifecycle for both.

---

# 11. Updated P2-R5/P2-R6 status

## P2-R5 — ownership hierarchy

**Strongly bounded / core closed for the recovered gameplay owner-node surface:**

- 32-node owner-local pool;
- `(kind,id)` identity;
- active/current-plan flag distinct from claim counter;
- explicit claim++/claim--;
- current-plan activity clear;
- zero-claim + inactive generic sweep;
- concrete stage transition `activity-clear -> sweep -> replan` pattern;
- kind5 bulk pin/unpin exception;
- kind6 bulk release exception;
- explicit Stage kind0/1/2 claim/release cycle.

**Still open breadth:**

- all consumer classes using this owner-node manager;
- cross-profile/build differences;
- exact failure behavior under claim underflow/corruption and allocation exhaustion;
- other ownership subsystems outside this 32-node manager.

## P2-R6 — readiness/failure taxonomy

**Enemy/Stage dependency slice now strongly bounded:**

```text
StageCfg manager-ready -> semantic cfg scan -> dependency emission
enemy object manager-ready -> gameplay factory construction
enemy sound manager-ready -> derived family processing -> original node release
```

**Still open:**

- family-specific failure semantics when construction/derived processing fails;
- MOD/EFM/SCM/SHW semantic-success criteria beyond central best-effort dispatch;
- complete demo/message semantic consumer edges;
- original-process timing/order receipts.

---

# 12. Architecture consequence

The current raw executable now supports a stronger model than a simple resource stack:

```text
RCP plans and re-plans dependencies
   |
   v
owner-node (kind,id)
   |
   +-- active/current-plan membership
   +-- explicit claims
   +-- family-specific pin/release rules
   |
   v
LoadedResource manager lifecycle
   |
   v
family-specific semantic processing
   |
   v
consumer object/system
```

No new top-level L4 is needed.

The important missing breadth is now **family failure semantics and dynamic/original-process validation**, not discovery of another generic layer.

## Pass-4 acceptance statement

P2-R3 loader-kind identity breadth and the core P2-R5 gameplay owner-node hierarchy are now directly bounded from the current canonical raw EXE. P2-R6 is strongly bounded for StageCfg + enemy object + enemy sound but remains a breadth program for other families and failure paths.
