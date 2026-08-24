# GDSpaces L1 Completeness Estimate

**Snapshot:** 2026-08-24  
**Canonical main:** `ad13a8797b8c2dd5668e9029e16664415968fcbb`  
**Status:** planning estimate only; mandatory acceptance gates remain authoritative.

## Current percentages

| Axis | Estimate | Meaning |
| --- | ---: | --- |
| Implementation completeness | **94%** | Canonical L1 read/materialize/rebuild/reopen product mechanics implemented at bounded scope. |
| Reverse/evidence completeness | **88%** | Original-runtime boundaries needed by GDS are mostly recovered; several exact physical/I/O/lifetime edges remain open. |
| Validation/acceptance completeness | **62%** | Synthetic and product-side receipts are strong, but direct-retail representative provenance and original-game Level-E consumption remain open. |
| Combined engineering completeness | **84%** | Non-authoritative planning estimate across implementation, reverse/evidence and acceptance. |

These numbers must never be used to claim original-game equivalence. `L1 COMPLETE` requires every mandatory gate in `canonical-status.json` and `l1-roadmap.md` to be closed or explicitly evidence-pruned.

## Why implementation is high

Canonical `main` already contains NBZ STORE/raw-DEFLATE acquisition, artifact-bound member observation, shared no-replace publication, PAC/PNST parsing and size-changing reflow, nested reintegration, texture safe-subset authoring/runtime relocation validation, numbered-volume precedence, next-volume overlay generation, verified retail-member acquisition and verified immutable-source NBZ copy rebuild.

PR #201 is the active user-facing PAC/PNST relative-slot rebuild seam and is not counted as canonical until merged.

## Why acceptance is lower

The remaining high-value evidence gates are external to synthetic CI:

1. acquire a representative direct-retail request such as `obj\\em000.pac` from a real installation and record the resolver winner plus exact artifact/member hashes;
2. classify the observed retail representation against proven writer domains;
3. perform a real-retail bounded edit and bottom-up rebuild;
4. publish/reopen/rematerialize the rebuilt resource with exact receipts;
5. prove consumption by the protected original DMC3 executable;
6. complete the final cross-stack L1 acceptance audit.

## Update rule

Percentages should move only when a material canonical capability or evidence gate changes. Branch-only work may be described separately but must not increase the canonical implementation percentage until merged. Acceptance percentage should not increase for synthetic-only coverage when the missing gate explicitly requires retail or original-process evidence.
