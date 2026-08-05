# HITS Raw Reverse Pass 5/6 Promotion — 2026-08-05

## Authority

This promotion is derived from two canonical Google Drive reverse reports:

- `DMC Rengine — HITS Raw Reverse Pass 5 — Exact Spatial Writer — 2026-08-05`;
- `DMC Rengine — HITS Raw Reverse Pass 6 — Source 1 and Flags Correlations — 2026-08-05`.

No proprietary HITS/PAC/EXE bytes are committed.

## Promoted Pass 5 findings

Evidence status: **GAME VERIFIED**, correlated with prior **EXE CONFIRMED** runtime reads.

- Header `+0x3C` is `spatialTableRelativeOffset`, relative to `fileBase + 8`.
- Header `+0x40` is `triangleArrayRelativeOffset`, relative to `fileBase + 8`.
- The spatial pointer table starts directly at `fileBase + 8 + field(+0x3C)`, normally `0x44`.
- There is no unknown eight-byte prefix before the pointer table.
- There are no two global `-1` guard dwords before the triangle array.
- One pointer exists per flattened cell.
- Each list is physically contiguous, ordered by cell index, sorted by triangle byte offset, and terminated by one signed `-1`.
- The triangle array begins immediately after the final cell-list terminator.
- Triangle-to-cell assignment uses inclusive triangle-vs-AABB overlap with 13 separating axes.
- Reconstructed spatial sections matched all 16 supplied real HITS resources byte-for-byte: 40,789 references, zero missing, zero extra, zero differing bytes.

Runtime/game use of newly modified topology remains **RESEARCH REQUIRED** until controlled game tests.

## Promoted Pass 6 findings

Evidence status: structural and corpus properties **GAME VERIFIED**; exact gameplay naming **RESEARCH REQUIRED**.

- Source 0 maps to PAC member 3 and is the default detailed stage-local HITS source.
- Source 1 maps to optional PAC member 6 and is an independent coarse stage-local HITS source.
- Source 1 corpus: 189 records; 188 use raw flags `0x00000000`, one uses `0x00000001`.
- Source 1 has no upper-16 rejection bits in the supplied corpus.
- Source 1 is not a simple low-detail copy of source 0.
- Do not label source 1 camera/navigation/boundary/player-only without additional caller/runtime proof.
- Raw flags remain a 32-bit value with separate evidence views:
  - upper 16 bits: query rejection/classification mask;
  - lower 16 bits: surface value with unresolved gameplay semantics.
- Lower values 9–12 correlate strongly with class-2 up/down-oriented surfaces, but named semantics remain **RESEARCH REQUIRED**.

## Product impact

- Parser and writer now use the verified pointer-table/list/triangle layout.
- The writer assignment policy is promoted from a DMC Rengine approximation to a corpus-verified Capcom-compatible file algorithm.
- The warning emitted by successful rebuilds now concerns game validation of modified topology, not original-builder equivalence.
- UI/editor consumers may expose source and flag evidence profiles, but must not convert correlations into authoritative enums.

## Remaining gates

- controlled PAC/container rebuild;
- game load and collision tests;
- room transition and restart/reload;
- save/load where relevant;
- exact source-1 gameplay role;
- complete flag semantics;
- original CollisionResult ABI and dynamic-collider merge.
