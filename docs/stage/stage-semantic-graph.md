# Stage Semantic Graph v1

**Status:** canonical derived representation over Stage Ops assembly  
**Architecture authority:** ADR-0003  
**Tracking:** #53, #90

Stage Semantic Graph does **not** assemble or load a stage.

Its job is to expose an evidence-aware semantic/query representation of the scene state already assembled and operated by Stage Ops.

## Canonical flow

```text
EXE Stage descriptor / selector authority
        |
        v
GDSpaces resolution + bytes + provenance + container expansion
        |
        v
DMC profile Stage Ops adapter
        |
        v
StageAssemblyWorkspace                 Recovered Game Source Tree
        |                                runtime/evidence contracts
        +-------------------+-----------------------+
                            |
                            v
                   Stage Ops domain assembly
                            |
             +--------------+---------------+
             |                              |
             v                              v
   Stage Semantic Graph                 ModViz
   relation/evidence index              3D editor
```

`StageBundle` is a resource-materialization grouping/input milestone. It is not the semantic graph and it is not the complete scene.

## Role

The graph represents relationships such as:

```text
StageAssemblyWorkspace
  -> Stage / Room
  -> Geometry / Model / Texture
  -> Collision / HITS
  -> Lighting
  -> Camera
  -> Door / Transition
  -> Script / StageSet / Event
  -> Effect
  -> Audio
  -> Enemy / Spawn dependency
  -> Position / Marker
  -> Shared / aliased resource
  -> Runtime / factory / lifecycle evidence
  -> Unknown / unresolved node
```

The graph may join product-side structural facts with reverse-engineering evidence, but it must keep their authority and confidence distinct.

## Inputs

The graph consumes already-established state/identities:

- `StageAssemblyWorkspace` identity and assembly status;
- required roots, including unresolved requirements;
- unique canonical `ResourceId` instances;
- nested parent/slot memberships;
- Stage Ops domain objects and dependencies;
- ProjectWorkspace evidence/WorkingCopy/session references exposed through Stage Ops;
- Recovered Game Source Tree function/type/factory/lifecycle identities;
- Reverse Core evidence/claim/reconstruction identities;
- optional numeric Stage identity;
- semantic gameplay identity only when separately evidenced.

The graph does not own any of those sources.

## Graph authority levels

Every node/edge must be distinguishable as one of these broad classes:

1. **structural/product fact** — e.g. a ResourceId belongs to an assembled container hierarchy;
2. **recovered runtime fact** — e.g. executable evidence links a consumer/factory/lifecycle function;
3. **semantic confirmed** — gameplay meaning supported by explicit evidence;
4. **semantic inferred** — interpretation with confidence/provenance;
5. **unresolved** — known relationship/object whose meaning or target is not yet established.

Inference never auto-promotes to confirmed truth.

## Identity rules

The graph preserves independent axes:

- technical `resource_set_id / catalog_entry_id`;
- global/source catalog coordinates;
- numeric selector-facing Stage ID;
- semantic gameplay Stage/room identity;
- canonical GDSpaces `ResourceId`;
- Stage Ops domain-object identity;
- Reverse Core / Recovered Game Source identities.

No filename convention creates gameplay identity.

A shared ResourceId remains one resource node with multiple relationships; it is not cloned into invented per-stage resources.

## Forbidden behavior

Stage Semantic Graph must never:

- call GDSpaces lookup/resolution to discover missing stage resources;
- mount or traverse archives on its own;
- create a second StageBundle/scene assembly path;
- own WorkingCopy bytes;
- mutate source bytes directly;
- schedule recovered vanilla runtime behavior;
- become the cache/lifetime manager;
- create gameplay semantics from `stNNN` naming;
- describe a product object as an original Capcom runtime object without recovered evidence.

If graph construction needs information that Stage Ops does not expose, the missing capability belongs in Stage Ops or the relevant reverse/parser authority first.

## Determinism

For the same Stage Ops assembly revision and same evidence snapshot, graph output must be deterministic:

- stable node IDs;
- stable edge IDs/order;
- stable evidence references;
- explicit unresolved nodes instead of dropped relationships;
- no hidden source reads during graph construction.

This allows graph manifests to serve as comparison/debugging artifacts without becoming resource authority.

## Relationship with Stage Ops

Stage Ops is authoritative for:

- what scene is open;
- which resources/requirements belong to the scene;
- partial/complete assembly state;
- domain-object assembly;
- dependency invalidation;
- edit/reload/rebuild/validation orchestration.

Semantic Graph projects that state for navigation, reasoning, evidence linking and cross-tool queries.

The graph is therefore disposable/rebuildable; Stage Ops operational state is not.

## Relationship with ModViz

ModViz consumes:

- Stage Ops resource/domain state for actual scene editing;
- Semantic Graph relationships/evidence for navigation, overlays, selection context and semantic tooling.

ModViz edits return through Stage Ops/ProjectWorkspace mutation APIs. The graph may update after the Stage Ops revision changes; it does not own the edit.

## v1 milestone

The first implementation milestone is not `st001`-specific. It should demonstrate, with synthetic/public CI plus legal local validation as available:

1. graph construction from `StageAssemblyWorkspace` without resource resolution;
2. full preservation of technical/numeric/semantic identity axes;
3. nodes for descriptor-root requirements, including unresolved roots;
4. unique resource nodes plus nested parent/slot relationships;
5. representative typed Stage Ops domain nodes where parsers/reverse evidence already exist;
6. evidence-backed runtime links without moving runtime ownership into the graph;
7. deterministic graph rebuild;
8. one shared graph consumed by Stage Ops UI, ModViz, Binary Inspector and EXE/Reverse links.

`st001` may remain a regression fixture, but it is never the architecture or completion gate.
