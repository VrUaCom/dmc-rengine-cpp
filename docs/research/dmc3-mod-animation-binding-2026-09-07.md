# DMC3 MOD — canonical animation-binding composition (2026-09-07)

## Authority

Canonical executable:

`dmc3.exe` SHA-256 `e454272ed0fb0247fcbcf300e5d55d7a3e96d50b89b9ffaff81bb978dcbdd082`

Canonical base for this bounded promotion:

`main@6630f8cafa3f69d2eee8084da9e8feb479d38fd4`

## Scope

This pass canonicalizes only the **MOD-owned side of animation binding**. It composes already-promoted EXE-confirmed contracts into one analysis module:

```text
MOD hierarchy/order domain
        +
MOD motion_group projection
        +
caller-owned evaluated animated-local matrices
        ↓
currentWorld[node]
        ↓
inverseRestWorld[node] * currentWorld[node]
        ↓
skin palette
```

It does **not** decode MOT bytes, assign semantics to MOT tracks/channels, mutate serialized MOD rest-local transforms, or establish writer authority.

## 1. MOD node domain -> CMotion selector

The immediately preceding motion-group pass closed the third serialized node-domain table as a CMotion selector.

At `0x14030F850`, the runtime binds model node domains into CMotion. The relevant bounded flow is:

- manager `+0x10` (`nodeAtOrderPosition`) -> CMotion `+0x08`;
- manager `+0x08` (`parentByOrderPosition`) -> CMotion `+0x10`;
- manager `+0x18` (third serialized node-domain table) -> CMotion `+0x18`;
- `0x14030F99A` reads the third-domain byte at hierarchy order position;
- `0x14030F99E..0x14030F9A6` maps that order position through `nodeAtOrderPosition`;
- `0x14030F9AE` stores the byte in the corresponding CMotion joint at `+0xF8`.

The resulting `joint +0xF8` is repeatedly compared against a requested group id, including `0x14030E658..0x14030E662`, `0x14030ED98..0x14030ED9F`, `0x14030F378..0x14030F382`, `0x1403101B8..0x1403101C2`, `0x140310348..0x140310353`, `0x140310A90..0x140310A9B`, `0x1403112B8..0x1403112C1` and `0x140311408..0x140311411`.

Therefore `analysis/mod/animation_binding` reuses the canonical `analysis/mod/motion_group` projection rather than duplicating or reinterpreting the raw third table.

## 2. Animated local state remains runtime-owned

The MOD parser owns serialized rest-local transforms. Animation evaluation is a later runtime concern.

This module consequently accepts **evaluated animated-local matrices as caller input**. It never writes them back into `formats::mod::transform_domain::LocalTransformRecord` and never treats a posed matrix as serialized MOD source data.

That separation is deliberate under ADR-0003:

```text
serialized MOD rest-local
    !=
evaluated animation local pose
```

The new API therefore expresses animation linkage without making an unsupported MOT decoding claim.

## 3. currentWorld composition

The canonical MOD/EFM transform work already recovered the DMC3 row-vector hierarchy used by `0x1402F9700`:

```text
root:
currentWorld[root] = animatedLocal[root] * rootBase

child:
currentWorld[node] = animatedLocal[node] * currentWorld[parent]
```

`build_animated_world_matrices` applies exactly that composition in the serialized topological order, using `parentByOrderPosition` as node indices and `nodeAtOrderPosition` as the evaluation permutation.

The function fails closed if:

- the hierarchy is not a complete topological permutation;
- a node is repeated or out of range;
- a non-root parent has not already been evaluated;
- the motion-group domain cannot be projected exactly once per node;
- the caller provides the wrong number of animated-local matrices.

## 4. Skin palette completion

The previously promoted palette loop at `0x140300580..0x1403006C3` establishes:

```text
skinMatrix[node] = inverseRestWorld[node] * currentWorld[node]
```

`build_animated_skin_palette` therefore performs only:

```text
evaluated animated local
 -> currentWorld
 -> existing canonical build_skin_palette
```

At the rest pose, supplying matrices reconstructed from the serialized MOD rest-local records produces identity skin matrices. A synthetic pose delta then verifies that a +3 X translation in one animated local propagates through the recovered multiplication order as +3 in that node's skin matrix while unaffected nodes remain identity.

These are regression checks of the already-recovered composition rules, not evidence that the synthetic pose exists in retail data.

## 5. Deliberate evidence boundary

The handoff research records additional CMotion joint fields around `+0x40/+0x80/+0xF0/+0xFA/+0x108/+0x110`. The exact canonical executable bytes for a fresh independent re-census of those offsets are not available in this session.

Accordingly this slice does **not** re-promote, rename or depend on those offsets. The module is intentionally narrower: it needs only the already-canonical hierarchy, motion-group selector, currentWorld composition and skin-palette composition.

A later direct EXE pass may extend the binding model after those fields are independently revalidated.

## 6. Canonical API

Added:

```text
include/dmc_rengine/analysis/mod/animation_binding.hpp
src/analysis/mod/animation_binding.cpp
```

The module exposes:

- `AnimationJointBinding` — node index, parent node, motion group and hierarchy order position;
- `AnimationBindingProjection` — CMotion-facing node projection without MOT parsing;
- `project_animation_binding(...)`;
- `build_animated_world_matrices(...)`;
- `build_animated_skin_palette(...)`.

Regression coverage is kept in the existing `mod_transform_domain` test target so this bounded semantic slice does not create another test executable or CMake surface unnecessarily.

## Status

- MOD hierarchy/order semantics: `EXE_CONFIRMED`;
- MOD motion-group selector: `EXE_CONFIRMED`;
- row-vector `animatedLocal * parent/rootBase` currentWorld composition: `EXE_CONFIRMED` as reuse of the canonical MOD currentWorld contract;
- `inverseRestWorld * currentWorld` skin palette: `EXE_CONFIRMED`;
- MOT track/channel decoding: `PRESERVED_UNDECODED` / out of scope;
- additional CMotion joint offsets not used by this module: not re-promoted in this pass;
- MOD writer authority: not promoted.
