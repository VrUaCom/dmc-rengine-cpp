# GDSpaces EXE Grey-Boundary — Raw Pass 3: Request Ingress + StageCfg Dependency Graph

**Date:** 2026-08-26  
**Tracking:** #225 / PR #226  
**Pass class:** canonical raw-EXE evidence promotion; no synthetic/runtime-equivalence claim.  
**Canonical artifact:** `dmc3.exe`  
**Size:** `6,356,432` bytes  
**SHA-256:** `e454272ed0fb0247fcbcf300e5d55d7a3e96d50b89b9ffaff81bb978dcbdd082`

## Purpose

Continue P2-R1..P2-R4 using the executable itself as authority rather than promoting old Wave-3 summaries.

This pass answers four narrower questions:

1. What actually sits immediately above the three direct `OpenGameResource` call sites?
2. Does current raw EXE evidence really support StageCfg-driven dependency preload?
3. Is the historical enemy `factory selector + resource-set selector` split real in the current canonical artifact?
4. Can one identity edge be followed mechanically from loader-node `(kind,id)` through descriptor selection into `LoadedResource +0x18`?

No proprietary executable bytes are committed. Addresses, bounded instructions, static strings/table identities and derived control relationships are recorded only as evidence metadata.

---

# 1. P2-R1 — `OpenGameResource` direct callers are two ingress families

Canonical `OpenGameResource`:

```text
0x14002FCA0
```

Whole-image direct-call surface remains exactly:

```text
0x14003340A
0x1403380C7
0x1403381F7
```

All three direct calls set `EDX = 1` before the call.

The raw caller walk now shows these are not three independent gameplay root requests.

## 1.1 Family A — catalog/direct-path resource-object acquisition

Function around `0x1400333F0` receives a ready logical path in `RCX`, calls `OpenGameResource`, queries metadata/size after success, allocates a small owner object and stores the resulting resource handle/metadata.

The nearby wrapper around `0x140033480` selects the path by category/index from static pointer tables and then enters the same `0x1400333F0` acquisition helper.

Raw pointer-table classes include:

| Table | Representative strings | Mechanical role |
|---|---|---|
| `0x140553050` | `pl000.pac`, `pl011.pac`, `pl013.pac` | master resource-name catalog surface |
| `0x14055A920` | `Battle_00.adx`, `Battle_0a.adx`, ... | legacy audio-name catalog |
| `0x14055ADF0` | `Title.sfd`, `Dolby.sfd`, `GameOver.sfd`, ... | video-name catalog A |
| `0x14055AE90` | `m99_s00.sfd`, `m01_s00.sfd`, ... | video/demo-name catalog B |

Therefore one current-raw ingress edge is:

```text
numeric/catalog identity
 -> static pointer table
 -> logical path
 -> OpenGameResource(flags=1)
 -> resource owner object
```

A higher task path around `0x1402EF790` can use either the category/index wrapper or a direct caller-supplied path. This proves that catalog identity and direct-path identity coexist above L2 rather than one universal request-id ABI.

## 1.2 Family B — `DATA\DATA\` whole-file metadata/auxiliary reads

Function around `0x140338020`:

```text
build/normalize path
 -> synchronized OpenGameResource(flags=1)
 -> size/end-position query
 -> close
```

Function around `0x140338140`:

```text
size probe through 0x140338020
 -> build/normalize same path
 -> OpenGameResource(flags=1)
 -> whole-file read into caller destination
 -> close
```

Wrapper paths use the literal root:

```text
DATA\DATA\
```

Upstream raw strings around callers include:

```text
/demo/%s/%s
/demo/%s
.ptx / .PTX / .Ptx
.clt / .CLT / .Clt
.hid / .HID
.tsc / .TSC
/eff/texture/TextureDataNameTbl.bin
/eff/movie/MovieDataNameTbl.bin
/eff/curve/CurveDataNameTbl.bin
/eff/texanime/TexAnimeDataNameTbl.bin
/eff/effect/EffectDataNameTbl.bin
/eff/particle/PtclParamFiles.bin
/eff/generator/GeneratorDataNameTbl.bin
/eff/value/ValueDataNameTbl.bin
/eff/model/EfcMdlDataNameTbl.bin
/demo/DemoDataNameTbl.bin
```

This family is therefore a whole-file metadata/demo/effect ingress, not another independent resolver policy.

## 1.3 P2-R1 promotion

The three direct `OpenGameResource` sites reduce to **two bounded ingress families**:

```text
A. catalog/direct-path resource-object acquisition
B. DATA\DATA whole-file metadata/auxiliary read
```

