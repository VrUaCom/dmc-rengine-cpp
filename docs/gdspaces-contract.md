# GDSpaces Contract

GDSpaces is the single public resource-access architecture of DMC Rengine.

It is a product resource authority. Recovered original DMC3 resource-runtime functions, registry/cache structures, loader-node ownership and scene-transition lifetime code belong to the Recovered Game Source Tree, not to GDSpaces.

## Current C++ contracts

### `ResourceId`

Canonical product identity fields:

- source ID;
- logical path;
- container chain;
- byte offset in the identity's current byte domain;
- byte size.

`ResourceId` is not a display label and does not contain transient OS handles. A logical resource identity, a provider lookup candidate, a physical archive entry and a materialized child span are not automatically the same identity.

### `ResourceRef`

Adds presentation and classification metadata:

- display name;
- format;
- game/profile label;
- synthetic-name flag;
- container flag.

### `ByteProvenance`

Tracks where the bytes of a materialized payload came from without pretending every payload is a direct physical-file span.

Current origin classes:

- `direct_source_span` — unchanged bytes map directly to one authority span;
- `transformed_source_span` — stored bytes were transformed before materialization, for example ZIP/NBZ DEFLATE;
- `materialized_parent_span` — a child span is addressed inside already materialized parent bytes.

Current transform identities include `none`, `zip_stored`, `zip_deflate` and an explicit `unknown` boundary.

A child of a transformed parent must never fabricate a physical archive offset by adding its container-relative offset to the parent's compressed storage offset.

### `ResourcePayload`

Carries:

- the resolved reference;
- owned read-only materialized bytes;
- diagnostics;
- optional byte provenance.

A payload is readable only when its reference is valid and it has no error diagnostics.

### `ISource`

A source owns transient access to a mounted origin. It can enumerate references and read a resource by canonical ID.

Canonical `main` implementation currently includes `LocalDirectorySource`. Production DMC3 archive/provider implementations are promoted separately from evidence-backed branches rather than being assumed complete from historical code.

### `SourceRegistry`

Owns mounted sources, rejects duplicate source IDs, enumerates all sources deterministically, and routes reads to the owning source.

### `ResourceGraph`

Stores stable resources and typed edges:

- contains;
- depends-on;
- stage-member;
- evidence-for;
- opens-with.

### `ContainerExpander`

Consumes a parser result plus a supplied parent payload. It does not reopen source paths. Child resources preserve declared physical slot identity and are reclassified centrally from their materialized bytes.

When parent bytes directly map to one source span, a populated child may inherit a direct source span with an adjusted offset. When the parent was transformed, the child receives `materialized_parent_span` provenance instead of a fabricated archive-physical coordinate.

### `ContainerTreeExpander`

Recursively composes registered structural parsers with `ContainerExpander` under explicit depth/container/node/parsed-byte budgets.

Recursive expansion preserves **resource identity** independently from **byte parse reuse**. Two declared slots may share one immutable byte span and still remain separate graph nodes with separate descendant identities.

`IContainerParser::supports_byte_identity_reuse()` is an explicit opt-in capability. The default is false. A byte-pure parser may reuse a `ContainerParseResult` only when valid `ByteProvenance` proves the shared byte-domain identity. Reuse never suppresses the current parent `ResourceId` or its graph edges.

The clean DMC3 parser registry adapts the canonical `PacParser` and `PnstParser`; it does not contain a second PAC/PNST decoding implementation.

### `OpenRouter`

Maps a resource/context to a tool target. Explicit preferred targets override defaults. Menu and stage contexts are first-class.

Unknown formats route to Binary Inspector rather than disappearing.

### `WorkingCopy`

Editing is separate from immutable source payloads. Working copies own revisioned mutable bytes and operation history; source payloads never silently become mutable original storage.

### `StageBundle`

Groups safely materialized resources under one product-side stage/resource-set view. It accepts unknown resources and scoped diagnostics.

`StageBundle` is not an original Capcom runtime object and successful materialization is not equivalent to original state-3 game-ready completion.

## Dependency direction

```text
Source implementation
  -> ResourceId / ResourceRef / ResourcePayload / ByteProvenance
  -> SourceRegistry
  -> parser registry / ContainerExpander / ContainerTreeExpander / ResourceGraph
  -> Stage Ops ingress / OpenRouter
  -> tool-specific parser/editor views
```

The direction may not be reversed. UI and format editors do not own source resolution.

Recovered original-game runtime remains physically separate:

```text
Recovered Game Source Tree
  -> evidence-backed original request / materialization / post-load / ownership / lifecycle model

GDSpaces
  -> safe product resolver / source / materialization / provenance contracts
```

Validation receipts connect those layers; code ownership does not collapse them.

## Identity rules

1. Display name is presentation only.
2. Synthetic names are explicit.
3. Container slots/offsets remain distinct from names.
4. Declared slot identity is preserved across sparse containers; populated children are not compacted or renumbered.
5. Equal byte spans do not erase distinct declared slot/resource identities.
6. EXE-backed semantic identity may link to, but does not erase, source identity.
7. Logical resource identity, provider candidate identity, physical archive entry identity and materialized byte identity remain provenance-linked but distinct where transforms or lookup indirection exist.
8. Parse-result reuse is an optimization and may never act as resource-identity deduplication.
9. Re-enumeration should reproduce canonical IDs for unchanged sources.
10. Future source revision/hash fields must extend identity without making tools source-aware.

## Read-only and safety rules

- source bytes are immutable;
- reads return owned payloads;
- malformed or inaccessible resources produce diagnostics;
- traversal outside a mounted root is rejected;
- a resource may remain visible even if its format is unknown;
- recursive expansion is bounded by explicit product safety budgets;
- parser-result reuse requires an explicit byte-pure parser capability plus valid byte provenance;
- product hardening checks must not be mislabeled as original DMC3 acceptance behavior;
- original compatibility evidence and safe product validation are separate receipt dimensions.

## Evidence/promotion rule

Historical GDSpaces implementations and stacked PRs are implementation evidence, not automatic canonical authority. Promotion into `main` requires reconciliation against current executable/corpus evidence.

Current archive/resource reverse authority is tracked in issue #100 and the synchronized Drive document `DMC Rengine — Archive Runtime Reverse Program — PAC PNST NBZ AFS`. Issue #55 remains the broader request-to-unload runtime authority.

## Architecture anti-patterns

- editor opens a local path directly;
- editor parses an archive to locate its own child resources;
- UI stores `FileSystemHandle` as canonical identity;
- Binary Inspector reopens a path instead of consuming bytes;
- stage loader special-cases PAC inside generic source resolution;
- display names are used as map keys across tools;
- compressed NBZ storage coordinates are reused as materialized PAC/PNST child offsets;
- equal `source_id/offset/size` is used to discard a second declared slot/resource identity;
- a parser is cached by byte identity without explicitly declaring byte-only parse semantics;
- profile wiring reimplements PAC/PNST parsing instead of adapting canonical format authorities;
- GDSpaces absorbs original `LoadedResource`, loader-node or scene-lifecycle implementation;
- one global normalized-path cache is presented as original DMC3 behavior without family-specific evidence.

These patterns are rejected as second-resolver or evidence-boundary violations.
