# GDSpaces Resource Runtime Reconstruction

**Status:** canonical research and implementation direction  
**Scope:** DMC3 HD resource system, with reusable contracts where evidence supports them

## Purpose

GDSpaces is not merely a virtual filesystem or archive browser. It is the DMC Rengine **product authority for resource access and resource behavior**: discovery, identity, resolution, loading, expansion, classification, dependency relationships, runtime metadata, and working-copy integration.

This does **not** mean the game's resource-runtime functions belong to GDSpaces. Recovered executable functions, data, types, factories, caches, and lifetime code remain part of the **Recovered Game Source Tree** under the reconstructed game architecture.

Completing GDSpaces requires reverse engineering the executable-side resource runtime, then implementing or exposing the confirmed behavior behind GDSpaces contracts. Reverse Core stores generic evidence/reconstruction records; the recovered game source tree stores the reconstructed game code; GDSpaces consumes those findings as product infrastructure.

See [Recovered Game Source Tree](../reverse-core/game-source-tree.md) and [DMC3 Vanilla Deep Research Wave 2](../research/dmc3-vanilla-deep-research-wave-2.md).

## Separation of responsibilities

```text
DMC3 executable resource-runtime code
        |
        v
Recovered Game Source Tree
  resource-runtime functions / data / types
        |
        v
Reverse Core evidence + reconstruction identities
        |
        v
GDSpaces product implementation and resource API
        |
        v
Binary Inspector / Stage Ops / ModViz / Item Editor / other consumers
```

A game loader function may inform GDSpaces behavior without becoming a `GDSpaces` function. Tool linkage and game-subsystem membership are separate relationships.

## Reconstruction target

The complete target pipeline is:

```text
game startup / subsystem request
  -> source and archive discovery
  -> logical name / identifier lookup
  -> index or table lookup
  -> archive / volume selection
  -> compressed/raw byte acquisition
  -> decompression / extraction where applicable
  -> nested container expansion
  -> child identity and slot preservation
  -> typed post-load traversal / in-place normalization
  -> classification / factory dispatch
  -> dependency discovery
  -> runtime allocation / object construction
  -> cache / ownership / lifetime registration
  -> ready-state consumer handoff
  -> reload / transition behavior
  -> release / unload / shutdown behavior
```

Every arrow is a reverse-engineering target. A parser that can read bytes or enumerate PAC/PNST children is not sufficient proof that the runtime system is understood. Wave 2 directly confirms a higher-level 363-entry load manager and a state-2 -> typed post-load fixup -> state-3 ready boundary, so raw materialization/expansion must not be labeled game-ready Level C.

## Wave 2 Stage/runtime constraints

The earlier single 110 x 4 Stage-table model is retained only as a historical/compatibility subset. Stronger direct evidence establishes:

- a corrected `StageResourceCell` ABI: `kind16 @ +0x00`, unresolved bytes `+0x02..+0x07`, resource path pointer `@ +0x08`, stride `0x10`;
- Bank A: 110 descriptors at `VA 0x1405C4AA0`;
- Bank B: 79 descriptors at `VA 0x1405C3080`;
- 189 observed descriptors in total, without claiming 189 independent gameplay stages;
- a 193-entry selector space at `VA 0x1405C4440` and a 10-pointer group-base table at `VA 0x1405C4A50` used by numeric Stage-ID resolution;
- `kind16 == 0` has an evidenced original-path -> `.lst` fallback branch;
- a 363 x `0x48` higher-level resource/load pool at `VA 0x140C99D30` with observed states `0 -> 1 -> 2 -> 3` and teardown state `4`;
- typed post-load fixup paths for at least MOD/EFM/SCM/SHW, recursively reached through PNST and PAC traversal.

Until selector/fallback semantics are fully reconstructed, the current 110-row `StageCatalog` implementation is explicitly **Wave-2 Bank-A compatibility coverage**, not the complete DMC3 Stage universe. `st001` may remain a regression fixture only.

## Required reverse passes

### 1. Bootstrap and source discovery

Determine from executable evidence:

