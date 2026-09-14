# Current Project Status

**Snapshot date:** 2026-09-13  
**Canonical implementation base reviewed:** `main@64d2d27db4b4b974addcc0e78c342fb169e2fd97`  
**Latest reviewed public/status promotion:** PR #377 — canonical model-format truth + discovery reconciliation  
**Latest SCM reverse completion:** `reverse/mod-completion-20260907` — terminal field/runtime map + machine receipt  
**Latest SCM authoring promotions:** bounded real alpha/translation/rotation authoring + PAC reintegration receipts  
**Latest MOD/SCM/MOT consolidation:** PR #372 — unified model-format consolidation and MOD container gates  
**Active branch advancement:** `reverse/mod-completion-20260907`  
**Latest canonical Native Reader integration review:** `NativeReaderModuleRegistry` verified on current `main`  
**Primary execution program:** proof-gated L2 -> L1 -> L3 vertical acceptance  
**Overall status:** L1/L2/L3 remain incomplete. The canonical DMC3 HD MOD and SCM reverses are complete for their defined serialized/runtime scopes; writer/integration/original-game acceptance remain separate bounded programs. MOT continues on its runtime/player proof track.

## Authority split

- GitHub `main` is canonical implementation truth.
- A PR branch is branch truth until promoted.
- Reverse claims are bounded to exact artifact/address/range/scope.
- Retail corpus claims require hash/provenance-bound receipts.
- Synthetic/public CI proves DMC Rengine behavior only.
- Original-game equivalence requires the appropriate original-process/consumer evidence.
- Discovery/SEO pages are navigation/readability surfaces, never a second technical authority.

## Canonical executable authority

Canonical analysis `dmc3.exe`:

- SHA-256 `e454272ed0fb0247fcbcf300e5d55d7a3e96d50b89b9ffaff81bb978dcbdd082`;
- size 6,356,432;
- PE32+ x86-64;
- ImageBase `0x140000000`.

The protected distribution execution candidate remains a separate build and requires independent RVA/process mapping before runtime identity claims.

## Native Reader current state

The directly verified `NativeReaderModuleRegistry` registers:

- DDS;
- PTX;
- HITS;
- DCA;
- LIG2/LIG;
- Stage TXT;
- SCM;
- MOD;
- MOT;
- SO graph;
- SO volume;
- SO link;
- SHW;
- PE/EXE.

Registry membership is a product-integration fact; it does not by itself grant complete semantic or writer authority.

EFM is still not a canonical Native Reader module. MRP/MCV/CAM/CLT/TSC and other recognized families remain evidence-gated.

SHW remains structural/read-only. Matrix-palette ownership, universal revision coverage and writer/original-game acceptance remain open.

## Executable structural reverse state

### Structural reverse — COMPLETE for declared structure

Every PE structure the canonical target declares is now parsed and recorded as evidence: headers, the data-directory array, imports, exports, the exception directory, the debug directory, base relocations, TLS and resources, plus the MSVC RTTI class graph. See [the structural reverse](../reverse/dmc3-exe-structural-reverse-2026-09-13.md) and [`dmc3-hdc-structural-reverse.evidence.json`](../../evidence/executable/dmc3-hdc-structural-reverse.evidence.json).

Recovered and confirmed:

- build identity: CodeView GUID `8DACCD58-89B6-4E0C-9B74-EC2A1292FC90` age 1, `C:\dev\dmc\dmc3\build\x64\dmc3.pdb`;
- 7,389 functions across 12,235 unwind ranges, covering 89.1 percent of `.text`;
- 26 imported modules / 229 functions; exports `dmc3_main` (RVA `0x2C5DF0`) and `FMODGetCodecDescription`;
- 408 RTTI type descriptors, 915 complete-object locators, 396 distinct polymorphic types, 915 vtables;
- object spine: `CWork` under 264 of 396 types, `CActor` / `IActor` / `ICollisionHandle` under ~180 each.

### Function map — INITIAL

