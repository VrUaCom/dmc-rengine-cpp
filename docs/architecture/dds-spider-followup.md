# DDS / Spider follow-up note

Status: recorded requirement only. No architecture review is being performed in this pass.

## Direct DDS reader

Direct `.dds` reading in DMC Rengine is a first-class canonical capability and must remain independent of PTX.

The current restoration/fix of the direct DDS path is not considered the final architecture decision by itself. Before it is treated as fully closed, the DDS path must receive a fresh dedicated review covering the reader contract, strict DMC3 profile separation, binary-document projection, corpus behavior, partial/full mip handling, and interaction with descriptor-backed texture framing.

PTX may expose DDS payloads, but PTX must never become a prerequisite for opening a standalone DDS resource.

## Spider status

Spider C++20 remains experimental and its final relationship with ReaderCore / product execution is still open.

Previous Spider experiments are evidence and should be preserved, but they do not automatically define the final architecture. The project must later review whether Spider should orchestrate reader execution, which responsibilities belong in Spider, and which must remain inside canonical format modules.

Until that review is explicitly performed, do not treat either of these as final:

- promoting Spider into ReaderCore;
- excluding Spider from future reader orchestration.

The invariant remains: format algorithms and reverse-engineered knowledge must not be duplicated inside Spider. Spider, if promoted, may orchestrate execution; canonical modules remain the authority for format logic.

## Modular architecture requirement

DMC Rengine remains strictly modular.

This requirement applies across the full project, not only texture reading:

- reverse-engineering code and evidence pipelines;
- format readers and parsers;
- writers / rebuilders / authoring paths;
- validation and binary-document projection;
- container and GDSpaces integration;
- runtime / EXE analysis;
- product-facing reader slices;
- Spider or any future orchestration layer.

New work must reuse canonical modules instead of creating parallel implementations of the same format knowledge or decoding logic. Shared behavior belongs in reusable C++ modules with explicit contracts and tests.

## Modern C++ evolution policy

DMC Rengine should actively evaluate and adopt newer C++ capabilities where they materially improve the architecture, implementation quality, safety, performance, maintainability, or developer ergonomics.

The project must not treat one language-standard version as a permanent global ceiling. Different targets/modules may use different C++ standard levels when the toolchain and ABI boundary make that safe and useful.

Current policy direction:

- C++20 remains the stable baseline for canonical code that benefits from maximum portability and proven toolchain support;
- C++23 is an active production candidate for new or upgraded modules where its features simplify or strengthen the implementation;
- C++26 features may be explored in isolated experimental targets/modules before promotion;
- later standards, including C++29-era work, are research candidates only until concrete compiler/library support and project value are demonstrated.

A newer standard must be adopted by evidence, not by version number alone. Before using a new C++23/C++26 feature or moving a module to a newer standard, perform a focused research/review pass covering:

1. the exact problem being solved;
2. compiler and standard-library support on the required targets;
3. Android/NDK, Windows, Linux and other relevant product constraints;
4. ABI/API boundary compatibility with C++20 modules;
5. binary-size, performance and build-system impact;
6. whether the feature reduces code/duplication or merely introduces novelty;
7. testability and fallback/rollback path.

The preferred model is modular experimentation: upgrade the smallest useful target first, prove it, then promote the pattern only if it is clearly better.

Mixing language-standard levels inside the same repository/project is allowed when boundaries are explicit. Newer-standard modules should expose stable contracts that older-standard consumers can safely use, avoiding unnecessary leakage of newer standard-library-only types across target boundaries.

This policy also applies to Spider: Spider may become a good proving ground for C++23/C++26 techniques, but experiments do not automatically become canonical architecture.

## Research-before-promotion rule

For any significant architecture fork, new C++ standard, compiler feature, library technology, execution model, or replacement of an existing subsystem:

- research first;
- compare alternatives;
- prototype in the narrowest possible scope;
- collect compile/test/runtime evidence;
- review against the modular architecture;
- only then promote into canonical Rengine code.

The purpose is to keep DMC Rengine modern without turning the codebase into a collection of unrelated experiments.

## Follow-up gates

This note records future work only. Do not perform these reviews as part of the current Native Reader APK pass unless explicitly requested.

Required later:

1. Fresh dedicated review of the restored direct DDS reader.
2. Re-evaluation of Spider experiments and its final ReaderCore/product boundary.
3. Modular-architecture audit for any affected reverse/read/write/integration paths before promotion to canonical status.
4. Modern C++ capability review for C++23/C++26 candidates and target-by-target adoption opportunities.
5. Maintain a research-first decision record for future language/toolchain upgrades.
