# Урок 4 — Як насправді створити нового enemy

## 1. Спочатку визначити, що означає «новий»

Є три різні задачі, і вони сильно відрізняються за складністю.

### A. Variant існуючого enemy

Той самий original `CEm*` gameplay class, але змінені ресурси.

Приклади майбутніх змін:

- нова texture;
- нова MOD geometry;
- нова animation, сумісна зі skeleton;
- змінений effect pack;
- інший CLT/TSC state.

Це найкоротший шлях.

### B. Новий actor/resource ID

Новий `emNNN`-подібний package, але behavior бере існуючий `CEm*` class.

Тут уже треба розширити або замінити таблиці identity/factory/spawn.

### C. Новий gameplay enemy

Нові AI, actions, damage/death rules, projectiles і lifecycle.

Це повний runtime authoring/recompilation task.

---

## 2. Мінімальний asset checklist

Для resource-level enemy треба визначити, що реально необхідно конкретному archetype.

Не кожен enemy обов'язково має всі формати, але em000 показує такі можливі компоненти:

```text
[ ] PTX texture bundle
[ ] primary MOD model(s)
[ ] auxiliary MOD/EFM model(s)
[ ] MOT animations / motion PACs
[ ] CLT + local chain models, якщо є cloth
[ ] TSC, якщо є texture/motion control
[ ] effect pack
[ ] SO-working spatial/control resources, якщо цей runtime path їх потребує
[ ] correct PAC/PNST physical topology
[ ] naming / resource identity metadata
```

## 3. Skeleton contract

Перед animation authoring треба закрити skeleton contract:

```text
node count
parent hierarchy
node order
motion groups
rest transforms
skin indices
skin weights
```

MOT channel domain має бути сумісний із target node domain.

Не можна просто взяти MOT від іншого enemy і припустити, що record index = bone index без validation.

## 4. Texture contract

Потрібно узгодити:

```text
MOD/EFM mesh.texture_slot
TSC TexNo / control references
PTX texture entry count
wrapped DDS effect textures
```

Texture slot out of range — це structural error, а не щось, що editor має «виправити» автоматично.

## 5. Cloth contract

Якщо enemy має CLT:

```text
CLT Bone -> local chain nodes
CLT WindParent -> external actor-domain candidate
local model chain -> MOD або EFM
```

Writer має зберігати обидва domain-и окремо.

## 6. Effect contract

Effect pack — це graph, не список незалежних blobs.

Current em000 evidence вже показує:

```text
E subtype 1/2 -> T + optional A
E subtype 5   -> M
M             -> MOD + optional companion
```

Тому зміна одного effect resource може вимагати оновлення reference graph, а не тільки заміни bytes.

## 7. Container contract

Навіть правильний MOD/MOT не є готовим enemy, якщо порушено physical slots.

Потрібно зберегти:

- sparse slots;
- empty slots;
- paired/grouped slots;
- nested PNST/PAC topology;
- original coordinate spaces;
- naming evidence окремо від physical identity.

## 8. Identity mapping — ключовий blocker

Для нового resource ID треба знайти точний runtime chain:

```text
enemy/spawn selector
 -> enemy logical ID
 -> emNNN/resource path
 -> dependency request
 -> factory/class selector
 -> CEm* instance
```

Поки цей mapping не закритий, можна робити replacement/variant research, але не можна чесно заявляти підтримку довільного нового enemy ID.

## 9. Factory mapping

Треба відновити:

```text
enemy id -> concrete CEm* constructor / factory
```

Тут можливі два сценарії.

### Reuse existing class

Новий resource package підключається до вже існуючого gameplay class.

Це майбутній Level B authoring.

### New class

Потрібний новий constructor/vtable/state machine або контрольована заміна існуючої dispatch entry.

Це Level C.

## 10. Spawn integration

Потрібно закрити:

- EventTbl dynamic spawn opcode/record;
- stage dependency declaration;
- spawn position/orientation;
- spawn conditions;
- lifetime/despawn conditions;
- limits/pools, якщо є.

Просто покласти `em999.pac` у NBZ недостатньо.

## 11. Gameplay integration

Повноцінний new enemy має пройти:

```text
spawn
 -> initialize
 -> idle/action
 -> movement
 -> target selection
 -> attacks
 -> collision/contact
 -> receive damage
 -> reaction/stagger
 -> death
 -> reward/drop if applicable
 -> despawn
 -> cleanup
```

## 12. Тестова стратегія

Порядок безпечної імплементації:

```text
1. parse all child resources
2. no-edit roundtrip
3. reopen all children
4. rebuild PAC/PNST
5. reopen full package
6. replacement test using existing enemy identity
7. controlled visible asset change
8. animation test
9. effect test
10. collision/contact test
11. spawn/death/reload test
12. new ID mapping test
13. rollback test
```

Кожен крок має власний evidence receipt.

## 13. Коли можна сказати «ми створили нового ворога»

Тільки коли original `dmc3.exe`:

- сам запитує/завантажує new enemy package;
- створює потрібний runtime object;
- показує правильну модель;
- відтворює animations;
- має правильні effects/cloth;
- collision і damage працюють;
- AI працює;
- death/despawn працює;
- reload не ламається;
- rollback повертає vanilla behavior.

До цього моменту слід називати capability точніше: `resource variant`, `replacement`, `new package candidate` або `runtime prototype`.
