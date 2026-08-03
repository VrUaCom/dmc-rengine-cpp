# HITS Spatial Differential Validator — 2026-08-03

## Canonical status

- Runtime spatial-list reading and record-offset ABI: **EXE CONFIRMED**.
- Spatial lists in supplied original HITS resources: **GAME VERIFIED**.
- DMC Rengine triangle-box SAT writer: implemented and structurally validated.
- Equivalence with the original Capcom offline builder: **RESEARCH REQUIRED**.

## Purpose

A rebuilt file parsing successfully is necessary but insufficient. It proves structural validity, not that the generated cell ownership matches the original offline builder. This module compares original and candidate spatial lists as stable surface identities rather than raw triangle ordinals.

## Comparison domain

A report is comparable only when:

- both HITS scans are structurally valid;
- bounds, cell sizes and grid dimensions are bit-identical;
- every original and candidate triangle has one unique stable surface ID;
- both triangle-offset domains are mapped completely;
- every cell-list reference resolves through that mapping;
- no cell contains a duplicate reference.

Fit-bounds rebuilds intentionally produce an incompatible-grid result and require a different geometric comparison method.

## Report contents

For every cell:

- original stable surface set;
- candidate stable surface set;
- missing surfaces;
- extra surfaces;
- exact-match state.

For every stable surface:

- original owning cells;
- candidate owning cells;
- missing cells;
- extra cells;
- exact-match state.

Global metrics:

- original, candidate and shared reference counts;
- missing and extra reference counts;
- exact cell count;
- exact surface count;
- precision;
- recall;
- Jaccard similarity;
- full exact-match state.

## Evidence use

The deterministic JSON report is intended for Evidence Packets, Drive synchronization and Knowledge Base receipts. A high similarity score is not by itself proof of original-builder equivalence. Promotion requires consistent corpus-wide results, explanation of every systematic difference and controlled game-runtime validation.

## Next corpus gate

Run the comparator against all available original source-0 and source-1 HITS payloads without committing copyrighted bytes. Record per-resource hashes and generated report hashes. Cluster extra/missing assignments by triangle orientation, boundary contact, cell face/edge/corner contact, flags and source layer. Only then should the writer policy be refined.
