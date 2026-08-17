# GDSpaces Recursive Container Tree Expansion

**Reconciled:** 2026-08-17  
**Status:** CLEAN IMPLEMENTATION SLICE / SYNTHETIC REGRESSION; REAL NESTED CORPUS RECEIPT REMAINS SEPARATE

## Purpose

`ContainerTreeExpander` is the generic GDSpaces product mechanism for recursively expanding already-materialized nested containers through a supplied `ContainerParserRegistry`.

It does not open source paths, resolve game resources, emulate original `LoadedResource`, or assign gameplay semantics. It consumes `ResourcePayload` objects and parser results only.

## Reconciliation origin

Historical PR #58 already had the correct high-level ideas:

- generic recursion;
- partial-result preservation;
- explicit depth/container/node/byte budgets;
- unsupported nested containers remain graph nodes with diagnostics.

However, the historical repeated-range guard keyed a container by:

```text
source_id | ResourceId.offset | ResourceId.size
```

That is no longer a safe universal byte identity after `ByteProvenance` reconciliation. `ResourceId.offset` is an offset in the resource identity's current byte domain; after transforms it is not automatically a physical archive-storage coordinate. In addition, separate declared slots may intentionally share one byte span while remaining distinct slot identities.

The clean implementation therefore separates **resource identity expansion** from **parse-result reuse**.

## Identity rule

Every container `ResourceId` is expanded as its own identity.

If two physical slots point to the same bytes:

```text
PAC slot 0 -> byte span X
PAC slot 1 -> byte span X
```

both slot identities remain in the graph and both receive their own descendant identities/edges.

The tree expander never uses shared byte range as a reason to discard one alias identity.

## Safe parse-result reuse

Parser reuse is an optimization only; it is not identity deduplication.

`IContainerParser` now exposes:

```text
supports_byte_identity_reuse()
```

Default is `false`.

A parser may opt in only when its parse result depends solely on supplied bytes rather than logical path/context. The clean DMC3 PAC and PNST adapters opt in because `PacParser` / `PnstParser` are byte-structural decoders.

Even for an opted-in parser, reuse is allowed only when the payload carries valid `ByteProvenance`.

The cache key includes:

- parser identity;
- byte origin kind;
- provenance authority identity;
- byte-domain offset;
- stored size;
- materialized size;
- transform identity;
- optional CRC value where present.

A cache hit reuses only `ContainerParseResult`. `ContainerExpander` is still run for the current parent `ResourceId`, producing distinct child identities and graph edges.

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

`parsed_container_bytes` is charged only to actual parser executions. Reusing a parse result does not consume the byte budget again, while expanding the alias identity still consumes container/node budgets.

## DMC3 parser registry

`profiles::dmc3::make_container_parser_registry()` registers adapters for the canonical clean format authorities:

```text
PAC  -> PacParser
PNST -> PnstParser
```

The adapters do not contain another PAC/PNST decoder.

This closes the old architecture risk in which profile integration could accidentally restore a second format authority beside the clean parsers.

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
- the second PNST identity reuses the byte-pure parse result;
- both PNST identities remain distinct graph nodes despite equal offset/size;
- each receives its own DDS descendant identity and `contains` edge;
- equal physical/materialized offsets never collapse resource identity;
- depth and byte budgets stop recursion deterministically while preserving already-known nodes.

## Evidence boundary

This is a GDSpaces product recursion mechanism over confirmed PAC/PNST structural behavior. It is not a claim that the original DMC3 runtime stores a recursive graph object or uses these product budget/cache policies.

Original runtime evidence separately confirms recursive PNST typed traversal and PAC member traversal before state-3 readiness. The product tree exists to provide safe resource materialization/navigation for DMC Rengine tools.

## Still open

- real current-generation nested PAC<->PNST corpus execution receipt;
- integration with clean NBZ source so transformed parent provenance is exercised end-to-end;
- `.index` metadata attachment without changing physical topology;
- `.lst` synthesized-container integration;
- Stage Ops ingress and representative lifecycle receipts.
