# DMC3 HD MOD — source flag 0x00200000 canonical typed-runtime closure

**Branch:** `reverse/mod-completion-20260907`  
**Canonical executable:** `dmc3.exe`  
**SHA-256:** `e454272ed0fb0247fcbcf300e5d55d7a3e96d50b89b9ffaff81bb978dcbdd082`  
**Closure follow-up:** 2026-09-10

## Target and evidence boundary

The target is bit `0x00200000` (bit 21) in serialized MOD object source flags at object `+0x10`.

This bit must not be confused with numerically equal masks in two independent runtime domains:

- manager `+0xE0` bit `0x00200000`;
- runtime object `+0x304` bit `0x00200000`.

Equal numeric masks do not establish common provenance or semantic meaning.

The result of this pass is deliberately split in two:

```text
canonical typed runtime consumption    EXE_CONFIRMED: carried but uninterpreted/dormant
serialized high-level semantic         PRESERVED_UNDECODED
writer policy                          preserve exact source bit
```

“Dormant” is a canonical-runtime behavior statement, not permission to rename the serialized bit as padding/reserved or to normalize it.

## Corpus

Current multi-corpus evidence contains seven objects with serialized source bit21 set. All seven are in the em000 corpus, within the observed header `+0x14 = 100407` cluster. Current pl000 and id100 samples do not carry the bit.

This is `CORPUS_CONFIRMED` distribution only. It does not justify an artistic name such as `cloth`.

## Serialized -> runtime carriage

The object initializer rooted at `0x1403029E0` establishes exact whole-word transfer:

```text
0x140302AB2  read serialized object +0x10 source_flags
0x140302ABF  write same dword to runtime object +0x14 effective flags
0x140302AC9  write same dword to runtime object +0x10 baseline flags
```

Thus bit21 is `EXE_CONFIRMED` as runtime-carried state.

The two runtime words have distinct roles:

```text
runtime +0x10 = baseline/source flags
runtime +0x14 = mutable/effective flags
```

## State-machine preservation, not interpretation

`0x1403058F0` selectively restores low mode + bit20:

```text
effective = (effective & 0xFFEFFFF0) | (baseline & 0x0010000F)
```

The mask preserves the current effective bit21; baseline bit21 is not selected by this partial refresh.

`0x140305B90` clears the low nibble and, on the zero-override path, ORs the complete baseline `+0x10` word into effective `+0x14`. This can reintroduce serialized bit21. Both paths then forward the effective word to `0x140302640`.

This proves deliberate carriage/restoration of the container. It does not assign a terminal semantic to bit21.

## Terminal local GS-state decoder

`0x140302640` interprets only:

```text
0x0000000F  low mode nibble
0x00010000  separate state selector
0x00100000  GS TEST_1 / ZBUF_1 selector
```

The low-nibble path is reduced with `AND 0xF` before it reaches table helper `0x1402F17C0`; therefore the table index cannot depend on bit21.

There is no test, shift, bit-test, table-index dependency, or arithmetic extraction of source bit21 in this decoder.

## Common material decoder

Canonical MOD construction derives a runtime object as:

```text
manager +0x100 + object_index * 0x380
```

The only direct read of runtime object `+0x14` found in the complete direct-derivation surface is:

```text
0x1402F9ED9  mov r8d,[runtime_object+0x14]
0x1402F9EEF  call 0x1402F9890
```

The common material helper then reduces the incoming flag dword to:

```text
0x1402F98E6  AND 0x00004000
0x1402F98EB  TEST
```

This selects the already-established legacy GS TEX1 filtering state. Bit21 does not reach the decision after the mask.

## External effective-word mutator

`0x1402F28E0` is a separate live consumer of effective `+0x14`:

```text
0x1402F2901  read effective +0x14
0x1402F2904  BTS bit17
0x1402F290C  write complete dword back to +0x14
```

It sets `0x00020000`, preserving pre-existing bit21 but never testing/interpreting it.

## Expanded typed runtime-object census

A new canonical-EXE pass derives the runtime object structurally from the manager instead of searching raw displacement `+0x14`.

The direct surface contains:

```text
31 functions with manager-bound index*0x380 + manager+0x100 derivation
61 direct object-passing call sites
39 unique direct helper targets
```

Across those 31 derivation functions, the only provenance-confirmed direct runtime-object `+0x14` read is `0x1402F9ED9`, the material path described above.

The 39 helper targets were then seeded into a deliberately conservative interprocedural pass. The pass over-approximates ambiguous object arguments so false positives enlarge the audited surface rather than hiding consumers.

Result:

```text
45 typed interprocedural states
19 typed propagation edges
0 tagged indirect/vtable calls
0 new bit21 semantic decoders
```

The only provenance-confirmed flag operations recovered are the already-known paths:

```text
0x140302640  -> AND 0xF / 0x10000 / 0x100000
0x1402F9890  -> AND 0x4000
0x1402F28E0  -> BTS bit17, write-back only
0x1403058F0  -> restoration/container masking
0x140305B90  -> restoration/override OR
```

