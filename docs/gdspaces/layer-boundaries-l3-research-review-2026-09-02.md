# GDSpaces layer boundaries + Layer 3 research/review — 2026-09-02

**Repository base:** `main@9483663959e5452f9a224c1535445bb5a3b33520`  
**Primary target:** canonical unpacked DMC3 HD analysis `dmc3.exe`  
**SHA-256:** `e454272ed0fb0247fcbcf300e5d55d7a3e96d50b89b9ffaff81bb978dcbdd082`  
**Size:** `6,356,432` bytes  
**ImageBase:** `0x140000000`

## Purpose and method

This checkpoint follows the required order:

1. research the current L1/L2/L3 ownership boundaries;
2. research Layer 3 against all current relevant merged documentation and stronger newer branch evidence;
3. review the actual current Layer-3 implementation/reverse state;
4. derive an implementation-oriented Layer-3 work order without laundering branch evidence into merged truth.

This document distinguishes four authority classes:

- **MERGED CANONICAL** — present on current `main`;
- **STRONGER UNMERGED EVIDENCE** — newer raw-EXE/reconciliation work in an open PR; useful for review, not yet canonical status;
- **IMPLEMENTATION CANDIDATE** — code exists on an open branch but is not integrated in `main`;
- **PROPOSAL** — architecture/planning only; never reported as current runtime truth.

Historical pass documents remain evidence chronology. A newer document does not become merged authority merely because its conclusions are stronger.

---

# 1. Research — updated layer boundaries

## 1.1 Canonical semantic model

### Layer 2 — Resource Resolution

L2 answers:

> Which logical resource/provider/source/volume/member does the original runtime select?

```text
logical request
 -> candidate construction
 -> path normalization
 -> mount/provider/source traversal
 -> volume/member selection
 -> ambiguity / fallback / provider failure
 -> usable selected resource identity
```

L2 owns **selection**, not the selected bytes' physical transformation and not the later lifetime of the resulting resource.

Current merged examples include `RuntimeResourceResolver`, exact `ResourceId`/`ResourceRef` identity and the fail-closed `RuntimeNamingBridge` link into L1.

### Layer 1 — Resource Materialization

L1 answers:

> Given a selected resource identity, what exact byte image is required, how is it acquired/transformed/reproduced, and how is that byte authority preserved through edit/rebuild/reopen?

```text
selected identity
 -> logical/materialized size
 -> allocation/capacity
 -> byte/span acquisition
 -> final-chunk / EOF / short-read / progress semantics
 -> decompression / representation handling
 -> exact destination bytes
 -> PAC/PNST/.lst nested materialization
 -> byte provenance
 -> edit/rebuild/repack/publication
 -> reopen/rematerialization
```

Names, display suffixes and semantic hints are not physical/write authority. The merged `RuntimeNamingBridge` correctly links L2 to L1 only by exact `ResourceId` equality.

### Layer 3 — Original Runtime / Resource Lifecycle

L3 answers:

> Once selected/materialized resource work is owned by the runtime, how is it scheduled, published, typed-ready, shared, cancelled, released, reset and torn down?

```text
request/lifecycle ownership
 -> scheduling / callback lifetime
 -> LoadedResource state publication
 -> typed post-load
 -> optional ready callback
 -> state-3 consumer visibility
 -> loader-node/family ownership and reuse
 -> cancellation/replacement
 -> deferred cleanup
 -> owner/group/full release/reset
 -> runtime / CRT / process-lifetime teardown
```

L3 is a **semantic/lifetime boundary**, not one contiguous EXE address range.

### Downstream DOMAIN

Stage Assembly, Stage Ops, ModViz and editors are downstream consumers. They do not become L3 merely because they consume state-3-ready resources.

### Validation / live observation

Validation is cross-cutting and must not become a speculative L4. Open PR #223 proposes V/LV as non-numbered validation/observation planes; that architecture is useful but remains unmerged/proposal authority on current `main`.

## 1.2 Updated seam rules

The strongest current boundary model is behavior-scoped rather than function-scoped.

