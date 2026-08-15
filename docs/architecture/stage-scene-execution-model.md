# Stage Ops scene execution model

## Purpose

This document defines the canonical operational lifecycle for a DMC Rengine
stage/scene after GDSpaces has materialized the selected resource set.

It applies to Stage TXT today and is the required integration model for future
camera, collision, geometry, lighting, enemy, event, effect, audio and other
stage domains.

It does **not** define original Capcom runtime scheduling. Vanilla behavior is
owned by `recovered-game`; this document defines how the DMC Rengine product
coordinates recovered knowledge, editable bytes and scene projections.

## Ownership

```text
GDSpaces
  owns ResourceId / source lookup / bytes / ByteProvenance / containers

ProjectWorkspace
  owns per-resource session / WorkingCopy / parser result / events

StageOperationsSession
  owns stage revision + stage-scoped mutation/orchestration state

StageDomainKnowledgeWorkspace
  owns product-side domain objects + structural relations + explicit runtime links

StageSceneSnapshot
  owns no mutable authority; it is an immutable revision-coherent publication

Stage Semantic Graph
  is a deterministic projection of Stage Ops knowledge

ModViz / UI / MCP agents
  consume snapshots and submit edit/refresh commands

Recovered Game Source Tree
  owns reconstructed vanilla parser/factory/object/lifetime behavior
```

No consumer may independently rediscover stage membership or build a competing
scene authority.

## Canonical lifecycle

```text
1. GDSpaces materialization
       |
       v
2. StageAssemblyWorkspace
       |
       v
3. ProjectWorkspace sessions attached
       |
       v
4. StageOperationsSession revision N
       |
       v
5. canonical analysis of active bytes
       |
       v
6. StageDomainWorkspace@N
       |
       +--> profile/recovered runtime-link provider@N
       |
       v
7. StageDomainKnowledgeWorkspace@N
       |
       v
8. optimistic concurrency / TOCTOU gate
       |
       v
9. commit_derived_refresh(N)
       |
       v
10. StageSceneSnapshot@N CURRENT
       |
       +--> Semantic Graph
       +--> ModViz
       +--> Stage UI / Inspector navigation
       +--> validation / export / simulation
```

Steps 5–10 are the responsibility of `StageSceneController`. Product/UI callers
must not reproduce that sequence manually.

## Revision model

There are two intentionally different revision concepts.

### WorkingCopy revision

Per-resource revision owned by `ProjectWorkspace`/`WorkingCopy`.

It changes when that exact resource is edited or undone.

### Stage revision

Scene-wide revision owned by `StageOperationsSession`.

It changes when any resource byte-state relevant to the stage changes, including
an edit performed by another tool/agent through the shared ProjectWorkspace.

A valid current snapshot requires:

```text
snapshot.stage_revision == StageOperationsSession.stage_revision
knowledge.source_stage_revision == snapshot.stage_revision
semantic_graph.source_stage_revision == snapshot.stage_revision
all retained typed parser results describe active bytes
StageOperationsSession.derived_state_stale == false
```

A resource WorkingCopy revision and a Stage revision are not interchangeable.

## Edit transition

```text
CURRENT scene@N
   |
   | Stage Ops edit OR external ProjectWorkspace edit
   v
bytes change
   |
   +--> resource WorkingCopy revision changes
   +--> Stage revision becomes N+1
   +--> derived_state_stale = true
   v
STALE scene@N+1
```

Old typed data may remain visible for diagnostics/navigation, but it must carry
`current_for_active_bytes=false` and must not be presented as current scene
state.

## Refresh transaction

`StageSceneController` performs the following transaction for target revision N:

1. detect external ProjectWorkspace changes;
2. establish exact target Stage revision N;
3. run canonical analyzers on active bytes;
4. fail if an attempted parser fails;
5. fail if a materialized Stage Ops resource lacks its ProjectWorkspace session;
6. assemble `StageDomainWorkspace@N` from retained typed parser results;
7. require those domains to describe active bytes;
8. invoke the profile/recovered runtime-link provider against **those exact
   domains@N**;
9. distinguish a valid empty link set from provider failure;
10. validate every explicit runtime link against domain identities and evidence
    requirements;
