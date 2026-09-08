# Урок 1 — Що таке enemy у DMC3

## 1. Не один файл

У DMC3 ворог не є одним `.mod`, `.pac` або `.bin`.

У грі існує runtime C++ family `CEm*`, а ресурси лише постачають дані для цього runtime object.

Тому найкраща модель:

```text
Enemy
= runtime CEm* object
+ resource package
+ skeleton/animation
+ textures/materials
+ effects
+ secondary motion/cloth
+ collision/contact relations
+ spawn/dependency rules
+ AI/actions
+ damage/death/lifetime
```

## 2. Чому `em000.pac` — не «enemy format»

`em000` — actor/enemy resource identity. Усередині нього знаходяться різні формати:

```text
PTX
MOD
CLT
EFM
TSC
MOT
SO-working resources
effect PNST
```

Іншими словами:

```text
em000.pac = container/resource ecosystem
MOD       = one model format inside it
MOT       = animation format
CLT       = cloth/deformation control format
PTX       = texture bundle
```

## 3. Runtime object важливіший за файл

Canonical DMC3 research показує великий MSVC C++ runtime з окремими class families:

```text
CPlayer
CEm*
CStage / CStageSet
CCamera*
...
```

Тому повний reverse enemy має пройти шлях:

```text
serialized data
 -> resource loader
 -> runtime manager
 -> factory / constructor
 -> CEm* object
 -> behavior
 -> lifetime
```

Парсер ресурсу закриває лише першу частину.

## 4. Три рівні «нового ворога»

### Variant

Той самий runtime class, але змінені ресурси.

Наприклад у майбутньому:

- інша модель;
- інші textures;
- інші MOT, сумісні зі skeleton;
- інші effects.

### New resource identity

Новий actor package, але гра все ще запускає існуючий `CEm*` class.

Для цього треба знати таблиці ID, resource paths, factory mapping і spawn mapping.

### New gameplay class

Повністю новий AI/behavior enemy.

Тут треба вже відновити або замінити runtime code: factory, actions, damage, death, projectiles, collision, cleanup.

## 5. Що вже відомо добре

Resource side `em000` уже дуже сильний:

- 302 leaf resources обліковані;
- 35 MOD;
- 82 MOT;
- 8 CLT;
- 1 TSC;
- 1 EFM;
- PTX;
- effect pack;
- SO-working structures.

Але gameplay/runtime side ще має великі open gates.

## 6. Правильна кінцева ціль

Не «зробити editor для em000.pac».

Правильна ціль:

```text
Enemy Definition / Enemy Resource Graph
        |
        v
validated child resources
        |
        v
spawn + factory mapping
        |
        v
runtime CEm instance
        |
        v
original-game acceptance
```

Тільки тоді DMC Rengine зможе чесно сказати: «створює нового ворога».
