# DMC3 Phase 12 Stage Resource Plan

This layer represents confirmed stage-table metadata and the first `st001` resource plan without claiming knowledge of the complete executable pointer-table encoding.

## Stage table descriptor

`phase12_stage_resource_table()` records:

- canonical executable SHA-256;
- public Evidence Packet ID;
- file offset `0x005C30A8`;
- RVA `0x005C4AA8`;
- VA `0x1405C4AA8`;
- 110 rows;
- four roles per row;
- 440 total entries.

Column order:

1. script;
2. room configuration;
3. room effects;
4. room sound.

The descriptor validates its artifact hash against the profile-specific known executable target and checks the VA/ImageBase/RVA relationship.

## `st001` plan

`phase12_st001_resource_plan()` records the confirmed first target paths in normalized public form:

```text
scr/st001.pac
room/st001cfg.pac
room/st001_effect.pac
se/snd_r001.pac
```

The plan links to evidence record `ev-dmc3-stage-resource-table`.

## Matching model

`StageResourceMatcher` consumes:

- a typed row plan;
- a span of already enumerated `ResourceRef` values.

It does not:

- open files;
- scan folders;
- parse NBZ/AFS/PAC;
- read the executable;
- create an independent resolver.

Matching normalizes:

- ASCII case;
- `\` and `/` separators;
- repeated separators;
- leading `./` or `/`;
- optional source-root prefixes.

## Diagnostics

- zero matches for a role → warning `dmc3.stage.resource_missing`;
- more than one match → error `dmc3.stage.resource_ambiguous`;
- invalid plan → error `dmc3.stage.invalid_resource_plan`.

Only roles with exactly one match become `StageMemberCandidate` values.

## Stage categories

- script → scripts;
- room config → unknown until nested evidence/classification resolves it;
- room effects → effects;
- room sound → sounds.

The room configuration PAC is intentionally not overclassified. Its child resources will later provide cameras, lighting, collision, events, positions, and unknown categories through container expansion.

## Current completion boundary

Implemented and tested:

- descriptor;
- fixed `st001` row plan;
- matching and ambiguity diagnostics;
- conversion to unique candidates;
- `StageBundleAssembler` integration using synthetic `ResourceRef` inputs.

Not yet implemented:

- parsing the real executable table/pointers;
- resolving the four parents from a user game installation;
- PAC/PNST/NBZ production materialization for the selected members; binary AFS
  is not part of the evidenced DMC3-HD path;
- nested child expansion for actual `st001` data;
- game-backed integration test.

This distinction prevents documented historical addresses from being misrepresented as a finished runtime parser.
