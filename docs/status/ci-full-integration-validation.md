# Full Integration Validation

This validation branch checks the current cross-tool integration architecture against the latest `main`.

## Scope

- GDSpaces, Tool and Format Integration registries;
- HITS$ scanner and Binary Inspector adapter;
- Resource Workspace Session and append-only Event Journal;
- producer-aware WorkingCopy edits and validation;
- deterministic workspace manifest;
- Spider Hub ProjectGraph with resource, format, parser, tool, evidence, stage, binary document, working copy, manifest, and event nodes;
- explicit contains/depends-on/stage/evidence/tool relations;
- synchronized multi-resource ProjectWorkspace;
- one ResourceGraph plus one ProjectGraph for parent PAC, HITS, TXT, StageBundle, evidence, edits, validation, and manifests;
- pending PAC/PNST/AFS/NBZ formats remaining read-only;
- Windows and Ubuntu configure/build/test.

## Architectural invariant

```text
Sources -> GDSpaces ResourceGraph -> ResourceWorkspaceSession(s)
        -> Tool/Format policies -> parser/Binary/Evidence/Stage contexts
        -> append-only events -> WorkingCopy/validation/manifests
        -> Spider Hub ProjectGraph
```

No tool may introduce an independent resolver, mutate immutable source bytes, or maintain a competing copy of project relationships.
