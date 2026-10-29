# DMC3 — відновлена поведінка CPtxManager

Дата: 2026-09-15. Гілка: **Ада-Астра**.
База: `a1c50e812b3ea35f9cb8df3771f1acf80a30c5ca`.
EXE SHA-256: `e454272ed0fb0247fcbcf300e5d55d7a3e96d50b89b9ffaff81bb978dcbdd082`.

Продовження: [структура payload, завантажувачі та очищення пулу](dmc3-ptx-payload-state-2026-09-15.md)
перенесені на C++ й перевірені окремо. Зазначені нижче loader/release межі
описують стан саме цього попереднього проходу.

## Результат

Відновлено C++-операції над станом `CPtxManager`: конструювання, окреме
скидання ключів, два варіанти acquire, release, звичайний і deleting cleanup.
**79 диференційних перевірок — PASS.** Це реалізація поведінки кешу з
явними залежностями від texture loaders, payload release та алокатора.
Декодування PTX, GPU-ресурси та Windows exception paths цим результатом не закриті.

Реалізація: `include/dmc_rengine/reverse/ptx_manager_state.hpp` і
`src/reverse/ptx_manager_state.cpp`. `PtxManagerImage` — типізований стан
канонічного образу. Поле `vtable_va` є адресною міткою, а не callable host vtable.
Це не оголошення, що оригінальний C++ клас або його двійковий ABI повністю замінено.

## Структура, підтверджена споживачами полів

Глобальний об'єкт: `0x140D5B860`. Розмір **0x4308** також передається
в sized delete на `0x140314DAE..0x140314DB6`.

| Область | Розмір / зміст | Доказ |
|---|---|---|
| Manager `+0x00` | 8-byte vptr, `0x140507B60` | constructor `0x140314C66/0x140314C6D` |
| Manager `+0x08` | 32 записи, stride `0x218` | constructor helper args; цикли пошуку/reset |
| Entry `+0x00` | 64-bit ключ ідентичності ресурсу | порівняння в `0x140314E40`, запис у `0x140314F0F` |
| Entry `+0x08` | 32-bit лічильник посилань | initialize-to-1, increment, decrement/test |
| Entry `+0x0C` | 4 невизначені байти | Ці методи їх зберігають |
| Entry `+0x10` | Payload **0x208 байтів** | 0x200-byte copy + останній qword; constructor 65 qword stores |

Типи внутрішніх полів payload не присвоєно. Рівність ключів означає
точну рівність 64-bit значень, без порівняння вмісту ресурсу.

## Функції й поведінка

| Адреса EXE | Відновлена операція |
|---|---|
| `0x140314CB0` | Обнулити лише payload одного entry, залишити перші 16 байтів |
| `0x140314C50` | Записати derived vptr, сконструювати 32 entries, повернути `this` |
| `0x140315150` | Обнулити ключ і count у всіх 32 entries, зберегти payload/padding |
| `0x140314E00` | Acquire через loader `0x140336BB0` |
| `0x140314FA0` | Acquire через loader `0x140336A70` із додатковим 64-bit аргументом |
| `0x140315180` | Зменшити count; при переході до нуля очистити ключ і викликати payload release |
| `0x140314D00` | Derived vptr → 32 порожні element callbacks у зворотному порядку → base vptr |
| `0x140314D60` | Такий самий cleanup; за `flags & 1` — sized delete `0x4308`; повернути адресу `this` |

У таблиці `0x140507B60` збережені targets: deleting cleanup, reset,
acquire-with-argument, acquire-basic, release. Сусідня таблиця IPtxManager
`0x140507B30` містить deleting entry і чотири `_purecall` targets.
Збережені pointers перевірені; вичерпний типізований census всіх virtual
consumers лишається окремою задачею.

### Acquire

1. Нульовий ключ повертає null без loader call.
2. Перший запис із таким ключем: `count++`, повертається адреса payload.
   Обидва acquire використовують **один кеш лише за ключем**; додатковий
   аргумент не є частиною cache identity й ігнорується при cache hit.
