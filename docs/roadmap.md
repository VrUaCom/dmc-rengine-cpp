# Roadmap

## Phase 0 — Foundation

- C++20 and CMake baseline
- Cross-platform CI
- Stable namespaces and module boundaries
- Evidence and status vocabulary
- No proprietary game data in the repository

## Phase 1 — GDSpaces Core

- `GDResourceId` and source identities
- Read-only source mounting
- Container-chain representation
- Format classification
- Resource graph and diagnostics
- OpenRouter contracts

## Phase 2 — Evidence Tooling

- Binary regions and ownership
- PE/RVA/VA mapping
- EXE evidence packets
- Source hash and expected-byte guards
- Annotation storage

## Phase 3 — Stage Vertical Slice

- EXE-backed stage identity
- `GDStageBundle`
- Open `st001` through one resource path
- Stage Ops and ModViz consume the same resource IDs

## Phase 4 — Editors

- Stage format editors
- Item Editor migration
- ModViz Scene/Model Editor
- ModViz Menu/HUD Editor

## Phase 5 — Safe Export

- Working copies
- Validation
- Patch plans
- Manifests and rollback
- Mod package generation

## Phase 6 — Decompilation and Recompilation

- Evidence-backed recovered C++ units
- ABI and ownership recovery
- Isolated replacement modules
- Runtime comparison harness
- Progressive recompilation toward a working executable
