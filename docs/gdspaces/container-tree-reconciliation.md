# GDSpaces Recursive Container Expansion — Reconciled Slice

**Date:** 2026-08-18  
**Status:** IMPLEMENTED / WHOLE-HEAD CI REQUIRED BEFORE PROMOTION

## Purpose

Promote bounded recursive PAC/PNST traversal without reviving the historical assumption that a `ResourceId` offset is always a physical source coordinate.

The clean dependency is:

```text
DMC3 ContainerParserRegistry
  -> parser result
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

That key is not safe after byte-provenance reconciliation:

- `ResourceId.offset` can be a coordinate in materialized parent bytes rather than physical archive storage;
- two declared physical slot identities may intentionally reference the same byte span;
- suppressing the second identity because the bytes match would destroy valid graph/container-chain topology.

The clean implementation therefore separates **parse-result reuse** from **resource-identity expansion**.

## Parse cache

A parser result may be reused when the parser identity and byte-domain provenance are equal.

For valid provenance, the cache key includes:

- parser id;
- `ByteOriginKind`;
- authority id;
- provenance offset;
- stored size;
- materialized size;
- transform kind;
- optional CRC32.

When provenance is absent, resource identity is used as the conservative cache boundary. Present-but-invalid provenance is never used to merge two evidence domains.

A cache hit reuses only the immutable parser result. `ContainerExpander` still runs for every parent `ResourcePayload`, so child `ResourceId`, container-chain and graph edges remain parent-identity-specific.

## Cycle and budget policy

An ancestry-local active byte-domain set prevents recursive cycles without globally suppressing unrelated alias identities.

Independent product safety limits cover:

- nested depth;
- expanded container identities;
- graph nodes;
- bytes actually submitted to parsers.

Cache hits do not charge parsed-byte budget again because the parser is not invoked again.

These are GDSpaces safety policies, not recovered original DMC3 limits.

## Alias regression

The synthetic regression contains one outer PAC where physical slots 0 and 1 both point to the same nested PAC byte span.

Expected behavior:

- root plus both nested slot identities are expanded;
- nested bytes are parsed once and reused once;
- both `PAC[0]` and `PAC[1]` remain distinct graph identities;
- each alias receives its own child identity/contains edge;
- depth, parsed-byte and container-count limits fail closed.

This directly prevents the historical repeated-physical-range optimization from becoming an identity-loss bug.

## Evidence boundary

This is product traversal/materialization infrastructure. It does not implement original DMC3 `LoadedResource` groups, loader-node claims, typed ready-state, scene reset policy, FileSlot ownership or original cache/refcount semantics.

Those remain Recovered Game Source Tree/evidence concerns. Validation receipts may connect the layers; implementation ownership remains separate.
