# Урок 3 — Runtime lifecycle ворога

## 1. Ворог починається не з MOD

Гра спочатку повинна вирішити, **якого enemy треба мати в stage/mission state**, потім підготувати його ресурси, а вже після цього створити gameplay object.

Поточна evidence-backed модель:

```text
Stage / Mission / Event
        |
        v
Enemy dependency / spawn demand
        |
        v
Resource resolver
        |
        v
PAC/PNST materialization
        |
        v
MOD / MOT / PTX / CLT / TSC / effects ...
        |
        v
runtime managers
        |
        v
CEm* object construction
        |
        v
AI / animation / render / collision
        |
        v
damage / death / despawn / release
```

## 2. Stage-side evidence

Canonical format research уже встановив два важливі факти.

### EventTbl

`EventTblNN.bin` належить до mission/event control bytecode і має dynamic-spawn behavior на EXE-backed/high-confidence рівні.

Повний opcode grammar ще не закритий.

### StageCfg dependency scan

Коли StageCfg має фізичний slot 9, payload передається в helper `0x1401A9BC0`, а його dependency/control scan впливає на enemy-resource demand.

Ми ще не маємо права стверджувати, що кожен slot 9 завжди є EST або що кожен enemy spawn проходить через один і той самий record layout.

## 3. Resource acquisition

Після demand гра повинна знайти actor resources через загальний DMC3 resource stack:

```text
logical identity
 -> namespace/provider
 -> NBZ / loose backend
 -> PAC/PNST
 -> child slots
 -> format-specific post-load/runtime objects
```

Це означає, що новий enemy потребує не тільки правильних bytes, а й **правильної discoverability**.

## 4. Runtime CEm object

RTTI census підтверджує family `CEm*`.

Але ще треба відновити:

- common enemy base layout;
- конкретні enemy subclasses;
- factory/constructor table;
- manager ownership;
- vtables / virtual dispatch;
- initialization sequence;
- action/state dispatch;
- destruct/release sequence.

Це головна межа між resource reverse і gameplay reverse.

## 5. Animation frame

Runtime animation side концептуально:

```text
MOT/CMotion evaluation
 -> animated local transforms
 -> MOD node hierarchy
 -> currentWorld[]
 -> inverseRest * currentWorld
 -> skin palette
 -> GPU skinning
```

Serialized MOD rest transforms не повинні перезаписуватися анімаційним state у writer representation.

## 6. Cloth / secondary motion

CLT асоціюється з окремими local chain models.

Ймовірна runtime sequence:

```text
actor skeleton
 + CLT config
 + local cloth model chain
 -> cloth/deformation solver
 -> final matrices / geometry state
```

Але exact solver ABI і meaning axis/wind fields ще треба довести executable consumer-ом.

## 7. Effects

Effect pack містить logical graph та physical resource groups.

Він не повинен вбудовуватися в CEm parser. Runtime enemy лише отримує/викликає effect resources через окрему effect system.

## 8. Collision/contact

SO-working family має сильну spatial/transform correlation, але ми ще не закрили semantic labels типу hitbox/hurtbox/attack volume.

Тому gameplay collision layer поки треба описувати як:

```text
spatial/contact resources + runtime consumer still under reverse
```

## 9. Despawn і cleanup

Повний enemy authoring повинен пройти не тільки spawn, а й cleanup:

- remove runtime object;
- stop/retire animations;
- release or reuse resources;
- remove dynamic collision/contact state;
- destroy projectiles/effects as needed;
- survive stage reload;
- restore deterministic original state after rollback.

Якщо enemy може з'явитися, але ламає reload або lifetime — pipeline не завершений.
