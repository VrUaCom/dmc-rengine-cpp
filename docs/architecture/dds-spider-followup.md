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

New work must reuse canonical modules instead of creating parallel implementations of the same format knowledge or decoding logic. Shared behavior belongs in reusable C++20 modules with explicit contracts and tests.

## Follow-up gates

This note records future work only. Do not perform these reviews as part of the current Native Reader APK pass unless explicitly requested.

Required later:

1. Fresh dedicated review of the restored direct DDS reader.
2. Re-evaluation of Spider experiments and its final ReaderCore/product boundary.
3. Modular-architecture audit for any affected reverse/read/write/integration paths before promotion to canonical status.
