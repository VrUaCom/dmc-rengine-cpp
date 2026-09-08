# DMC3 Enemy Architecture Learning Hub

Ця папка пояснює, що таке ворог у DMC3 HD не як «один файл em000.pac», а як повний runtime/resource system.

Навчальний матеріал базується на поточному evidence DMC Rengine і на `em000-extract.zip`. Він не замінює `docs/research/` та `data/reverse/`; останні лишаються технічною authority.

## Уроки

1. [Що таке enemy у DMC3](01-what-is-an-enemy.md)
2. [З яких ресурсів будується enemy](02-resource-anatomy.md)
3. [Як enemy проходить runtime lifecycle](03-runtime-lifecycle.md)
4. [Як насправді створити нового enemy](04-create-new-enemy.md)
5. [Карта форматів і research branches](FORMAT-BRANCH-MAP.md)

## Головна схема

```text
Stage / Event / Dependency control
              |
              v
        Enemy identity
              |
              v
       Enemy resource pack
              |
    +---------+----------+----------+---------+
    |         |          |          |         |
   MOD       MOT        PTX        CLT       effects
    |         |          |          |         |
    +---------+----------+----------+---------+
              |
              v
       CEm* runtime object
              |
    +---------+----------+----------+
    |         |          |          |
 animation   render    collision    AI/actions
    |         |          |          |
    +---------+----------+----------+
              |
              v
      damage / death / despawn
```

## Ключова думка

`em000` — це не «формат ворога». Це ім'я actor/enemy resource family, усередині якої живе багато окремих форматів.

Тому:

```text
Enemy != MOD
Enemy != PAC
Enemy != AI class
Enemy != one file
```

Правильніше:

```text
Enemy = runtime object + resource graph + spawn rules + gameplay behavior
```

## Поточна межа DMC Rengine

Ми вже дуже добре бачимо ресурсну анатомію `em000`: моделі, текстури, animation, cloth, effects, cross-resource references.

Але повна система створення нового enemy ще не закрита, бо треба відновити:

- enemy ID -> resource package;
- enemy ID -> concrete `CEm*` factory/class;
- spawn/event selector grammar;
- common enemy runtime ABI;
- AI/action state machine;
- damage/death/despawn;
- writer authority для змінених ресурсів;
- original-game acceptance.

Саме ці пункти відділяють «ми можемо прочитати ворога» від «ми можемо створити нового ворога для DMC3».
