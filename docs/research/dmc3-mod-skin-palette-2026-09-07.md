# DMC3 HD MOD skin palette — inverse rest ownership and runtime composition

Date: 2026-09-07

## Authority

Canonical executable:

- `dmc3.exe`
- SHA-256: `e454272ed0fb0247fcbcf300e5d55d7a3e96d50b89b9ffaff81bb978dcbdd082`
- MOD/EFM transform initializer: `0x1402FA080`
- current-world updater: `0x1402F9700`
- runtime palette loop: `0x140300580..0x1403006C3`
- rigid inverse helper: `0x140030DC0`
- compose wrapper: `0x140030E40`
- raw matrix multiply helper: `0x1400312B0`

This pass closes the previously open ownership relationship between MOD rest transforms, runtime world matrices, inverse-rest matrices and the matrices emitted for skinning.

## 1. Runtime node and manager matrix ownership

The MOD/EFM initializer `0x1402FA080` consumes the serialized `0x20` transform records already represented by `formats::mod::transform_domain`.

For each node it materializes a runtime node record with stride `0xA0`:

- runtime node `+0x40..+0x7F` — local matrix;
- runtime node `+0x00..+0x3F` — rigid inverse of that node's model-space rest/world-at-load matrix;
- runtime node `+0x80` — parent/current-world pointer used by later hierarchy propagation;
- runtime node `+0x89` — node index byte observed in the initializer.

Manager state contains:

- `+0x08` — parent-by-order-position array;
- `+0x10` — node-at-order-position array;
- `+0x188` — current world-matrix array, stride `0x40` per node;
- `+0x1B0` — external/root matrix storage used by the later pose update path.

At initialization the `+0x188` array contains the model-space rest/world matrices. `0x1402F9700` later recomputes the same array from current local pose matrices and the caller-supplied external/root matrix.

## 2. Rigid inverse helper

`0x140030DC0(destination, source)` reconstructs the inverse of a rigid affine matrix in the engine's row-major / row-vector convention.

It:

1. transposes the source 3x3 rotation basis;
2. preserves affine row/column constants;
3. computes inverse translation as `-T * R^T`.

For source rotation rows `R0/R1/R2` and translation `T=(tx,ty,tz)`, output row-3 XYZ is:

```text
x = -dot(T, R0)
y = -dot(T, R1)
z = -dot(T, R2)
```

`0x1402FA080` calls this helper after each rest/world matrix has been constructed and writes the result into runtime node `+0x00`.

This establishes runtime node `+0x00` as the inverse model-space rest/world-at-load transform. In skinning terminology it is the inverse-rest / inverse-bind matrix for that node in the recovered MOD path.

## 3. Matrix multiplication order

`0x1400312B0(destination, left, right)` computes conventional row-major:

```text
result = left * right
```

The SIMD implementation forms each output row as the weighted sum of the four rows of `right` using one source row from `left`.

`0x140030E40(destination, A, B)` deliberately swaps the public operands before calling `0x1400312B0`, therefore:

```text
0x140030E40(destination, A, B)
    => destination = B * A
```

This is consistent with the already recovered hierarchy convention:

```text
world = local * parentWorld
```

for row-vector transforms.

## 4. Current-world update

`0x1402F9700` walks nodes in `nodeAtOrderPosition` order and updates manager `+0x188`.

For each node it combines:

- the node's current local matrix at runtime node `+0x40`;
- the root/parent current world reached through runtime node `+0x80`.

The same `0x140030E40` helper produces the current world relation used throughout the model subsystem.

Thus manager `+0x188` is not merely a static bind table: it is the live/current world-matrix array and initially holds the rest pose.

## 5. Skin palette generation

The loop at `0x140300580..0x1403006C3` iterates every node in topological order.

For node `N` it obtains:

- runtime node record `nodeBase + N * 0xA0`;
- current world matrix `manager+0x188 + N * 0x40`;
- palette destination `selectedPaletteBase + N * 0x40`.

The call at `0x1403006BE` is:

```text
0x140030E40(
    palette[N],
    currentWorld[N],
    runtimeNode[N].inverseRestWorld)
```

Because `0x140030E40(dest, A, B) = B * A`, the exact palette formula is:

```text
skinMatrix[N] = inverseRestWorld[N] * currentWorld[N]
```

in DMC3's row-vector convention.

At the rest pose:

```text
currentWorld == restWorld
inverseRestWorld * restWorld == identity
```

which is the expected skinning invariant.

## 6. Canonical C++ promotion

`formats::mod::world_transform` now exposes:

- `rigid_inverse_dmc3_matrix()` — reconstruction of `0x140030DC0`;
- `build_model_space_inverse_rest_matrices()` — inverse matrices from the canonical MOD rest hierarchy;
- `build_skin_palette(domain, currentWorld)` — exact `inverseRest * currentWorld` palette composition.

The implementation deliberately reuses the existing `Matrix4f` and `multiply_dmc3_matrices()` contract so hierarchy and skinning cannot drift into separate matrix conventions.

## 7. Regression

`tests/mod_transform_domain_tests.cpp` now verifies:

1. a rotated + translated local matrix multiplied by its reconstructed rigid inverse yields identity in both orders;
2. model-space inverse-rest matrices are available only when the spatial hierarchy capability gate passes;
3. feeding the rest world matrices to `build_skin_palette()` yields identity for every node;
4. adding `+3` to one current-world X translation produces exactly `+3` in that node's palette matrix;
5. wrong current-world cardinality fails closed;
6. malformed hierarchy also blocks inverse-rest construction.

## 8. Evidence status

- runtime node stride `0xA0`: `EXE_CONFIRMED`;
- runtime node local matrix `+0x40`: `EXE_CONFIRMED`;
- runtime node inverse rest/world-at-load matrix `+0x00`: `EXE_CONFIRMED`;
- runtime node parent-world pointer `+0x80`: `EXE_CONFIRMED`;
- manager current-world array `+0x188`, stride `0x40`: `EXE_CONFIRMED`;
- rigid inverse formula: `EXE_CONFIRMED`;
- matrix multiply order `0x1400312B0`: `EXE_CONFIRMED`;
- compose-wrapper operand reversal `0x140030E40`: `EXE_CONFIRMED`;
- skin palette operands and formula `inverseRestWorld * currentWorld`: `EXE_CONFIRMED`.

## 9. Product capability impact

This closes the mathematical/ownership evidence needed to represent a truthful MOD skeleton in model space and to compute skin matrices when a valid current-world pose is available.

It strengthens:

- 3D bone hierarchy overlay;
- selected-bone weighted-region visualization;
- pose-aware skin preview once current animation pose input is materialized by canonical tooling.

It does **not** by itself authorize writing or editing MOD files.

## 10. Remaining writer boundary

Still open before MOD skin editing / production writer authority:

1. preserve and classify unresolved mesh bytes/fields, including MOD mesh `+0x0C..+0x0F`, `+0x38` and `+0x4C`;
2. close the node adapter/type array at node-domain `+0x08` if it affects pose/palette selection;
3. canonicalize current animation/pose input ownership rather than accepting invented frontend transforms;
4. broaden real MOD revision/corpus coverage;
5. no-edit byte-identical rebuild;
6. edited MOD reintegration through PAC/NBZ;
7. canonical `dmc3.exe` acceptance tests and rollback evidence.

Until those are complete:

```text
spatial_hierarchy = read_only_exe_confirmed
skin_weights = read_only_decoded
inverse_rest = read_only_exe_confirmed
skin_palette_math = read_only_exe_confirmed
skin_weight_visualization = allowed
skin_weight_editing = blocked
production_mod_writer = blocked
```