| Behavior | Owner |
|---|---|
| logical candidate/provider/volume/member selection | L2 |
| selected exact `ResourceId` | L2 output / L1 input identity seam |
| packed/loose representation choice and byte construction | L1 |
| byte extent, EOF, short-read, decompression, terminal byte/result semantics | L1 |
| FileSlot/ReadRequest byte/result mechanics | L1-support behavior |
| FileSlot/ReadRequest object/queue/callback lifetime | L3-support behavior |
| scheduler admission/persistence/retirement | L3 |
| normal `LoadedResource state 1 -> 2` publication | L3 |
| typed post-load / callback / `2 -> 3` | L3 |
| cancellation/reset/release/shutdown | L3 |

A function may therefore contain behavior from both sides of a seam.

## 1.3 `0x1401B8CA0` and the L1/L3 seam

Merged docs already classify `0x1401B8CA0` as mixed materialization/lifecycle context, but newer unmerged #245/#258/#269 evidence gives a more precise cut:

```text
[L1]
representation/planner
 -> admitted type-2 byte execution
 -> native terminal byte/result status

===== semantic seam =====

[L3]
scheduler/FIFO reaches admitted type-3 completion callback
 -> 0x1401B8DC0 state 1 -> 2
 -> typed post-load / ready lifecycle
```

For admitted type-2 jobs, newer raw evidence reports:

- status `2` = pending, no retirement;
- status `4` = retry/reset local phase, no retirement;
- status `3` = retire current byte job and advance FIFO;
- only after retirement can a later admitted type-3 normal callback become current.

This is **STRONGER UNMERGED EVIDENCE** until reconciled onto current main.

Important unsafe-original boundary: `0x1400335A0` can publish status 3 without independently proving accumulated actual bytes equal planned total. Product exactness must remain stricter than original short-success behavior.

## 1.4 `0x1402EF4D0`, `0x140033500/0x1400335A0` and semantic split

These helpers must not be assigned wholesale to one layer:

- byte ingress/result/status mechanics can be L1 evidence;
- queue/job ownership, callback persistence and cancellation timing belong to L3.

This semantic classification supersedes subsystem/address-only ownership.

---

# 2. Research — Layer 3 against current documentation

## 2.1 Merged canonical L3 core

Current `main` strongly supports:

```text
LoadedResource registry base 0x140C99D30
363 records
stride 0x48
seven groups [4,136,60,28,1,128,6]
```

Central lifecycle anchors remain:

```text
0x1401B84E0  acquisition / state 0 -> 1
0x1401B8DC0  normal completion / state 1 -> 2
0x1401B92D0  typed post-load -> optional callback -> state 2 -> 3
0x1401B8430  cancellation state 1|2 -> 4
0x1401B8F00  deferred state 4 -> 0 cleanup
0x1401B9530  ordinary owner release
0x1401B9560  group reset
0x1401B95E0  full registry reset
```

State 3 is an original runtime lifecycle boundary after typed post-load and optional callback. It is not equivalent to “parser succeeded”, “StageBundle exists”, “preview works”, or “game did not crash”.

## 2.2 R1 state-writer research status

### Merged canonical status

Main includes #230-era direct/leaf/derived writer evidence and therefore strongly closes major writer classes, but `l3-audit-2026-08-25.md`, `l3-boundary-audit-2026-08-26.md`, `decompilation-layer-classification.md`, `master-roadmap.md` and issue #88 still describe residual R1 work as open.

### Stronger unmerged R1 review

Open #240 challenged the writer map with additional independent classes:

- startup full-manager zero vs runtime per-record state0 initialization;
- bulk-zero hidden paths;
- interprocedural `arg+0x04` writers;
- exact record producer -> stored alias -> later callee mutation;
- partial-width/non-enum `+0x04` writes;
- atomic/lock forms;
- static callback registration surfaces;
- malformed odd completion context;
- provenance-backed state values outside `0..4`.

It found no contradictory `LoadedResource.state` writer and proposed:

> **R1 = STATIC BOUNDED-CLOSED / APPROVED / CONTRADICTION-GATED**

However #240 is based on old `main@a90b017...` and remains open.

### Compatibility check against newer 31.08–01.09 research

The newer merged type/family/payload research changes type identity and typed-family detail, not the `LoadedResource +0x04` destination provenance model:

- runtime type evidence is split into three independent identification paths;
- MCV gains family-mask identity;
- EFW/EFE remain container-dispatch sentinels;
- MOD/EFM/SCM are a related runtime model-document family;
- SHW is corrected to a distinct self-contained shadow-hull layout;
- real MOD/SHW payload bindings strengthen handler-side semantics.

No reviewed newer document supplies a contradictory state writer with exact `LoadedResource` record provenance.

**Research conclusion:** #240's R1 closure remains semantically compatible with current merged evidence, but must be **ported/reviewed against current main rather than merged mechanically**.

## 2.3 Runtime type identity is not one L3 dispatcher

Merged 31.08 evidence proves at least three distinct instruction-backed mechanisms:

### A — registry content probe `0x1402DB1F0`

Three-byte site-scoped tags:

```text
MOD EFM SCM MRP SHW
```

### B — container post-load dispatcher `0x1401B9FA0`

```text
MOD -> 0x1402FE3B0
EFM -> 0x1402F7A90
SCM -> 0x1403051B0
SHW -> 0x1403204C0
EFW/EFE -> recognized sentinel prefixes, no normal handler proven
PNST -> recursive child traversal
```

### C — family-mask classifier `0x1402FD650`

Exact four-byte tags with trailing space:

```text
MOD  EFM  SCM  MRP  MCV  SHW 
```

Therefore L3 implementation must not expose one universal `RuntimeFormatEnum` whose values automatically imply registration behavior, post-load handler existence, geometry capability and downstream family behavior.

## 2.4 New typed-family research changes R3 scope

Merged research after the older L3 audits adds:

- MOD — real payload bound to the handler-derived `0x40` outer / `0x50` inner document layout;
- EFM — mesh-bearing effect-model family, not metadata-only;
- SCM — mesh-bearing stage/scene model with format-specific normalizer;
- SHW — self-contained closed shadow-hull mesh with vertices, triangles, exact adjacency and per-vertex transform-matrix selectors;
- MRP — byte-backed runtime identity but no normal central post-load handler established;
- MCV — four-byte family identity, not registry-three-byte identity;
- family-mask direct `object+0xE0` consumers on the bounded scanned surface specialize MOD/EFM/SCM, not MRP/MCV/SHW.

This advances L3-R3 **family semantics**, but does not close external factory/dependency behavior or make all runtime type systems equivalent.

## 2.5 Naming/type evidence remains outside lifecycle authority

`RuntimeNamingBridge` is correctly fail-closed:

```text
L2 resolved ResourceId == L1 naming parent ResourceId
```

No fallback by basename, display name, `.index`, embedded alias or semantic extension is allowed.

L3 must consume the same physical/resource identity chain for dynamic receipts; it must not create a second naming authority.

## 2.6 Shared ownership remains family-specific

Strong loader-node evidence remains:

```text
0x1401AE220 claim++
0x1401AF6A0 claim--
0x1401AF6F0 zero-claim sweep/release
```

This does not prove a universal `LoadedResource.refCount`. Fixed groups, dynamic group 5, loader-node claims and specialized family managers must remain separate until independently reconciled.

## 2.7 Dynamic cancellation remains more open than normal completion

Newer static seam evidence strengthens the normal admitted materialization -> completion ordering, but explicitly leaves **current-slot cancellation/concurrency** open.

Therefore a static FIFO result must not be escalated to dynamic cancellation timing equivalence. V5 remains necessary.

---

# 3. Review — current Layer 3 state on `main@9483663...`

## 3.1 Current status by work package

| L3 package | Current review |
|---|---|
| R1 state-writer/caller census | **MERGED: strongly bounded; stronger #240 closure compatible but unmerged** |
| R2 `+0x08/+0x18/+0x20/+0x28` ownership | **OPEN** |
| R3 typed/factory/dependency | **PARTIAL, materially advanced by 31.08–01.09 merged research** |
| R4 shared-owner breadth | **PARTIAL** |
| R5 transition/cancel/reset/shutdown timing breadth | **OPEN dynamic breadth** |
| lifecycle trace validator | **IMPLEMENTATION CANDIDATE in open #218, absent from main** |
| trusted original-process L3 publisher/binder | **NOT IMPLEMENTED** |
| V1–V7 original-process receipts | **OPEN** |
| final L3 acceptance | **NOT READY** |

