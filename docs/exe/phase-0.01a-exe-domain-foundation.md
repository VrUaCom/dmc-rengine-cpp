# Phase 0.01a — C++20 EXE Domain Foundation

## Purpose

Phase **0.01a** establishes the internal executable model that allows DMC Rengine to represent `dmc3.exe` as a C++ runtime system rather than a flat list of disassembled addresses.

This phase is the foundation for long-term source reconstruction and eventual generation of a new modern C++20 executable. It does not claim that DMC3 is fully decompiled or recompilable today.

## Core rule

The project reconstructs original behavior in modern C++20 while preserving compatibility contracts only where they are required:

- binary and serialized layouts;
- calling and object ABI boundaries;
- initialization and destruction order;
- allocator domains and alignment;
- resource identities and slot semantics;
- save, stage, script and resource compatibility;
- runtime ownership and synchronization.

Modern APIs are layered above compatibility models. They must not silently erase legacy behavior.

## Two-layer runtime model

### Compatibility core

`dmc::rengine::compat::dmc3` will contain evidence-backed structures and behavior contracts that must remain compatible with the original program and data.

### Modern runtime

`dmc::rengine::runtime` will use C++20 ownership, spans, views, variants, filesystem paths, typed handles, diagnostics and deterministic jobs.

The compatibility core is not a permanent copy of low-level decompiler output. It is the verified boundary from which safer modern systems can be developed.

## Implemented in this foundation

The `dmc::rengine::exe` domain now defines:

- typed file-offset, RVA and VA addresses;
- overflow-aware address ranges;
- runtime unwind ranges;
- function fragments and logical functions;
- recovered C++20 source units;
- source-line to output-address mappings;
- class layouts and evidence-ranked fields;
- multiple/subobject vtables and typed slots;
- global initializer models;
- memory arena models;
- subsystem models;
- resource bindings;
- a typed executable knowledge graph;
- a persistent executable project manifest;
- Pass 0–32 authority metadata;
- validation that the project remains Phase 0.01a and C++20.

## Required graph relationships

The model is designed to express:

```text
runtime range → belongs to → function fragment
function fragment → belongs to → logical function
logical function → calls → logical function
logical function → reads/writes → global
constructor → installs → subobject vtable
class → inherits → class
source unit/line → maps to → output range
resource path → resolves to → mount/backend/container slot
container slot → binds to → class field
source representation → owns/creates → runtime GPU resource
```

## Pass 0–32 authority

`data/registries/dmc3-exe-pass-0-32.v1.json` records the complete research line from the canonical executable baseline through the reviewed Pass 32 save-system promotion.

The registry is an index, not a substitute for Evidence Packets. Individual findings must continue to be promoted narrowly with exact evidence, correction history, recovered source, tests and CI receipts.

## Persistent EXE project target

The future `.rengine-exe` workspace is expected to contain:

```text
project.rengine-exe/
├── project.json
├── target/
├── evidence/
├── recovered/include/
├── recovered/src/
├── mappings/
├── classes/
├── functions/
├── globals/
├── graphs/
├── patches/
├── builds/
└── history/
```

User-edited C++ must never be destroyed by a later decompilation pass. Raw recovered, semantic recovered and user-edited representations remain distinct and are reconciled through explicit merge operations.

## Recompilation ladder

1. syntax-valid recovered units;
2. isolated unit compilation;
3. ABI and layout validation;
4. subsystem library build;
5. behavioral differential testing;
6. replacement subsystem;
7. hybrid executable;
8. standalone rebuilt executable.

Each level requires evidence and tests. A source file compiling does not prove behavioral equivalence.

## Current boundaries

This foundation does not yet implement:

- Pass JSON parsing into the project model;
- automatic PE `.pdata` logical-function reconstruction;
- RTTI/vtable import;
- recovered-source filesystem persistence;
- EXE Editor desktop UI;
- compiler/linker orchestration;
- hybrid or standalone DMC3 executable output.

Those systems now have a stable C++20 domain model to target.
