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
| `+0x14` | u32 | raw runtime-carried metadata; high-level semantic unresolved |
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

### Multi-corpus baseline

Після розширення corpus за межі `em000` поточна bound вибірка містить:

```text
38 unique MOD
166 objects
180 meshes
20,976 vertices
285 transforms
```

Вона охоплює `em000`, два незалежні `pl000` MOD і `id100` HUD/model resource. Це важливо: invariant, який колись був лише enemy-family observation, більше не можна автоматично називати enemy-only — але й не можна оголошувати глобальним правилом для всіх DMC3 MOD без окремого evidence.

`+0x12` — не live texture authority. Runtime domain size приходить із companion.

`+0x13` EXE-confirmed: serialized byte переноситься в manager `+0xFA`, використовується як `JntNo` fallback і як index у `currentWorld[]`. Не доведено, що це завжди root.

### Header `+0x14`

`+0x14` — raw little-endian `u32`, який canonical initializer переносить у manager `+0xE4`.

Старий `em000`-only pass помічав візуально структуровані значення на кшталт:

```text
100407
202900
601715
700601
```

і використовував lossless arithmetic decomposition. Після multi-corpus pass з'явилися:

```text
pl000 main   -> 217
pl000 cloth  -> 217
id100        -> 1000000
```

Це **спростовує universal semantic interpretation** виду:

```text
family * 100000 + model_set * 100 + sub_index
```

Будь-який `u32` можна арифметично розкласти й зібрати назад, тому сам факт decomposition не доводить semantic field boundaries.

Поточний безпечний contract:

```text
serialized offset/width          STRUCTURAL_CONFIRMED
serialized -> manager +0xE4      EXE_CONFIRMED
old universal decimal semantic   REJECTED
high-level meaning               PRESERVED_UNDECODED
writer policy                    preserve exact source u32
```

У canonical C++ contract це поле навмисно представлено нейтральною projection-назвою `runtime_metadata_u32`: назва фіксує width і підтверджений runtime carry, але не вигадує high-level semantic.

Не переносити сюди SCM `LegacyResourceCode` semantics і не відновлювати `+0x14` із filename/resource name.

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

Оскільки `0x00000200` активний, це узгоджується з canonical EXE path, який копіює `+0x18/+0x1C` у runtime state. Тому факт, що ці поля є genuinely populated retail inputs, `EXE_AND_CORPUS_CONFIRMED`. Їх artistic meaning залишається відкритим.

### Object flags

Serialized `+0x10` копіюється verbatim у runtime baseline `+0x10` і mutable/effective `+0x14`.

Для `0x00100000` технічний renderer semantic тепер закритий. У активному low-mode path canonical GS packet builder перемикає:

```text
bit clear:
  TEST_1 AREF = 0
  ZBUF_1.ZMSK = 1   // depth writes masked

bit set:
  TEST_1 AREF = 16
  ZBUF_1.ZMSK = 0   // depth writes enabled by this selector
```

Alpha-test method лишається `GREATER`, Z test — `GEQUAL`. Це `EXE_CONFIRMED`, але не дає права назвати bit художньою категорією на кшталт `cloth`, `transparent` або `alpha material`.

`0x00200000` має іншу межу: він реально присутній у retail corpus і `EXE_CONFIRMED` переноситься в baseline/effective runtime words, може переживати/повертатися через state restoration, але підтверджений local GS packet helper його не інтерпретує. Equal-mask hits у manager `+0xE0` та runtime object `+0x304` — інші provenance domains і не є доказом semantic equality. High-level meaning bit21 залишається `PRESERVED_UNDECODED`.

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
| `+0x28` | u64 | BLENDINDICES stream |
| `+0x30` | u64 | packed weight/topology stream |
| `+0x38` | u64 | family-specific auxiliary slot; inactive in canonical MOD path, source-preserved |
| `+0x40` | u64 | generated topology workspace relative offset |
| `+0x48` | u32 | generated topology count |
| `+0x4C` | u32 | preserved undecoded |

## Streams

- position: `float3`, stride 12;
- normal: `float3`, stride 12;
- UV: `int16x2`, stride 4, scale `4096`;
- BLENDINDICES: `u8x4`;
- control: `u16`.

Recovered source streams normally мають `0x10` alignment. У full `em000` sweep усі stream starts були `0x10` aligned. Canonical parser попереджає про deviation, а не переписує raw bytes.

## Relative coordinate spaces

Не всі offsets мають однакову базу:

- більшість mesh streams post-load relocates як `resourceBase + relative`;
- mesh `+0x40` post-load обчислюється як `meshRecordAddress + relative`.

Це один із типових writer bugs: правильне числове значення з неправильною coordinate base дає валідний-looking, але зламаний resource.

## Generated topology workspace

У `em000` 147/147 retail meshes фізична місткість workspace точно відповідає:

```text
align16(6 * (vertex_count - 2))
```

для `vertex_count > 2`; для менших counts canonical corpus helper повертає 0.

Цей corpus-confirmed layout formula зафіксований у `analysis/mod/corpus_observations.hpp`. Він корисний для майбутнього layout planner, але ще не є самостійною writer authority.

## Unknown/preserved mesh fields після multi-corpus + EXE closure

Поточна bound вибірка:

```text
mesh +0x0C == 0   180 / 180
mesh +0x38 == 0   180 / 180
mesh +0x48 == 0   180 / 180 serialized meshes
mesh +0x4C == 0   180 / 180
```

Але статуси різні:

- `+0x0C`: physical `u32` і zero histogram відомі; semantic `PRESERVED_UNDECODED`; synthetic preservation regression забороняє writer zero-normalization.
- `+0x38`: common physical slot, **EXE_CONFIRMED inactive у canonical MOD runtime builder**; homologous EFM slot live і forward-иться як COLOR0. Це доказ format-specific semantics, а не причина називати MOD slot COLOR0 чи padding.
- `+0x4C`: physical `u32`, zero histogram і відсутність companion transfer у bounded load/build chain; semantic `PRESERVED_UNDECODED`.

Правило writer-а для всіх трьох однакове:

```text
read exact
preserve exact
never synthesize zero from corpus frequency alone
```

Повний byte-coverage sweep 35 `em000` MOD залишив 5,106 uncovered bytes після маркування всіх відомих sections; усі 5,106 були нульовими в цьому bounded corpus. Це сильний corpus result про observed bytes, але **не** доказ padding/alignment semantics, не вимога `zero`, і не глобальна гарантія інших actor families або revisions.
