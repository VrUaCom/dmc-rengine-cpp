# GDSpaces Resource Runtime Reconstruction

**Status:** canonical research and implementation direction  
**Scope:** DMC3 HD resource system, with reusable contracts where evidence supports them

## Purpose

GDSpaces is not merely a virtual filesystem or archive browser. It is the DMC Rengine authority for reconstructing how the game discovers, identifies, resolves, loads, expands, classifies, links, owns, caches, instantiates, reloads, and unloads resources.

The current C++ implementation is intentionally incomplete. Completing GDSpaces requires reverse engineering the executable-side resource runtime, not inventing replacement behavior from file formats alone.

Reverse Core may store evidence about resource-runtime functions and recovered source, but it must not create a second resource layer. Confirmed resource behavior is implemented behind GDSpaces contracts.

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
  -> type / magic / semantic classification
  -> dependency discovery
  -> runtime allocation / object construction
  -> cache / ownership / lifetime registration
  -> consumer handoff
  -> reload / transition behavior
  -> release / unload / shutdown behavior
```

Every arrow is a reverse-engineering target. A parser that can read bytes is not sufficient proof that the runtime system is understood.

## Required reverse passes

### 1. Bootstrap and source discovery

Determine from executable evidence:

- which resource systems are initialized at process/game startup;
- which game roots, archive sets, volume tables, or built-in tables are registered;
- initialization ordering and dependencies;
- profile differences between DMC1/DMC2/DMC3/launcher where relevant;
- failure and fallback behavior when expected sources are missing.

### 2. Request and lookup ABI

Recover the request path from a gameplay/system caller to the resource runtime:

- caller-facing function signatures;
- path/name/ID representation;
- normalization rules;
- case and slash behavior;
- hash/table/index use where present;
- duplicate-name behavior;
- lookup priority between mounted sources;
- fallback and error semantics.

No hash algorithm, lookup priority, or `.index` role is to be invented without evidence.

### 3. Archive and volume runtime

Reverse the runtime handling of the actual source/container stack, including the evidenced subsets of:

- NBZ;
- AFS;
- PAC;
- PNST;
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

Container formats remain implementation details inside GDSpaces rather than top-level editors.

### 4. Byte acquisition and transformation

Recover the path from archive entry to usable byte span:

- file/volume I/O calls;
- buffering strategy where observable;
- decompression entry points and parameters;
- temporary versus retained buffers;
- alignment requirements;
- copy versus view behavior;
- error codes and partial-read behavior.

### 5. Classification and factory dispatch

Determine how loaded bytes become typed runtime resources:

- magic/extension/table-driven dispatch;
- resource factory functions;
- type IDs or registration tables;
- constructors/destructors;
- unknown-type behavior;
- dependencies triggered by construction;
- EXE-backed semantic identity where the game uses tables instead of filenames.

### 6. Runtime ownership, cache, and lifetime

This is mandatory for a complete reconstruction.

Recover evidence for:

- cache key and cache lookup behavior;
- whether duplicate requests reuse one object/buffer;
- ownership model;
- reference counts or equivalent lifetime mechanisms where present;
- pinning/permanent resources;
- stage/room scoped resources;
- allocation arenas or pools where observable;
- destruction/release paths;
- shutdown ordering.

Readable loader code without lifetime evidence is not considered a complete reconstruction.

### 7. Dependency graph

Recover explicit and implicit resource dependencies:

- parent container -> child resource;
- stage -> room resources;
- model -> texture/material/animation dependencies;
- UI/HUD -> atlas/model/runtime-value dependencies;
- script/event -> stage/runtime resources;
- executable tables -> semantic resource identities.

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

Record what is retained, replaced, released, or reconstructed at each boundary.

### 9. Error and fallback behavior

Reverse failure paths deliberately:

- missing resource;
- unknown resource type;
- invalid archive entry;
- corrupt/truncated data;
- duplicate logical identity;
- failed dependency;
- failed allocation/decompression;
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
- reconstructed source revision when promoted.

Reverse Core owns these generic evidence/reconstruction records. GDSpaces owns the resulting game-resource behavior and public resource API.

## Product mapping

```text
Recovered executable resource behavior
        |
        v
Reverse Core evidence + reconstruction records
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

GDSpaces reproduces evidenced game lookup/identity rules for a bounded corpus.

### Level C — load-path reconstruction

Request -> source -> bytes -> transform -> typed resource is understood and behavior-tested.

### Level D — lifetime reconstruction

Cache, reuse, ownership, dependency, reload, and unload behavior are recovered for the subsystem.

### Level E — validated runtime model

Representative game lifecycle tests agree with the reconstructed model and produce ValidationReceipts.

A subsystem is not called "fully reversed" until the relevant Level E gates are satisfied or unresolved behavior is explicitly documented.

## First practical slice

Use the existing `st001` path as the first integrated proof:

```text
canonical EXE stage request/table
  -> evidenced logical resource identities
  -> production NBZ/AFS/PAC/PNST read path
  -> nested typed resources
  -> StageBundle
  -> Stage Semantic Graph
  -> runtime load/transition observations
  -> cache/lifetime evidence
  -> deterministic validation receipt
```

This closes both file-level and executable-runtime understanding without creating a parallel resource subsystem.

## Non-goals

- inventing a new resource runtime unrelated to observed game behavior;
- moving archive/resource resolution into Reverse Core;
- tool-local archive readers or path resolvers;
- declaring a parser complete because one file opens;
- assuming `.index` is runtime truth without executable evidence;
- reviving PAC Editor/PAC Manager product architecture;
- requiring byte-identical internal implementation when behavioral equivalence is sufficient and explicitly documented.
