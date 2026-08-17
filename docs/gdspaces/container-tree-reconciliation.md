# GDSpaces Recursive Container Expansion — Reconciled Slice

**Date:** 2026-08-17  
**Status:** IMPLEMENTED IN `main`; PARSE-REUSE TRUST-BOUNDARY HARDENING ACTIVE

## Purpose

`ContainerTreeExpander` performs bounded recursive PAC/PNST traversal without reviving the historical assumption that a `ResourceId` offset is always a physical source coordinate.

The clean dependency is:

```text
DMC3 ContainerParserRegistry
  -> canonical parser result
  -> ContainerExpander
  -> ResourceGraph
  -> recursive traversal
```

Format-specific tools do not recurse containers independently.

## Correction against the historical recursion branch

The historical tree expander used a repeated-range key equivalent to:

```text
source_id | ResourceId.offset | ResourceId.size
```

That key is not a safe universal resource identity after byte-provenance reconciliation:

- `ResourceId.offset` can be a coordinate in materialized parent bytes rather than physical archive storage;
- two declared physical slot identities may intentionally reference the same byte span;
- suppressing the second identity because the bytes match destroys valid graph/container-chain topology.

The canonical implementation therefore separates **parse-result reuse** from **resource-identity expansion**.

## Resource identity remains authoritative

A cache hit never removes a `ResourceId`.

If two declared slots share one immutable span, both parent identities are expanded independently through `ContainerExpander`, and each receives its own child identities and `contains` edges.

Byte equivalence is never semantic/resource identity equivalence.

## Hardened parse-reuse trust boundary

Parse-result reuse is an optimization, not an automatic consequence of provenance metadata.

`IContainerParser` exposes an explicit default-false capability:

```text
supports_byte_identity_reuse()
```

A parser may opt in only when its structural parse result depends solely on the supplied bytes and not on logical path/context. The canonical DMC3 PAC and PNST adapters opt in because they delegate to the byte-structural `PacParser` / `PnstParser` authorities.

Reuse additionally requires:

- present and valid `ByteProvenance`;
- `provenance.materialized_size == payload.bytes.size()`;
- the same parser identity;
- the same provenance byte-domain identity;
- **SHA-256 of the actual materialized bytes supplied to `parse()`**.

The content hash closes a trust gap in the first clean recursion generation: copied/stale provenance coordinates alone cannot borrow another payload's parse result.

The cache key therefore binds:

- parser id;
- `ByteOriginKind`;
- authority id;
- provenance offset;
- stored size;
- materialized size;
- transform kind;
- optional CRC32;
- SHA-256 of actual materialized bytes.

A parser that keeps the default `supports_byte_identity_reuse()==false` is invoked separately for each resource identity even if two payloads have identical valid provenance and bytes.

## Cycle and budget policy

An ancestry-local active-domain set prevents recursion cycles without globally suppressing unrelated sibling aliases.

For a safely reusable byte-pure parser, the active domain is the same content-bound byte identity used by the parse cache. For other parsers the fallback active domain is parser id + current `ResourceId` identity.

Independent product safety limits cover:

- nested depth;
- expanded container identities;
- graph nodes;
- bytes actually submitted to parsers.

Cache hits do not charge parsed-byte budget again because the parser is not invoked again.

These are GDSpaces product safety policies, not recovered original DMC3 limits.

## Alias regression

The synthetic regression contains one outer PAC where physical slots 0 and 1 both point to the same nested PAC byte span.

With canonical byte-pure DMC3 adapters:

- root plus both nested slot identities are expanded;
- nested bytes are parsed once and reused once;
- both alias identities remain distinct graph nodes;
- each alias receives its own child identity/edge.

A second regression uses a parser that deliberately does **not** opt into byte-identity reuse. The same root + two alias identities then require three parser invocations and zero cache hits.

This locks both sides of the contract: alias identity preservation and conservative parser-context isolation.

## Evidence boundary

This is product traversal/materialization infrastructure. It does not implement original DMC3 `LoadedResource` groups, loader-node claims, typed ready-state, scene reset policy, FileSlot ownership or original cache/refcount semantics.

Original executable evidence separately establishes PAC-member traversal and recursive PNST typed post-load before state-3 readiness. Validation receipts may connect the product tree to those recovered runtime boundaries; implementation ownership remains separate.

## Remaining validation

- current-generation real nested PAC↔PNST corpus execution receipt;
- end-to-end recursion from transformed NBZ materialization;
- `.index` metadata attachment without topology mutation;
- `.lst` synthesized-container integration;
- Stage Ops ingress and representative lifecycle receipts.
