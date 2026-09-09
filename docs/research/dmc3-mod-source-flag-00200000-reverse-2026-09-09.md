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

The object initializer rooted at `0x1403029E0` establishes an exact whole-word transfer:

```text
0x140302AB2  read serialized object +0x10 source_flags
0x140302ABF  write same dword to runtime object +0x14
0x140302AC9  write same dword to runtime object +0x10
```

Thus bit `0x00200000` is `EXE_CONFIRMED` as runtime-carried state in both baseline and effective flag words.

## Runtime flag roles

Subsequent code distinguishes the two runtime words:

```text
runtime +0x10 = baseline/source flags
runtime +0x14 = mutable/effective flags
```

`0x1403058F0` selectively rebuilds low mode + bit20:

```text
effective = (effective & 0xFFEFFFF0) | (baseline & 0x0010000F)
```

Bit21 is therefore retained from the current effective word rather than refreshed by this selective path.

`0x140305B90` first clears the effective low nibble. When its override mask is zero, it ORs the **complete baseline +0x10 word** into effective `+0x14`. Consequently source bit21 is deliberately capable of being reintroduced into effective state. With a non-zero override, the supplied override mask is ORed instead.

This proves bit21 participates in the state container; it does not prove a renderer interpretation.

## Provenance-aware object-state consumer census

The bounded canonical MOD object-state pipeline was audited from the object initializer/state helpers through the local GS packet builder. Provenance-confirmed runtime flag reads include:

```text
effective +0x14:
  0x140305911
  0x140305930
  0x140305942
  0x14030599D
  0x1403059AE
  0x140305BBC
  0x140305BDA
  0x140305BEF
  0x140305BFF

baseline +0x10:
  0x140305903
  0x140305924
  0x140305BD7
```

The object-state paths that rebuild renderer state pass effective `+0x14` to `0x140302640`.

Direct disassembly of `0x140302640` shows its flag-word interpretation is bounded to:

```text
0x0000000F   low mode nibble
0x00010000   separate state bit
0x00100000   GS TEST_1 / ZBUF_1 selector
```

There is no `0x00200000` test in this helper and no arithmetic extraction that reaches bit21. The helper therefore does **not** interpret source bit21 into the GS packet.

## External effective-word mutation

Whole-model follow-up found a separate runtime-object consumer at `0x1402F28E0`.

```text
0x1402F2901  read effective +0x14
0x1402F2904  BTS bit 17
0x1402F290C  write complete dword back to +0x14
```

This sets `0x00020000`. It preserves every pre-existing unrelated bit in the effective dword, including bit21, but never tests or interprets `0x00200000`.

This is another live consumer of the **container**, not a semantic bit21 consumer.

## Common material path

Two independent construction paths read runtime effective `+0x14` and pass the complete dword as the third argument (`R8D`) to common material helper `0x1402F9890`:

```text
MOD path:
  0x1402F9ED9  mov r8d,[runtime_object+0x14]
  0x1402F9EEF  call 0x1402F9890

parallel model path:
  0x1402FA042  mov r8d,[runtime_object+0x14]
  0x1402FA058  call 0x1402F9890
```

The MOD provenance is explicit in the containing construction function: manager `+0x100 + object_index*0x380` selects the runtime object, and manager `+0xE0` high nibble `0x10000000` selects the canonical MOD runtime mesh builder before the material loop.

Direct disassembly of `0x1402F9890` then gives the decisive mask:

```text
0x1402F98E0  load incoming effective/source flag dword
0x1402F98E6  AND 0x00004000
0x1402F98EB  TEST result
```

That helper uses only source/effective flag `0x00004000` to select the legacy GS TEX1 filtering state. It does not test, shift, compare or otherwise extract bit21.

Therefore source bit21 now has an additional EXE-confirmed propagation edge:

```text
serialized source bit21
 -> baseline/effective +0x10/+0x14
 -> runtime object construction
 -> common material helper 0x1402F9890
 -> no bit21 interpretation
```

Passing a whole dword into a helper is not proof that every bit in that dword is semantically consumed.

## Rejected equal-mask false positives

### `0x140303F2F`

Tests manager `+0xE0` with mask `0x00200000`. That manager bit belongs to a separate runtime domain raised through a different source-flag path (`0x00000200/0x00000400`). It is `REJECTED` as the serialized source-bit consumer.

### `0x1402F4C21`

Tests `0x00200000` in runtime object `+0x304`. This is another dynamic runtime word. No source `+0x10/+0x14 -> +0x304` derivation has been proven, so it is also `REJECTED` as a direct consumer.

## What is now closed

The following is `EXE_CONFIRMED`:

- serialized bit21 survives object initialization;
- it exists in baseline `+0x10` and effective `+0x14`;
- selective reset preserves its current effective state;
- zero-override restoration can reintroduce it from baseline;
- external helper `0x1402F28E0` mutates another effective bit while preserving bit21;
- the proven object-state-to-GS helper `0x140302640` does not interpret bit21;
- the common material helper `0x1402F9890` receives the complete effective word but interprets only `0x00004000`, not bit21;
- equal-mask hits in manager `+0xE0` and runtime `+0x304` are separate domains unless a future provenance edge proves otherwise.

## What remains open

A semantic consumer outside the audited model object-state/material paths could still exist, or a future proof could connect source bit21 to another runtime domain. Neither has been demonstrated.

Therefore the correct status remains:

```text
corpus presence                       CORPUS_CONFIRMED
serialized -> baseline/effective      EXE_CONFIRMED
mutation/restoration participation    EXE_CONFIRMED
local GS packet interpretation        EXE_CONFIRMED: no bit21 consumer
common material interpretation        EXE_CONFIRMED: no bit21 consumer
manager +0xE0 equal-mask candidate    REJECTED
runtime +0x304 equal-mask candidate   REJECTED
high-level semantic                   PRESERVED_UNDECODED
writer policy                         preserve
```

## C++ contract

`include/dmc_rengine/analysis/mod/object_flags.hpp` models exact whole-word carry, baseline/effective restoration, the external bit17 mutator and the common material interpreted mask. Compile-time guards explicitly verify that neither the mutator set-mask nor the material interpreted mask includes `0x00200000`.

## Next gate

A future promotion requires one of two things:

1. a provenance-confirmed semantic consumer of bit21 outside the audited object-state/material paths; or
2. a proven copy/derivation from source-carried `+0x10/+0x14` to another runtime word whose bit21 semantic is independently known.

Until one of those exists, byte preservation is mandatory and semantic naming is prohibited.
