# Урок 8 — Evidence, unknowns і безпечний reverse

## Чому unknown bytes — це актив, а не сміття

Reverse помиляється найчастіше тоді, коли «не знаю» перетворюють на `reserved = 0`.

У MOD source bytes є preservation authority. Якщо semantic meaning не доведено, writer зобов’язаний зберегти raw bytes — навіть коли bound retail corpus показує лише zero.

## Поточна multi-corpus база

Після розширення за межі `em000` bound evidence містить:

```text
38 unique MOD
166 objects
180 meshes
20,976 vertices
285 transforms
```

Resource contexts:

- `em000` enemy/effect actor ecosystem;
- `pl000` main model;
- `pl000` cloth-associated model із CLT cross-resource provenance;
- `id100` HUD/model resource.

Це сильніше за single-family corpus, але все одно не дорівнює “усі DMC3 MOD”.

## Поля, де новий reverse змінив старе формулювання

### Header `+0x14`

Фізично це raw `u32`, який EXE переносить у manager `+0xE4`.

Старий `em000`-only decimal hypothesis був візуально правдоподібним, але broader corpus додав:

```text
pl000 -> 217
id100 -> 1000000
```

Тому universal semantic formula `family*100000 + model_set*100 + sub_index` — `REJECTED` для MOD. Поки нема typed downstream manager consumer, правильний статус:

```text
runtime-carried u32        EXE_CONFIRMED
high-level semantic        PRESERVED_UNDECODED
writer policy              preserve exact source value
```

### Transform `+0x1C`

```text
MOD 285/285 -> 0.0f
bound EFM 5/5 -> 0.0f
```

Canonical local rotation helper читає тільки X/Y/Z і не читає copied fourth scalar. Отже local matrix non-consumption — `EXE_CONFIRMED`, але global semantic усе ще `PRESERVED_UNDECODED`.

### Mesh `+0x0C`

Physical `u32`, `180/180 == 0` у bound MOD corpus. Bounded load/build path не дає positive semantic transfer. Synthetic non-zero regression доводить preservation contract. Не називати padding.

### Mesh `+0x38`

Це найкращий приклад, чому однаковий offset ≠ однакова semantic.

Для MOD:

```text
180/180 == 0
canonical runtime builder не forward-ить slot
corresponding runtime auxiliary positions explicitly disabled
```

Для EFM homologous `+0x38` live і forward-иться як COLOR0.

Отже shared physical slot існує, але MOD role — inactive в audited path. Source byte range все одно preserve.

### Mesh `+0x4C`

Physical trailing `u32`, `180/180 == 0`, positive companion transfer не доведений. Status `PRESERVED_UNDECODED`, writer preserve.

### BLENDINDICES.x

Раніше: “observed zero / exact purpose open”.

Тепер canonical runtime consumption gate закритий:

```text
serialized +0x28
 -> runtime mesh +0x140
 -> draw descriptor
 -> BLENDINDICES uint4
 -> compiled DXBC ReadWriteMask 0xE
```

Compiled shaders читають Y/Z/W, **не X**. Provenance-aware CPU census також не знаходить direct lane-X consumer. Bound corpus має X==0 у 20,976/20,976 vertices.

Але semantic label лишається `PRESERVED_UNDECODED`, бо “not consumed by canonical path” не робить byte padding. Writer зберігає raw X.

## Object flags: closed technical semantics vs still-open semantics

### `0x00100000`

Технічний renderer semantic закритий:

```text
bit clear -> TEST_1 AREF=0,  ZBUF_1.ZMSK=1
bit set   -> TEST_1 AREF=16, ZBUF_1.ZMSK=0
```

ATST лишається `GREATER`, ZTST — `GEQUAL`.

Це EXE-confirmed **GS alpha-test-reference/depth-write-state selector**. Artistic/material category не доведена.

### `0x00200000`

Bit21 переноситься у runtime baseline/effective flags і бере участь у restoration state machine. Але confirmed GS packet helper його не інтерпретує. Equal-mask candidates у manager/object інших domains відхилені без provenance edge.

Поточний статус:

```text
runtime carriage/restoration  EXE_CONFIRMED
local GS consumer             EXE_CONFIRMED: none
high-level semantic           PRESERVED_UNDECODED
writer policy                 preserve
```

### Object `+0x18/+0x1C`

Вони EXE-consumed для source flags `0x200/0x400`, а retail `em000_033.mod` має real non-zero values. Факт live runtime input — `EXE_AND_CORPUS_CONFIRMED`; artistic names відкриті.

## Negative evidence

«Цей path не читає поле» — сильний факт, але це не те саме, що «поле ніде не використовується».

Правильне формулювання:

```text
PRESERVED_UNDECODED,
unconsumed in the confirmed path
```

Неправильне:

```text
unused padding
```

## Cross-format analogy

Якщо SCM, MOD і EFM мають однаковий offset, це може довести shared physical shell лише з independent evidence. Semantic meaning може відрізнятися.

Конкретні приклади:

- MOD `+0x14` не отримує SCM `LegacyResourceCode` semantics;
- EFM `mesh+0x38 -> COLOR0` не дає права назвати MOD `mesh+0x38` COLOR0;
- transform `+0x1C` у MOD не успадковує meaning із homologous field іншої family.

## Evidence ladder

1. structural observation;
2. corpus census;
3. executable producer/consumer;
4. runtime cross-check;
5. regression;
6. cross-resource consistency;
7. writer/reopen;
8. original-game acceptance;
9. rollback.

Не перескакуй із кроку 2 на крок 8.

## `RESERVED_OBSERVED_ZERO` / multi-corpus zero

Zero histogram означає буквально: у bound corpus поле було нульовим.

Він не означає:
- zero is required;
- non-zero invalid;
- field safe to erase;
- writer may synthesize zero.

Навіть 20,976 нульових BLENDINDICES.x або 285 нульових transform `+0x1C` не змінюють цього правила без semantic/writer proof.

## Writer authority

Щоб поле стало editable, треба не тільки знати meaning. Треба знати serialization rule, valid ranges, dependencies, derived fields, alignments, container reintegration і прийняття оригінальним runtime.

Поточний MOD closure суттєво скорочує unknown surface, але **не дорівнює production writer acceptance**.

## Правило ADR-0003

MOD architecture розділяється на:

```text
serialized ABI
typed IR
read-only parser
runtime/semantic analysis
reverse evidence
writer/authoring
```

Це навмисно заважає hypothesis leakage з analysis у parser/writer.
