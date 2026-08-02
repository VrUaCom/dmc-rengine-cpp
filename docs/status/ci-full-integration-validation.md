# Full Integration Validation

This validation branch checks the current cross-tool integration architecture against the latest `main`.

## Scope

- GDSpaces ResourceGraph and canonical ResourceId identity;
- Tool and Format Integration registries;
- HITS$ scanner and Binary Inspector adapter;
- ResourceWorkspaceSession and append-only Event Journal;
- synchronized multi-resource ProjectWorkspace;
- Spider Hub ProjectGraph and deterministic graph manifest;
- unified Stage Workspace Manifest for Stage Ops and ModViz;
- ArtifactRegistry and EvidencePacketRegistry;
- transactional Evidence Packet import with conflict rejection;
- Evidence Packet, artifact identity, evidence record, and artifact-location provenance nodes/edges;
- actual public DMC3 Phase 12 packet import into ProjectWorkspace;
- deterministic resource workspace manifest;
- integration CLI and smoke tests;
- Windows and Ubuntu configure/build/test.

## Architectural invariant

```text
Evidence Packet -> Artifact + Record registries -> Spider Hub provenance graph
Sources -> GDSpaces -> ProjectWorkspace -> ResourceWorkspaceSession(s)
        -> parser / Binary / Evidence / Stage / Tool policies
        -> event-driven WorkingCopy and validation
        -> Resource, Stage, and Spider Hub manifests + CLI
```

Evidence conflicts must fail before partial import. No tool may introduce an independent resolver, mutate immutable source bytes, or maintain a competing copy of project relationships.
