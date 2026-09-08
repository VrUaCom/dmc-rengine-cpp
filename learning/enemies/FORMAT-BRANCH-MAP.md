# Enemy formats — branch ownership map

Цей файл показує, де саме продовжується reverse кожного формату, що входить у enemy resource ecosystem.

| Формат / domain | Гілка | Що там досліджуємо |
|---|---|---|
| MOD | `reverse/mod-completion-20260907` | повний MOD ABI, runtime, writer prerequisites |
| CLT | `reverse/clt-em000-20260908` | CLT grammar, Bone/Wind domains, cloth binding |
| TSC | `reverse/tsc-em000-20260908` | TSC grammar, texture/UV control semantics |
| MOT | `reverse/mot-em000-20260908` | MOT channels, compression, keys, CMotion binding |
| EFM | `reverse/efm-em000-20260908` | EFM model-family ABI, EFM-specific streams |
| effect pack | `reverse/effect-pack-em000-20260908` | G/V/E/P/T/A/M records and cross-references |
| SO working family | `reverse/so-em000-identity-20260908` | true identity of 038/039/040 and their consumers |
| PTX binding | `reverse/ptx-em000-binding-20260908` | classification of em000 texture bundle |
| em000 census | `reverse/em000-format-census-20260908` | 302/302 leaf identity accounting |
| full enemy system | `research/enemy-architecture-em000-20260908` | how formats combine with spawn/runtime/CEm behavior |

## Не створюємо branches для extraction labels

Неправильно:

```text
reverse/bin
reverse/txt
```

бо `.bin` і `.txt` у extraction не є semantic formats.

Приклади:

```text
.bin -> PTX / MOT / SO-working / effect record
.txt -> CLT / TSC / effect manifest
```

## Не дублюємо canonical infrastructure

PAC, PNST, DDS і canonical PTX reader уже мають власну архітектуру. Якщо enemy використовує їх, enemy layer просто посилається на ці modules.

## Системна гілка enemy потрібна окремо

Вона не є ще одним format parser.

Її задача:

```text
individual resources
 -> cross-resource edges
 -> enemy resource graph
 -> spawn/factory mapping
 -> CEm runtime lifecycle
 -> authoring validation
```

Саме тут у майбутньому має з'явитися `analysis/enemy/*`, а не всередині MOD/MOT/CLT parser-а.
