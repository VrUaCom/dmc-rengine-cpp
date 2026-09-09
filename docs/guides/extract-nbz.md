# How to Extract Files from DMC3 NBZ Archives

This guide is for people searching for a **DMC3 NBZ extractor**, **Devil May Cry 3 HD unpacker**, or a way to browse files stored in the HD Collection's numbered archives.

DMC Rengine treats NBZ extraction as part of a provenance-aware materialization path rather than as a blind archive dump. The current product path can index classic ZIP-backed NBZ volumes, materialize STORE and raw-DEFLATE members, preserve CRC/size/SHA observations, and carry byte provenance into nested container work.

## What an NBZ archive contains

DMC3 HD discovers numbered volumes such as `DMC3-0.nbz`. A materialized NBZ member may itself be a PAC or PNST container, so finding the archive member is often only the first layer of extraction.

Typical resource traversal is:

```text
DMC3-*.nbz
  -> archive member
  -> PAC / PNST when present
  -> physical slot
  -> nested resource
  -> MOD / SCM / DDS / PTX / TXT / other typed payload
```

DMC Rengine keeps archive/member identity and nested slot identity separate. That matters for reverse engineering and later reintegration because a resource's provenance is more useful than a folder containing anonymous extracted bytes.

## What is supported now

The canonical project status records NBZ classic ZIP indexing/materialization, STORE plus raw-DEFLATE extraction, integrity/provenance observations, PAC/PNST recursive expansion, immutable NBZ copy rebuild, and a bounded next-contiguous STORE overlay authoring path.

Those product capabilities do **not** mean every edited resource is already proven to work in the original game. Original-process resolver selection, consumer-visible acceptance, and final L1/L2/L3 completion remain separate proof gates.

## When you need PAC or PNST next

If the extracted member is a PAC or PNST container, continue through the container layer instead of treating the outer member as the final resource. DMC Rengine preserves sparse and empty slots, aliases, relative-slot relationships, and nested slot paths so the materialized child keeps its physical identity.

For model or texture work, the useful path is usually NBZ -> PAC/PNST -> resource. From there use the dedicated MOD/SCM or DDS/PTX research and inspection guides.

## Search-intent boundary

This page intentionally uses words such as **unpack** and **extract** because NBZ materialization is a real product capability. It does not use those words as evidence that every possible modified archive or nested resource has complete original-game authoring acceptance.

For exact maturity and proof gates, see `docs/status/current.md`, `docs/gdspaces/`, and the canonical PAC/PNST documentation.
