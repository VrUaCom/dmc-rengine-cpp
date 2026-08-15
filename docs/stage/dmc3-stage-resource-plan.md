# DMC3 Phase 12 Stage Resource Plan — Historical Compatibility Slice

**Status:** historical bounded implementation / superseded as Stage architecture authority.

This document preserves the earlier Phase-12 Bank-A / `st001` compatibility slice. It must be interpreted through the current Stage Catalog authority and `docs/status/completion-and-evidence-policy.md`.

## What this slice established

The original Phase-12 implementation recorded a Bank-A-oriented descriptor view and a first four-role `st001` compatibility plan.

It preserved:

- canonical executable target identity;
- a Stage resource descriptor/table location for the then-scoped model;
- four resource roles per descriptor:
  1. script;
  2. room configuration;
  3. room effects;
  4. room sound;
- normalized matching through already-enumerated GDSpaces `ResourceRef` values;
- missing/ambiguous diagnostics;
- no independent file/archive/source resolver in Stage matching.

The historical `st001` compatibility paths were:

```text
scr/st001.pac
room/st001cfg.pac
room/st001_effect.pac
se/snd_r001.pac
```

They remain useful regression data only.

## Current Stage authority superseding the architectural scope

Later Wave-2 executable evidence expanded the model to:

- Bank A: 110 observed descriptors;
- Bank B: 79 observed descriptors;
- **189 observed descriptors total**;
- descriptor stride `0x40`;
- four role cells per descriptor;
- separate **193-entry selector space**;
- separate **10-pointer group-base table**;
- numeric Stage resolution through `stageId / 100` and `stageId % 100` group/selector indirection.

Therefore:

> `st001` is not the architectural Stage target and 189 descriptors are not automatically 189 gameplay stages.

Keep these identities separate:

1. `resource_set_id / catalog_entry_id`;
2. `numeric_stage_id`;
3. semantic/gameplay Stage/room/variant identity only when independently evidenced.

Do not derive the catalog from `stNNN` filename templates.

## Matching model that remains valid

A Stage resource matcher may consume:

- an evidence-backed selected descriptor/resource plan;
- already enumerated/resolved GDSpaces resource identities.

It must not:

- open files independently;
- scan directories as a second resolver;
- parse/mount archives to discover its own resources;
- invent semantic Stage identity from filenames.

Normalization/matching is a product lookup helper, not Stage authority.

## Current pipeline

```text
canonical executable Stage descriptor + selector authority
  -> selected catalog/resource-set identity
  -> exact four descriptor-role references
  -> GDSpaces resolution/materialization/provenance/container expansion
  -> StageBundle / runtime load report
  -> Stage Ops StageAssemblyWorkspace
  -> Semantic Graph / ModViz projections
```

Recovered original-game typed post-load/factory/cache/lifetime behavior remains a separate Recovered Game Source Tree bridge. Product materialization is not automatically game-ready/state-3 equivalence.

## Completion boundary

This historical document must **not** be used to claim:

- a complete Stage universe;
- `st001` as the Stage exit gate;
- complete game-backed Stage semantics;
- complete resource runtime lifecycle;
- Stage Ops completion;
- original-game state-3 equivalence.

Current Stage completion/validation is tracked by issue #4, issue #55, issue #90 and the active Stage/GDSpaces/Stage Ops implementation stacks.
