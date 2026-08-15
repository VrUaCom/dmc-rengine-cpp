# GDSpaces Contract

GDSpaces is the single **product-side resource authority** of DMC Rengine.

It is not the original DMC3 resource runtime and must not become a home for reconstructed Capcom runtime functions/types/factories/cache/lifetime behavior. Those belong to the Recovered Game Source Tree.

See also `docs/status/completion-and-evidence-policy.md`.

## Current resource contracts

### `ResourceId`

Canonical source/resource identity includes stable source, logical path/container lineage and source-span identity. It is not a display label or transient OS handle.

A critical mutation rule follows:

> `ResourceId` identifies immutable source identity. A revisioned `WorkingCopy` may change active byte length without changing that source identity.

Binary/parser views over edited content therefore track the active byte lineage/revision rather than treating source-span size as mutable identity.

### `ResourceRef`

Carries presentation/classification metadata over a stable identity, including display name, format/profile classification, synthetic-name state and container metadata as applicable.

### `ResourcePayload`

Carries a resolved reference, immutable source bytes and diagnostics. Source payload bytes are read-only.

### Sources / providers

Sources own access to mounted origins and expose deterministic enumeration/read contracts through GDSpaces.

Current/active stacks include local-directory and production-oriented provider/container work for game files and nested resource sources. Exact support level varies by provider and must be described by its own evidence/tests; the existence of generic source/container contracts does not mean complete PAC/PNST/NBZ/AFS runtime equivalence.

### `SourceRegistry`

Owns mounted product sources, rejects conflicting source identity, enumerates deterministically and dispatches reads to the owning source.

Source precedence, duplicate behavior, fallback and original DMC3 registration semantics are only claimed where separately evidenced under the resource-runtime reconstruction program.

### `ResourceGraph`

Stores stable resource nodes and typed relationships such as containment, dependency, Stage membership, evidence and tool routes. Graph edges do not redefine source identity or original-game ownership.

### `OpenRouter`

Maps stable resources/context to product tools. Unknown resources remain inspectable rather than disappearing. Routing is not resource resolution.

### `StageBundle`

A StageBundle groups **product-side materialized resources** for an exact Stage resource-set/catalog selection and preserves diagnostics/provenance/unknowns.

It is not automatically an original DMC3 runtime Stage object.

```text
StageBundle materialized bytes
        !=
original DMC3 typed-postload/factory/state-3 game-ready object
```

The game-ready boundary remains in recovered resource-runtime/factory/lifecycle evidence and Stage Ops integration.

## Stage Catalog interaction

GDSpaces does not derive Stage identity from `st001` or `stNNN` filename patterns.

The active executable-derived Stage model distinguishes:

- 110 Bank-A observed descriptors;
- 79 Bank-B observed descriptors;
- 189 observed descriptors total;
- separate 193-entry selector space;
- separate 10-pointer group-base table;
- numeric Stage group/remainder selector indirection.

GDSpaces consumes the selected exact descriptor/resource references and preserves resource identity/provenance. It does not invent semantic gameplay Stage identity.

Keep separate:

1. `resource_set_id / catalog_entry_id`;
2. `numeric_stage_id`;
3. separately evidenced semantic/gameplay identity.

`st001` is only a regression/compatibility fixture.

## WorkingCopy boundary

Editing never mutates immutable source payloads in place.

A `WorkingCopy` owns:

- mutable active bytes;
- revision identity;
- operation history;
- expected-byte guards;
- undo/reset behavior;
- dirty state.

A size-changing edit is legal if permitted by the format/workflow. Parser/Binary Document attachments must correspond to the exact active WorkingCopy revision and byte view.

## Dependency direction

```text
Mounted source/provider
  -> ResourceId / ResourceRef / ResourcePayload
  -> SourceRegistry / resolver / provenance / container expansion
  -> Stage/resource materialization
  -> Stage Ops / domain workspaces / tool projections
  -> editors / validation / guarded export
```

Original-game runtime reconstruction is parallel evidence consumed by product layers, not an implementation dependency that moves original functions into GDSpaces:

```text
canonical dmc3.exe
  -> Recovered Game Source Tree
  -> original lookup/postload/factory/cache/lifetime contracts
  -> product equivalence tests / Stage Ops readiness gates
```

## Read-only/source safety rules

- source bytes remain immutable;
- malformed/unavailable resources produce bounded diagnostics;
- path traversal outside mounted roots is rejected;
- unknown formats/resources remain representable;
- product tools do not reopen source paths independently;
- direct original-file writes are forbidden by default.

## Architecture anti-patterns

Rejected patterns include:

- editor opens a local path directly as its primary resource contract;
- Stage Ops or ModViz performs its own archive/path resolution;
- Semantic Graph traverses containers or discovers scene membership independently;
- UI-local file handles become canonical identity;
- Binary Inspector reopens a path instead of consuming supplied bytes;
- recovered DMC3 factory/cache/lifetime code is implemented inside GDSpaces;
- product StageBundle is labeled game-ready without recovered-runtime evidence;
- `st001` or `stNNN` filenames are used as the canonical Stage universe.

These are Second Resolver / ownership-collapse regressions.

## Completion status

GDSpaces has substantial implemented/tested product infrastructure, but the **full DMC3 resource runtime request-to-unload equivalence is NOT COMPLETE**.

Open equivalence areas include typed post-load/factory handoff, cache/reuse/ownership, source/fallback details where unresolved, lifecycle transitions and representative game-backed ValidationReceipts.
