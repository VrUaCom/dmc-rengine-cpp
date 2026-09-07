# Урок 6 — Animation, motion groups і MOT/CMotion boundary

## Головне розділення

```text
serialized MOD rest-local
!=
runtime evaluated animated-local
```

MOD не повинен поглинати MOT parser або зберігати animated pose назад у serialized rest transforms.

## Motion group

Третя node-domain byte array (`+0x08` pointer у domain header) тепер EXE-confirmed як motion-group selector.

Runtime chain:

```text
MOD third-domain
 -> manager +0x18
 -> CMotion third-domain pointer
 -> read by hierarchy order position
 -> map through nodeAtOrderPosition
 -> CMotionJoint +0xF8
```

Кілька CMotion evaluator/control sites порівнюють `joint +0xF8` із requested group id.

## em000 corpus

Bound archive:

`em000-extract.zip`
SHA-256 `306130125f09824811289366324f4208c3c1aba880c5a7efa3953a88d566d07b`.

Census:

- 35 MOD;
- 147 meshes;
- 14,804 vertices;
- 226 node order positions.

Group histogram:

```text
0 -> 132
1 -> 91
2 -> 3
```

Це доводить multi-group domain, але не дозволяє назвати 0/1/2 «body», «weapon», «upper body» тощо без окремого direct proof.

## Default joint

Header `+0x13` = MOD `default_joint_index`.

`JntNo` parser використовує його як fallback для out-of-range selector. Окремий consumer використовує його для `currentWorld[]`.

Не називай його root без доказу.

## Canonical animation binding

`analysis/mod/animation_binding` робить тільки MOD-owned composition:

```text
MOD hierarchy
+ MOD motion_group
+ caller-provided evaluated animated-local matrices
 -> currentWorld[]
 -> inverseRestWorld * currentWorld
 -> skin palette
```

`AnimationJointBinding` містить:

- node index;
- parent node index;
- motion group;
- order position.

## Що навмисно не робить цей module

- не parse MOT bytes;
- не assign semantics MOT channels;
- не переписує MOD rest transforms;
- не re-promote невалідавані CMotion offsets;
- не дає writer authority.

## Правильний майбутній design

```text
MOT parser/evaluator
     |
     v
evaluated animated-local matrices
     |
     v
MOD animation_binding
     |
     v
currentWorld
     |
     v
skin palette
```

Так ми зберігаємо format boundaries і можемо окремо тестувати MOT та MOD.
