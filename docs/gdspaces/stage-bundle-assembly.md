# StageBundle Assembly

`StageBundleAssembler` groups **already selected/resolved GDSpaces resources** into a typed product Stage bundle. It is not a Stage catalog resolver, archive loader, original-game factory, or scene assembler.

See `docs/status/completion-and-evidence-policy.md` and issue #4.

## Input authority

StageBundle assembly consumes evidence-backed Stage selection output, including where available:

- `resource_set_id / catalog_entry_id`;
- source bank/source row/global catalog row;
- optional numeric Stage selector identity;
- exact descriptor-role resource references;
- already resolved GDSpaces `ResourceRef` / payload/provenance data;
- recursively expanded resources from the canonical GDSpaces container path.

It must not infer catalog identity from `st001` or a generic `stNNN` filename pattern.

## Stage identity rule

Keep separate:

1. `resource_set_id / catalog_entry_id` — technical Stage resource-set identity;
2. `numeric_stage_id` — selector-facing executable identity;
3. semantic/gameplay Stage/room/variant identity — only when separately evidenced.

Current Wave-2 executable authority contains 189 observed descriptors (110 Bank A + 79 Bank B), a separate 193-entry selector space and 10 group-base pointers.

The 189 descriptors are **not** automatically 189 gameplay stages. `st001` is only a regression/compatibility fixture.

## Candidate/resource behavior

Each candidate preserves stable resource identity and may carry evidence-backed role/category/provenance information.

Explicit stronger evidence wins over generic format inference. Generic format/category hints remain product classification helpers and must not be promoted into original-game semantic claims.

Unknown resources are preserved. Unsupported or unresolved members are not silently dropped.

## Diagnostics / partial results

Assembly must preserve valid work when part of a Stage resource set fails.

Examples:

- missing role/resource -> diagnostic with remaining valid members preserved;
- ambiguous resource identity -> error without inventing a winner;
- unknown nested child -> retained as unknown/unresolved;
- duplicate/shared resource -> preserve stable `ResourceId` identity rather than duplicating conceptual ownership.

A partial StageBundle is a product-side result, not evidence that the original game accepts the same partial state.

## Dependency direction

```text
canonical EXE descriptor/selector authority
        +
GDSpaces source/resolution/materialization/provenance/container expansion
        |
        v
exact selected resource-set members
        |
        v
StageBundle / StageRuntimeLoadReport
        |
        v
Stage Ops StageAssemblyWorkspace
        |
        +--> Stage Semantic Graph
        +--> ModViz
        +--> Binary Inspector / EXE Editor / evidence links
```

Stage Ops, Semantic Graph and ModViz may not reverse this flow by locating game resources independently.

## Materialized vs game-ready

A product StageBundle means the product has materialized/organized resources according to the supported GDSpaces path.

It does **not** prove:

- original DMC3 typed post-load normalization;
- original factory/object construction;
- cache/ownership/lifetime registration;
- consumer handoff;
- room/stage transition behavior;
- original state-3/game-ready equivalence.

Those boundaries require Recovered Game Source Tree evidence and representative behavioral ValidationReceipts.

## Representative validation target

The Stage integration exit program is not `st001`-only.

Representative validation must include, as evidence/data permit:

- Bank-A descriptor;
- Bank-B descriptor;
- selector alias/fallback behavior where evidenced;
- repeated/shared resource case;
- partial/unresolved case;
- deterministic catalog enumeration;
- lifecycle linkage to recovered resource-runtime behavior.

`st001` may remain one regression fixture inside that set.

## Completion status

StageBundle assembly is a bounded product capability. The complete game-backed Stage/runtime/Stage Ops vertical remains **NOT COMPLETE** until the broader completion gates are satisfied.
