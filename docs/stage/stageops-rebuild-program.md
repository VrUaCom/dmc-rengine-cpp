# Stage Ops Rebuild Program

**Status:** active architecture migration  
**Authority:** ADR-0003  
**Umbrella:** #90  
**Semantic Graph:** #53  
**Active implementation:** PR #91

## Why this migration exists

The old stage stack had no single product-side scene authority between resource materialization and tool views. `StageBundle`, `ProjectWorkspace`, Stage Ops, ModViz and planned Stage Semantic Graph could all be interpreted as partial scene representations. That made it possible for a consumer/view layer to gradually absorb orchestration responsibility.

The corrected architecture establishes one ownership chain:

```text
EXE descriptor / selector authority
        |
        v
GDSpaces resolution + materialization + ByteProvenance + expansion
        |
        v
DMC profile Stage Ops ingress
        |
        v
StageAssemblyWorkspace
        |
        +--> ProjectWorkspace resource sessions / WorkingCopies / events
        |
        v
StageOperationsSession
        |
        +--> shared typed parser results
        +--> StageDomainWorkspace
        +--> StageWorkspaceView
        +--> Stage Semantic Graph
        +--> ModViz
        +--> validation / rebuild / export
```

Recovered vanilla functions, typed post-load, factories, caches and lifetime remain in the Recovered Game Source Tree and are linked into this product model only through explicit evidence-backed contracts.

## Canonical APIs after migration

### Stage resource ingress

Canonical runtime-backed path:

```text
StageRuntimeLoader
  -> StageRuntimeLoadReport
  -> profiles::dmc3::StageOpsIngress
  -> stageops::StageAssemblyWorkspace
  + ProjectWorkspace sessions
```

`profiles::dmc3::StageWorkspaceBuilder` is compatibility-only. It remains useful for legacy payload import and synthetic fixtures but must not become a second runtime-backed stage-open path.

### Stage operational state

`stageops::StageOperationsSession` is the stage-scoped product operations controller.

It coordinates:

- edits through ProjectWorkspace WorkingCopy APIs;
- stage revision;
- conservative derived-state invalidation;
- external ProjectWorkspace change detection;
- canonical active-byte re-analysis;
- validation requests;
- derived refresh commit.

It does not own raw mutable bytes and does not implement original DMC3 runtime behavior.

### Typed format interpretation

`integration::ResourceAnalyzer` executes the canonical format adapter once per requested byte authority:

- immutable source -> `immutable-source@0`;
- active WorkingCopy -> `working-copy@revision`.

The typed result is retained in `ResourceWorkspaceSession::parsed_resource()` and reused by Stage Ops and inspection consumers. Stage Ops must not introduce a second HITS/DCA/LIG2/TXT parser path.

### Stage domain assembly

`stageops::StageDomainWorkspace` projects the retained typed resource results into stage-oriented structural domains.

Current structural domains:

- HITS collision summary;
- DCA record set;
- LIG2 lighting record set;
- Stage TXT token set;
- exact Stage TXT lexical markers for `#SET`, StageSet values, `DOOR`, `BoxIn` and `NextRoom`.

TXT lexical marker identity includes resource identity plus exact token byte offset. Marker attributes retain value, token kind, size, line and column.

These markers are not original runtime objects. Runtime meaning is added only through separate recovered-game evidence links.

### Stage Semantic Graph

The graph is disposable and deterministic. It projects:

- stage assembly identity;
- requirements;
- canonical resources;
- containment/membership;
- Stage Ops domain objects;
- later recovered-runtime/evidence relationships.

It never resolves resources or mutates the scene.

### ModViz

ModViz consumes Stage Ops assembly/domain state. It may submit edits, but scene membership and dependency ownership remain in Stage Ops.

## Migration gates

### Gate A — one stage aggregate authority

Required:

- [x] generic `StageAssemblyWorkspace`;
- [x] separate technical/numeric/semantic identity axes;
- [x] unresolved requirements retained;
- [x] unique canonical ResourceIds;
- [x] nested parent/slot relationships;
- [x] ByteProvenance preservation;
- [x] product materialization separate from original-game readiness;
- [x] DMC3 adapter outside generic Stage Ops.

