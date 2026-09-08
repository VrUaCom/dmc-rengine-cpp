# DMC3 HD MOD — manager bit 0x00200000 runtime-vector chain (2026-09-09)

**Branch:** `reverse/mod-completion-20260907`  
**Canonical executable:** `dmc3.exe`  
**SHA-256:** `e454272ed0fb0247fcbcf300e5d55d7a3e96d50b89b9ffaff81bb978dcbdd082`

## Why this pass exists

The MOD object-runtime reverse had already established that serialized object flags `0x00000200` and `0x00000400` can promote a manager-global runtime bit `0x00200000` (bit 21), while object-local parameters `+0x18/+0x1C` are live on that path. The downstream use of the manager-global bit was still open.

This pass follows the original executable instead of relying on external importers.

## 1. Direct manager-bit consumer

Canonical function range around `0x140303F22` reads manager/runtime flags at `manager +0xE0` and tests bit `0x00200000`:

```text
140303F22  mov  rax, [rbp + manager]
140303F29  mov  eax, [rax + 0xE0]
140303F2F  and  eax, 0x00200000
140303F34  test eax, eax
140303F36  je   0x140303F48
140303F38  lea  rdx, [local_float4]
140303F3C  mov  rcx, manager
140303F43  call 0x140306560
```

Therefore manager bit 21 is not passive bookkeeping. It gates construction of a runtime float4/vector by helper `0x140306560`.

Status: **EXE_CONFIRMED**.

## 2. What helper 0x140306560 does

The helper does not return a fixed preset. It derives a vector from live global/runtime state:

1. obtains the active global/environment object through `0x140CF2330 -> +0x240`, with fallback `0x1405CEB10`;
2. loads two float4 values from that state at `+0x70` and `+0x80`;
3. subtracts them;
4. normalizes the resulting vector through `0x140330390`;
5. reads three values from the current per-index runtime matrix/state table (`manager-derived table + 0x40` across three 0x10-spaced rows);
6. builds a second vector with W forced to zero;
7. adds it to the normalized environment vector;
8. normalizes again into the caller-provided output.

The exact artistic name of this vector is not promoted here. The machine-code behavior proves it is a normalized runtime direction/vector derived from environment/global state plus current indexed runtime state.

Status: **EXE_CONFIRMED_BEHAVIOR / SEMANTIC_NAME_OPEN**.

## 3. Distribution into runtime objects

After the helper returns, the caller iterates manager runtime-object records (`manager +0x100`, stride `0x380`). For active records it checks object runtime bits `0x1000` / `0x2000`. On that path the generated vector components are copied into:

```text
runtime object +0x160 = vector.x
runtime object +0x164 = vector.y
runtime object +0x168 = vector.z
```

This closes the downstream role of manager bit `0x00200000`: it enables generation and distribution of a shared runtime direction/vector to qualifying render objects.

## 4. Relationship to serialized MOD object flags

Earlier MOD object-runtime projection already proved:

```text
serialized source flag 0x00000200
    -> runtime object bit12
    -> manager global bit21
    -> live object +0x18/+0x1C parameters

serialized source flag 0x00000400
    -> runtime object bit13
    -> manager global bit21
    -> live object +0x18/+0x1C parameters
```

This pass closes the next edge:

```text
manager global bit21 (0x00200000)
    -> 0x140306560 runtime vector generation
    -> runtime object +0x160/+0x164/+0x168 for qualifying objects
```

So `0x00000200/0x00000400` are part of a real special render/runtime-vector feature. It is still unsafe to assign an artistic label such as light-vector, environment-vector, reflection-vector, wind-vector, etc. until the final consumer of runtime object `+0x160..+0x168` is recovered.

## 5. Evidence status

| Claim | Status |
|---|---|
| manager `+0xE0 & 0x00200000` gates helper | `EXE_CONFIRMED` |
| helper `0x140306560` computes normalized live vector | `EXE_CONFIRMED` |
| vector copied to runtime object `+0x160/+0x164/+0x168` | `EXE_CONFIRMED` |
| serialized `0x200/0x400` can raise manager bit21 | `EXE_CONFIRMED` from prior MOD projection |
| final artistic/render semantic | `OPEN` |
| writer mutation authority | `NOT_PROMOTED` |

## 6. Next closure target

Trace reads of runtime object `+0x160/+0x164/+0x168` into the final render/shader packet. That will determine whether this vector is lighting, environment/reflection, special-effect orientation, or another legacy render input without guessing from shape alone.