### Architectural consequence

Current raw evidence does **not** justify a generic top-level `L0` ABI.

Keep `Consumer / Request Ingress` outside core L2 and model concrete ingress classes. L2 begins when the logical path request enters `OpenGameResource`/resolver policy.

### P2-R1 status

- direct caller-family classification: **CLOSED / current raw**;
- exhaustive semantic classification of every higher consumer that feeds those helpers: **PARTIAL breadth**.

---

# 2. Important false-positive correction — `0x1401B9A50` is not StageCfg/enemy authority

During reacquisition, a state-machine around `0x1401B9A50` initially looked structurally compatible with a dependency-control path.

Raw table decoding rejects using that path as StageCfg-enemy evidence.

The table around `0x1405B1660` contains weapon/style sound resources such as:

```text
se\snd_wp00a.pac
...
se\snd_wp14a.pac
se\snd_wp00b.pac
...
se\snd_wp14b.pac
se\snd_sty00.pac
...
```

Therefore:

**REJECT:** `0x1401B9A50` as evidence for StageCfg-driven enemy dependency preload.

It belongs to another player/audio/common resource-control family at the recovered scope.

This correction is important because the older Wave-3 claim is not allowed to choose its own reacquisition target by superficial state-machine similarity.

---

# 3. Generic loader-node dispatcher mechanically defines stage/dependency kinds

Function:

```text
0x1401AE310
```

accepts an owner/context plus `(kind,id)`, reuses a matching loader node when available or allocates a free owner-local node, then selects the corresponding descriptor/resource cell before calling `0x1401B8DF0`.

The jump-table cases directly recover:

```text
kind 0 -> Stage descriptor role cell +0x00
kind 1 -> Stage descriptor role cell +0x10
kind 2 -> Stage descriptor role cell +0x20
kind 5 -> [0x1405B1230 + id*8] enemy object descriptor
kind 6 -> [0x1405B14C0 + id*8] enemy sound descriptor
```

Given the already canonical Stage descriptor ABI:

```text
+0x00 script
+0x10 cfg
+0x20 effect
+0x30 sound
```

this directly establishes the bounded gameplay loader kinds:

```text
0 = stage script
1 = stage cfg
2 = stage effect
5 = enemy object resource set
6 = enemy sound resource set
```

Stage sound is separately loaded from descriptor `+0x30` by the numeric Stage sound path around `0x1401B985D`.

Kinds 3/4 have distinct table/language-sensitive behavior and are not renamed further in this pass.

---

# 4. P2-R2 — current raw StageCfg dependency-preload graph

Function around:

```text
0x1401AF000
```

is a six-state control routine over owner field `+0x63C`.

The jump table is mechanically:

| state | target | bounded action |
|---:|---|---|
| 0 | `0x1401AF18E` | terminal/ready return for this control step |
| 1 | `0x1401AF03B` | request current-stage `kind=1` (cfg), store node, advance to 2 |
| 2 | `0x1401AF069` | wait cfg resource state, derive payload/subview, scan dependencies, advance to 3 |
| 3 | `0x1401AF11C` | request current-stage `kind=0` and `kind=2` (script/effect), advance to 4 |
| 4 | `0x1401AF16E` | advance to 5 |
| 5 | `0x1401AF17A` | terminal condition/reset path |

## 4.1 StageCfg request and readiness gate

State 1 executes:

```text
id   = owner +0x644
kind = 1
call 0x1401AE310
store loader node at owner +0x650
state = 2 on success
```

State 2 obtains the underlying `LoadedResource` and only progresses when its state is `0` or `3` in the observed branch. For the materialized ready case it obtains the payload and handles the kind16/container envelope before calling the scanner.

The scanner call is:

```text
0x1401A9BC0
```

with current Stage id and the selected cfg payload/subview.

`0x1401A9BC0` has one direct static call site in this canonical image: this StageCfg control path.

## 4.2 StageCfg scanner — direct dependency discovery

Inside `0x1401A9BC0`, cfg records are iterated as variable-length command records:

```text
opcode = low byte of first dword
record size in dwords = ((first dword >> 8) & 0xFF) + 1
terminal opcode = 0x08
```

For opcode `0x7D`:

```text
external_enemy_id = dword(record + 0x04)
resource_set_selector = dword(0x1405A29A4 + external_enemy_id * 8)
```

The selector is linearly deduplicated into a local selector set. After the cfg scan reaches terminal opcode `0x08`, each unique selector is passed to:

```text
0x1401AE8F0
```

