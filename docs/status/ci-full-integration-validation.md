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
- ModViz route consistency enforced only for visual stage categories;
- TXT/scripts remain Stage Ops/Binary/Evidence resources without a false direct ModViz requirement;
- deterministic resource workspace manifest;
- integration CLI and smoke tests;
- Windows and Ubuntu configure/build/test.

## Architectural invariant

```text
Sources -> GDSpaces -> ProjectWorkspace -> ResourceWorkspaceSession(s)
        -> parser / Binary / Evidence / Stage / Tool policies
        -> event-driven WorkingCopy and validation
        -> Resource, Stage, and Spider Hub manifests + CLI
```

Stage Ops consumes all stage resources. ModViz consumes the same stage read model and direct routes for visual categories. No tool may introduce an independent resolver, mutate immutable source bytes, or maintain a competing copy of project relationships.
