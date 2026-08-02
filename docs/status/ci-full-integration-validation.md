# Full Integration Validation

This validation branch checks the first concrete cross-tool integration slice.

## Scope

- shared format diagnostics used by container and HITS parsers;
- HITS$ magic and confirmed 56-byte record scanning;
- Binary Inspector document adapter for HITS records;
- stable regions, fields, ownership, unknown gaps, and selection context;
- existing Evidence import/export and public packet validation;
- existing GDSpaces source, classification, container, graph, WorkingCopy, and StageBundle contracts;
- existing DMC3 stage descriptor and `st001` resource matcher;
- Windows and Ubuntu configure/build/test.

## Architectural invariant

The integration path must remain:

```text
GDSpaces resource identity/payload
  -> format parser
  -> Binary Inspector document/evidence context
  -> StageBundle/tool adapters
  -> WorkingCopy and guarded patch/export boundaries
```

No format parser, Binary Inspector adapter, Stage Ops component, ModViz component, or Item Editor component may introduce an independent filesystem/container resolver.
