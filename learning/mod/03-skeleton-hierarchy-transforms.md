# Урок 3 — Скелет, node domain і transforms

## Node-domain block

Block header size `0x20`:

```text
+0x00 u32 parent_rel
+0x04 u32 order_rel
+0x08 u32 motion-group/raw third-domain rel
+0x0C u32 transform_rel
```

Для `N` nodes:

```text
parent  = 0x20
order   = 0x20 + align4(N)
group   = 0x20 + 2*align4(N)
xforms  = align16(0x20 + 3*align4(N))
```

## Parent/order semantics

`parent_by_order_position` — `i8[N]`. `0xFF` = `-1` root.

`node_at_order_position` — `u8[N]` permutation.

Критична corrected формула:

```text
for order_position i:
    node   = nodeAtOrderPosition[i]
    parent = parentByOrderPosition[i]   // already a node index
```

Не роби inverse-permutation для parent. Ця стара heuristic відхилена EXE evidence.

## Spatial capability gate

Model-space hierarchy можна вважати валідною тільки якщо:

- order array — повна permutation;
- root стоїть на першій order position;
- root parent = `-1`;
- кожен non-root parent уже evaluated;
- hierarchy topological/acyclic;
- local transform records complete і finite.

Саме це робить `supports_spatial_hierarchy()`.

## Local transform record: `0x20`

```text
+0x00 f32 tx
+0x04 f32 ty
+0x08 f32 tz
+0x0C f32 translation magnitude
+0x10 f32 rx radians
+0x14 f32 ry radians
+0x18 f32 rz radians
+0x1C f32 unresolved
```

MOD/EFM initializer: `0x1402FA080`.

Shared helpers:
- translation: `0x140031200`;
- XYZ rotation: `0x140330450`.

Recovered rotation application: X → Y → Z, equivalent to `Rz * Ry * Rx` in recovered matrix convention.

## World matrices

DMC3 model path uses row-vector-oriented composition:

```text
root:
world[root] = local[root] * rootBase

child:
world[node] = local[node] * world[parent]
```

Для model-space preview `rootBase = identity`.

World position береться з matrix row 3 XYZ (`values[12..14]` у canonical `Matrix4f`).

### Заборонена shortcut-heuristic

Не визначай bone position із mesh vertices. Vertex positions — geometry, а bone/node position — transform hierarchy.

## Runtime node ABI

Recovered runtime node stride = `0xA0`:

```text
+0x00 inverse model-space rest/world-at-load matrix
+0x40 local matrix
+0x80 parent/current-world pointer
```

Manager:

```text
+0x08 parent-by-order-position
+0x10 node-at-order-position
+0x188 currentWorld[] (stride 0x40)
+0x1B0 external/root matrix storage
```

## Inverse rest і skin matrix

`0x140030DC0` — rigid inverse helper.

Palette loop `0x140300580..0x1403006C3` дає:

```text
skinMatrix[node] = inverseRestWorld[node] * currentWorld[node]
```

У rest pose результат для кожного node має бути identity. Це сильний regression oracle.

## `+0x1C`

У bound em000 corpus transform `+0x1C` спостерігався zero, але direct global semantic proof відсутній. Тому це не «free padding», а preservation boundary.