Instruction-accurate walks, the call graph and mechanical attribution are in place. See [the function map](../reverse/dmc3-function-map-2026-09-14.md) and [`dmc3-hdc-function-map.evidence.json`](../../evidence/executable/dmc3-hdc-function-map.evidence.json).

- x86-64 length decoder, fail-closed, cross-validated against objdump at 875,067 positions with 99.93 percent agreement (every disagreement in embedded data);
- recursive-descent walks with switch recovery: 680,902 instructions over 3,042,423 bytes — **98.6 percent of the unwind extent** — 28,866 call edges, 27,821 data references, 7,381 of 7,389 functions complete;
- 637 switch dispatch tables recovered yielding 6,867 block addresses; 1,177 indirect jumps remain unresolved;
- prologue facts per function from unwind codes: 694,008 bytes reserved in total, 308 functions with a frame pointer, 2,210 declaring a handler (matching an independent flag census);
- **correction:** 12,235 exception-directory entries resolve to 7,389 functions plus 4,846 continuation ranges. Supersedes `ev-dmc3-function-inventory`;
- 2,889 functions attributed: 1,879 bound to a class vtable slot, 710 referencing a class vtable (construction sites), 535 calling imports, 160 referencing literals;
- dispatch census: 10,958 indirect call sites across 2,070 functions over 145 distinct offsets;
- resource-name literals are packed into data tables and shader bytecode, not code constants — measured, and it bounds what literal attribution can yield;
- 222 fixed-width name arrays holding 5,692 entries recovered, stride sized to each run's longest name; layout only, contents not extracted;
- 352-byte cutscene localisation record resolved exactly: 32-byte archive name plus eight 40-byte per-language message names, validated across all 49 records — and independently re-derived by the gap-period detector;
- 25 multi-field name record layouts recovered holding 5,076 name fields, with 2, 4, 9, 12, 16 or 32 fields per record; largest is an 80-byte four-field record repeated 119 times;
- extension census adds `.adx`, `.ogg`, `.sfd`, `.fxh` and `.tm2` to the family classifier; `.sfd` corroborates the FMV subsystem alongside RTTI classes and Media Foundation imports;
- three candidate resource-resolution functions recorded at `high` confidence (NBZ volume path, PTX extension matching, MOT/CLT extension matching);
- functions with no structural referrer at all fell from 2,101 to 848 once switch edges existed;
- 3,588 of 13,894 vtable slots bound to inventoried functions.

### Open

- **no function semantics.** The map attributes and counts; it names and explains nothing. A function known to be slot 7 of `CEm010`'s third vtable and to call `sqrtf` still has no established behavior. Per-function recovery is a separate program;
- **reachability is a floor, not a measure.** 2,249 of 7,389 functions are structurally reachable; the rest are reached by virtual dispatch, function pointers and callbacks that the graph cannot follow. This is not evidence of dead code;
- **virtual call sites are the largest remaining gap.** 10,958 dispatch sites carry offsets but no receiver type, and 1,177 indirect jumps stay unresolved. Construction sites give the next foothold: a function installing a class's vtable is handling that class;
- **no field layouts.** RTTI supplies subobject offsets, not members;
- **renderer surface invisible to imports.** D3D11 is reached through COM vtables; recovering those is separate work;
- subsystem attribution from the dependency set is recorded at confidence `high`, not `confirmed`.

## Runtime host layer state

### Implementation status — INITIAL

A new responsibility boundary (`DMCRengine::Runtime`, Specification 010) providing platform surface/lifecycle, a fixed-step frame loop and a rendering device abstraction. Host-validated on Ubuntu and Windows across C++20 and C++23; the C++26 configuration steps down automatically where the toolchain lacks it.

Implemented and covered by tests:

- `IPlatform` with `HeadlessPlatform` and `AndroidPlatform`;
- `FrameClock` fixed-step accumulator with bounded catch-up;
- `IRenderDevice` with the `NullRenderDevice` reference backend;
- `RenderBackendRegistry` distinguishing declared from implemented backends;
- `ResourceBridge` over `SourceRegistry`;
- `StageHost` mirroring a Stage Ops `StageBundle`;
- `RuntimeApplication` loop with surface-loss, suspend/resume and focus handling;
- Android Gradle project, JNI bridge and Java shell.

