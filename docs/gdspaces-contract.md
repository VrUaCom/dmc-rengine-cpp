# GDSpaces Contract

GDSpaces is the single public resource-resolution/materialization authority of DMC Rengine.

It is a **product** resource layer. Recovered original DMC3 resource-runtime functions, FileSlot/LoadedResource internals, registries, loader nodes, cache/claim semantics and scene-transition lifetime code belong to the **Recovered Game Source Tree**, not GDSpaces.

The current execution/completion plan for Layer 1 is [GDSpaces L1 Roadmap](gdspaces/l1-roadmap.md).

## Core product contracts

### `ResourceId`

Canonical product identity fields:

- source ID;
- logical path;
- container chain;
- byte offset in the identity's current byte domain;
- byte size.

`ResourceId` is not a display label or OS handle. Logical request identity, provider candidate, physical archive entry and materialized child identity remain separate where indirection exists.

### `ResourceRef`

Adds presentation/classification metadata without changing canonical identity.

### `ByteProvenance`

Tracks where materialized bytes came from while preserving byte-domain boundaries.

Current origin classes include:

- `direct_source_span`;
- `transformed_source_span`;
- `materialized_parent_span`.

A child of a transformed parent must never fabricate a physical archive coordinate by adding a container-relative offset to compressed storage coordinates.

### `ResourcePayload`

Carries the resolved reference, owned read-only materialized bytes, diagnostics and optional provenance. A payload is readable only under the explicit product validity/diagnostic contract.

### `ISource` / `SourceRegistry`

A source owns access to one mounted origin. `SourceRegistry` owns source lifetimes, rejects duplicate IDs and routes reads by canonical identity.

Tools do not bypass GDSpaces to open resource sources directly.

## PAC / PNST authority

PAC and PNST share an evidenced relative-slot physical envelope but not semantic slot schemas:

```text
RelativeSlotContainer
  -> PacParser  (PAC\0)
  -> PnstParser (PNST)
```

Sparse/empty/alias declared slot identity is preserved. Structural parser spans are not automatically intrinsic editable-child EOF authority.

`ContainerExpander` and `ContainerTreeExpander` consume already supplied/materialized bytes and must not reopen source paths. Equal byte spans never erase distinct declared resource identities.

## NBZ authority split

GDSpaces deliberately separates four concepts:

1. **NBZ materialization** — index/read STORE or raw-DEFLATE members into exact materialized bytes;
2. **serialization observation** — preserve/bind raw local/central/EOCD/opaque framing where needed;
3. **DMC Rengine authoring/publication** — generate product output under explicit writer contracts;
4. **original Capcom writer/tool equivalence** — a separate claim requiring separate evidence and not implied by the first three.

A working `NbzZipSource` does not prove lossless retail writer equivalence.

## Artifact-stability contract

Evidence-grade provenance has a stronger requirement than ordinary filesystem reading.

When a receipt claims:

```text
archive identity
 -> central/member identity
 -> materialized bytes
```

those observations must be bound to one stable artifact state.

Current `NbzZipSource` may build its index from one file open and reopen the archive for a later member read. Therefore an acquisition/evidence seam must not assume source stability merely because the path string is unchanged.

A provenance-grade flow must either:

- keep the relevant observation bound to one stable file/artifact handle/state; or
- perform fail-closed identity/stability revalidation that proves the archive did not change across index, member read and final artifact identity observation.

A receipt that combines stale index metadata with later member bytes or a later archive SHA is invalid.

## Publication contract

Generated artifacts and evidence outputs must use explicit output-only publication.

### No-clobber meaning

`exists() -> ofstream` is **not** an acceptable no-clobber implementation because it has a TOCTOU race.

A component may claim atomic/no-replace publication only when the final publication operation itself fails if the destination already exists.

The project should expose one shared no-replace publication primitive and reuse it across:

- retail-NBZ repack output;
- next-volume overlay output;
- retail-member acquisition output;
- evidence/receipt files where overwrite would corrupt provenance.

### Retail immutability

