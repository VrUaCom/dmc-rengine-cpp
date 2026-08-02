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
- deterministic resource workspace manifest;
- parent PAC, HITS, and TXT resource sessions with container, stage, evidence, edit, validation, and manifest relations;
- CLI commands `list-tools`, `list-formats`, `integration-status`, and `inspect-workspace`;
- CLI smoke tests and all previous foundation tests;
- pending PAC/PNST/AFS/NBZ formats remaining read-only;
- Windows and Ubuntu configure/build/test.

## Architectural invariant

```text
Sources -> GDSpaces -> ProjectWorkspace -> ResourceWorkspaceSession(s)
        -> parser / Binary / Evidence / Stage / Tool policies
        -> event-driven WorkingCopy and validation
        -> Resource, Stage, and Spider Hub manifests + CLI
```

Stage Ops and ModViz consume one shared stage read model. Spider Hub consumes one shared project graph. No tool may introduce an independent resolver, mutate immutable source bytes, or maintain a competing copy of project relationships.
