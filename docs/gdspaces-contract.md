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

`ResourceId` is not a display label and does not contain transient OS handles. Logical resource identity, provider candidate identity, physical archive-entry identity and materialized child identity are not automatically the same thing.

### `ResourceRef`

Adds presentation/classification metadata:

- display name;
- format;
- game/profile label;
- synthetic-name flag;
- container flag.

### `ByteProvenance`

Tracks where materialized bytes came from without pretending every payload is a direct physical-file span.

Current origin classes:

- `direct_source_span` — unchanged bytes map directly to one authority span;
- `transformed_source_span` — stored bytes were transformed before materialization, for example NBZ/ZIP DEFLATE;
- `materialized_parent_span` — a child is addressed inside already materialized parent bytes.

A child of a transformed parent must never fabricate a physical archive offset by adding its container-relative offset to the parent's compressed storage coordinate. Present-but-invalid provenance fails closed rather than being laundered into valid child lineage.

### `ResourcePayload`

Carries:

- resolved reference;
- owned read-only materialized bytes;
- diagnostics;
- optional byte provenance.

A payload is readable only when its reference is valid and it has no error diagnostics.

### `ISource` / `SourceRegistry`

A source owns transient access to a mounted origin and reads resources by canonical identity. `SourceRegistry` owns mounted sources, rejects duplicate source IDs, enumerates deterministically and routes reads to the owning source.

Tools do not open source paths directly.

### PAC / PNST structural authority

The clean generation has one shared relative-slot structural core with distinct format authorities:

```text
RelativeSlotContainer
  -> PacParser  (PAC\0)
  -> PnstParser (PNST)
```

PAC and PNST share the evidenced physical envelope, not semantic slot schemas. Sparse declared slot identity is preserved and never compacted.

The canonical DMC3 profile parser registry adapts these format authorities and preserves exact parser diagnostics. It does not implement another PAC/PNST decoder.

### `ContainerExpander`

Consumes a parser result plus a supplied parent payload. It does not reopen paths.

When parent bytes map directly to source storage, a populated child may inherit direct mapping. When parent bytes are valid transformed/materialized bytes, children are anchored to the materialized parent byte domain. Invalid parent provenance yields diagnostics rather than fabricated lineage.

### `ContainerTreeExpander`

Recursively composes canonical container parsers with `ContainerExpander` under explicit product safety budgets.

**Resource identity and parse-result reuse are separate dimensions.** Two declared slot identities may share one immutable byte span and still remain distinct graph nodes with distinct descendants.

`IContainerParser::supports_byte_identity_reuse()` is explicit and defaults to `false`. A parser may opt in only when its structural result depends solely on supplied bytes rather than logical path/context.

Even for an opted-in parser, reuse requires:

- valid `ByteProvenance`;
- materialized-size agreement with the actual byte buffer;
- matching parser/lineage identity;
- SHA-256 of the actual materialized bytes submitted to `parse()`.

A cache hit reuses only `ContainerParseResult`; it never suppresses the current `ResourceId` or graph edges.

An ancestry-local active-domain guard prevents recursion cycles without globally suppressing valid sibling aliases.

### `ResourceGraph`

Stores stable resources and typed edges such as `contains`, `depends-on`, `stage-member`, `evidence-for` and `opens-with`.

### `WorkingCopy`

Editing remains separate from immutable source payloads. Working copies own revisioned mutable bytes and operation history; original source bytes never silently become mutable.

### `StageBundle`

`StageBundle` is a product materialization/view concept, not an original Capcom runtime object. Materialization success is not equivalent to original state-3 game-ready completion.

## Dependency direction

```text
Source implementation
  -> ResourceId / ResourceRef / ResourcePayload / ByteProvenance
  -> SourceRegistry
  -> canonical parser registry
  -> ContainerExpander / ContainerTreeExpander / ResourceGraph
  -> Stage Ops ingress / OpenRouter
  -> tool-specific views/editors
```

Recovered original runtime remains physically separate:

```text
Recovered Game Source Tree
  -> original request / materialization / post-load / ownership / lifecycle reconstruction

GDSpaces
  -> safe product resolver / source / materialization / provenance contracts
```

Validation receipts connect those layers; code ownership does not collapse them.

## Identity rules

1. Display names are presentation only.
2. Synthetic names are explicit.
3. Declared container slot identity is preserved across sparse/alias layouts.
4. Equal byte spans do not erase distinct resource identities.
5. Parse-result reuse is an optimization, never resource-identity deduplication.
6. EXE-backed semantic identity may link to but does not erase source/materialization identity.
7. Logical path, provider candidate, physical archive entry and materialized byte identity remain separate where lookup/transform indirection exists.
8. Re-enumeration should reproduce canonical IDs for unchanged sources.

## Original behavior vs product safety

GDSpaces may intentionally validate more strictly than the recovered original runtime.

For archive work, receipts must distinguish at least:

```text
OriginalCompatibilityBehavior
SafeProductValidation
```

CRC checks, method whitelists, ZIP64 diagnostics, malformed-input guards or other product hardening must not silently become claims about original DMC3 acceptance behavior.

## Architecture anti-patterns

Rejected patterns include:

- editor/tool opens a local path directly;
- editor/tool parses an archive to discover its own resources;
- Binary Inspector reopens a path instead of consuming supplied bytes;
- Stage logic installs private PAC/PNST semantics into generic parsers;
- compressed NBZ coordinates are reused as materialized child coordinates;
- equal `source/offset/size` is used to discard a second declared slot identity;
- parser results are reused without explicit byte-pure capability and actual content binding;
- profile wiring reimplements canonical format decoders;
- GDSpaces absorbs original `LoadedResource`, loader-node or scene-lifecycle implementation;
- one global normalized-path cache is presented as original DMC3 behavior without evidence.

## Evidence/promotion rule

Historical branches and PRs are implementation evidence, not automatic canonical authority. Promotion into `main` requires reconciliation against current EXE/corpus evidence and fresh whole-head CI.

Archive/runtime reverse authority is tracked in issue #100 and the synchronized Drive document `DMC Rengine — Archive Runtime Reverse Program — PAC PNST NBZ AFS`. Issue #55 remains the broader request-to-unload authority.
