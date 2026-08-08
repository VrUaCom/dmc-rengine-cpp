# EXE Reconstruction Pipeline

**Status:** canonical planned pipeline with partial foundations implemented  
**Snapshot date:** 2026-08-08

The EXE Editor is not treated as a disassembler with a source-looking panel. Its long-term contract is controlled source recovery: binary evidence must remain traceable through recovered C++ and validation.

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
  -> ownership and subsystem membership
  -> types / ABI / lifetime hypotheses
  -> evidence-linked reconstruction
  -> C++ source unit
  -> isolated build
  -> behavioral comparison
  -> validation receipt
```

A reconstruction may remain compilable but unconfirmed when its ABI, ownership, side effects, or lifetime semantics are still unresolved.

## Function provenance contract

Every recovered function must be able to answer:

- which executable SHA-256 it came from;
- file offset/RVA/VA identity;
- covered bytes/ranges;
- discovered callers/callees and data references where known;
- recovered signature and calling/ABI assumptions;
- owning subsystem;
- linked Evidence records and hypotheses;
- source reconstruction revision;
- compile result;
- behavioral comparison result;
- current confidence/correction state.

## First compilable island

The next source-recovery milestone is not a bulk import of the recovered-source skeleton. Select one bounded subsystem with strong existing evidence and small external dependencies.

Acceptance requires:

1. exact canonical binary identity;
2. reviewed function/data/type set;
3. explicit ABI and lifetime assumptions;
4. isolated C++ build target;
5. deterministic test harness;
6. behavioral comparison against the canonical executable or a captured equivalent boundary;
7. recorded `ValidationReceipt`;
8. correction/rejection path when behavior diverges.

## EXE Editor user model

C++ is the durable central representation. Supporting panes may show disassembly, hex, CFG, xrefs, symbols, resource links, Evidence, hypotheses, tests, and patch plans, but they must all resolve back to the same recovered object identity.

## Safety and non-goals

- no recovered function is `confirmed` from readability alone;
- no patch is applied across executable builds without exact artifact identity and expected-byte guards;
- no bulk recovered-source promotion without per-unit review;
- no claim of complete recompilation until runtime behavior and build provenance satisfy the project gates.
