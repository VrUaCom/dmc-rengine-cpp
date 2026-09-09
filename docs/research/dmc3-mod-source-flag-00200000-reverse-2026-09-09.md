# DMC3 HD MOD — source flag 0x00200000 reverse

**Branch:** `reverse/mod-completion-20260907`  
**Canonical executable:** `dmc3.exe`  
**SHA-256:** `e454272ed0fb0247fcbcf300e5d55d7a3e96d50b89b9ffaff81bb978dcbdd082`

## Target domain

The target is serialized object source flag bit `0x00200000` in object `+0x10`.

It must not be confused with two numerically equal masks in other runtime domains:

- manager `+0xE0` bit `0x00200000`;
- runtime object `+0x304` bit `0x00200000`.

Equal numeric masks do not imply shared provenance or semantic meaning.

## Corpus

Current multi-corpus evidence contains seven objects with the serialized source bit set. All seven are in the em000 corpus, within the observed header `+0x14 = 100407` cluster. The current pl000 and id100 samples do not carry the bit.

This is `CORPUS_CONFIRMED` distribution only. It does not justify a name such as `cloth`.

## Canonical initializer transfer

The canonical object initializer rooted at `0x1403029E0` establishes an exact whole-word transfer:

```text
0x140302AB2  read serialized object +0x10 source_flags
0x140302ABF  write same dword to runtime object +0x14
0x140302AC9  write same dword to runtime object +0x10
```

Therefore bit `0x00200000` is not dropped at load time. It is `EXE_CONFIRMED` as runtime-carried state in both runtime flag words.

## Rejected false positives

### `0x140303F2F`

This instruction tests manager `+0xE0` with mask `0x00200000`. That manager bit is raised by the separately proven source `0x00000200/0x00000400` path and belongs to a different runtime domain. It is `REJECTED` as the serialized source-bit consumer.

### `0x1402F4C21`

This instruction tests `0x00200000` in runtime object `+0x304`. Provenance is the dynamic `+0x304` state word, not source-carried runtime `+0x10/+0x14`. It is therefore also `REJECTED` as a direct consumer of serialized source bit `0x00200000` unless a future dataflow proof connects those words.

## Consumer census boundary

Within the audited canonical model subsystem, no separate immediate-mask test of `0x00200000` has yet been proven against runtime `+0x10` or `+0x14` after the initializer transfer. This is negative evidence only; the bit may be consumed through a copied/masked aggregate or a subsystem outside the currently bounded path.

## C++ contract

`include/dmc_rengine/analysis/mod/object_flags.hpp` models the exact whole-word carry into runtime `+0x10/+0x14`. A synthetic compile-time guard uses a non-trivial word (`0xA5200000`) and requires exact preservation of all bits, including `0x00200000`.

## Status

```text
corpus presence                     CORPUS_CONFIRMED
serialized -> runtime +0x10/+0x14   EXE_CONFIRMED
manager +0xE0 equal-mask candidate  REJECTED
runtime +0x304 equal-mask candidate REJECTED
high-level semantic                 PRESERVED_UNDECODED
writer policy                       preserve
```

## Next gate

Trace all reads/copies of runtime object `+0x10` and `+0x14` with object-type provenance, following the complete word through any transformations until either the `0x00200000` bit is explicitly masked/branched on or the state reaches the terminal renderer/material consumer. Until then the source bit remains `PRESERVED_UNDECODED`.