- which resource systems are initialized at process/game startup;
- which game roots, archive sets, volume tables, or built-in tables are registered;
- initialization ordering and dependencies;
- profile differences between DMC1/DMC2/DMC3/launcher where relevant;
- failure and fallback behavior when expected sources are missing.

Recovered functions discovered here remain game bootstrap/resource-runtime source, even when GDSpaces later reproduces the behavior.

### 2. Request and lookup ABI

Recover the request path from a gameplay/system caller to the resource runtime:

- caller-facing function signatures;
- path/name/ID representation;
- normalization rules;
- case and slash behavior;
- hash/table/index use where present;
- duplicate-name behavior;
- lookup priority between mounted sources;
- Stage descriptor `kind16` behavior;
- `.lst` fallback and selector/group behavior;
- fallback and error semantics.

No hash algorithm, lookup priority, selector fallback meaning, complete `.lst` grammar, or `.index` role is to be invented without evidence.

### 3. Archive and volume runtime

Reverse the runtime handling of the actual source/container stack, including the evidenced subsets of:

- NBZ;
- AFS;
- PAC;
- PNST;
- `.lst`-driven PAC indirection where evidenced;
- loose/local files where the game supports them;
- nested container chains.

For each layer recover:

- header/table interpretation;
- entry identity;
- slot/index semantics;
- offsets/sizes/alignment;
- compression or transformation boundaries;
- duplicate and empty entry semantics;
- malformed-data behavior;
- ownership of buffers returned to upper layers.

Container formats remain implementation details inside GDSpaces rather than top-level editors. The original game parsing/loading functions remain recovered game code.

### 4. Byte acquisition and transformation

Recover the path from archive entry to usable byte span:

- file/volume I/O calls;
- buffering strategy where observable;
- decompression entry points and parameters;
- temporary versus retained buffers;
- alignment requirements;
- copy versus view behavior;
- error codes and partial-read behavior.

### 5. Typed post-load normalization, classification, and factory dispatch

Determine how materialized bytes become **ready** typed runtime resources:

- traversal of PAC/PNST roots and populated members;
- MOD/EFM/SCM/SHW post-load fixup contracts;
- relative-offset -> in-memory-pointer normalization behavior;
- magic/extension/table-driven dispatch;
- resource factory functions;
- type IDs or registration tables;
- constructors/destructors;
- unknown-type behavior;
- dependencies triggered by construction;
- EXE-backed semantic identity where the game uses tables instead of filenames.

The current product-side `StageRuntimeLoader` materializes bytes, validates provenance, and recursively expands containers. That is a useful bounded product composition, but it is **not yet game-ready Level C equivalence** until the evidenced post-load normalization/factory phase is represented and validated.

### 6. Runtime ownership, cache, and lifetime

This is mandatory for a complete reconstruction.

Recover evidence for:

- the 363-entry higher-level resource manager and per-group subtype contracts;
- cache key and cache lookup behavior;
- whether duplicate requests reuse one object/buffer;
- ownership model;
- completion/callback fields;
- reference counts or equivalent lifetime mechanisms where present;
- pinning/permanent resources;
- stage/room scoped resources;
- allocation arenas or pools where observable;
- state-4 teardown/cancellation behavior;
- destruction/release paths;
- shutdown ordering.

Readable loader code without lifetime evidence is not considered a complete reconstruction.

### 7. Dependency graph

Recover explicit and implicit resource dependencies:

- parent container -> child resource;
- Stage descriptor -> four role resources;
- cross-stage descriptor aliasing such as `st600–st612` effect/sound reuse;
- model -> texture/material/animation dependencies;
- UI/HUD -> atlas/model/runtime-value dependencies;
- script/event -> stage/runtime resources;
- executable tables/selectors -> semantic resource identities.

GDSpaces `ResourceGraph` is the product representation of confirmed or confidence-tagged relationships. Inferred relationships must remain distinguishable from confirmed runtime edges.

### 8. Stage transition, reload, and unload behavior

Trace representative lifecycle events:

- initial stage load;
- room transition;
- stage transition;
- restart;
- reload;
- return to menu;
- shutdown.

Record 363-pool state changes and what is retained, replaced, released, or reconstructed at each boundary.

### 9. Error and fallback behavior

Reverse failure paths deliberately:

