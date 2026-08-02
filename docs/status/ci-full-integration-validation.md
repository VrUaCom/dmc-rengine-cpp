# Full Integration Validation

This validation branch checks the first concrete cross-tool integration slice against the latest `main`.

## Scope

- shared format diagnostics used by container and HITS parsers;
- HITS$ magic and confirmed 56-byte record scanning;
- HITS to Binary Inspector document adapter;
- Tool Capability Registry and canonical product/lore names;
- Format Integration Registry with maturity and write policies;
- Resource Workspace Session;
- one canonical ResourceId shared across parser, Binary Inspector, Evidence, StageBundle, ModViz/Stage Ops routes, WorkingCopy, and Build & Test Lab;
- immutable source payload plus revisioned local WorkingCopy;
- PAC and other pending formats remaining read-only;
- existing Evidence import/export and public packet validation;
- existing GDSpaces source, classification, container, graph, and `st001` contracts;
- Windows and Ubuntu configure/build/test.

## Architectural invariant

```text
GDSpaces resource identity/payload
  -> Format Integration Registry
  -> parser diagnostics / Binary Inspector document
  -> Evidence and StageBundle context
  -> Tool Capability routes
  -> WorkingCopy (when policy permits)
  -> Build & Test / guarded export boundary
```

No format parser, Binary Inspector adapter, Stage Ops component, ModViz component, Item Editor component, or validation component may introduce an independent filesystem/container resolver or mutate the immutable source payload.
