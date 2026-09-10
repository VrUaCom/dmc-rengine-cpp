# How to Open and Inspect DMC3 SCM Files

This guide targets searches such as **open DMC3 SCM file**, **DMC3 SCM viewer**, **DMC3 stage model format**, and **Devil May Cry 3 SCM format**.

SCM is a scene-oriented resource family. DMC Rengine models scene nodes, object bindings, meshes, streams, transforms and texture-facing state through the canonical parser/IR rather than collapsing the format into an editor-only representation.

## What SCM inspection covers

The canonical path exposes hierarchy, object/mesh relationships, positions, normals, UV data, topology reconstruction, transforms and evidence-backed rendering-facing fields.

## Current writer/rebuild authority

PR #372 promotes one selected canonical SCM authoring stack:

- `preserve_layout` for source-bound same-layout edits;
- deterministic `canonical_rebuild` from typed IR;
- typed edits for position, normal, UV, texture slot, alpha, nearest-filter bit, GS CLAMP REGION_REPEAT and node translation/rotation;
- dependent metadata derivation;
- mandatory canonical output reparse;
- source-bound mutation guards;
- fail-closed canonical reflow if retained source contains non-zero unmodeled bytes;
- bounded SCM/texture-companion coherence through `ScmResourceBundleWriter` and the existing texture framing/reflow implementation.

The consolidated no-edit corpus is:

```text
paths                           78
unique SHA-256 inputs           68
parse                           78/78 PASS
preserve-layout exact parity    78/78 PASS
canonical rebuild + reparse     78/78 PASS
canonical exact no-edit parity  78/78 PASS
```

This closes that explicit no-edit corpus gate. It does **not** prove universal stage editing, every retail SCM variant, Capcom offline-tool equivalence or original-game acceptance.

## Still open

- provenance-bound representative same-layout semantic edits across SCM domains;
- provenance-bound retail texture rewrite;
- real-retail size-changing canonical rebuild;
- SCM PAC/PNST/NBZ reintegration;
- original `dmc3.exe` acceptance.

## Why SCM is different from MOD

MOD and SCM both contain geometry-related information, but they serve different resource roles and have different physical/semantic contracts. Shared model-family infrastructure is not permission to copy field meanings across formats.

## Ecosystem workflow

Use GDSpaces/PocketGDS to locate and materialize the containing resource, DMC Rengine for canonical SCM parsing/writing evidence, and DMC Native Reader where the current client exposes scene/model inspection capabilities.

For the exact current authority boundary, use `docs/research/dmc3-model-formats-unified-frontier-2026-09-09.md` and `docs/status/current.md`.