This is direct current-raw proof of:

```text
StageCfg record
 -> external enemy identity
 -> resource-set selector
 -> dedupe
 -> resource-demand emission
```

## 4.3 Opcode `0x3E` also participates in dependency discovery

Opcode `0x3E` does not directly use the simple `record+4 -> selector` path. It delegates to:

```text
0x1401A48A0
```

That helper walks another bounded record/subtable context and itself uses `0x1405A29A4` to add/deduplicate resource-set selectors. It can also apply a special selector insertion around the mechanically observed `0x1C` case.

The exact gameplay semantic name of opcode `0x3E`, its variant parameter and the special `0x1C` rule are **not promoted** here.

What is promoted is narrower:

**both direct opcode `0x7D` and the `0x3E -> 0x1401A48A0` path can contribute enemy resource-set selectors to the same deduplicated preload set.**

## 4.4 Dependency preload precedes script/effect scheduling

After `0x1401A9BC0` completes, owner state advances from 2 to 3.

The state-3 branch then requests:

```text
kind=0 -> current-stage script
kind=2 -> current-stage effect
```

Therefore the current raw control order is:

```text
request StageCfg
 -> wait/obtain cfg payload
 -> scan cfg dependency commands
 -> map + dedupe enemy resource-set selectors
 -> emit enemy dependency loads
 -> request stage script + stage effect
```

This confirms the core historical StageCfg dependency-preload ordering on the current canonical executable.

### P2-R2 status

**CORE DEPENDENCY-EMISSION ORDER: CLOSED / current raw.**

Still open as breadth/failure work:

- semantic names for all cfg opcodes involved;
- exact failure policy when one emitted dependency cannot acquire a record/backing;
- complete later barrier that proves every dependency family is semantically ready rather than only manager-ready;
- release/transition breadth for the emitted dependency set.

---

# 5. P2-R4 — paired enemy object + sound demand is directly confirmed

The dependency emitter called by the StageCfg scanner is:

```text
0x1401AE8F0
```

For ordinary resource-set selectors it searches/reuses or allocates owner-local loader nodes and performs two distinct acquisitions.

## 5.1 Enemy object side

Descriptor selection:

```text
rdx = [0x1405B1230 + selector*8]
call 0x1401B8DF0
node kind = 5
node id   = selector
```

The descriptor source table ultimately references cells such as:

```text
obj\em000.pac
obj\em006.pac
obj\em007.pac
...
obj\em037.pac
```

## 5.2 Enemy sound side

Descriptor selection:

```text
rdx = [0x1405B14C0 + selector*8]
call 0x1401B8DF0
node kind = 6
node id   = selector
```

The sound cells include:

```text
se\snd_em00a.pac
se\snd_em06.pac
...
se\snd_em37.pac
se\snd_em00b.pac
se\snd_emsr.pac
```

For the generic branch, selector identity is preserved in the owner node for both kind 5 and kind 6.

Special handling exists for selectors `0x1C` / `0x1D`; this pass records the branch but does not assign gameplay names beyond the exact mapped object/sound descriptors.

## 5.3 Resource-set table breadth

The current image contains 30 selector slots in the object/sound pointer maps used by kind5/kind6 loading.

Representative exact pairings include:

| selector | object | sound |
|---:|---|---|
| 0 | `obj\em000.pac` | `se\snd_em00a.pac` |
| 12 | `obj\em017.pac` | `se\snd_em17.pac` |
| 20 | `obj\em029.pac` | `se\snd_em29.pac` |
| 27 | `obj\em037.pac` | `se\snd_em37.pac` |
| 28 | `obj\em000.pac` | `se\snd_em00b.pac` |
| 29 | `obj\em000.pac` | `se\snd_emsr.pac` |

### P2-R4 status

**ENEMY RESOURCE-DEMAND CORE: CLOSED / current raw.**

Gameplay factory internals remain outside GDSpaces; only the resource-demand mapping edge is RCP/Identity-plane authority.

---

# 6. P2-R3 — external enemy identity has two independent axes

The current canonical `.data` block beginning at:

```text
0x1405A29A0
```

contains 64 observed contiguous records of two dwords each before the next unrelated data region.

Raw xrefs establish distinct uses of the two dwords.

## 6.1 First dword — factory/class selector

Enemy factory path around:

```text
0x1401AC6D0
```

uses:

```text
external_enemy_id = dword(input)
factory_selector   = dword(0x1405A29A0 + external_enemy_id*8)
```

The selector then drives the bounded enemy factory dispatch.

