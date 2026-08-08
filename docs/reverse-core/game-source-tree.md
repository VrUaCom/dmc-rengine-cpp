# Recovered Game Source Tree

**Status:** canonical architecture rule  
**Snapshot date:** 2026-08-08

## Core distinction

Recovered game code is the reconstructed source representation of the game itself. It does **not** belong to Reverse Core, EXE Editor, GDSpaces, Binary Inspector, Stage Ops, ModViz, or any other tool.

Tools discover, inspect, annotate, reconstruct, validate, visualize, or consume the code. They do not become the semantic owner of game functions merely because they operate on them.

This distinction must remain explicit throughout DMC Rengine.

## Three independent axes

### 1. Game source identity

A recovered function, global, type, class, vtable, table, or source unit belongs to the reconstructed **game source tree**.

Examples:

- game startup/bootstrap code;
- resource runtime;
- renderer;
- stage runtime;
- collision runtime;
- AI;
- save system;
- UI/HUD runtime;
- input;
- audio;
- gameplay systems.

These are semantic areas of the game, not DMC Rengine tools.

### 2. Game subsystem membership

A `Function`, `DataObject`, or `RecoveredType` may be classified as belonging to one or more reconstructed game subsystems.

Subsystem membership is a reverse-engineering conclusion and therefore may be:

- unknown;
- candidate;
- confidence-tagged;
- corrected;
- split across multiple subsystems when the executable architecture requires it.

A function that loads a resource may be classified under the game's resource runtime. That does **not** mean the function belongs to GDSpaces.

### 3. Tool relationships

Tools may reference the same recovered game object for different purposes:

- **Reverse Core** stores stable reverse identities, evidence, hypotheses, reconstructions, claims, and validation receipts;
- **EXE Editor** presents and edits recovered executable/source views;
- **Binary Inspector** exposes byte/structure/ownership evidence;
- **GDSpaces** implements the DMC Rengine resource API and consumes confirmed resource-runtime findings;
- **Stage Ops** consumes stage-related semantics;
- **ModViz** consumes visual/model/UI semantics;
- **Build & Test Lab** compiles and validates reconstructed source.

These are capability relationships, not code ownership.

## Canonical model

```text
                         Recovered Game Source Tree
                                  |
                 +----------------+----------------+
                 |                                 |
        Game subsystem membership             Source units
        resource / stage / UI / ...        functions / data / types
                 |                                 |
                 +---------------+-----------------+
                                 |
                       Reverse Core identities
                    evidence / reconstruction /
                     validation / TaskClaims
                                 |
          +----------+-----------+-----------+----------+
          |          |                       |          |
     EXE Editor  Binary Inspector        GDSpaces   Build/Test
          |          |                       |          |
          +----------+-----------------------+----------+
                   tool relationships only
```

No arrow from a tool to a game function means ownership.

## Source tree placement rule

Recovered source organization follows the reconstructed architecture of the **game**, not the architecture of DMC Rengine tools.

Conceptually:

```text
recovered-game/
  bootstrap/
  runtime/
    resources/
    stage/
    renderer/
    collision/
    ui/
    audio/
    input/
  gameplay/
  save/
  shared/
  unknown/
```

The exact names and boundaries must be evidence-driven and may evolve. Unknown functions must remain under unresolved/unknown groupings rather than being forced into a tool-shaped folder merely for convenience.

Bad organization:

```text
recovered-game/
  gdspaces-functions/
  binary-inspector-functions/
  modviz-functions/
```

Those are DMC Rengine tools, not game subsystems.

## Product implementation versus recovered game implementation

DMC Rengine may contain product code that reproduces or exposes behavior learned from the game.

Example:

- recovered game loader functions live in the **Recovered Game Source Tree** and represent the game's code/behavior;
- GDSpaces may separately implement a safe resource loader/resolver API based on confirmed findings;
- Reverse Core links both sides through evidence and validation provenance.

The two source trees must not be silently conflated even when algorithms or structures are similar.

## Function identity rule

A canonical recovered function identity is based on the game artifact and reverse evidence, for example:

```text
GameArtifactId + AddressRange/entry + reconstruction lineage
```

It is **not** based on:

- which editor opened it;
- which agent reversed it;
- which tool currently displays it;
- which DMC Rengine feature consumes its findings.

## TaskClaim rule

A `TaskClaim` grants temporary coordination ownership over **work on a reconstruction**, not ownership of the underlying game function or source concept.

When a claim is released, superseded, or transferred, the game function identity and recovered source identity remain unchanged.

## Cross-tool rule

The same game function may legitimately appear in multiple workflows simultaneously.

For example a resource-load function may have:

- disassembly and reconstructed C++ in EXE Editor;
- byte ranges and tables in Binary Inspector;
- evidence/reconstruction records in Reverse Core;
- behavior mapped into GDSpaces runtime reconstruction;
- stage dependency edges used by Stage Ops;
- validation receipts produced by Build & Test Lab.

This is one game function with multiple tool relationships, not six copies of the function.

## Recompilation rule

The long-term recompilation target is assembled from the **Recovered Game Source Tree** plus controlled replacement/rebinding infrastructure. It is not assembled from tool implementation folders simply because those tools helped recover the behavior.

## Non-goals

- assigning game code ownership to editors;
- organizing recovered source by UI/tool names;
- treating agent ownership as semantic ownership;
- duplicating one recovered function per consumer tool;
- forcing uncertain game subsystem boundaries to look cleaner than the evidence supports.