### Open

- **no GPU backend.** Vulkan, GLES, D3D11 and Metal are declared; creating one fails closed;
- **no device acceptance.** CI assembles the Android debug APK; no physical-device acceptance record exists;
- **no game logic.** The runtime executes no DMC3 behavior and makes no equivalence claim. Original runtime behavior remains L3 work under the Recovered Game Source Tree;
- **Article VII amendment pending.** The Constitution's responsibility-boundary list does not yet name a runtime host; promotion beyond initial implementation requires that amendment.

The runtime is not a resolver and not a scene authority. `ResourceBridge` is the only unit in the layer that reads bytes, and `StageHost` replaces its mirror wholesale on rebind rather than merging.

## MOD / model-family state

### Reverse status — COMPLETE

**The DMC3 HD MOD reverse is complete for the canonical project scope.**

The reverse is considered closed because every relevant serialized/runtime domain now has a terminal evidence state. Terminal outcomes include typed semantics, explicit family-sensitive boundaries, `PRESERVED_UNDECODED`, `RESERVED_OBSERVED_ZERO` and `REJECTED` hypotheses. Preservation-only fields are therefore not open reverse blockers simply because no unsupported artistic/material label was invented.

The completed canonical MOD map includes:

- document/object/mesh ABI;
- hierarchy/order domain and default-joint behavior;
- local/world transforms and inverse-rest/current-world skin palette construction;
- position/normal/fixed-point UV, blend-index and packed skin/topology streams;
- texture slot + legacy GS CLAMP state;
- external texture-companion/runtime-descriptor ownership and binding validation;
- object runtime projection;
- post-load relocation and generated topology behavior;
- MOD-side motion-group / MOT/CMotion boundary;
- source-byte preservation rules for every field whose strongest honest semantic is preservation-only.

Important terminal preservation outcomes include header `+0x14`, `BLENDINDICES.x`, source flag `0x00200000`, mesh `+0x0C/+0x38/+0x4C` and transform `+0x1C`. These are closed reverse results, not unfinished MOD research.

### Writer/container state — bounded, separate from reverse completion

Canonical authoring evidence includes:

- PR #365 — Preserve-Layout Writer Gate 1 with immutable-source binding and authorized-byte-span enforcement;
- PR #367 — deterministic writer corpus runner;
- PR #368 — 38/38 provenance-bound retail no-op exact byte parity across 882,736 bytes;
- PR #369 — one provenance-bound real retail `bounding_radius` edit with exactly three changed bytes and independent raw-diff/reopen validation;
- PR #369 — MOD writer receipt -> `AuthoredChildImage` trust bridge + synthetic PAC reintegration;
- PR #372 — provenance-bound real retail PNST reintegration of the authored MOD child: physical slot 23 in `m20_s00_012.pac`, unchanged parent size/slot table, only the expected three parent bytes changed, exact authored MOD recovered after canonical reparse/re-expand;
- PR #372 — synthetic MOD -> container -> next-volume NBZ overlay -> reopen gate using existing NBZ infrastructure.

Still open are typed-IR-only layout synthesis/reflow, transform/skin/material/texture-companion authoring, broader preservation-only-field mutation authority, provenance-bound **retail NBZ** overlay acceptance and original `dmc3.exe` no-op/edited MOD acceptance.

Those are writer/integration/acceptance gates and do not reopen the completed MOD reverse.

## SCM state

### Reverse status — COMPLETE

**The DMC3 HD SCM reverse is complete for the canonical serialized/runtime scope.**

Completion is recorded by:

- `docs/research/dmc3-scm-reverse-completion-2026-09-13.md`;
- `data/reverse/dmc3-scm-reverse-completion-20260913.json`;
- the reconciled canonical `docs/formats/scm.md`.

