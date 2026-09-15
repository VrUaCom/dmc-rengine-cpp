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

Real hash-bound same-layout receipts additionally cover alpha-control, node translation and node rotation through parent PAC reintegration and canonical reopen/extraction.

PR #386 adds a bounded real-retail size-changing path. On hash-bound `st001.scm`, `append_break_vertex_copy()` grows object 0 / mesh 0 from 167 to 168 vertices, keeps non-degenerate strip triangles at 118, grows the SCM by 16 bytes, reparses canonically, then reintegrates the authored child into physical slot 2 of real `st001.pac`. The parent grows by the same 16 bytes; re-extraction returns the exact authored SCM and all seven non-target top-level physical slots remain byte-identical.

Machine receipt: `data/reverse/dmc3-scm-real-size-changing-pac-reintegration-attestation-20260913.json`.

This closes that explicit real-retail size-changing SCM -> PAC gate. It does **not** prove universal stage editing, every retail SCM variant, Capcom offline-tool equivalence, NBZ delivery or original-game acceptance.

## Still open

- provenance-bound representative semantic edit receipts where additional writer-domain promotion is useful;
- provenance-bound retail texture rewrite;
- broader PNST authored-resource delivery for SCM;
- retail NBZ authored-resource delivery and resolver selection;
- original `dmc3.exe` acceptance.

## Why SCM is different from MOD

MOD and SCM both contain geometry-related information, but they serve different resource roles and have different physical/semantic contracts. Shared model-family infrastructure is not permission to copy field meanings across formats.

## Ecosystem workflow

Use GDSpaces/PocketGDS to locate and materialize the containing resource, DMC Rengine for canonical SCM parsing/writing evidence, and DMC Native Reader where the current client exposes scene/model inspection capabilities.

For the exact current authority boundary, use `docs/research/dmc3-model-formats-unified-frontier-2026-09-09.md` and `docs/status/current.md`.
