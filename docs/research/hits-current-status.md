# HITS integration current status

**Canonical integration branch:** `hits`  
**Canonical DMC3 executable SHA-256:** `e454272ed0fb0247fcbcf300e5d55d7a3e96d50b89b9ffaff81bb978dcbdd082`

## Current implementation

- Four-byte `HITS` magic is canonical; obsolete `HITS$` recognition is rejected.
- Header `0x44`, 3-D grid, `-1`-terminated cell lists and `0x38` triangle-plane records are parsed.
- `0x18060001` is preserved as an observed raw flag value, not treated as a universal record marker.
- Binary Inspector exposes header/grid/flags/points/normal/plane-D fields.
- Game-agnostic modules cover runtime grid mapping, static-source selection, contact helpers, spatial comparison/matching and editing/writer foundations.
- DMC3-specific Pass 8–10 reverse ABI is isolated under `profiles/dmc3` and SHA-gated to the canonical executable.
- Pass 10 query evidence covers the combined wrapper, static HITS pass, dynamic-category passes, source switching, helper anchors and specialized query variants without inventing one monolithic result ABI.
- Stage-CFG collision evidence models `0x04` entries, `0x50` primitive descriptors, modern slots 39/40, legacy CEM008 slots 22/23, reference validation and referenced-descriptor census.
- Transform-selector bounds/ownership remain explicitly unresolved.
- Structured evidence packets live in `evidence/hits/`.
- Canonical synthesis: `docs/research/hits-canonical-reverse-through-pass10.md`.
- Reverse depth taxonomy: `docs/research/decompilation-depth-layers.md`.
- Machine-readable canonical snapshot: `data/reverse/dmc3-hits-canonical-20260904.json`.

## Modular architecture

1. `formats/hits.*` — shared binary format authority, no executable addresses.
2. `hits/*` — game-agnostic geometry/spatial/contact/edit/writer logic.
3. `profiles/dmc3/hits_*` — exact-build DMC3 reverse evidence and ABI.
4. `evidence/hits/*` — structured receipts and preservation evidence.
5. `docs/research/*hits*` — human-readable reverse synthesis and open gates.

This boundary follows the same principle used by the `scm` work: shared format code must not become a dumping ground for one executable's addresses or unproven gameplay names.

## Decompilation depth

- serialized format/layout: closed structurally through DL2;
- recovered static/dynamic query pipeline: DL6 for the proven canonical paths;
- stage-CFG collision binding: DL6 for the proven slot/descriptor path;
- universal flag semantics: open beyond local proven masks/filters;
- transform-selector game semantics: open;
- topology-changing writer acceptance in the original game: no DL8 claim.

Depth and confidence are separate axes.

## Remaining gates

1. Exact semantic names for raw flag bits/masks.
2. Stage-CFG transform-selector source ownership and bounds.
3. Remaining primitive type semantics and runtime-only categories.
4. Remaining specialized query/result ABI gaps retained by Pass 10.
5. Original-game validation of topology-changing spatial rebuilds.

No merge to `main` is authorized by this status document. The `hits` branch remains the review and integration surface until semantic reconciliation, evidence audit and Windows/Ubuntu CI are clean.