The completion rule matches the MOD standard: every relevant field/domain must terminate in a typed/structural semantic, a preservation-only state (`PRESERVED_UNDECODED` / `RESERVED_OBSERVED_ZERO`) or a rejected hypothesis. Human-friendly names are not required for bytes whose strongest honest result is exact preservation.

Closed SCM domains include:

- complete header/object/mesh/scene serialized ABI and physical layout;
- `LegacyResourceCode` arithmetic structure and runtime carry;
- `alpha_control` packet/shader projection;
- object runtime flag projection including GS TEX1 filter and the shared GS TEST_1/ZBUF_1 technical selector;
- terminal `PRESERVED_UNDECODED` disposition for source bit `0x00200000` after whole-image provenance rejection of false consumers;
- GS CLAMP REGION_REPEAT fields;
- position/normal/fixed-point UV/RGB-topology streams;
- generated index-workspace algorithm;
- scene hierarchy/order/object binding;
- SCM-specific local transform path `0x1402FA360`, XYZ radians and `Rz*Ry*Rx`;
- local-to-world relation `world = local * parentOrRootWorld`;
- external texture-companion ownership and mesh-slot binding;
- terminal preservation contracts for zero/dormant header, object, mesh, scene-shell and transform lanes.

Historical claims that `object+0x01`, mesh `+0x04..+0x0B`, SCM world composition or the SCM transform initializer remain unresolved are superseded by later evidence.

### Writer/container state — bounded, separate from reverse completion

The selected canonical SCM authoring stack includes:

- `preserve_layout` source-bound same-layout authoring;
- deterministic `canonical_rebuild` typed-IR layout planning;
- typed geometry, normal, UV, texture-slot, alpha, nearest-filter, GS CLAMP REGION_REPEAT and node transform edits;
- dependent metadata derivation;
- mandatory canonical output reparse;
- source-bound mutation guards;
- fail-closed canonical reflow when non-zero unmodeled source bytes exist;
- bounded SCM/texture-companion coherence through `ScmResourceBundleWriter` and existing texture framing/reflow.

Consolidated no-edit corpus receipt:

```text
paths                           78
unique SHA-256 inputs           68
parse                           78/78 PASS
preserve-layout exact parity    78/78 PASS
canonical rebuild + reparse     78/78 PASS
canonical exact no-edit parity  78/78 PASS
```

Real hash-bound same-layout receipts additionally cover alpha-control, node translation and node rotation through parent PAC reintegration, canonical reopen/extraction and exact inverse restoration.

Still open are real-retail size-changing rebuild, provenance-bound retail texture rewrite, broader PAC/PNST/NBZ authored-delivery coverage and original `dmc3.exe` acceptance. These are writer/integration/acceptance gates and do not reopen SCM reverse.

## MOT state

MOT is present in the canonical `NativeReaderModuleRegistry` as `native_reader_modules::mot()`. PR #372 consolidates the underlying structural path onto one modular parser/IR and advances semantic recovery. The current `reverse/mod-completion-20260907` branch carries additional bounded runtime recovery that remains branch truth until promoted.

Current canonical/branch evidence includes:

- `MOT\0` marker and aligned header/channel-mask contract;
- nine-bit channel mask and record-count/popcount relationship;
- typed compression-2 and compression-3 key payloads;
- three hash-bound real MOT payloads parsing through the modular path;
- exact normal-path binding traversal and serialized track ordinal consumption;
- exact Translation/Rotation/Scale channel semantics and CMotionJoint channel-base offsets;
- signed track start-time offsets;
- quantization `raw * range / 65535 + min`;
- compression-3 cached forward/backward segment search at `0x1402E8C80..0x1402E8E10`;
- endpoint and cache-dependent duplicate-time behavior in that recovered static search;
- compression-3 linear/Hermite segment evaluation with slope orientation;
- bounded composition from serialized compression-3 track -> semantic joint channel -> EXE-confirmed MOD motion group.

The current analysis API therefore closes the following bounded normal path:

```text
MOT mask
 -> serialized track ordinal
 -> MOD/CMotion node
 -> semantic T/R/S channel
 -> cached key selection
 -> decoded/interpolated scalar
 -> selected motion group
```

