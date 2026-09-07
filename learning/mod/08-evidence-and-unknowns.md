# Урок 8 — Evidence, unknowns і безпечний reverse

## Чому unknown bytes — це актив, а не сміття

Reverse помиляється найчастіше тоді, коли «не знаю» перетворюють на `reserved = 0`.

У MOD source bytes є preservation authority. Якщо semantic meaning не доведено, writer зобов’язаний зберегти raw bytes.

## Поточні важливі unknowns

### Header `+0x14`
Runtime-carried до manager `+0xE4`, exact MOD semantic consumer open.

### Transform `+0x1C`
Observed zero у em000 corpus; global purpose open.

### Mesh `+0x0C`
Typed preserved `u32`, runtime-unconsumed у confirmed mesh path.

### Mesh `+0x38`
Typed preserved `u64`, post-load deliberately не relocates його у confirmed MOD path.

### Mesh `+0x4C`
Typed preserved `u32`.

### BLENDINDICES.x
Observed constant/reserved-like; shader skin influences використовують y/z/w. Exact x purpose open.

### Object `+0x18/+0x1C`
Вони EXE-consumed для flags `0x200/0x400`, але artistic names не встановлені.

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

Якщо SCM і MOD мають однаковий offset, це доводить лише shared layout тоді, коли є independent evidence. Semantic meaning може відрізнятися.

Приклад: MOD `+0x14` не отримує SCM resource-code semantics автоматично.

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

## `RESERVED_OBSERVED_ZERO`

Цей статус означає буквально: у bound corpus поле було нульовим.

Він не означає:
- zero is required;
- non-zero invalid;
- field safe to erase.

## Writer authority

Щоб поле стало editable, треба не тільки знати meaning. Треба знати serialization rule, valid ranges, dependencies, derived fields, alignments, container reintegration і прийняття оригінальним runtime.

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