Layer 3 is therefore **NOT COMPLETE**.

## 3.2 Documentation drift found

### Main docs that are semantically useful but status-stale

- `docs/gdspaces/master-roadmap.md` — old base/latest-L3 metadata and old residual-R1 work order;
- `docs/gdspaces/decompilation-layer-classification.md` — correct behavior-based model, stale operational snapshot/work order;
- `docs/gdspaces/l3-audit-2026-08-25.md` — R1 residual wording predates #240 and newer type/family evidence;
- `docs/gdspaces/l3-boundary-audit-2026-08-26.md` — strong core boundary but old remaining-work list;
- issue #88 — R1 and L1/L3 seam wording needs reconciliation;
- issue #217 — completion/precondition wording predates newer seam and R1 review.

### Open PR drift

- #240 — strong R1 review, but old base; semantic port required;
- #218 — sound anti-laundering lifecycle validator design, but old base and no current-main integration;
- #245/#258/#269 — stronger boundary/L1-terminal seam evidence, not merged;
- #254 — still carries an old global “exactly five payload tags” statement superseded by merged 31.08 evidence;
- #223/#226 — useful V/LV/RCP architecture proposals, not current canonical layer truth.

## 3.3 Current implementation reality

Merged code has strong support for:

- L2 resolver/resource identity;
- L1 naming/semantic evidence separation;
- `RuntimeNamingBridge` exact identity seam;
- DMC3 `ResourceTypeContract` preserving independent registry/container/family-mask identification paths.

Current main does **not** contain the `validation` domain from #218. Therefore there is currently no integrated canonical `L3LifecycleTrace` validator or trusted runtime lifecycle publisher in `main`.

This is the largest implementation gap between L3 reverse knowledge and L3 executable validation infrastructure.

---

# 4. Research for implementation

## 4.1 Do not implement L3 as another GDSpaces resolver/materializer

The implementation boundary should be:

```text
L2 exact selected identity
        |
        v
L1 exact byte/provenance identity
        |
        v
L3 lifecycle observation/model
```

L3 may reference L2/L1 identities but must not reconstruct them by filename or magic.

## 4.2 Recommended L3 code domains

### A. `Dmc3LoadedResourceContract` — static/profile contract

Read-only instruction-backed contract for:

- registry base/count/stride/group tables;
- stable record offsets;
- state domain and known transition writers;
- known scheduler/completion/cancellation anchors;
- evidence hashes/authority where available.

This belongs with profile/recovered-runtime evidence, not with generic GDSpaces container code.

### B. `Dmc3RuntimeTypeEvidence` — keep split identities

Reuse/extend current `ResourceTypeContract`; do not replace it with one universal enum.

Expose separately:

- registry type evidence;
- container post-load disposition/handler evidence;
- family-mask evidence;
- semantic parser/format evidence.

A caller must request the evidence domain it actually needs.

### C. `L3LifecycleTrace` — validation/observation contract

The design in #218 is worth retaining, especially:

- strict ordered events;
- L1/L2 identity binding;
- state range/order validation;
- explicit V1–V6 scopes and V7 aggregation boundary;
- dropped-event/overflow/semantic-intrusion/rollback rejection;
- manual JSON can never self-promote.

But #218 should be **respawned/semantic-ported from current main**, not mechanically merged from its old base.

### D. Trusted original-process publisher/binder

This is the mandatory dynamic implementation after the structural validator.

It must bind:

- exact protected process instance;
- executable identity and actual module mapping;
- observer/instrumentation identity and configuration;
- L2 exact selected provider/member identity;
- L1 exact materialized byte/provenance identity;
- ordered L3 events;
- no event loss/overflow;
- rollback when an authored overlay is used.

Editable JSON fields such as `original_process=true` must remain content metadata only.

### E. Family-specific typed post-load observation

Instrumentation must distinguish:

```text
manager state-3 ready
!= family post-load handler identity
!= family semantic readiness
!= consumer-visible effect
```