3. Якщо збігу немає, береться перший запис із нульовим ключем. Якщо всі
   зайняті, повертається null без спроби завантаження чи eviction.
4. Тимчасовий payload обнуляється до loader call. Нульовий результат loader
   повертає null і не комітить тимчасовий payload у кеш. Будь-який ненульовий
   результат записує key, count=1 і 0x208 байтів payload.

Лічильник має modulo-2^32 поведінку: `0xFFFFFFFF + 1 = 0`.
Реалізація не вводить нового saturation/overflow guard.

### Release і cleanup

Нульовий/відсутній ключ повертає false. Для першого збігу count зменшується.
Якщо результат ненульовий, повертається false, навіть коли ключ знайдений.
Лише перехід до нуля очищає key **до** виклику `0x1403317D0(payload)`
і повертає true. Count=0 зменшується до `0xFFFFFFFF`; release не викликається.

Reset не звільняє payload. Звичайний і deleting destructor також не викликають
payload release: element callback `0x14024EA30` лише повертається. C++
cleanup відтворює кінцевий стан із base-vtable tag, а не число проміжних
записів vptr. Звільнення реальних GPU-ресурсів не можна приписувати destructor
без дослідження окремої процедури release й зовнішнього порядку teardown.

## Інтеграційні адреси

- `0x140025050` викликає constructor для глобального manager, потім реєструє
  `0x14034ECA0`. Цей callback передає той самий глобальний об'єкт у destructor.
- `0x1400897BD/0x1400897C4` — глобальний receiver і прямий release call.
- `0x140089991/0x140089998` — глобальний receiver і acquire-basic call.
- `0x140331DCF/0x140331DD6` — глобальний receiver і reset call.

Ці пари перевіряються за інструкціями, а не лише пошуком рядків у дизасемблері.
Повний порядок reset/acquire/release між сценами поки не доведений.

## Перевірки

`verify_ptx_manager.py` компілює C++20 із `-O2 -Wall -Wextra -Wconversion
-Werror` і виконує канонічні інструкції в Unicorn 2.1.4. Порівнює:

- усі **0x4308 байтів** manager і дві guard-області по 64 байти;
- повернений payload offset/null, bool release, `this` identity;
- аргументи й кількість зовнішніх викликів, zeroed loader temporary,
  key/count і vptr на момент release/delete;
- реальний порядок 32 constructor та 32 reverse-cleanup callbacks;
- переповнення/underflow, null, повний кеш, перший/середній/останній slot,
  дублікати ключів, loader failure, nonzero success `1/-1`, перехресні acquire,
  повторне використання entry після release, reset без обнулення payload.

Конструктор, destructor, array helpers `0x140345B24/0x140345B94`, guard
`0x140346714 → 0x14024EA30` і cookie-check `0x1403455F0` виконуються реально
в емуляторі. Вихідний EXE не модифікується. Перехоплюються тільки
`0x140336BB0`, `0x140336A70`, `0x1403317D0`, `0x140345554`, `0x1403458D0`.
Їхня поведінка визначена тестовим контрактом; це не доказ їхньої повної
канонічної реалізації. Windows exception unwinding та reentrant/concurrent
сценарії в цю перевірку не входять.

```sh
python scripts/reverse/verify_ptx_manager.py /path/to/dmc3.exe . data/reverse/ptx-manager-state-20260915
```

Залежності: Capstone 5.0.7, Unicorn 2.1.4, g++; запускати без `python -O`.
`evidence.json` містить адреси інструкцій, хеші тіл, карту полів і прямих
споживачів. `verification.json` містить усі 79 випадків та хеші C++/скриптів.

## Наступна межа

Пріоритет: `0x140336BB0`, `0x140336A70`, `0x1403317D0` — структура
0x208-byte payload, створення/звільнення його ресурсів і значення додаткового
аргументу. Потім зіставити це з відновленими PTX-парсерами та шляхами scene
initialize/draw/shutdown. Додатково відкриті чотири байти entry `+0x0C`,
повний набір callers, exception rollback і решта структурної черги
(899 неохоплених кандидатів vtable). Повнота всієї гри не заявляється.