Acquisition/evidence commands must not publish outputs inside the measured retail game tree. Product authoring remains output/export based unless a future explicit write contract deliberately changes that policy.

## DMC3 runtime lookup boundary

GDSpaces may reproduce evidenced resolver behavior while preserving the original-vs-product distinction.

Strong current recovered behavior includes:

- basename-oriented request candidate construction;
- archive candidate ordering;
- archive-first complete pass before physical-provider pass;
- numbered contiguous `DMC3-N.nbz` bootstrap/precedence;
- archive normalization/index behavior.

The exact final type-0 physical-provider Win32 filename/case/open/failure semantics after recovered `0x0C` normalization remain unresolved. Product `LocalDirectorySource` behavior must not be relabeled as exact original behavior.

For acquisition, begin with a **game request** and record the actual resolver-selected member. Do not predeclare `GData*.afs/...` archive member identity from filename intuition.

## `ResourceGraph`

Stores stable resources and typed relationships. Graph relationships do not replace source/materialization identity.

## `WorkingCopy` and authoring

Editing remains separate from immutable source payloads. Source `ByteProvenance` is immutable history and must not be copied onto newly authored output as though it were original source provenance.

Writers return authored bytes plus bounded receipts. New source provenance begins only after explicit persistence/reopen/materialization of that output.

## Stage boundary

`StageBundle` and Stage Ops workspaces are product concepts, not original Capcom runtime objects. Product materialization success is earlier than original game-ready state-3 equivalence.

Dependency direction remains:

```text
GDSpaces source/resolver/materialization/provenance
 -> Stage Ops assembly/orchestration
 -> Stage Semantic Graph
 -> ModViz/editor consumers
```

No downstream consumer may install a second resolver/materializer.

## Recovered runtime ownership

```text
Recovered Game Source Tree
  -> original request/open/I/O/post-load/cache/claim/lifecycle behavior

GDSpaces
  -> safe product resolver/materializer/provenance/authoring contracts
```

Validation receipts connect these layers without moving original-game functions into product resource code.

## Identity rules

1. Display names are presentation only.
2. Synthetic names remain explicit.
3. Sparse/alias container slot identity is preserved.
4. Equal byte spans do not erase distinct `ResourceId`s.
5. Parse reuse never implies resource identity deduplication.
6. Logical request, provider candidate, physical archive entry and materialized bytes remain distinct axes.
7. Authored output never inherits source provenance by convenience.
8. Evidence-grade archive/member receipts require artifact-stability binding.
9. Acquisition starts from canonical resolver input and records the actual winner.

## Original behavior vs product safety

GDSpaces may be stricter than the recovered game. Receipts must keep product hardening separate from original acceptance behavior, including CRC checking, method whitelists, ZIP64 diagnostics, malformed-input bounds and output publication safeguards.

## Evidence-gated freezes

- `.afs/` logical namespaces do not prove a binary AFS backend.
- Historical GDSpaces PACK parser code does not prove original DMC3 PACK runtime authority.
- Capcom offline-packer equivalence is not inferred from successful DMC Rengine authoring.
- Stage/HITS/gameplay semantic rules do not belong in generic resource/container parsers.

## Architecture anti-patterns

Rejected patterns include:

- tool-specific filesystem/archive loading;
- a second resolver outside GDSpaces;
- compressed storage coordinates presented as materialized-child coordinates;
- inferred packed parent span treated automatically as intrinsic child EOF;
- `exists() -> ofstream` described as no-clobber publication;
- evidence command writing into the measured retail tree;
- archive SHA computed independently of the member/index state while claiming one provenance snapshot;
- hard-coded archive member winner where the runtime resolver has not observed it;
- original LoadedResource/cache/lifecycle implementation moved into GDSpaces;
- binary AFS/PACK authority inferred from strings or product history.

## Promotion rule

Historical branches/PRs are evidence history, not automatic current authority. Promotion requires reconciliation with current evidence, exact current-main composition and fresh required CI.

Current detailed reverse ledger: issue #100.  
Direct-retail/game-consumption acceptance gate: issue #182.  
Broader request-to-unload reverse authority: issue #55.
