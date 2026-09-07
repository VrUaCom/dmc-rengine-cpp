# Урок 10 — Вправи та debugging

Ці вправи навчають працювати з MOD без небезпечних semantic shortcuts.

## Вправа 1 — Намалюй resource stack

Для будь-якого MOD віднови:

```text
NBZ
 -> namespace
 -> PAC/PNST
 -> slot
 -> MOD
 -> texture companion
```

Мета: навчитися відрізняти container identity від resource identity.

## Вправа 2 — Розбери header вручну

З hex dump випиши:

- version;
- object count;
- node count;
- texture mirror;
- default joint;
- raw `+0x14`;
- node-domain offset.

Після цього звір із canonical parser.

## Вправа 3 — Один mesh

Для одного `0x50` mesh record:
- порахуй усі offsets;
- познач, які resource-base-relative;
- окремо познач `+0x40` mesh-record-relative;
- перевір stream spans.

## Вправа 4 — UV

Візьми raw signed int16 UV і поділи на 4096. Переконайся, що ти не трактуєш 4 bytes як два floats.

## Вправа 5 — Skin vertex

Для одного packed word:
1. відділи `0x8000`;
2. витягни q0/q1/q2;
3. поділи на 31;
4. візьми blend y/z/w;
5. поділи raw indices на 4;
6. перевір bone range.

## Вправа 6 — Hierarchy

Створи synthetic order:

```text
order = [0, 2, 1]
parents-by-order = [-1, 0, 2]
```

Покажи, чому parent `2` — node index, а не order position. Потім спробуй неправильний inverse-permutation і побач різницю.

## Вправа 7 — Rest palette oracle

Побудуй:
- rest local;
- rest world;
- inverse rest;
- palette.

Для кожного node palette має бути identity.

Потім додай +3 по X до одного animated local/current chain і перевір, що delta проходить у правильний node.

## Вправа 8 — Texture mismatch

Сценарій:

```text
header mirror = 3
companion count = 2
mesh slots = [0,1]
```

Правильний висновок:
- mirror mismatch;
- runtime bindings valid.

Тепер постав slot 2:
- runtime binding invalid.

## Debugging: модель вибухає

Перевір у такому порядку:

1. element count / stream bounds;
2. blend raw index divisible by 4;
3. bone range;
4. weight packing;
5. hierarchy parent semantics;
6. matrix multiplication order;
7. rootBase.

## Debugging: texture неправильна

1. companion parsed?;
2. texture count?;
3. mesh slot?;
4. descriptor stride `0x40`?;
5. TEX0?;
6. GS CLAMP?;
7. nearest-filter source flag?

## Debugging: topology зламана

1. serialized `0x8000` semantics;
2. чи не декодуєш post-load-cleared stream як original;
3. `+0x40` coordinate base;
4. generated count `+0x48`;
5. workspace capacity/bounds.

## Debugging: bone overlay неправильний

1. `supports_spatial_hierarchy()`;
2. full permutation?;
3. root parent = -1?;
4. parents evaluated before children?;
5. row-vector `local * parentWorld`?;
6. position з matrix row 3?;
7. не використані vertices як bone positions?

## Фінальна вправа

Напиши пояснення одного unknown field у форматі:

```text
Known bytes:
Known runtime paths:
Corpus observations:
Rejected assumptions:
Safe current status:
What evidence would close it:
```

Якщо можеш зробити це без слова «скоріше за все», ти вже працюєш у стилі DMC Rengine evidence-first reverse.
