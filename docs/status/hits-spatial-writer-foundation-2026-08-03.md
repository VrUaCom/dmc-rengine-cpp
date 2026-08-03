# HITS Spatial Writer Foundation — 2026-08-03

## Canonical status

- HITS runtime reader, grid mapping, cell-list ABI and 0x38 triangle-plane record: **EXE CONFIRMED**.
- Existing safe topology-preserving byte patch mode: implemented and retained.
- Deterministic DMC Rengine spatial rebuild policy: implemented in this change.
- Equivalence with Capcom's original offline HITS builder: **RESEARCH REQUIRED**.
- Runtime operation of rebuilt files in the game: **RESEARCH REQUIRED** until controlled game tests.

## Why the rebuild mode is separately labeled

Static EXE saturation recovers the runtime reader but not an original offline writer. Corpus analysis shows that original cell lists are pruned triangle-vs-cell overlap sets and are not reproduced by naive triangle-AABB enumeration.

The new rebuild mode therefore uses a complete triangle-vs-axis-aligned-box separating-axis test. This is a deterministic, conservative DMC Rengine authoring policy. It must not be described as recovered Capcom source or as binary-identical to the unknown original builder.

## Implemented contracts

- Stable non-zero surface IDs independent of output order.
- Raw 32-bit flags preserved without normalization.
- Normal and plane D recomputed from point A/B/C winding.
- Non-finite and degenerate geometry rejected.
- Preserve-source-grid policy rejects out-of-bounds surfaces.
- Fit-bounds policy preserves source cell size and recomputes bounds/grid counts.
- Full triangle-vs-cell SAT assignment.
- Deterministic ascending record-offset order inside each cell.
- One unique relative pointer per grid cell.
- Signed int32 record byte offsets, divisible by 0x38.
- `-1` terminator for every cell list.
- Two `-1` guard dwords before the terminal triangle array.
- Canonical `+0x3C/+0x40`, triangle count and endOffset updates.
- Unknown eight-byte spatial prefix preserved from the source resource.
- Final file size aligned to 16 bytes with zero padding.
- Mandatory canonical parser round trip before a result is accepted.

## Safety boundary

A successful rebuild is structurally valid according to the merged parser/runtime specification. It is not release-ready evidence of original-game compatibility. Rebuilt resources require controlled runtime tests against source 0 and source 1, room transitions and restart/reload before their evidence status can advance.
