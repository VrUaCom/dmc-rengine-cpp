# Complete Cross-Tool Stack Validation — 2026-08-03

This branch validates the exact current `main` state after the integration expansion.

## Validated architecture

- GDSpaces canonical `ResourceId`, immutable payloads, source registry, classifier, container contracts, resource graph, WorkingCopy, and StageBundle;
- Tool and Format Integration registries;
- ResourceWorkspaceSession and synchronized multi-resource ProjectWorkspace;
- append-only producer-aware Workspace Event Journal;
- ArtifactRegistry, EvidenceRegistry, EvidencePacketRegistry, strict JSON import/export, and exact SHA-scoped evidence linking;
- public DMC3 Phase 12 Evidence Packet and item-runtime Evidence Packet;
- Binary Inspector regions, fields, ownership, annotations, conflicts, manifests, and HITS adapter;
- generic PE analysis, ExecutableResourceContext, ResourceAnalyzer, known-target recognition, and EXE Workspace Manifest;
- Spider Hub ProjectGraph and deterministic graph manifest;
- DMC3 `st001` StageWorkspaceBuilder consuming resolved GDSpaces payloads only;
- Stage Ops full stage read model;
- ModViz visual projection of the same stage state;
- Stage Ops/ModViz consistency validation;
- resource, stage, executable, and project graph manifests;
- integration CLI, workspace inspection, and `build-stage-workspace st001` command;
- all historical foundation, malformed-input, safety, and public evidence tests.

## Required invariants

```text
Sources -> GDSpaces -> ProjectWorkspace -> ResourceWorkspaceSession(s)
        -> ResourceAnalyzer -> Binary / EXE / Evidence / Stage contexts
        -> event-driven WorkingCopy and validation
        -> Stage Ops / ModViz / Spider Hub / Item Editor read models
```

- no second resolver;
- no direct mutation of source bytes;
- no PAC Editor/PAC Manager architecture;
- no DMC3-specific evidence attached to an unmatched executable;
- Stage Ops and ModViz share canonical IDs, roles, categories, revisions, and evidence state;
- PAC/PNST/AFS/NBZ remain read-only until evidence-backed parsers exist;
- Windows and Ubuntu must configure, compile, and pass every test.
