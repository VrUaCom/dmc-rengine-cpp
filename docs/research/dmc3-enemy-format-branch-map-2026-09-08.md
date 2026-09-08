# DMC3 Enemy resource formats — dedicated branch map

**Date:** 2026-09-08  
**System branch:** `research/enemy-architecture-em000-20260908`  
**Reference main:** `22cfb1073f73d8290a04968dbf0f038048f01990`

## Rule

Each real resource format owns its own reverse branch and modular C++ boundary. Cross-format enemy composition is tracked separately.

A branch is justified when it owns one independently testable binary/text grammar or one cross-format system whose work cannot live truthfully inside any single format.

Do not create branches for extractor suffixes such as `.bin` or generic text encoding.

## Current branch ownership

| Domain | Branch | Purpose | Current status |
|---|---|---|---|
| MOD | `reverse/mod-completion-20260907` | complete MOD ABI/runtime/writer prerequisites | strong reader/reverse; writer open |
| CLT | `reverse/clt-em000-20260908` | CLT grammar + cloth/model binding | structural parser + corpus binding research |
| TSC | `reverse/tsc-em000-20260908` | TSC grammar + texture/control semantics | structural parser; semantic consumers open |
| MOT | `reverse/mot-em000-20260908` | MOT binary grammar + CMotion binding | deep corpus grammar; evaluator/EXE binding open |
| EFM | `reverse/efm-em000-20260908` | EFM model-family adapter | real em000 payload structurally bound |
| effect pack | `reverse/effect-pack-em000-20260908` | G/V/E/P/T/A/M grouped effect grammar | logical/physical grouping and partial ref graph recovered |
| SO working family | `reverse/so-em000-identity-20260908` | true identity + graph/link/volume consumers | original extension/name still open |
| PTX em000 binding | `reverse/ptx-em000-binding-20260908` | prove `em000_000.bin` routes to existing PTX | binding research; parser already canonical |
| complete em000 census | `reverse/em000-format-census-20260908` | account for every leaf and prevent suffix misclassification | 302/302 leaf accounting |
| enemy system | `research/enemy-architecture-em000-20260908` | resource graph + runtime/spawn/new-enemy model | system synthesis branch |

## Existing canonical infrastructure reused

These are already first-class modules and should not get duplicate parser branches merely because they occur inside an enemy package:

```text
PAC
PNST
DDS
PTX reader / texture slot framing
MOD canonical reader/analysis pieces
resource identity / container expansion
```

If a new bounded semantic slice is required for one of these, use its existing lineage or a short-lived exact-head integration branch rather than inventing a second format architecture.

## Semantic ownership rules

### CLT

Owns only CLT serialization/IR and CLT-specific analysis.

It does not own MOD hierarchy parsing. Cross-resource association such as:

```text
CLT Bone -> following MOD/EFM local node domain
CLT WindParent -> external actor domain candidate
```

belongs in `analysis/clt/model_binding`.

### TSC

Owns the `.TSC` textual DSL. It may reference texture slots, but does not own PTX/DDS parsing.

### MOT

Owns serialized motion grammar and MOT-specific evaluation. MOD owns its node/rest/skin structures. Their bridge belongs in dedicated analysis/binding code.

### Effect pack

Owns logical manifest/group semantics layered over PNST. Generic PNST must remain unaware of G/V/E/P/T/A/M meaning.

### SO working family

Owns the recovered three-resource structure while preserving:

```text
working_name = SO
original_extension = unresolved
```

Never convert every `.bin` to SO.

## Promotion discipline

Each format branch should produce, before main promotion:

1. exact corpus provenance/hash;
2. binary/text grammar;
3. known and unknown field table;
4. EXE consumers where available;
5. corpus statistics/invariants;
6. ADR-0003 modular code;
7. fail-closed tests;
8. research note;
9. machine-readable reverse receipt;
10. current-main rebase/semantic transplant;
11. exact-head Ubuntu + Windows CI;
12. merge only when the branch is 0 behind.

The enemy-system branch should not merge unfinished format parsers together. It documents and later implements the higher-level graph after individual format contracts are independently strong enough.