The current type/family research makes this non-negotiable.

## 4.3 R1 implementation/review decision

Before R2 implementation/research becomes the active static target:

1. port #240 final R1 review onto current main;
2. explicitly compare it against merged 31.08–01.09 type/family/payload evidence;
3. record that no new evidence contradicts the `LoadedResource.state` writer map;
4. only then promote R1 to current-main **BOUNDED-CLOSED / CONTRADICTION-GATED**.

The compatibility review in this document found no contradiction, so a current-main R1 promotion is justified once its evidence text is ported/reconciled.

## 4.4 R2 implementation-research target after R1 promotion

R2 should not begin as speculative field naming. Start from exact writer/reader ownership:

```text
+0x08 family-specific selector/index metadata
+0x18 descriptor/resource-definition authority
+0x20 published payload alias
+0x28 inline backing/owned subobject
+ stable adjacent fields
```

Every semantic field name must be tied to writer + reader + lifetime evidence. Runtime backing release and CRT destruction must remain distinct.

## 4.5 R3 implementation-research target

Do not repeat generic magic research. Current next questions are:

1. external factory/dependency behavior outside `0x1401B9FA0`;
2. MRP downstream ownership/consumer path;
3. MCV downstream family consumers;
4. EFW/EFE sentinel semantics only if a concrete consumer path exists;
5. SCM format-specific post-load/consumer details;
6. SHW matrix-palette ownership linking transform selectors to the owning model/bone resource;
7. family failure behavior vs central best-effort dispatcher behavior.

## 4.6 R4/R5 and dynamic order

Recommended dynamic order remains:

```text
trusted observer/binder
 -> V1 initial load
 -> V5 in-flight cancellation
 -> V2 room/stage transition
 -> V3 restart/reload
 -> V4 return-to-menu/full reset
 -> V6 shutdown
 -> V7 family/build breadth
 -> final contradiction audit
```

V1 validates the normal ready path. V5 specifically targets the concurrency/cancellation seam left open by static FIFO research.

---

# 5. Review decision and canonical next work order

## Layer-boundary decision

Adopt behavior-scoped boundaries:

```text
L2 owns selection identity.
L1 owns exact byte/result/materialization and authoring/reopen authority.
L3 owns request/scheduler/callback/resource lifetime and ready/use/release authority.
```

No function/address is assigned wholesale when its concrete behaviors straddle a seam.

## L3 decision

- central lifecycle core remains strong;
- new 31.08–01.09 family/type evidence materially improves R3 but does not alter the layer boundary;
- R1 stronger #240 closure is compatible with newer evidence but is not current-main authority until ported;
- R2 remains genuinely open;
- #218 is a useful implementation candidate but must be rebased/semantic-ported;
- trusted dynamic publisher and V1–V7 remain mandatory;
- L3 remains NOT COMPLETE.

## Immediate execution order

```text
1. reconcile this 2026-09-02 boundary/L3 review into #88/#217
2. port current-compatible #240 R1 closure to current main and retire stale #240 branch
3. update master-roadmap / layer-classification / L3 audit status surfaces
4. only then activate R2 field/backing ownership research
5. in parallel, respawn #218 lifecycle validator from current main
6. implement trusted original-process lifecycle publisher/binder
7. V1 + V5 first
8. R3/R4 breadth and remaining transition receipts
9. final L3 audit
```

## Reopen rule

Any bounded static slice may be reopened only by specific contradictory evidence with exact authority/provenance. New format names, display names, numeric field coincidences or a successful product test are insufficient by themselves.

---

# 6. Status summary

```text
L1 boundary research: UPDATED; strongest exact terminal seam partly unmerged
L2 boundary research: UPDATED; selection identity remains distinct
L3 boundary research: UPDATED
L3 R1: merged strong / current-main final promotion pending port of #240
L3 R2: OPEN
L3 R3: PARTIAL, materially advanced by merged type/family/payload research
L3 R4: PARTIAL
L3 R5: OPEN dynamic breadth
L3 validator: open implementation candidate #218, not in main
trusted L3 live publisher: NOT IMPLEMENTED
V1–V7: OPEN
L3 COMPLETE: NO
```
