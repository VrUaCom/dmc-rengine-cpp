# DMC Rengine social preview asset

This directory contains the project-owned source and raster delivery asset for the canonical DMC Rengine repository social preview.

## Approved canonical design

The project owner approved this visual identity on 2026-09-07.

The card binds **DMC Rengine** to **DMC3 HD Reverse Engineering** and the repository's evidence-first engineering model without using Capcom artwork, screenshots, logos, extracted textures/models, character art, or other proprietary game assets.

Primary copy:

```text
DMC Rengine
DMC3 HD Reverse Engineering
C++20 · File Formats · Evidence-First Tooling
NO CLAIM WITHOUT EVIDENCE
```

Canonical visual language:

- dark technical grid background;
- crimson/red engineering linework and accent rule;
- a triangular mark containing a single eye and one tear;
- one coin-like circular inscription band around the triangle;
- ring inscriptions: `DEVIL MAY CRY`, `DMC RENGINE`, `MONKS OF BINARY CODE`, `SECT OF NEUROSLOP`;
- faint hexadecimal/byte-string field as decorative reverse-engineering texture;
- no proprietary Capcom/game imagery.

## Canonical assets

- `social-preview.svg` — editable deterministic 1280×640 source.
- `social-preview.png` — approved 1280×640 raster delivery target for GitHub repository settings.

Approved raster SHA-256:

```text
a81a726bcff355cad5a6b25ecc7d35570d178ffdc3f1dddce8750e45f17bf21f
```

The approved PNG is retained as the repository social-preview settings payload. If repository settings cannot be written by the active connector, applying that setting remains an explicit settings-only gate; the canonical editable source and raster hash stay versioned here.

## Evidence boundary

The asset must not be modified to imply completed whole-game decompilation, behaviorally equivalent recompilation, Capcom tooling equivalence, or any other capability not promoted by `docs/status/current.md`.

The phrases `MONKS OF BINARY CODE` and `SECT OF NEUROSLOP` are project lore/identity language, not technical capability claims.

Related: issue #301 and `docs/discovery/README.md`.
