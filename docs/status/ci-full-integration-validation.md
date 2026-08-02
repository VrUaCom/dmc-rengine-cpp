# Full Integration Validation

This validation branch checks the current cross-tool integration architecture against the latest `main`.

## Scope

- GDSpaces ResourceGraph and canonical ResourceId identity;
- Tool and Format Integration registries;
- HITS$ scanner and Binary Inspector adapter;
- ResourceWorkspaceSession with immutable source bytes;
- append-only Event Journal and producer-aware WorkingCopy operations;
- deterministic resource workspace manifest;
- Spider Hub ProjectGraph;
- synchronized multi-resource ProjectWorkspace;
- parent PAC, HITS, and TXT resource sessions sharing one project graph;
- explicit container `contains` relations;
- StageBundle attachment only to matching member sessions;
- evidence links, Binary Document, edit, validation, and manifest synchronization;
- pending PAC/PNST/AFS/NBZ formats remaining read-only;
- Windows and Ubuntu configure/build/test.

## Architectural invariant

```text
Sources -> GDSpaces ResourceGraph -> ProjectWorkspace
        -> ResourceWorkspaceSession(s)
        -> parser / Binary / Evidence / Stage contexts
        -> event-driven WorkingCopy and validation
        -> Spider Hub ProjectGraph and manifests
```

No tool may introduce an independent resolver, mutate immutable source bytes, or maintain a competing copy of project relationships.
