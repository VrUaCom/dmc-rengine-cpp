# How to Extract DMC3 PAC Files

PAC is one of the container layers used by Devil May Cry 3 HD Collection resources. In DMC Rengine it is treated as a slot-preserving container rather than as a flat folder of independent files.

## What you can do today

The canonical GDSpaces path can parse PAC containers, preserve sparse and empty slots, recursively expand nested resources and materialize child payloads without normalizing away physical slot identity. Size-changing relative-slot reflow is also implemented for the supported authoring path.

A practical extraction workflow is therefore:

```text
NBZ member
  -> PAC container
  -> physical slot
  -> nested container or resource
  -> materialized bytes
```

Use the physical slot index and provenance as part of the extracted resource identity. Two payloads that look identical at the file level can still represent different container positions.

## Why PAC extraction is not just unzip

PAC offsets, sparse entries and nested payload relationships matter. DMC Rengine keeps those relationships available to downstream tools instead of flattening the archive into anonymous output files.

A materialized child can then be routed through the broader DMC Rengine C++ Native Reader integration registry when its format identity is established. The current DMC Native Reader Android application has a narrower promoted `main` surface — MOD, SCM, DDS and PTX — so C++ registry coverage must not be advertised as identical Android product coverage.

## What this guide does not claim

PAC parsing and materialization do not prove that every arbitrary modified payload can be safely rebuilt and accepted by the original DMC3 runtime. Reintegration, nested-resource validity and original-game acceptance are separate evidence gates.

For technical structure see the PAC format page and GDSpaces documentation; for current maturity see `docs/status/current.md`. For application-facing inspection claims, use the current DMC Native Reader app baseline rather than the wider C++ integration registry.
