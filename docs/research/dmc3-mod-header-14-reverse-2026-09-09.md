# DMC3 HD MOD — header +0x14 reverse closure

**Branch:** `reverse/mod-completion-20260907`  
**Canonical executable:** `dmc3.exe`  
**SHA-256:** `e454272ed0fb0247fcbcf300e5d55d7a3e96d50b89b9ffaff81bb978dcbdd082`

## Serialized ABI and runtime transfer

The serialized field is a raw little-endian `u32` at MOD header `+0x14`.

Canonical MOD manager initialization at `0x1402F9570` proves the transfer:

```text
0x1402F95C2  read  serialized header +0x14
0x1402F95C5  write manager +0xE4
```

This transfer is `EXE_CONFIRMED`. It proves runtime carriage, not the higher-level meaning of the value.

## Corpus falsification of the old decimal hypothesis

Expanded corpus observations include:

```text
pl000        -> 217
pl000 cloth  -> 217
id100        -> 1000000
```

These values falsify the earlier universal interpretation:

```text
family * 100000 + model_set * 100 + sub_index
```

That hypothesis is therefore `REJECTED`; the field must not be exposed as a universal `LegacyResourceCode` in the MOD contract.

## manager +0xE4 xref census

A direct model-core census over `0x1402F9000..0x14030D000` finds the provenance-confirmed manager write above and no direct provenance-confirmed read of this field.

A whole-executable disassembly scan finds 83 instructions using raw object displacement `+0xE4`. This is not 83 xrefs to the MOD manager. The candidates operate on unrelated object layouts and include different scalar widths and floating-point accesses. Offset equality alone cannot establish manager type provenance.

Therefore no candidate is promoted merely because it contains `[reg+0xE4]`. A valid downstream consumer requires reconstruction of the base pointer back to the manager instance initialized by `0x1402F9570`.

Known call sites into the manager initializer include:

```text
0x1403039E8
0x140303B0D
0x140303C38
0x140303D1D
```

They establish manager construction context but do not themselves give the `+0xE4` value a semantic identity.

## Semantic boundary

The following possibilities remain unproven:

- resource class;
- actor/model identity;
- variant selector;
- render-family selector;
- cache key;
- effect-attachment identity;
- manager-routing value.

None is selected as the field name without a type-aware downstream consumer.

## C++ contract

The MOD header keeps the raw field as `runtime_metadata_u32`. The evidence layer records:

```text
serialized -> manager transfer   EXE_CONFIRMED
old decimal identity formula     REJECTED
global semantic                  PRESERVED_UNDECODED
writer policy                    preserve
```

A future writer must reproduce the source `u32` unless an explicitly requested edit targets a subsequently proven semantic. It must not regenerate the value from model filenames or the rejected decimal decomposition.

## Status

```text
serialized offset/width          STRUCTURAL_CONFIRMED
runtime transfer to manager+E4   EXE_CONFIRMED
multi-corpus falsification       CORPUS_CONFIRMED
high-level semantic              PRESERVED_UNDECODED
writer policy                    preserve
```

## Next direct-EXE action

Continue from provenance-confirmed manager instances, not from raw displacement searches: identify where a pointer to the manager escapes after `0x1402F9570`, follow each typed consumer, then test whether any consumer reads `+0xE4` and what behavior depends on it. Until such a chain is established, `PRESERVED_UNDECODED` is the only evidence-safe global status.