No reachable typed helper uses bit21 as a branch condition, table/address index, shift-derived selector, compare input, or terminal packet/material state.

## Pointer-escape control

A deliberately path-insensitive pointer pass produced several candidate stores/returns from functions that had previously derived a `0x380` object. Each candidate was manually checked in canonical disassembly before classification.

The candidates are not runtime-object pointer escapes. Representative corrections include:

- `0x1402F5640`: RAX is a shifted scalar/state value by the store, not the runtime object pointer;
- `0x1402F8996` / `0x1402FF2AF`: the stored pointers are derived from manager `+0x120` / per-draw structures, not the `0x380` object;
- `0x140302F09`: the initializer returns its original manager/allocation argument from `[rbp+0x88]`, not the per-object pointer;
- `0x14030732A`: writes a manager `+0x118`-derived mesh/subobject pointer into a per-mesh runtime record;
- `0x1403075A9`: writes a newly assembled 64-bit packet/state value into a per-mesh record.

No provenance-confirmed getter/store exporting the `0x380` runtime object was recovered from the audited canonical surface.

## Literal and non-literal bit21 census

A whole-image literal-mask/bit-index pass was used only as a negative/positive control, not as the sole provenance method.

The model subsystem contains two literal/equivalent bit21 operations with different provenance:

### Runtime object `+0x304`

`0x1402F2C30` derives the same `0x380` object but operates on the separate dynamic word `+0x304`. At `0x1402F2CDD` it executes `BTS bit21` and writes the word back. It does **not** read baseline/effective `+0x10/+0x14`.

The initializer also zeroes `+0x304` independently after source-flag handling. Therefore `+0x304` bit21 has its own writer/state domain and is `REJECTED` as a derivation from serialized source bit21.

### Manager `+0xE0`

Inside `0x1403029E0`, serialized source flags `0x00000200` and `0x00000400` cause `BTS bit21` on manager `+0xE0` at `0x140302CF9` / `0x140302D59` while also activating separate object/runtime transfers. This is an independently generated manager-domain bit.

`0x140303F2F` later tests that manager bit. It is `REJECTED` as a serialized source-bit21 consumer.

## Rejected external-layout candidate

A weaker whole-image `+0xE8/+0x100/0x380` coincidence around `0x140320DDC..0x140321117` was inspected directly. That routine allocates and interpolates float arrays; its `+0x100`, `+0xE8`, and `+0x380` accesses belong to an unrelated generated-array layout. It lacks the model-manager provenance chain and is `REJECTED` as an external MOD runtime-object consumer.

## Canonical closure

For the canonical executable, the provenance-aware model-manager/runtime-object surface is now closed strongly enough to answer the downstream-effect question:

```text
serialized bit21 presence                    CORPUS_CONFIRMED
serialized -> baseline/effective              EXE_CONFIRMED
state-machine carriage/restoration             EXE_CONFIRMED
local GS-state bit21 interpretation             EXE_CONFIRMED: none
common material bit21 interpretation            EXE_CONFIRMED: none
expanded typed helper graph bit21 consumer      EXE_CONFIRMED: none
indirect/vtable typed escape                     EXE_CONFIRMED: none recovered
manager +0xE0 equal-mask domain                 REJECTED as source-bit consumer
runtime object +0x304 equal-mask domain         REJECTED as source-bit consumer
canonical typed runtime effect                  EXE_CONFIRMED: dormant/uninterpreted
serialized high-level semantic                  PRESERVED_UNDECODED
writer policy                                   preserve exact source bit
```

This closes the `0x00200000` **canonical runtime-consumption gate**. It does not establish why the bit exists in serialized files, whether another executable/version uses it, or an artistic/material category. Those questions remain outside the semantic promotion boundary.

## C++ / regression contract

`include/dmc_rengine/analysis/mod/object_flags.hpp` already models:

- exact source -> baseline/effective carriage;
- selective restoration behavior;
- the external bit17 mutator mask;
- the common-material interpreted mask.

`tests/mod_skin_tests.cpp` already supplies synthetic non-zero bit21 and verifies that it survives the modeled carry/mutation path while the proven material/mutator masks do not alias bit21.

No parser/writer normalization is authorized. A future writer must preserve the bit verbatim on no-edit round-trip.

## Rejected hypotheses

- `0x00200000 = cloth` from corpus clustering — `REJECTED`;
- manager `+0xE0` bit21 is the same field because the numeric mask matches — `REJECTED`;
- runtime object `+0x304` bit21 is automatically derived from source bit21 — `REJECTED`;
- whole-word propagation into a helper means every bit is semantically consumed — `REJECTED`;
- canonical runtime non-consumption means the serialized bit is padding/reserved — `REJECTED`;
- a writer may clear or synthesize the bit — `REJECTED`.
