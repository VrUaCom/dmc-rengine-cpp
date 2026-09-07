# Урок 2 — Document, objects і meshes

## Document header: `0x40`

Канонічний shared model document shell:

| Offset | Type | MOD meaning |
|---|---:|---|
| `+0x00` | char[4] | `MOD ` magic |
| `+0x04` | f32 | version; recursive `em000` retail corpus confirms `0.82`, `0.84`, `1.00`, `1.01` |
| `+0x10` | u8 | outer object count |
| `+0x11` | u8 | node/transform-domain count |
| `+0x12` | u8 | serialized texture-domain mirror |
| `+0x13` | u8 | MOD `default_joint_index` |
| `+0x14` | u32 | runtime-carried; decimal shape corpus-confirmed, high-level semantics unresolved |
| `+0x20` | u64 | node-domain block offset |
| `+0x40` | — | outer object table begins |

### Version coverage

Повний recursive sweep `em000-extract.zip` (SHA-256 `306130125f09824811289366324f4208c3c1aba880c5a7efa3953a88d566d07b`) дав:

```text
1.01 -> 24 MOD
1.00 -> 6 MOD
0.84 -> 4 MOD
0.82 -> 1 MOD
```

Усі 35 зберігають ту саму recovered structural grammar: header `0x40`, object `0x40`, mesh `0x50`, ті самі stream/node-domain contracts і skin packing. Це `CORPUS_CONFIRMED` structural compatibility, а не глобальна гарантія всіх можливих MOD revisions.

`+0x12` — не live texture authority. Runtime domain size приходить із companion.

`+0x13` EXE-confirmed: serialized byte переноситься в manager `+0xFA`, використовується як `JntNo` fallback і як index у `currentWorld[]`. Не доведено, що це завжди root.

### Header `+0x14`

`+0x14` переноситься в manager `+0xE4`, але MOD-specific downstream meaning ще не встановлено.

Повний `em000` corpus показує сильну decimal structure:

```text
raw = high_component * 100000
    + middle_component * 100
    + low_component
```

Приклади:

```text
100407 -> 1 / 4 / 7
202900 -> 2 / 29 / 0
601715 -> 6 / 17 / 15
700601 -> 7 / 6 / 1
```

Цей арифметичний projection тепер зафіксований у `analysis/mod/corpus_observations.hpp`. Не переносити сюди SCM `LegacyResourceCode` semantic без MOD-specific EXE consumer.

## Outer object: `0x40`

| Offset | Type | Meaning/status |
|---|---:|---|
| `+0x00` | u8 | child mesh count |
| `+0x01` | u8 | raw alpha/control |
| `+0x02` | u16 | aggregate element count |
| `+0x08` | u64 | mesh table offset |
| `+0x10` | u32 | source flags |
| `+0x18` | f32 | live render parameter for some flags, artistic name open |
| `+0x1C` | u32 | live render parameter for some flags, artistic name open |
| `+0x30` | f32[3] | bounding center |
| `+0x3C` | f32 | bounding radius |

### Real retail population of `+0x18/+0x1C`

`em000_033.mod` дає два реальні retail records:

```text
source_flags = 0x00100201
+0x18        = 1.25f
+0x1C        = 2
```

Оскільки `0x00000200` активний, це узгоджується з already canonical EXE path, який копіює `+0x18/+0x1C` у runtime state. Тому факт, що ці поля є genuinely populated retail inputs, тепер `EXE_AND_CORPUS_CONFIRMED`. Їх artistic meaning залишається відкритим.

### Object flags

Direct runtime projection already відновлена. Наприклад `0x4000` дає nearest-filter material signal; `0x200/0x400` активують manager bit21 і копіюють `+0x18/+0x1C`; інші підтверджені bits проектуються у runtime object flags. Але ми не даємо їм artistic names без downstream proof.

Повний `em000` sweep також підтвердив реальну присутність `0x00200000` у 7 retail objects. Отже цей bit не можна класифікувати як padding/reserved лише через відсутність завершеної high-level semantic.

## Inner mesh: `0x50`

| Offset | Type | Meaning |
|---|---:|---|
| `+0x00` | u16 | element count |
| `+0x02` | u16 | texture slot |
| `+0x04` | u16 | GS CLAMP MINU |
| `+0x06` | u16 | GS CLAMP MAXU |
| `+0x08` | u16 | GS CLAMP MINV |
| `+0x0A` | u16 | GS CLAMP MAXV |
| `+0x0C` | u32 | preserved undecoded |
| `+0x10` | u64 | positions offset |
| `+0x18` | u64 | normals offset |
| `+0x20` | u64 | UV offset |
| `+0x28` | u64 | blend-index stream |
| `+0x30` | u64 | packed weight/topology stream |
| `+0x38` | u64 | preserved undecoded |
| `+0x40` | u64 | generated topology workspace relative offset |
| `+0x48` | u32 | generated topology count |
| `+0x4C` | u32 | preserved undecoded |

## Streams

- position: `float3`, stride 12;
- normal: `float3`, stride 12;
- UV: `int16x2`, stride 4, scale `4096`;
- blend indices: `u8x4`;
- control: `u16`.

Recovered source streams normally мають `0x10` alignment. У full `em000` sweep усі stream starts були `0x10` aligned. Canonical parser попереджає про deviation, а не переписує raw bytes.

## Relative coordinate spaces

Не всі offsets мають однакову базу:

- більшість mesh streams post-load relocates як `resourceBase + relative`;
- mesh `+0x40` post-load обчислюється як `meshRecordAddress + relative`.

Це один із типових writer bugs: правильне числове значення з неправильною coordinate base дає валідний-looking, але зламаний resource.

## Generated topology workspace

У всіх `147/147` retail meshes `em000` фізична місткість workspace точно відповідає:

```text
align16(6 * (vertex_count - 2))
```

для `vertex_count > 2`; для менших counts canonical corpus helper повертає 0.

Цей corpus-confirmed layout formula зафіксований у `analysis/mod/corpus_observations.hpp`. Він корисний для майбутнього layout planner, але ще не є самостійною writer authority.

## Unknown fields

`+0x0C`, `+0x38`, `+0x4C` мають typed preservation view. Вони не consumed у підтвердженому runtime mesh path і залишаються zero у `147/147` em000 meshes, але це не доводить, що вони padding або globally unused.

Повний byte-coverage sweep усіх 35 MOD залишив 5,106 uncovered bytes після маркування всіх відомих sections; усі 5,106 були zero alignment/padding. Тобто в `em000` не знайдено нового non-zero serialized section поза поточною MOD section map. Це сильний corpus result, не глобальна гарантія інших actor families.
