# How to Open and Inspect DMC3 MOD Files

This guide targets searches such as **open DMC3 MOD file**, **DMC3 MOD viewer**, **DMC3 model viewer**, **DMC3 MOD editor**, and **Devil May Cry 3 MOD format**.

## MOD reverse status

**DMC3 HD `.MOD` reverse is complete for the canonical project scope.**

That means the serialized format, runtime-facing structure and preservation boundaries have all reached terminal evidence states. Some bytes intentionally remain `PRESERVED_UNDECODED`: their storage and runtime carriage are understood, but the project does not invent artistic/material names that the executable evidence does not prove.

This is a completed reverse contract, not a promise that every field is freely writable.

## What the canonical MOD reader exposes

The completed MOD model covers:

- document, object and mesh records;
- position, normal and fixed-point UV streams;
- blend indices, packed skin weights and topology control;
- hierarchy/order relationships and default-joint behavior;
- local transforms, world propagation and inverse-rest/current-world skin palette construction;
- mesh texture slots, texture-companion validation and runtime texture descriptor relationships;
- post-load relocation and generated topology workspace behavior;
- object bounds, alpha/runtime flag projection and legacy GS-facing state;
- the MOD-side boundary into MOT/CMotion animation evaluation;
- source-byte preservation for fields whose strongest honest semantic remains preservation-only.

That is enough for structural inspection, skeleton and weight visualization, texture-binding analysis, pose-aware research and a complete evidence-backed map of the MOD resource.

## Why “reverse complete” can still include preserved fields

Reverse engineering is complete when every relevant byte/field/path has a defensible terminal classification, not when every value has a convenient English label.

Examples:

- header `+0x14` is runtime-carried, while the old universal decimal interpretation is rejected;
- `BLENDINDICES.x` is part of the serialized ABI but is not consumed by the audited shader path;
- source flag `0x00200000` is preserved through the recovered runtime state without a proven artistic label;
- mesh `+0x0C/+0x4C`, mesh `+0x38` and transform `+0x1C` have explicit preservation/family-sensitive contracts.

Those are closed evidence outcomes rather than remaining MOD reverse tasks.

## What the preserve-layout writer can do

MOD Preserve-Layout Writer Gate 1 starts from an immutable original serialized image and allows only fixed-size edits whose byte spans are already authorized. The current edit surface includes object bounding center/radius and existing position, normal and UV values.

The writer protects every byte outside those spans and reparses output through the canonical MOD parser before success.

The provenance-bound retail no-op corpus gate passes:

```text
38 / 38 MOD files
882,736 source bytes
38 / 38 exact byte equality
38 / 38 canonical reopen
0 unauthorized modified bytes
```

A controlled retail edit to `em000_021.mod` changes exactly three bytes inside the serialized `bounding_radius` span and passes independent raw diff, disk hash/reread and canonical reopen.

## Container reintegration already proved

The authored MOD child has a provenance-bound real retail PNST reintegration receipt in `m20_s00_012.pac` physical slot 23:

- parent size unchanged;
- slot table unchanged;
- only the expected authored child bytes changed;
- canonical parent reparse/re-expand reproduces the exact MOD writer output.

A synthetic MOD -> container -> NBZ overlay -> reopen chain also passes through the existing NBZ infrastructure.

## What remains outside the claim

These are authoring/integration/acceptance tasks, not unfinished MOD reverse:

- typed-IR-only layout synthesis and structural reflow;
- transform and skin authoring;
- material/source-flag/texture-binding authoring;
- texture-companion rewriting;
- unrestricted mutation of preservation-only fields;
- provenance-bound retail NBZ acceptance;
- original `dmc3.exe` selection and acceptance of rebuilt or edited MOD resources;
- rollback-backed original-game acceptance.

The project therefore distinguishes **MOD reverse complete** from **full MOD writer complete**.

## Related resources

Animation evaluation is owned by the neighboring MOT/CMotion authority, and texture payload authoring belongs to the texture-companion/TIM2/DDS/PTX stack. Those dependencies do not reopen the MOD binary reverse.

Use DMC Native Reader where the current application exposes model inspection, and GDSpaces/PocketGDS to reach the physical archive/container slot that supplied the file.

For exact field-level evidence and writer receipts, follow the canonical MOD research and status documents linked from the format page.
