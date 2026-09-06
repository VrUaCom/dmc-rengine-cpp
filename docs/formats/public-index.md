# DMC3 HD File Formats and Archives — DMC Rengine Index

This page is a public navigation layer for **Devil May Cry 3 HD Collection (DMC3 HD) file-format and archive research** in DMC Rengine.

It does not replace the canonical [format catalog](dmc3-hd-format-catalog.md), [presence census](dmc3-hd-format-presence-census.md), machine-readable registries, or format-specific evidence. Use those sources for exact claims.

## Start by question

| I want to understand… | Start here |
| --- | --- |
| Which DMC3 HD resource families are currently known? | [Format and resource-purpose catalog](dmc3-hd-format-catalog.md) |
| Which formats have real bound payloads versus EXE-only identities? | [Format presence census](dmc3-hd-format-presence-census.md) |
| Which formats/readers are actually promoted in the current product? | [Current project status](../status/current.md) |
| How does DMC Rengine classify resources without trusting extensions alone? | [GDSpaces resource classification](../gdspaces/classification.md) |
| What is the machine-readable semantic inventory? | [Format-purpose registry](dmc3-hd-format-purpose-registry.json) |

## DMC3 SCM format

SCM has dedicated structural and reverse-engineering documentation. Do not reduce SCM to a generic “3D file” label: scene hierarchy, object/mesh structure, streams, transforms, runtime flag projection and writer boundaries are documented separately.

- [SCM structural specification](scm.md)
- [DMC3 SCM deep reverse](../research/dmc3-scm-deep-reverse-2026-09-02.md)
- [DMC3 SCM runtime object flags](../research/dmc3-scm-runtime-object-flags-2026-09-03.md)

Current product status and write/edit maturity must be taken from [Current Project Status](../status/current.md), not inferred from the existence of the specification.

## DMC3 MOD format

MOD is part of the current canonical Native Reader set and has corpus/runtime reverse work. For exact stream/layout evidence and current support boundaries use:

- [Primary 3D / render family reverse](../research/dmc3-primary-3d-render-family-reverse-2026-08-31.md)
- [Real MOD / SHW payload binding](../research/dmc3-real-mod-shw-payload-binding-2026-09-01.md)
- [Format catalog](dmc3-hd-format-catalog.md)
- [Current Project Status](../status/current.md)

Reader support does not automatically imply universal MOD writing, conversion or original-game equivalence.

## DMC3 SHW format

SHW is currently represented by an evidence-bounded structural Native Reader. The promoted scope is deliberately narrower than a universal SHW specification.

- [Real MOD / SHW payload binding](../research/dmc3-real-mod-shw-payload-binding-2026-09-01.md)
- [Primary 3D / render family reverse](../research/dmc3-primary-3d-render-family-reverse-2026-08-31.md)
- [Current Project Status](../status/current.md)

Do not infer a SHW writer, universal revision coverage or complete matrix-palette semantics unless a later canonical promotion explicitly closes those gates.

## DMC3 HITS collision format

The current HITS model is header-driven and supersedes the rejected historical `HITS$` / universal fixed-marker interpretation.

- [HITS collision resource](hits.md)
- [Format catalog](dmc3-hd-format-catalog.md)

The repository also contains product-side spatial validation/writer work, but Capcom offline-builder equivalence is not implied by a successful DMC Rengine writer.

## DMC3 PAC format

PAC is handled as a relative-slot container family in the current product architecture. Sparse slots, empty slots and alias identity matter and must not be flattened away by simplified extraction models.

- [PAC structural parser](pac-readonly-parser.md)
- [GDSpaces classification](../gdspaces/classification.md)
- [Layout-preserving relative-slot writer](../gdspaces/dmc3-layout-preserving-relative-slot-writer.md)
- [Runtime-synth relative-slot writer](../gdspaces/dmc3-runtime-synth-relative-slot-writer.md)
- [Nested relative-slot reintegration](../gdspaces/dmc3-nested-relative-slot-reintegration.md)

## DMC3 PNST format

PNST shares relative-slot physical/container infrastructure with PAC in supported paths, but DMC Rengine does **not** assume one global semantic slot schema for both families.

- [PNST structural parser](pnst-readonly-parser.md)
- [GDSpaces classification](../gdspaces/classification.md)
- [Nested relative-slot reintegration](../gdspaces/dmc3-nested-relative-slot-reintegration.md)

## DMC3 NBZ archives

NBZ is treated as a source/materialization layer for numbered DMC3 archive volumes rather than as a Native Reader resource module.

Relevant documentation:

- [GDSpaces contract](../gdspaces-contract.md)
- [DMC3 runtime resource resolver](../gdspaces/dmc3-runtime-resource-resolver.md)
- [DMC3 resource lookup policy](../gdspaces/dmc3-resource-lookup-policy.md)
- [NBZ STORE overlay writer](../gdspaces/dmc3-nbz-store-overlay-writer.md)
- [NBZ retail serialization preservation](../gdspaces/dmc3-nbz-retail-serialization.md)

A synthetic/reopened NBZ success does not by itself prove that the original DMC3 runtime selected and consumed the authored resource. The live proof gates remain in [Current Project Status](../status/current.md).

## DMC3 textures: DDS and PTX

DDS and PTX are in the current canonical Native Reader set. Texture recognition/read support must still be distinguished from editing/writer support and from universal coverage of every texture variant.

Use the [format catalog](dmc3-hd-format-catalog.md), [presence census](dmc3-hd-format-presence-census.md), and [current status](../status/current.md) for exact promoted boundaries.

## Other DMC3 resource families

DMC Rengine tracks additional families and working identities beyond the public highlights above. Some are purpose-confirmed, some are high-confidence/candidate, some are EXE-only references, and some remain research-required.

Use:

- [Canonical format and resource-purpose catalog](dmc3-hd-format-catalog.md)
- [Format presence census](dmc3-hd-format-presence-census.md)
- [Residual format census](../gdspaces/l3-residual-format-pass-2026-08-26.md)

Do not promote an extension, filename, short ASCII sequence or historical working name into a format semantic claim without stronger evidence.

## Evidence rule

The public index follows the repository’s normal authority order:

```text
validated magic / structural grammar
 -> original EXE consumer or typed post-load path
 -> hash-bound corpus structure
 -> index/path label
 -> fallback/unknown
```

A known resource purpose does not imply a complete binary schema. A parser does not imply a writer. A writer does not imply Capcom-tool equivalence. A synthetic test does not imply original-game behavioral equivalence.
