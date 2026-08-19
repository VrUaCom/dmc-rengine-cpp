# EXE Reconstruction Pipeline

**Status:** canonical planned pipeline with partial foundations implemented  
**Snapshot date:** 2026-08-08

The EXE Editor is not treated as a disassembler with a source-looking panel. Its long-term contract is controlled source recovery: binary evidence must remain traceable through recovered C++ and validation.

The recovered C++ belongs to the **Recovered Game Source Tree**, not to EXE Editor, Reverse Core, Binary Inspector, GDSpaces, or any other tool. Tools provide workflows and metadata around the reconstructed game code.

See [Recovered Game Source Tree](../reverse-core/game-source-tree.md).

## Current foundation

The reviewed C++ repository already contains:

- bounded PE32/PE32+ inspection;
- file offset/RVA/VA conversion;
- canonical executable identity and hash-gated target matching;
- Evidence Address Resolver;
- executable workspace manifests;
- guarded patch contracts;
- source-to-binary mapping and Custom Build lineage models;
- selected promoted executable/runtime findings.

Full DMC3 decompilation and a behaviorally equivalent rebuilt executable are not complete.

## Required reconstruction chain

```text
bytes
  -> artifact identity
  -> address/range identity
  -> function boundary / CFG
  -> calls and data references
  -> semantic game-subsystem membership
  -> types / ABI / lifetime hypotheses
  -> evidence-linked reconstruction
  -> C++ source unit in Recovered Game Source Tree
  -> isolated build
  -> behavioral comparison
  -> validation receipt
```

A reconstruction may remain compilable but unconfirmed when its ABI, ownership, side effects, subsystem classification, or lifetime semantics are still unresolved.

## Function provenance contract

Every recovered function must be able to answer:

- which executable SHA-256 it came from;
- file offset/RVA/VA identity;
- covered bytes/ranges;
- discovered callers/callees and data references where known;
- recovered signature and calling/ABI assumptions;
- candidate/confirmed semantic game-subsystem membership;
- linked Evidence records and hypotheses;
- source reconstruction revision;
- compile result;
- behavioral comparison result;
- current confidence/correction state;
- tool relationships that reference it, without treating those relationships as ownership.

A function identity must not be derived from the tool that opened it or the agent that reversed it.

## Game subsystem membership versus tool relationship

A recovered function may be classified as part of a game subsystem such as resource runtime, renderer, stage runtime, collision, UI/HUD, save, input, audio, gameplay, or an unresolved group.

That classification is about the target game architecture. Separately, the same function may be referenced by:

- EXE Editor for source/disassembly workflow;
- Binary Inspector for bytes/structures;
- Reverse Core for evidence/reconstruction identity;
- GDSpaces when confirmed resource-runtime behavior informs the product resource API;
- Stage Ops/ModViz when runtime behavior is relevant to their domain;
- Build & Test Lab for compilation and behavioral validation.

These tool links never change semantic game-code ownership.

## First compilable island

The next source-recovery milestone is not a bulk import of the recovered-source skeleton. Select one bounded game subsystem with strong existing evidence and small external dependencies.

Acceptance requires:

1. exact canonical binary identity;
2. reviewed function/data/type set;
3. explicit ABI and lifetime assumptions;
4. evidence-backed semantic subsystem boundary;
5. isolated C++ build target from the Recovered Game Source Tree;
6. deterministic test harness;
7. behavioral comparison against the canonical executable or a captured equivalent boundary;
8. recorded `ValidationReceipt`;
9. correction/rejection path when behavior diverges.

## EXE Editor user model

C++ is the durable central representation, but the durable source is the Recovered Game Source Tree. Supporting panes may show disassembly, hex, CFG, xrefs, symbols, resource links, Evidence, hypotheses, tests, and patch plans, but they must all resolve back to the same recovered game object identity.

EXE Editor is a view/workflow over the reconstruction, not the semantic owner of the reconstructed code.

## Safety and non-goals

- no recovered function is `confirmed` from readability alone;
- no function is assigned to a tool merely because the tool discovered or displays it;
- no patch is applied across executable builds without exact artifact identity and expected-byte guards;
- no bulk recovered-source promotion without per-unit review;
- no claim of complete recompilation until runtime behavior and build provenance satisfy the project gates.
