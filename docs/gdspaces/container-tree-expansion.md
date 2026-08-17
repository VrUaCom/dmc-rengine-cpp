# GDSpaces Recursive Container Tree Expansion

**Reconciled:** 2026-08-17  
**Status:** CLEAN IMPLEMENTATION SLICE / SYNTHETIC REGRESSION; REAL NESTED CORPUS RECEIPT REMAINS SEPARATE

## Purpose

`ContainerTreeExpander` is the generic GDSpaces product mechanism for recursively expanding already-materialized nested containers through a supplied `ContainerParserRegistry`.

It does not open source paths, resolve game resources, emulate original `LoadedResource`, or assign gameplay semantics. It consumes `ResourcePayload` objects and canonical parser results only.

## Reconciliation origin

Historical PR #58 already had useful product-safety ideas:

- generic recursion;
- partial-result preservation;
- explicit depth/container/node/byte budgets;
- unsupported nested containers remain graph nodes with diagnostics.

Its repeated-range guard keyed a container by:

```text
source_id | ResourceId.offset | ResourceId.size
```

That model is rejected for clean promotion. `ResourceId.offset` is an offset in the identity's current byte domain, not universally physical archive storage. More importantly, separate declared slots may intentionally share one byte span while remaining distinct slot identities.

The clean implementation therefore separates **resource identity expansion** from **parse-result reuse**.

## Identity rule

Every container `ResourceId` is expanded as its own declared/product identity.

If two physical slots point to the same bytes:

```text
PAC slot 0 -> byte span X
PAC slot 1 -> byte span X
```

both slot identities remain in the graph and both receive their own descendant identities/edges.

A shared byte range is never a reason to discard one alias identity.

## Safe parse-result reuse

Parser reuse is an optimization only; it is not identity deduplication.

`IContainerParser` exposes:

```text
supports_byte_identity_reuse()
```

Default is `false`.

The canonical DMC3 PAC/PNST adapters opt in because the promoted `PacParser` / `PnstParser` structural result depends only on supplied bytes.

Even for an opted-in parser, reuse requires valid `ByteProvenance` and a provenance materialized size equal to the actual payload byte count.

The cache key binds both lineage and content:

- parser identity;
- byte origin kind;
- provenance authority identity;
- byte-domain offset;
- stored size;
- materialized size;
- transform identity;
- optional CRC value where present;
- **SHA-256 of the actual materialized bytes supplied to `parse()`**.

The content hash prevents a malformed/manually constructed payload from reusing another result merely by copying provenance coordinates.

A cache hit reuses only `ContainerParseResult`. `ContainerExpander` still runs for the current parent `ResourceId`, producing distinct child identities and graph edges.

## Budget semantics

Default product safety limits:

```text
max nested depth             = 8
max expanded identities      = 1024
max graph nodes              = 65536
max parser-executed bytes    = 256 MiB
```

The limits are product safety policy, not original DMC3 runtime capacities.

`expanded_container_count` counts resource identities.

`parser_execution_count` counts actual parser calls.

`reused_parse_count` counts safe parse-result cache hits.

`parsed_container_bytes` is charged only to actual parser executions. Reusing a parse result does not consume the parser-byte budget again, while expanding the alias identity still consumes identity/node budgets.

## Canonical DMC3 parser seam

The parser registry was promoted independently in PR #109. Recursive expansion consumes that seam instead of reimplementing or replacing it:

```text
DMC3 registry
  -> canonical PAC adapter -> PacParser
  -> canonical PNST adapter -> PnstParser
```

This slice only marks those adapters explicitly byte-pure for safe parse reuse.

## Synthetic alias regression

The regression fixture contains:

```text
outer PAC
  slot 0 -> nested PNST span X
  slot 1 -> nested PNST span X   (same bytes, distinct slot identity)
  slot 2 -> SCM

nested PNST span X
  slot 0 -> DDS
```

Expected result:

- root PAC plus two nested PNST identities are expanded: `expanded_container_count = 3`;
- only two actual parser executions occur: outer PAC + shared PNST bytes;
- the second PNST identity reuses the content-bound byte-pure parse result;
- both PNST identities remain distinct graph nodes despite equal offset/size;
- each receives its own DDS descendant identity and `contains` edge;
- equal physical/materialized offsets never collapse resource identity;
- depth and parser-byte budgets stop recursion deterministically while preserving already-known graph nodes.

## Evidence boundary

This is a GDSpaces product recursion mechanism over confirmed PAC/PNST structural behavior. It is not a claim that the original DMC3 runtime stores a recursive graph object or uses these product budgets/cache policies.

Original runtime evidence separately confirms PAC member traversal and recursive PNST typed traversal before state-3 readiness. The product tree exists to provide safe materialization/navigation for DMC Rengine tools.

## Still open

- real current-generation nested PAC<->PNST corpus execution receipt;
- clean NBZ integration so transformed-parent lineage is exercised end-to-end;
- `.index` metadata attachment without changing physical topology;
- `.lst` synthesized-container integration;
- Stage Ops ingress and representative lifecycle receipts.
