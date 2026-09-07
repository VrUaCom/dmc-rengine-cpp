# Урок 9 — Практичний reverse workflow для нового `.MOD`

## Крок 1 — Provenance

Перед аналізом:

- hash payload;
- запиши parent archive/member/slot;
- не змінюй original bytes;
- зафіксуй exact executable authority, якщо робиш EXE xref.

## Крок 2 — Header

Перевір:

- magic `MOD `;
- size >= `0x40`;
- version;
- counts;
- node-domain pointer.

Не відхиляй автоматично незнайому finite version: warning + preserve часто правильніше за unsupported silent rewrite.

## Крок 3 — Objects

Outer table починається з `0x40`, stride `0x40`.

Для кожного object:
- mesh count;
- aggregate count;
- mesh table;
- flags;
- bounds;
- preserved parameters.

## Крок 4 — Meshes

Stride `0x50`.

Перевір:
- stream offsets;
- alignment;
- element count;
- texture slot;
- GS CLAMP;
- generated workspace coordinate base.

## Крок 5 — Streams

Read-only decode:

- position float3;
- normal float3;
- UV int16/4096;
- blend u8x4;
- control u16.

Зберігай raw streams паралельно з decoded representation.

## Крок 6 — Skin

Для кожної вершини:
- lower 15 bits → q0/q1/q2;
- `/31`;
- y/z/w blend index;
- `/4` → bone index;
- range/duplicate/sum guards.

## Крок 7 — Node domain

Перевір:
- parent/order/group/transform rel offsets;
- permutation;
- topological order;
- root semantics;
- transforms complete/finite.

Не inverse-permute parents.

## Крок 8 — Rest spatial hierarchy

Побудуй local matrices, далі model-space world.

Oracle:
- hierarchy order коректний;
- world positions походять із row 3;
- model preview не використовує geometry vertices як bone positions.

## Крок 9 — Inverse rest / skin palette

Побудуй inverse rest і rest currentWorld.

Очікуй:

```text
inverseRest * restWorld == identity
```

Це сильний математичний regression.

## Крок 10 — Texture companion

Parse companion окремо.

Перевір:
- companion valid;
- `mesh.texture_slot < texture_count`;
- header mirror mismatch — окрема diagnostics.

## Крок 11 — Corpus census

Для unknown field збери:
- count;
- min/max;
- unique values;
- per-file distribution;
- correlations з mesh/node/flags;
- anomalies.

Нуль у 35 MOD — evidence, але не semantic proof.

## Крок 12 — EXE xref

Потрібен exact producer/consumer:

- де serialized field читається;
- куди переноситься;
- які branches залежать;
- який downstream helper його використовує;
- чи є формат-specific dispatcher proof.

## Крок 13 — Semantic promotion

Promotion має містити:
- canonical EXE hash;
- exact VA/range;
- bounded claim;
- non-claims;
- C++ module;
- regression;
- machine-readable receipt.

## Крок 14 — Writer — окремий етап

Тільки після reader/reverse closure:
- layout planner;
- unknown preservation;
- exact coordinate spaces;
- no-edit rebuild parity;
- edited reopen;
- container reintegration;
- original-game acceptance;
- rollback.

## Anti-patterns

Не можна:
- «підчистити» unknown zeros;
- mask out-of-range values без доказу;
- переносити semantics з SCM;
- відновлювати bone positions із vertices;
- серіалізувати post-load image;
- видавати synthetic test за original-game proof.
