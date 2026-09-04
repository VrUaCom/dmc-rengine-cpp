# Architecture decision: SCM + MOD as one DMC3 Model Family

**Date:** 2026-09-04  
**Integration branch:** `model-family`  
**Status:** ACCEPTED FOR REVERSE/INTEGRATION, NOT A MAIN-MERGE DECISION

## Decision

DMC3-HD `.SCM` and `.MOD` are developed and reversed together as one **Model Family**. They share one semantic/tooling layer for model inspection, visualization, validation and future authoring, while retaining separate binary adapters and evidence boundaries.

This decision replaces the workflow where SCM reverse and MOD skinning were advanced in isolated feature branches.

## Why

Both formats expose closely related model-domain concepts:

- mesh geometry;
- positions and normals;
- fixed-point UV data;
- generated topology/runtime workspaces;
- node/transform domains;
- model visualization requirements;
- texture/material concerns;
- eventual ModViz editing and archive reintegration.

MOD extends this family with a skeleton/skin layer. SCM extends it with stage-scene semantics, external texture-companion binding, alpha control and recovered legacy GS texture state.

The commonality is strong enough to justify one toolkit. It is **not** evidence that the serialized layouts are interchangeable.

## Non-negotiable boundary

Do not implement:

```text
SCM = MOD without skin
```

as a binary-layout assumption.

The correct architecture is:

```text
                    ModelFamily semantic layer
                              |
          +-------------------+-------------------+
          |                                       |
      SCM adapter                                MOD adapter
          |                                       |
  SCM binary contract                      MOD binary contract
```

Format-specific parsers remain responsible for offsets, record strides, flags, runtime-only fields, preservation policy and writer constraints.

## Shared semantic surface

The common toolkit should progressively own reusable concepts:

```text
ModelFamily
├── geometry
├── normals
├── UV
├── topology
├── node hierarchy
├── local/world transforms
├── bounds
├── texture/material presentation
├── validation/diagnostics
├── ModViz model view
└── authoring/acceptance orchestration
```

The initial capability registry is exposed by:

```text
include/dmc_rengine/formats/model_family.hpp
```

It deliberately models capabilities rather than forcing one large union structure.

## SCM capabilities

Currently evidence-backed:

- geometry, normals and fixed-point UV;
- topology reconstruction;
- scene-node hierarchy;
- XYZ-radian transforms, world matrices and inverse-world cache;
- external texture-companion slot binding;
- alpha-control path;
- legacy GS CLAMP REGION_REPEAT state;
- legacy GS TEX1 linear/nearest filter selection;
- experimental preserve-layout and canonical-rebuild writer paths.

SCM does **not** gain a skin-weight stream merely because MOD has one.

## MOD capabilities

Currently evidence-backed or bounded-high-confidence:

- geometry, normals and fixed-point UV;
- topology/control stream;
- node/transform domain;
- per-vertex BLENDINDICES-correlated matrix-row indices;
- three packed 5-bit skin weights normalized by `/31`;
- maximum three non-zero serialized influences in the recovered revision;
- skeleton/node domain cross-binding.

MOD production writer authority remains blocked until skin-matrix palette ownership, bind/inverse-bind construction, wider corpus stability and original-game acceptance are closed.

## Skin-weight visualization

Skin weights are a MOD-only capability exposed through the shared Model Family UI.

ModViz should present a single Model/Skeleton toolset:

```text
Open Model
├── Geometry
├── UV
├── Hierarchy
├── Transforms
├── Materials/Textures
└── Skeleton / Skin Weights   [only when capability exists]
```

Selecting a bone must allow a per-vertex heat map of its normalized influence. SCM simply reports `skeletal_skinning = false` and does not show this panel.

## Branch policy

- `scm` remains an historical/checkpoint branch for the deep SCM reverse and writer work.
- `mod-skin-reverse` remains an historical/checkpoint branch for the initial skin-weight closure.
- `model-family` becomes the active integration branch for all further SCM + MOD reverse and shared tooling.
- no automatic merge into `main` until format-specific acceptance gates are met.

## Next reverse priorities

1. Import MOD transform-domain and skin decoder evidence into `model-family`.
2. Build a common high-level ModelView/adapter boundary without copying binary structs.
3. Close MOD skin matrix palette owner and exact shader matrix selection.
4. Recover bind pose and inverse-bind construction.
5. Compare SCM and MOD hierarchy/transform code paths to extract genuinely shared math helpers.
6. Extend corpus verification to accept both `.scm` and `.mod` and report per-format capabilities.
7. Keep writer maturity independent: SCM experimental authoring must not imply MOD authoring authority.

## Definition of success

A client such as ModViz can open either SCM or MOD through the same model-facing API and reuse geometry, UV, hierarchy, transform and visualization tools, while format-specific features appear only when proven and supported.