- missing primary resource;
- `kind16 == 0` `.lst` fallback;
- unknown resource type;
- invalid archive entry;
- corrupt/truncated data;
- duplicate logical identity;
- failed dependency;
- failed allocation/decompression;
- failed typed post-load normalization;
- fallback/default resource behavior where present.

The product model must preserve diagnostics instead of silently normalizing observed failures away.

## Evidence model

Each recovered runtime behavior must link to:

- exact executable artifact identity;
- function/range/data-table identity;
- static xrefs/call graph where applicable;
- runtime observation or controlled experiment when needed;
- confidence state;
- correction/supersession history;
- reconstructed source revision when promoted;
- semantic game-subsystem membership where known;
- tool relationships separately from subsystem membership.

Reverse Core owns the generic evidence/reconstruction records. The Recovered Game Source Tree holds the reconstructed game source. GDSpaces owns the resulting DMC Rengine resource API and product behavior.

## Product mapping

```text
Recovered executable resource behavior
        |
        v
Recovered Game Source Tree
        |
        +--> Reverse Core evidence + reconstruction records
        |
        v
GDSpaces implementation
  - SourceRegistry
  - source/volume adapters
  - ResourceId / ResourceRef / ResourcePayload
  - container expansion
  - classifier/factory metadata
  - ResourceGraph
  - runtime/lifetime metadata
  - OpenRouter
  - typed bundles/workspaces
        |
        v
Binary Inspector / Stage Ops / ModViz / Item Editor / EXE-linked tools
```

Tools never bypass GDSpaces because a reverse finding exposes a convenient path or archive offset.

## Completion levels

### Level A — structural read

A source/container can be parsed safely and resources can be enumerated.

### Level B — lookup equivalence

GDSpaces reproduces evidenced game lookup/identity rules for a bounded corpus, including applicable selector/descriptor and fallback branches.

### Level C — load-path reconstruction

Request -> source -> bytes -> transform -> container traversal -> typed post-load normalization -> ready typed resource is understood and behavior-tested.

A materialized/expanded `StageBundle` without the state-2 -> state-3 post-load phase is an intermediate product milestone, not full game-ready Level C.

### Level D — lifetime reconstruction

Cache, reuse, ownership, dependency, reload, state-4 teardown, and unload behavior are recovered for the game resource-runtime subsystem.

### Level E — validated runtime model

Representative game lifecycle tests agree with the reconstructed model and produce deterministic ValidationReceipts.

A resource-runtime subsystem is not called "fully reversed" until the relevant Level E gates are satisfied or unresolved behavior is explicitly documented.

## Current practical proof program

The integrated proof must be selected from the **reconciled executable Stage descriptor/selector model**, not from a hard-coded `st001` family:

```text
canonical EXE
  -> corrected Stage descriptor banks
  -> numeric Stage selector/group mapping when reconstructed
  -> representative descriptor/resource-set selection
  -> recovered game resource-runtime request path
  -> evidenced logical lookup + kind16 fallback behavior
  -> production NBZ/AFS/PAC/PNST materialization
  -> nested resources
  -> typed post-load normalization / ready state
  -> GDSpaces StageBundle / Stage Semantic Graph projection
  -> transition/cache/lifetime observations
  -> deterministic ValidationReceipt
```

Representative coverage must include conventional descriptors, cross-stage/shared dependency patterns, fallback/special cases where evidence permits, and multiple lifecycle boundaries. The current Bank-A 110-row catalog may supply compatibility/regression cases while selector-derived full-universe coverage is unfinished. `st001` is one possible fixture only and never the architecture or completion gate.

## Non-goals

- treating recovered game loader functions as GDSpaces-owned code;
- inventing a new resource runtime unrelated to observed game behavior;
- moving archive/resource resolution into Reverse Core;
- tool-local archive readers or path resolvers;
- declaring a parser complete because one file opens;
- assuming `.index` is runtime truth without executable evidence;
- forcing the 110-row Bank-A subset to stand for the full Stage universe;
- inventing selector fallback or `.lst` ownership semantics;
- reviving PAC Editor/PAC Manager product architecture;
- requiring byte-identical internal implementation when behavioral equivalence is sufficient and explicitly documented.
