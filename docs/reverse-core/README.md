# Reverse Core

**Status:** canonical architecture direction / implementation pending  
**Snapshot date:** 2026-08-08

Reverse Core is the reusable reverse-engineering subsystem that DMC Rengine will exercise first. It is not a replacement for DMC Rengine, the recovered game source tree, GDSpaces, Binary Inspector, EXE Editor, Stage Ops, ModViz, SDD, or the MCP coordination layer. It formalizes the shared lifecycle that turns binary observations into evidence-backed recovered source.

## Platform relationship

The intended layering is:

```text
Triangle Forge
  -> reusable platform services and workspaces
  -> Reverse Core
       -> binary/function/type/evidence/reconstruction lifecycle
  -> DMC Rengine workspace
       -> Recovered Game Source Tree
       -> GDSpaces
       -> EXE Editor
       -> Binary Inspector
       -> Stage Ops
       -> ModViz
       -> Item Editor
       -> Build & Test Lab
```

DMC Rengine remains an independent project/workspace. Reverse Core must stay game-agnostic: it may know what a `BinaryArtifact`, `Function`, `DataObject`, `Type`, `EvidenceRecord`, `Claim`, `Experiment`, `Reconstruction`, or `ValidationReceipt` is, but it must not contain DMC-specific concepts such as Red Orbs, stage IDs, HITS semantics, or DMC3 item rules.

## Recovered game code is not tool-owned

The reconstructed source of DMC3 is a separate **Recovered Game Source Tree**. Functions, globals, types, classes, tables, and source units represent the game itself and are not owned by Reverse Core or by whichever editor currently displays them.

Three relationships must remain separate:

1. game source identity;
2. semantic membership in a reconstructed game subsystem;
3. tool relationships used to inspect, edit, validate, or consume the finding.

A resource-loader function may belong semantically to the game's resource runtime while being viewed by EXE Editor, structurally inspected by Binary Inspector, represented by Reverse Core records, and used to guide GDSpaces implementation. None of those tools becomes the owner of the game function.

See [Recovered Game Source Tree](game-source-tree.md).

## Canonical reverse lifecycle

```text
Binary artifact
  -> byte range / address identity
  -> function and data discovery
  -> CFG / call / reference relationships
  -> type and ABI hypotheses
  -> evidence and experiments
  -> reviewed reconstruction
  -> recovered C++ source unit in the game source tree
  -> isolated compilation
  -> behavioral comparison
  -> validation receipt
  -> evidence promotion / correction / rejection
```

Readable decompiler output alone is never sufficient for promotion.

## Core object schema v0.1

Reverse Core must provide stable identities and links for at least:

- `BinaryArtifact` — immutable artifact identity including SHA-256, size, profile/build metadata, and provenance;
- `AddressRange` — file offset/RVA/VA-aware range scoped to one artifact identity;
- `Function` — discovered or recovered game function with entry identity, ranges, calls, data references, and confidence;
- `DataObject` — game global/static/table/string/vtable/other data identity and relationships;
- `RecoveredType` — struct/class/enum/function signature/ABI model with evidence links;
- `EvidenceRecord` — claim support tied to exact artifact/range/runtime/test provenance;
- `Hypothesis` — explicit unresolved statement with confidence and competing alternatives;
- `Experiment` — reproducible static or runtime test intended to discriminate hypotheses;
- `TaskClaim` — temporary coordination ownership of work on a function/range/type/subsystem reconstruction by an agent or contributor;
- `Reconstruction` — source-level representation derived from evidence, including status and provenance;
- `ValidationReceipt` — compile/test/runtime/binary-comparison result for a reconstruction;
- `Subsystem` — semantic grouping in the reconstructed target program, not ownership by a DMC Rengine tool.

Every object must use stable IDs. Display names are never canonical identity.

## Evidence and confidence

Reverse Core reuses the existing DMC Rengine confidence vocabulary:

