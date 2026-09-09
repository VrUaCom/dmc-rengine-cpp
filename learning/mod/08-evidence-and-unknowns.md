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

Bit21 переноситься у runtime baseline/effective flags і бере участь у restoration state machine. Bounded local GS packet helper його не інтерпретує.

Follow-up whole-model pass перевірив ще два класи external consumers effective `runtime +0x14`:

```text
0x1402F28E0 / read @ 0x1402F2901
  -> ставить bit17 (0x00020000)
  -> existing bit21 зберігається
  -> bit21 не тестується

0x1402F9ED9 -> 0x1402F9890
0x1402FA042 -> 0x1402F9890
  -> whole effective dword передається в common material helper
  -> helper interprets only 0x00004000
  -> bit21 не тестується / не extract-иться
```

Equal-mask candidates у manager `+0xE0` і runtime object `+0x304` відхилені без provenance edge.

Поточний bounded статус:

```text
runtime carriage/restoration       EXE_CONFIRMED
external effective-word mutation   EXE_CONFIRMED: preserves bit21
local GS consumer                   EXE_CONFIRMED: none
common material consumer            EXE_CONFIRMED: none
high-level semantic                 PRESERVED_UNDECODED
writer policy                       preserve
```

Це хороший приклад правильної negative-evidence дисципліни: ми розширили кількість audited consumers, але не перетворюємо “не прочитано тут” у “unused globally”. Semantic promotion все ще потребує terminal consumer або provenance-confirmed derivation у інший runtime domain.

### Object `+0x18/+0x1C`

Вони EXE-consumed для source flags `0x200/0x400`, а retail `em000_033.mod` має real non-zero values. Факт live runtime input — `EXE_AND_CORPUS_CONFIRMED`; artistic names відкриті.

## Retained serialized-object pointer: як не переплутати escape з consumer

MOD/EFM initializer зберігає pointer на весь serialized object record у runtime object `+0x18`. Це означає, що initializer-local “no read” не закриває unknown object bytes автоматично.

Новий direct-EXE pass дав три різні класи результатів.

### Реальний MOD consumer

Canonical MOD runtime mesh builder:

```text
0x1402FE6A0..0x1402FE921
runtime object stride = 0x380
0x1402FE6F4 -> load runtime +0x18 retained pointer
0x1402FE700 -> read serialized object +0x08 mesh table
```

Тобто retained pointer справді живий. Але в цьому path він бере `object+0x08`, після чого provenance переходить у 0x50-byte mesh records. Secondary regions `+0x04..07`, `+0x14..17`, `+0x20..2F` тут не читаються.

### Load без downstream dereference

Canonical MOD render-command builder:

```text
0x1402FE930..0x1402FF563
0x1402FED8E -> load runtime +0x18
0x1402FED92 -> save to local rbp+0x30
later reads of rbp+0x30 = 0
```

Це `EXE_CONFIRMED` bounded non-consumption, але не whole-program proof.

### Cross-format false positive

Функція `0x140302F10..0x14030345A` теж має serialized `0x40` stride і retained `+0x18` pointer, але runtime object stride там `0x3C0`, а direct code містить відомі SCM compatibility rewrites `EA -> C5` і `C4 -> 80`.

Отже:

```text
same +0x18 offset      != same owner
same 0x40 record stride != same format
SCM 0x3C0 runtime path != MOD/EFM 0x380 runtime path
```

Це `REJECTED` як MOD consumer. Саме тому provenance важливіший за схожість offsets.

Secondary object regions поки залишаються:

```text
CORPUS_CONFIRMED zero
initializer direct read = none
several downstream paths classified
whole-program non-use = not yet proven
writer policy = preserve
```

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
- transform `+0x1C` у MOD не успадковує meaning із homologous field іншої family;
- SCM retained-pointer path із runtime stride `0x3C0` не є MOD consumer лише через shared `+0x18`/`0x40` physical pattern.

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