## 6.2 Second dword — resource-set selector

StageCfg dependency scanner and related dependency helpers use:

```text
resource_set_selector = dword(0x1405A29A4 + external_enemy_id*8)
```

That value drives kind5/kind6 object+sound acquisition.

## 6.3 Promotion

The EXE therefore directly implements:

```text
external enemy identity
    -> factory selector
    -> resource-set selector
```

as two separate mapped axes in one 8-byte record.

Do not collapse `enemy identity`, `factory selector` and `resource-set selector` into one ID in product/recovered evidence.

This is a strong current-raw Identity Plane edge.

---

# 7. P2-R3 — loader-node identity -> descriptor -> LoadedResource identity

`0x1401AE310` selects a concrete descriptor/resource-cell pointer from `(kind,id)` and calls:

```text
0x1401B8DF0
```

`0x1401B8DF0` forwards the selected descriptor pointer as `R8` into acquisition:

```text
0x1401B84E0
```

The first acquisition-side identity write is directly:

```text
record +0x18 <- selected descriptor/type pointer
```

Only later does acquisition establish backing/payload, perform the materialization dispatcher, and publish state 1 on success.

So the current raw identity edge is:

```text
loader-node (kind,id)
 -> exact descriptor/resource cell
 -> LoadedResource +0x18 descriptor/type authority
 -> payload/backing
 -> state1 lifecycle publication
```

This is stronger than table coincidence: the pointer itself is carried through the call chain and stored in the live LoadedResource record.

### P2-R3 status

**PARTIAL but materially advanced.**

Closed edges in this pass:

- stage `(kind,id)` -> role descriptor cell;
- enemy resource-set selector -> object/sound descriptor cell;
- descriptor pointer -> `LoadedResource +0x18`.

Still open:

- complete TypeInfo/descriptor domain beyond these families;
- all kind3/kind4 mapping semantics;
- consumer/factory object identity after manager-ready state;
- family-specific reverse mapping from live object back to descriptor/loader node where available.

---

# 8. Readiness consequence

The StageCfg control path uses `LoadedResource` state while planning dependencies, but the merged raw-L3 authority already proves generic state3 is only `manager_ready_state3` at the central boundary.

Therefore this pass does **not** infer:

```text
kind5/kind6 state3 == enemy gameplay object fully constructed
```

The stronger chain must remain:

```text
manager_ready_state3
 -> family-specific semantic construction/claim evidence
 -> consumer/gameplay object use
```

This directly informs P2-R6 and future V/LV instrumentation.

---

# 9. Updated RCP model after raw Pass 3

The architecture now has a current-raw concrete dependency loop:

```text
stage identity
 -> kind1 StageCfg request
 -> L2/L1 acquisition/materialization
 -> L3 manager-ready cfg
 -> StageCfg scanner
 -> external enemy identity
 -> resource-set selector
 -> dedupe
 -> kind5 enemy object + kind6 enemy sound demand
 -> L2/L1/L3 for dependencies
 -> script/effect request emission
```

This is direct evidence that resource loading is graph-shaped rather than a one-way stack.

The RCP model remains orthogonal and does not become L4.

---

# 10. Updated work order

## Closed/bounded by this pass

- **P2-R1A:** direct `OpenGameResource` caller-family classification.
- **P2-R2 core:** StageCfg dependency discovery/dedupe/emission ordering before script/effect scheduling.
- **P2-R4 core:** enemy resource-set -> paired object/sound demand.
- **P2-R3 partial:** external enemy dual-axis mapping and loader-node -> descriptor -> LoadedResource identity edge.

## Next static targets

1. **P2-R3 breadth** — kind3/kind4 and broader TypeInfo/descriptor xrefs.
2. **P2-R5 ownership hierarchy** — prove claim/retention/release behavior for dependencies emitted here, including transition preservation/replacement.
3. **P2-R6 readiness/failure matrix** — family-specific semantic-ready vs manager-ready and acquisition/factory failure behavior.
4. **P2-R1B breadth** — classify additional higher consumer contexts above the two direct ingress families where this changes evidence/validation needs.
5. Only after static graph closure, evolve future live validation toward dependency-aware parent/child receipts.

## Final Pass-3 statement

The key historical StageCfg dependency concept is no longer merely a Wave-3 acquisition target: its **core dependency-emission chain is independently reacquired and confirmed from the current canonical raw EXE**.

At the same time, the pass corrected a plausible but wrong StageCfg target (`0x1401B9A50`) and preserved the rule that semantic names/failure behavior are not promoted beyond instruction evidence.
