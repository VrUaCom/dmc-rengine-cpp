# GDSpaces Layer 2 Roadmap

**Status:** ADVANCED / INCOMPLETE / REAL ORIGINAL-SELECTION EVIDENCE OPEN  
**Snapshot:** 2026-09-02  
**Reviewed base:** `main@9483663959e5452f9a224c1535445bb5a3b33520`

Layer 2 owns **Resource Resolution**: which logical request resolves to which usable provider/source/volume/member identity. It does not own the selected bytes' transformation/rebuild semantics and it does not own later LoadedResource lifetime.

## Canonical boundary

```text
logical request
 -> basename/candidate construction
 -> normalization
 -> mounted-provider traversal
 -> archive/physical attempt policy
 -> volume/member identity
 -> ambiguity/provider-failure semantics
 -> exact selected ResourceRef / ResourceId
```

The selected identity is L2 output and becomes L1 input. L2 never recovers child identity by presentation name, `.index`, embedded alias or semantic extension.

## Strong merged authority

Current main includes the following bounded L2 authority:

- `OpenGameResource` canonical analysis anchor `0x14002FCA0`;
- three observed direct callers using the recovered direct-call `flags = 1` mode;
- basename-oriented candidate construction and six-prefix policy on that direct-call surface;
- archive-before-physical attempt ordering;
- archive normalization `0x0E` and physical normalization `0x0C` as separate path policies;
- normalized archive index/sort/search behavior;
- type-0 physical-provider final Win32 open semantics from the bounded #215 scope;
- protected-runtime RVA window acquisition/multi-anchor mapping tooling from #219;
- selected-identity content-candidate, deterministic normalizer and artifact binder from merged #221;
- exact `ResourceId`/`ResourceRef` identity used by the current `RuntimeNamingBridge` into L1.

Merged #221 is **tooling/candidate authority**, not trusted original-process selected-provider evidence.

## Current implementation correction still open

The current `VolumeBootstrapPlan` still models discovered pre-gap archives as `registered_archives`. Newer raw-EXE research in open #246 proves a stricter distinction:

```text
filename discovery / registration attempt
!=
successful linked runtime mount
```

Original bootstrap ignores registration return values. A discovered archive can fail registration while a later discovered pre-gap archive succeeds. Only successful registrations enter the prepend mount list used by the resolver.

Therefore the stronger semantic model is:

```text
VolumeBootstrapPlan     -> discovery / attempts only
RuntimeMountTopology    -> explicit successful physical/archive registrations
RuntimeResourceResolver -> traverses successful topology only
```

Example evidence-compatible topology:

```text
DMC3-0 exists -> mount success
DMC3-1 exists -> mount failure
DMC3-2 exists -> mount success
DMC3-3 missing -> discovery stops

resolver archive order = 2 -> 0
```

This correction is stronger unmerged evidence and remains a current product-model gap until semantically ported into current main code.

## Gate status

### L2-R1 — direct-call resolver/static provider model

**STRONG / BOUNDED-CLOSED**

Do not restart basename candidates, `0x0C`/`0x0E` normalization, archive-before-physical ordering or bounded type-0 final-open behavior absent contradictory direct evidence.

### L2-R2A — real-retail normalized-key collision census

**OPEN / EXTERNAL ARTIFACT EVIDENCE REQUIRED**

A cryptographically bound retail central-directory/member-name surface is required before claiming retail `0x0E` collision freedom or exact retail winner equivalence.

### L2-R2B — protected-process runtime mapping

**TOOLING MERGED / REAL RECEIPT OPEN**

The canonical analysis executable and protected distribution executable are different authority roles. Canonical RVAs cannot be applied to a protected process without bounded runtime mapping evidence.

### L2-R3 — trusted selected-provider identity

**CANDIDATE/BINDER TOOLING MERGED / TRUSTED ORIGINAL-PROCESS RECEIPT OPEN**

A valid promotion path requires:

```text
same protected process instance
 -> validated mapped observation anchors
 -> zero-loss resolver observation
 -> exact observer artifact binding
 -> exact mounted NBZ artifact binding
 -> trusted origin/publisher binding
 -> exact selected provider/member identity
```

Editable JSON cannot self-promote.

### L2-R4 — discovery vs successful mount topology product correction

**IMPLEMENTATION GAP / STRONGER UNMERGED EVIDENCE #246**

Current main should be corrected so resolver topology is built only from explicit successful registrations, not inferred filename discovery.

### L2-R5 — final Layer-2 audit

**OPEN**

Requires R2A, real R2B, trusted R3, R4 product reconciliation, exact-head validation and contradiction-free code/docs/evidence.

## Failure semantics that must remain explicit

A normalized archive-key hit is not identical to a usable selected resource. The wrapper/open path may fail after lookup. That provider/backend failure must fail closed; it must not be relabeled as a clean miss and silently continue to lower precedence.

Similarly:

```text
discovered filename
!= attempted registration success
!= linked mount
!= selected provider
```

These are separate evidence states.

## L2/L1 seam

`RuntimeNamingBridge` is the correct authority shape:

```text
L2 resolved ResourceId
== exact
L1 naming/materialization parent ResourceId
```

Forbidden identity joins include basename, display name, `.index` name, embedded alias and semantic suffix.

## Current work order

1. semantically port the discovery-vs-successful-mount topology correction (#246) into current-main product code;
2. retain first-gap filename discovery as discovery evidence only;
3. obtain retail central-directory/member-name evidence and run the `0x0E` collision census;
4. produce a real protected-process multi-anchor R2B packet;
5. implement/use a trusted process-bound resolver publisher/origin binder;
6. capture a zero-loss selected-provider identity and bind the exact mounted archive artifacts;
7. compare original selection with product resolution only after trusted origin is established;
8. run exact-head Windows + Ubuntu validation and final L2 audit.

## Completion rule

Layer 2 is not complete because the resolver implementation works on synthetic inputs. Completion requires trusted original-process selection evidence, retail collision/topology evidence where required, product topology semantics consistent with recovered behavior, and a final contradiction-free audit.
