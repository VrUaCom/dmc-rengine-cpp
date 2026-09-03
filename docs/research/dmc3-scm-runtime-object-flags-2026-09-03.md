# DMC3 SCM runtime object flags — canonical EXE pass 2026-09-03

## Scope

This record continues the SCM structural reverse on branch `scm` and narrows two previously unresolved object fields:

- source object byte `+0x01`;
- source object flags `u32 @ +0x10`.

Canonical analysis target:

- size: `6,356,432` bytes;
- SHA-256: `e454272ed0fb0247fcbcf300e5d55d7a3e96d50b89b9ffaff81bb978dcbdd082`;
- ImageBase: `0x140000000`.

This pass records **operational runtime behavior only**. It does not assign names such as transparency, collision, draw priority, material mode, or visibility unless those semantics are independently proven.

## 1. SCM-like runtime object initializer

Function `0x140302F10` walks source records with the already recovered SCM physical object stride:

```text
source = resourceBase + 0x40 + objectIndex * 0x40
```

It copies source object state into a runtime object with stride `0x3C0`.

This function is a high-confidence SCM-specific consumer because it uses the SCM 0x40 source-object envelope and differs from the nearby shared path `0x1403029E0`, whose downstream family tests explicitly specialize the MOD/EFM family masks `0x10000000` and `0x20000000`.

The exact higher-level factory/vtable binding remains a separate provenance closure item, so the function is described as **SCM-like / high-confidence SCM consumer**, not as a recovered original C++ symbol name.

## 2. Source object `+0x01`

At `0x14030303A`:

```text
source object +0x01 -> runtime object +0x07
```

The byte is copied verbatim.

Downstream initialization then uses runtime `+0x07` in branches. Observed exact comparisons include:

- `0xEA` in a special object/index combination;
- `0xC4` in another special combination;
- special-case code may rewrite runtime `+0x07` to `0xC5` or `0x80`;
- values above `0x80` can propagate into runtime `+0x178` while the byte itself also initializes runtime `+0x17C`.

Therefore the old status “completely unused unknown byte” is rejected.

Current safe description:

> **runtime-consumed object control/classification byte, exact semantics unresolved.**

Do not rename the serialized C++ field to a gameplay/render semantic yet.

### Corpus distribution

Across 68 unique SCM resources / 254 objects:

| value | objects |
|---:|---:|
| `0x80` | 237 |
| `0xC0` | 6 |
| `0xC2` | 3 |
| `0xC4` | 3 |
| `0xC1` | 2 |
| `0xC5` | 2 |
| `0xC3` | 1 |

The corpus distribution is evidence about shipped data, not a whitelist. The EXE also contains runtime handling for `0xEA`, which is not required to appear in the current corpus.

## 3. Source object flags `u32 @ +0x10`

`0x140302F10` loads source `+0x10` and preserves it in runtime object fields before projecting selected source bits into runtime state.

### Direct bit projection

| source condition | exact runtime action | EXE site |
|---|---|---|
| `(flags & 0x0000000F) != 0` | set runtime flag bit 8 | `0x1403032E1` |
| `(flags & 0x00000020) != 0` | set runtime flag bit 10 | `0x1403032FB` |
| `(flags & 0x00020000) != 0` | set runtime flag bit 9; initialize runtime floats `+0x160/+0x164/+0x168 = 1.0`, `+0x16C = 0` | `0x140303315` |
| `(flags & 0x00010000) != 0` | set runtime flag bit 7 | `0x14030337C` |
| `(flags & 0x00040000) != 0` | OR runtime flag `0x10` / bit 4 | `0x140303398` |
| `(flags & 0x0F000000) != 0` | set runtime flag bit 15; write `((flags >> 24) & 0xF) - 1` to runtime byte `+0x0D` | `0x1403033B3` |
| `(flags & 0x00080000) != 0` | OR runtime flag `0x20` / bit 5 | `0x1403033E3` |

This table is stronger than a corpus correlation: each row is directly recovered from the canonical executable.

## 4. Helper `0x140302640`

After direct flag projection, `0x140302F10` calls helper `0x140302640` with the same source flags.

Recovered input selection:

```text
lowMode = flags & 0xF

if lowMode != 0:
    helper mode = lowMode
    call 0x1402F17C0(mode)

    if lowMode == 4:
        state selector = 0x50007
    else if flags & 0x00100000:
        state selector = 0x5010D
    else:
        state selector = 0x5000D
else:
    helper mode = 9
    call 0x1402F17C0(9)
    state selector = 0x5080B
```

The source `0x00010000` bit independently flips a boolean carried into helper output state:

```text
flags & 0x00010000 -> false
otherwise          -> true
```

The numeric selectors and modes are confirmed. Their semantic names remain unresolved.

## 5. Corpus flags

Across 254 source objects:

| flags | count |
|---:|---:|
| `0x00000000` | 165 |
| `0x00080000` | 32 |
| `0x00200000` | 19 |
| `0x00000001` | 17 |
| `0x00100001` | 9 |
| `0x00180001` | 4 |
| `0x00080001` | 3 |
| `0x00080002` | 3 |
| `0x00020000` | 2 |

Union of all bits actually observed in the 68-file corpus:

```text
0x003A0003
```

Observed bit positions are `0, 1, 17, 19, 20, 21`.

Important separation:

- `0x20`, `0x10000`, `0x40000`, and high `0x0F000000` behavior is **EXE_CONFIRMED** even though these bits are absent from the current corpus;
- `0x200000` is **DATA_CONFIRMED** in 19 source objects, but no direct interpretation for that bit was recovered in the bounded functions above;
- absence of a bit from the corpus does not reject its original-runtime support.

## 6. C++20 integration rule

The clean module `scm_runtime_flags.hpp` exposes a pure projection from serialized source flags to the exact runtime operations recovered in this pass.

Its names are deliberately neutral:

- `source_mask_...` rather than invented semantics;
- `runtime_flags_to_set`;
- `helper_mode`;
- `helper_state_selector`;
- `helper_secondary_boolean`;
- `initialize_unit_vector`;
- `high_mode_minus_one`.

This is reverse/reconstruction infrastructure, **not writer authority**.

## 7. Header `+0x14` remains open

Corpus values are highly structured, including examples such as:

```text
300100
311500
341100
350900
400100
400101
441100..441105
4509xx
```

However:

- many nested resources share `311500`, so this is not a unique file/resource identity;
- the primary SCM normalizer does not establish a meaning for `+0x14`;
- bounded immediate-value searching did not reveal a useful canonical lookup table.

Status remains:

> **DATA_CONFIRMED structured field / semantic role RESEARCH_REQUIRED.**

## 8. Remaining SCM semantic frontier

1. exact original meaning of source object `+0x01`;
2. semantic names for the recovered `+0x10` flag operations;
3. source bit `0x00200000` consumer/meaning;
4. header `+0x14` producer/consumer;
5. exact scene-transform rotation consumer and convention;
6. material/texture-slot ownership chain;
7. full writer acceptance: no-edit byte identity -> edited reparse -> PAC/NBZ reintegration -> original-game consumption -> rollback.

No SCM completion/100% claim is made by this pass.