### Gate B — consumers stop assembling scenes

Required:

- [x] canonical `StageWorkspaceView` projects StageAssemblyWorkspace;
- [x] canonical ModViz view projects StageAssemblyWorkspace;
- [x] consistency checker validates ModViz as Stage Ops projection;
- [x] regression proves project-only extra stage tags do not enter canonical scene;
- [ ] retire compatibility project-only view overloads after remaining callers migrate.

### Gate C — operational scene session

Required:

- [x] `StageOperationsSession`;
- [x] WorkingCopy edit/undo/reset through ProjectWorkspace;
- [x] stage revision and conservative derived stale gate;
- [x] validation fan-out;
- [x] external resource revision detection;
- [x] explicit derived refresh commit;
- [x] canonical active-byte analysis refresh path.

### Gate D — one parser result authority

Required:

- [x] typed parser result retained in ResourceWorkspaceSession;
- [x] HITS/DCA/LIG2/TXT share typed result with Stage Ops;
- [x] parser byte authority/revision retained explicitly;
- [x] source and WorkingCopy use the same structural parser adapters;
- [x] Stage Ops detects stale typed results instead of silently reparsing;
- [ ] Binary Inspector document needs an explicit byte-source/revision contract equivalent to typed parser lineage.

### Gate E — first domain scene model

Required:

- [x] collision structural domain;
- [x] DCA structural domain;
- [x] lighting structural domain;
- [x] Stage TXT aggregate domain;
- [x] exact Stage TXT lexical marker identities;
- [ ] evidence-backed Stage TXT grammar relationships where recovered;
- [ ] recovered StageSet consumer/object links;
- [ ] recovered door/transition consumer/object links;
- [ ] camera runtime links;
- [ ] effect/audio runtime links;
- [ ] enemy/spawn dependency links;
- [ ] event/demo/timeline links.

### Gate F — runtime lifecycle bridge

Required only as reverse evidence permits:

- [ ] state-2 typed post-load receipts attached by recovered-game identity;
- [ ] validated SHW/MOD/EFM post-load relationships;
- [ ] SCM kept blocked until `mesh+0x28` conflict is reconciled;
- [ ] PAC/PNST recursive typed dispatch when reconstructed;
- [ ] factory/consumer handoff identities;
- [ ] cache/ownership/lifetime observations;
- [ ] stage transition/reload/unload lifecycle receipts.

No product-side success may close these gates automatically.

## Invalidation model

Current model is intentionally conservative:

```text
WorkingCopy byte edit
  -> ProjectWorkspace revision/event
  -> StageOperationsSession stage revision++
  -> derived_state_stale = true
  -> retained typed result may become stale
  -> canonical active-byte re-analysis
  -> StageDomainWorkspace rebuild
  -> Stage Semantic Graph / ModViz projection rebuild
  -> commit_derived_refresh(expected_stage_revision)
```

As evidence-backed dependencies are added, invalidation can become selective. Until then, conservative invalidation is safer than invented dependency precision.

## Forbidden migration shortcuts

During this program do not introduce:

- a Stage Semantic Graph resource resolver;
- a ModViz scene discovery pass;
- a second Stage Ops format parser;
- a second payload copy owned by Stage Ops;
- runtime object names for lexical/parser-only structures;
- gameplay identity inferred from filenames;
- automatic promotion from product-materialized to vanilla-ready;
- new top-level PAC Editor/PAC Manager architecture;
- a new stage builder parallel to StageRuntimeLoader + StageOpsIngress.

## Representative completion proof

Before the rebuild can be considered structurally complete, one representative stage must pass this continuous product pipeline:

```text
catalog/selector selection
  -> exact GDSpaces resolution/materialization
  -> StageOpsIngress
  -> StageAssemblyWorkspace
  -> shared ProjectWorkspace sessions
  -> typed structural analysis
  -> StageDomainWorkspace
  -> Stage Semantic Graph
  -> ModViz projection
  -> WorkingCopy edit
  -> exact revision re-analysis
  -> domain/graph refresh
  -> validation request
```

The proof must then be repeated across representative Bank-A, Bank-B, shared-resource, alias/fallback and partial/unresolved cases. `st001` is only a regression fixture and never the architecture gate.