11. build structural relations and runtime knowledge;
12. call the final optimistic-concurrency gate;
13. if bytes changed during steps 3–11, reject the obsolete transaction;
14. only after a successful `commit_derived_refresh(N)` publish a final current
    `StageSceneSnapshot@N`.

The provisional knowledge built before step 12 is never published as current.

## Why runtime links are generated after parsing

Domain identities may include exact byte offsets. A legal size-changing edit can
shift a token without changing its lexical meaning:

```text
revision 0: DOOR @ offset 0
revision 1: //x\nDOOR @ offset 4
```

The two markers intentionally have different domain IDs.

Therefore a DMC3 recovered-runtime provider must execute only after canonical
WorkingCopy parsing for the target revision. Precomputed links from revision 0
must fail or be regenerated; they may not silently attach to revision 1.

## Runtime-link provider result

A provider has three semantically distinct outcomes:

### Valid links

```text
valid = true
links = [ ... ]
error = empty
```

### Valid zero links

```text
valid = true
links = []
error = empty
```

This means the provider is healthy and no supported recovered semantic link
exists for the current domains.

### Provider failure

```text
valid = false
error = non-empty
```

This means the recovered/profile integration itself could not establish a valid
contract. The scene refresh must fail closed with
`runtime-link-provider-failed`.

Provider failure must never be converted into a valid empty link set.

## Evidence authority

Structural parser facts and recovered runtime facts are different authorities.

Examples:

```text
DOOR token at byte offset 123
  authority = structural-product-fact

DOOR token parsed by recovered Door parser contract
  authority = recovered-runtime-partial
  evidence = explicit ev-* IDs
```

Current runtime-link strength levels:

- `direct_reconstructed` — corresponding recovered implementation/contract and
  mapping are fully promoted;
- `disassembly_complete_corpus_pending` — executable behavior/contract is strong
  but recovered implementation/corpus closure remains incomplete;
- `executable_candidate` — incomplete executable-side hypothesis.

A Semantic Graph node/edge must preserve this distinction.

## DMC3 Door example

Current evidence supports:

```text
door_token
  -- parsed-by-runtime-consumer --> door_txt_parser@0x1401A9DE0

box_in_token
  -- parsed-by-runtime-consumer --> door_txt_parser@0x1401A9DE0
```

with `disassembly_complete_corpus_pending` authority and the canonical Door
parser evidence packet.

Current evidence does **not** justify:

```text
next_room_token -> Door parser/object
AUTO lexical token -> Door parser/object
DOOR token -> complete Door runtime class
```

without the missing contextual/control-flow proof.

## StageSet example

Current evidence proves a StageSet token classifier surface, but Knowledge Base
gap `G-P16-0039` remains open for:

```text
classifier result -> factory/constructor -> concrete CStageSet-derived type
```

Therefore StageSet lexical markers remain structural facts. Concrete type links
must wait for the two-sided data-flow proof defined in
`docs/reverse/stage-set-factory-recovery-packet.md`.

## Concurrency invariant

A refresh transaction is optimistic but fail-closed.

If another editor/agent changes a stage resource after analysis but before
commit:

```text
analysis built for N
external edit occurs
StageOperationsSession detects byte-state difference
Stage revision advances to N+1
derived_state_stale remains true
commit_derived_refresh(N) fails
```

The transaction may be retried against N+1. It may never absorb the external
change into a baseline and falsely mark the old derived state current.

## Adding a new domain

Camera/enemy/event/etc. integration must follow this order:

1. materialize exact resources through GDSpaces;
2. add/retain one canonical typed parser result in ProjectWorkspace;
3. project structural domain objects in Stage Ops;
4. define source spans and invalidation behavior;
5. add structural domain relations only when directly supported;
6. recover vanilla consumer/factory/lifetime contracts in `recovered-game`;
7. publish repo-local evidence IDs;
8. add profile bridge runtime links against exact domains@revision;
9. project links in Semantic Graph;
10. expose immutable scene data to ModViz/UI;
11. add edit -> stale -> reparse -> guarded refresh regression;
12. require Windows + Ubuntu CI.

Skipping directly from a filename/token to a gameplay object is not an allowed
integration path.
