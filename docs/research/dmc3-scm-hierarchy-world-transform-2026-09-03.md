# DMC3 HD SCM hierarchy/world-transform reverse — corrected provenance record

**Original pass:** 2026-09-03  
**Correction:** 2026-09-05  
**Canonical target:** `dmc3.exe` SHA-256 `e454272ed0fb0247fcbcf300e5d55d7a3e96d50b89b9ffaff81bb978dcbdd082`

## Stable result

The SCM scene block indexing contract remains confirmed:

```text
scene block +0x00 -> parentByOrderPosition[]
scene block +0x04 -> nodeAtOrderPosition[]
scene block +0x08 -> objectBindingByNodeIndex[]
scene block +0x0C -> transformByNodeIndex[]
```

`0x1402F1DB0` resolves the four arrays into common manager pointers `+0x08/+0x10/+0x18/+0x20`.

The 68-file SCM corpus still validates:

- `nodeAtOrderPosition` is a permutation;
- exactly one root at evaluation position 0;
- every non-root parent is already evaluated;
- each geometry object is bound exactly once;
- helper nodes use binding `-1`.

## Local SCM transform — corrected owner

The SCM-specific local transform initializer is **`0x1402FA360`**, reached by SCM setup `0x140303C10`.

The previously cited `0x1402FA080` belongs to the MOD/EFM setup chain (`0x1403039C0`). Both call the same lower helpers, so the recovered local transform semantics remain valid:

```text
rotation +0x10 XYZ -> 0x140330450 -> X, Y, Z -> Rz * Ry * Rx
translation +0x00 XYZ -> 0x140031200
translation +0x0C -> precomputed length, excluded from homogeneous W
```

## Parent/root pointer setup

SCM initializer `0x1402FA360` uses the common order/parent arrays to assign each runtime node's parent/root matrix pointer. Root nodes reference manager root/base state; non-root nodes point at the already allocated matrix for their parent node.

The format-specific third array is also read here for SCM object/node attachment state. Its SCM meaning must not be generalized to MOD.

## Generic world update — 0x1402F9700

`0x1402F9700` is a family-level world update helper rather than evidence of an SCM-only implementation.

For each node in evaluation order it obtains the local matrix and selected parent/root matrix and calls `0x140030E40`.

`0x140030E40` reaches the recovered 4x4 multiplier `0x1400312B0`, yielding the established row-vector relation:

```text
world[current] = local[current] * parentOrRootWorld
```

For SCM this gives:

```text
root:
  world[root] = local[root] * rootBase

non-root:
  world[current] = local[current] * world[parentByOrderPosition[i]]
```

The clean `scm::build_world_matrices()` remains consistent with this contract.

## Rigid inverse provenance correction

The arithmetic helper `0x140030DC0` is indeed a rigid transform inverse:

```text
inverse.rotation = transpose(rotation)
inverse.translation = -translation * transpose(rotation)
W = 1
```

However, the **direct initialization path previously attributed to SCM is not SCM-owned**. It occurs in `0x1402FA080`, which the corrected setup-chain evidence identifies as the MOD/EFM transform initializer.

SCM `0x1402FA360` does not execute that same direct inverse-cache initialization sequence.

Therefore:

- the mathematical reconstruction `invert_dmc3_rigid_transform()` remains valid as a DMC matrix utility;
- direct SCM inverse-world-cache ownership is **retracted**;
- the MOD/EFM inverse path becomes evidence for the active skeleton/palette reverse track.

## Object core cross-family correction

A parallel comparison of SCM `0x140302F10` and MOD/EFM `0x1403029E0` proves that several fields formerly treated as SCM-only are common Model Family object state:

```text
serialized +0x01 -> runtime alpha/control byte
serialized +0x10 -> runtime baseline/effective source flags
serialized +0x30 -> bounding sphere
```

SCM retains additional stage-specific alpha compatibility corrections, so the core behavior and the compatibility layer remain separate.

## Current boundary

SCM hierarchy/world behavior that remains confirmed:

- parent/order indexing;
- SCM object binding array semantics;
- local XYZ Euler transform;
- translation-magnitude exclusion from W;
- root/parent matrix-pointer setup;
- generic local * parent world propagation.

Retracted:

- `0x1402FA080` as the SCM local-transform initializer;
- direct SCM ownership of the `0x140030DC0` inverse-cache initialization path.

This corrected record supersedes those address/ownership claims from the 2026-09-03 version without changing the validated SCM serialized hierarchy or world-composition formulas.