`hypothesis -> candidate -> low -> medium -> high -> confirmed`

Corrections are explicit:

`corrected / rejected`

An AI agent must not promote a finding to `confirmed` merely because multiple generated analyses agree. Promotion requires recorded evidence and the acceptance rules of the owning specification.

## Ownership and parallel-agent protocol

Before mutating a canonical reconstruction or claiming authority over an unresolved binary region, an agent must create or acquire a `TaskClaim`.

Minimum claim fields:

- object/range/subsystem ID;
- owner identity;
- scope of allowed work;
- parent SDD/spec/task;
- start/update timestamps;
- status (`active`, `released`, `superseded`, `blocked`);
- dependencies and conflicting claims.

Claims are coordination metadata, not evidence and not semantic ownership of game code. Two agents may independently analyze the same bytes when explicitly requested, but they must not race to mutate the same canonical reconstruction without a negotiated ownership rule. Releasing or transferring a claim never changes the underlying game function identity.

## Recovered source tree contract

Recovered game source must be exportable as a normal C++ project that can be opened in VS Code or another IDE and built without requiring the UI application as the compiler.

The recovered source organization follows the reconstructed architecture of the game, not tool names. Conceptually:

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
  validation/
```

The exact filesystem layout and subsystem names remain evidence-driven. Unknown functions must not be forced into `gdspaces`, `modviz`, `binary-inspector`, or other tool-named folders.

Each source unit must preserve provenance to binary identity, address/range evidence, source reconstruction status, ABI assumptions, and validation receipts.

See [Recovered Game Source Tree](game-source-tree.md).

## Integration boundaries

### Binary Inspector

Binary Inspector supplies structural ranges, fields, ownership, annotations, byte diff, entropy, diagnostics, and future templates. Reverse Core adds durable reverse objects and cross-analysis provenance. Binary Inspector must not become a second project database or semantic owner of game code.

### EXE Editor

EXE Editor is the primary DMC Rengine UI/domain consumer for executable source recovery. A recovered function shown in EXE Editor must map to the same Reverse Core `Function`, `EvidenceRecord`, `RecoveredType`, and `Reconstruction` identities used by automated agents and validation tools. The function remains part of the recovered game source tree, not EXE Editor source.

### GDSpaces

GDSpaces remains the only game-resource authority. Reverse Core may link evidence to `ResourceId`, but it must not resolve game paths or expand containers independently. Recovered game resource-runtime functions remain game code; GDSpaces may separately implement safe product behavior based on confirmed findings.

### SDD, Kanban, MemPalace, Obsidian, and MCP

Reverse Core is intended to reuse the existing workflow rather than create a competing coordination system:

- SDD/Spec Kit defines accepted work and exit criteria;
- Kanban carries execution state;
- MCP mediates tools, claims, events, and agent coordination;
- Obsidian provides human-readable research chronicle;
- MemPalace provides long-term context;
- GitHub contains reviewed implementation and canonical public documentation.

No memory or agent system silently upgrades implementation/evidence status.

## First validation milestone

Do not begin mass decompilation as the first Reverse Core milestone.

The first proof must be one isolated real DMC3 executable subsystem that completes this loop:

```text
canonical dmc3.exe bytes
  -> Reverse Core identities
  -> evidence-backed recovered types/functions
  -> reviewed C++ source unit in Recovered Game Source Tree
  -> isolated build
  -> behavioral comparison against canonical executable
  -> ValidationReceipt
  -> accepted/corrected/rejected Canon update
```

This is the gate before scaling to hundreds or thousands of functions.

## Non-goals for v0.1

- full automated DMC3 decompilation;
- replacing Ghidra/IDA/Binary Ninja or other external analysis engines;
- game-specific resource resolution;
- tool ownership of recovered game functions;
- organizing recovered game code by editor names;
- automatic confidence promotion without evidence;
- direct original-file modification;
- a monolithic all-in-one Triangle Forge application;
- claiming recovered source is original Capcom source.
