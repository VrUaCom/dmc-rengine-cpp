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
+0x1C f32 preserved undecoded
```

MOD/EFM initializer: `0x1402FA080`.

Shared helpers:
- translation: `0x140031200`;
- XYZ rotation: `0x140330450`.

Recovered rotation application: X → Y → Z, equivalent to `Rz * Ry * Rx` in recovered matrix convention.

### Transform `+0x1C`: що тепер доведено

Bound multi-corpus evidence:

```text
MOD transforms  285 / 285 -> +0x1C == 0.0f
bound EFM          5 / 5 -> +0x1C == 0.0f
```

Canonical initializer копіює serialized `+0x10..+0x1F` у 16-byte scratch vector, але downstream rotation helper `0x140330450` читає лише scratch `+0x00/+0x04/+0x08`, тобто serialized X/Y/Z. Scratch `+0x0C`, який відповідає serialized `+0x1C`, у цьому path не читається.

CMotion binding path `0x14030F850` незалежно пропускає fourth scalar, зберігаючи 0x20 record stride.

Отже:

```text
serialized ABI                         STRUCTURAL_CONFIRMED
bounded +0x1C zero                     CORPUS_CONFIRMED
local rotation-matrix non-consumption  EXE_CONFIRMED
global semantic                        PRESERVED_UNDECODED
writer policy                          preserve source float exactly
```

Це **не** означає, що `+0x1C` можна назвати padding/reserved або завжди записувати `0.0f`. Негативний результат закриває лише canonical local rotation-matrix path, не кожен можливий subsystem.

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

## Evidence boundary

Transform record тепер має дві різні категорії доказу:

- XYZ translation/rotation і world propagation — positive runtime semantics;
- `+0x1C` — physical transfer + bounded non-consumption, але без global semantic name.

Не перетворюй другу категорію на `reserved = 0` лише тому, що поточний corpus весь нульовий.
