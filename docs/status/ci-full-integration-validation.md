# Full Integration Validation

This validation branch checks the current cross-tool integration architecture against the latest `main`.

## Scope

- shared format diagnostics;
- HITS$ scanner and Binary Inspector adapter;
- Tool and Format Integration registries;
- canonical product and lore names;
- Resource Workspace Session;
- append-only Workspace Event Journal;
- producer-aware edit, undo, reset, validation, and manifest events;
- immutable source payload plus revisioned local WorkingCopy;
- deterministic event-rich workspace manifest;
- one canonical ResourceId shared across parser, Binary Inspector, Evidence, StageBundle, ModViz/Stage Ops routes, WorkingCopy, Spider Hub, and Build & Test Lab;
- pending PAC/PNST/AFS/NBZ formats remaining read-only;
- Windows and Ubuntu configure/build/test.

## Architectural invariant

```text
GDSpaces ResourcePayload
  -> Format Integration Registry
  -> parser / Binary Document
  -> Evidence + StageBundle context
  -> Tool Capability graph
  -> event-driven WorkingCopy operations
  -> Build & Test validation
  -> deterministic workspace manifest
```

No tool may introduce an independent resolver, mutate the immutable source payload, or change WorkingCopy state without an append-only event identifying its producer and revision.