The interpolation/search helpers are semantic/static recoveries, not a claim of bit-identical SSE execution or complete CMotion player parity.

Still open:

- execution/differential confirmation of cache lifecycle and bit-identical SSE parity where required;
- header flag `0x2` alternate binding at `0x140310CBF`;
- compression-2 whole-track evaluation parity and other compression modes;
- exact mutable CMotion channel-state ownership;
- T/R/S channel state -> animated local matrix construction around `0x14030E9B0`;
- looping, blending, motion selection and scheduler/cache lifecycle;
- original-game output comparison;
- edited MOT authoring/original-game acceptance.

The next direct-EXE gate is explicitly encoded in `data/reverse/dmc3-mot-local-matrix-window-plan.v1.json`. Known scale evidence reaches the matrix path and `joint+0x110`, but that storage is not promoted to a final animated-local matrix until fresh canonical bytes close exact ownership and write ordering.

Once a trustworthy animated-local matrix exists, the downstream MOD path is already recovered:

```text
animatedLocal
 -> currentWorld
 -> inverseRestWorld * currentWorld
 -> skin palette
```

## SO module state

`NativeReaderModuleRegistry` also contains `so_graph`, `so_volume` and `so_link`. Their registry presence is canonical product integration. It must not be inflated into universal SO semantic/writer authority; cross-resource MOD/SO semantics remain evidence-gated by their dedicated analysis modules and receipts.

## L1 — Resource Materialization

Closed/advanced evidence includes NBZ/PAC/PNST product paths, authoring infrastructure, MOD Writer Gate 1, real retail PNST reintegration and synthetic NBZ reopen.

Mandatory remaining acceptance chain:

1. trusted original resolver-selected provenance;
2. exact representation classification for that selected lineage;
3. same-lineage authored overlay construction and original-runtime selection;
4. exact rematerialization to the authored child;
5. deterministic original consumer-visible effect;
6. rollback proving original retail immutability;
7. final L1 audit.

**L1 COMPLETE = NO.**

## L2 — Resource Resolution

Reverse-backed resolver/mount behavior and the exact `dmc3-0.nbz` zero-collision census remain canonical. Wider collision scope, real protected-process R2B mapping, trusted R3 selected-provider identity, direct-retail original resolver winner and final L2 audit remain open.

**L2 COMPLETE = NO.**

## L3 — Original Runtime / Lifecycle

The bounded LoadedResource spine remains canonical: materialization success -> state1 -> state2 -> typed post-load/callback -> state3, with bounded cancellation/quiescence/release rules.

Still open: current-main R1 promotion, family/backing ownership, exact scheduler terminal dependency, V1–V7 original-process lifecycle receipts, original `dmc3.exe` authored-resource selection/consumption and final L3 audit.

**L3 COMPLETE = NO.**

## Current P0 track

```text
OpenGameResource(request)
 -> successful mount topology
 -> selected provider/volume/member
 -> exact materialized bytes
 -> PAC/PNST expansion
 -> typed post-load
 -> LoadedResource state3
 -> deterministic consumer-visible effect
 -> repeat with authored higher-numbered NBZ
 -> rollback
```

The MOD/SCM reverse-complete status does not replace this cross-layer original-process acceptance chain.

## Discovery/publication state

The discovery surface remains production-active and separate from technical authority. GitHub Pages deployment/HTTP acceptance, sitemap/site-index parity, social metadata and IndexNow transport are product/discovery signals only; they do not change L1/L2/L3 completion.

## Navigation

- [Project roadmap](../roadmap.md)
- [Proof roadmap](../gdspaces/proof-roadmap-2026-09-05.md)
- [Canonical L1 roadmap](../gdspaces/l1-roadmap.md)
- [Three-layer master roadmap](../gdspaces/master-roadmap.md)
- [Blockers](blockers.md)
- [Machine-readable status](canonical-status.json)

No percentage, parser success, synthetic fixture, CI green state, successful rebuild or crash-free launch overrides the gate-based completion rule.
