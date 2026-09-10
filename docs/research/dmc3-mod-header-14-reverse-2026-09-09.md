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

## Raw displacement census

A direct model-core census over `0x1402F9000..0x14030D000` finds the provenance-confirmed manager write above and no direct provenance-confirmed read of this field.

A whole-executable disassembly scan finds 83 instructions using raw object displacement `+0xE4`. This is not 83 xrefs to the MOD manager. The candidates operate on unrelated object layouts and include different scalar widths and floating-point accesses. Offset equality alone cannot establish manager type provenance.

Known call sites into the manager initializer include:

```text
0x1403039E8
0x140303B0D
0x140303C38
0x140303D1D
```

They establish manager construction context but do not themselves give the `+0xE4` value a semantic identity.

## Typed manager escape census — 2026-09-10 follow-up

The canonical executable was re-hashed before this pass and matched the project SHA-256 exactly.

The construction call chains expose two embedded manager layouts in the owning object:

```text
0x140089DE0  -> return parent +0x80
0x140089DF0  -> return parent +0x50
```

These accessors give a type-aware escape route that is stronger than scanning every raw `[reg+0xE4]` instruction.

Whole-executable call census:

```text
calls to 0x140089DE0 (parent+0x80 manager)   2695
calls to 0x140089DF0 (parent+0x50 manager)    131
```

For all immediate caller-side uses of the returned manager pointer, there is no direct read of `manager+0xE4`.

The accessor-derived pointer is then forwarded into 67 unique first-hop helpers. A helper-body displacement census finds only one first-hop helper containing any raw `+0xE4` access: `0x14030F850`.

That hit is a provenance false positive for the manager field. At a representative call site:

```text
0x140110315  call 0x140089DE0
0x14011031A  mov  rax, rdx       ; accessor-derived manager -> RDX
...                              ; destination object -> RCX
0x14011032B  call 0x14030F850
```

Inside `0x14030F850`:

```text
RDX -> R15   manager/source object
RCX -> RDI   destination CMotion object

0x14030F8A7  write R15 to destination +0xE8
0x14030F8AE  write u32 to destination +0xE4
```

The function subsequently reads manager/source fields through `R15` (`+0xEA`, `+0x188`, and others). Therefore `0x14030F8AE` is a write to a different destination layout, not a read or write of the MOD manager `+0xE4` metadata field. It is `REJECTED` as a manager-`+0xE4` consumer.

Bounded typed result:

```text
accessor-derived immediate manager+0xE4 reads     0
unique first-hop helpers                          67
first-hop helpers with any raw +0xE4 operand      1
proven manager+0xE4 reads in first-hop helpers    0
```

This materially strengthens the negative census, but it is intentionally not promoted to a mathematical whole-program unused claim. A first-hop helper can still forward the manager pointer to deeper callees, so recursive typed escape closure remains open.

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
serialized -> manager transfer        EXE_CONFIRMED
typed immediate/first-hop non-use      EXE_CONFIRMED (bounded)
old decimal identity formula           REJECTED
global semantic                        PRESERVED_UNDECODED
writer policy                          preserve
```

A future writer must reproduce the source `u32` unless an explicitly requested edit targets a subsequently proven semantic. It must not regenerate the value from model filenames or the rejected decimal decomposition.

## Status

```text
serialized offset/width               STRUCTURAL_CONFIRMED
runtime transfer to manager+0xE4      EXE_CONFIRMED
multi-corpus falsification             CORPUS_CONFIRMED
typed immediate/first-hop census       EXE_CONFIRMED
global semantic                        PRESERVED_UNDECODED
writer policy                          preserve
```

## Rejected hypotheses / false positives

- universal `LegacyResourceCode = family*100000 + model_set*100 + sub_index` — `REJECTED`;
- every executable access at displacement `+0xE4` consumes this manager field — `REJECTED`;
- `0x14030F8AE` in `0x14030F850` consumes manager `+0xE4` — `REJECTED`; it writes destination `CMotion +0xE4` while the manager is the second argument;
- header `+0x14` may be renamed as resource identity before a typed downstream consumer is proven — `REJECTED`.

## Next direct-EXE action

Recursively follow the 67 typed first-hop helper paths that retain or forward the accessor-derived manager pointer. Classify deeper callees by register/stack provenance rather than displacement equality. Only a provenance-confirmed read of the original manager `+0xE4`, or a completed recursive escape census with no such read, can close the remaining semantic gate.
