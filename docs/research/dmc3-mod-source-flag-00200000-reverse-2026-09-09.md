# DMC3 HD MOD — source flag 0x00200000 canonical typed-runtime closure

**Branch:** `reverse/mod-completion-20260907`  
**Canonical executable:** `dmc3.exe`  
**SHA-256:** `e454272ed0fb0247fcbcf300e5d55d7a3e96d50b89b9ffaff81bb978dcbdd082`  
**Closure follow-up:** 2026-09-11

## Target and boundary

The target is bit `0x00200000` in serialized MOD object source flags at object `+0x10`.

It is distinct from numerically equal bit21 masks in manager `+0xE0` and runtime object `+0x304`. Numeric mask equality is not provenance.

Canonical result:

```text
canonical typed runtime consumption    EXE_CONFIRMED: carried but uninterpreted/dormant
serialized high-level semantic         PRESERVED_UNDECODED
writer policy                          preserve exact source bit
```

Dormancy is a canonical-runtime behavior statement, not permission to rename the serialized bit padding/reserved or clear it.

## Corpus

Seven current objects carry serialized source bit21, all in the em000 corpus and the observed header `+0x14 = 100407` cluster. Current pl000 and id100 samples do not carry it. This is `CORPUS_CONFIRMED` distribution only and does not justify an artistic name such as `cloth`.

## Serialized -> runtime carriage

`0x1403029E0` proves exact whole-word transfer:

```text
0x140302AB2  read serialized object +0x10 source_flags
0x140302ABF  -> runtime effective +0x14
0x140302AC9  -> runtime baseline +0x10
```

`0x1403058F0` selectively refreshes low mode + bit20:

```text
effective = (effective & 0xFFEFFFF0) | (baseline & 0x0010000F)
```

`0x140305B90` can restore the complete baseline word on its zero-override path. These paths establish carriage/restoration, not a bit21 semantic.

## Proven semantic decoders

The typed runtime graph reaches these known consumers:

```text
0x140302640  -> low nibble, 0x00010000, 0x00100000
0x1402F9890  -> 0x00004000 only
0x1402F28E0  -> BTS bit17 and complete-word write-back
```

No decoder tests, shifts, indexes, compares or emits state from source-carried bit21. The table path through `0x1402F17C0` receives a value already reduced by `AND 0xF`, so bit21 cannot affect its address.

## Whole-image runtime-object census — corrected root count

Runtime objects are structurally identified as:

```text
manager +0xE8 object count
manager +0x100 runtime-object array
runtime_object = manager +0x100 + object_index*0x380
```

The earlier model-core-bounded pass counted 31 exact derivation functions. Whole-image follow-up found one additional canonical root:

```text
0x14029F0B0
  -> call 0x140089DE0 (model-manager accessor)
  -> validate manager +0xE8
  -> derive manager +0x100 + index*0x380
```

Therefore the canonical whole-image count is **32 exact runtime-object derivation functions**.

The added root does not read runtime baseline `+0x10` or effective `+0x14`. It follows runtime `+0x18` to serialized object `+0x08` and then enters mesh data. Thus the bit21 semantic result is unchanged.

The semantic-decoder surface remains:

```text
32 exact runtime-object derivation functions
61 direct object-passing call sites
39 unique direct helper targets
45 conservative typed interprocedural states
19 typed propagation edges
0 tagged indirect/vtable calls
0 bit21 semantic decoders
```

The only provenance-confirmed direct runtime `+0x14` read on the manager-bound construction surface remains:

```text
0x1402F9ED9  mov r8d,[runtime_object+0x14]
0x1402F9EEF  call 0x1402F9890
```

and `0x1402F9890` reduces the word with `AND 0x00004000`.

## Equal-mask domains rejected

### Runtime object `+0x304`

`0x1402F2C30` independently mutates runtime `+0x304`, including `BTS bit21` at `0x1402F2CDD`. No source `+0x10/+0x14 -> +0x304` derivation exists. This is `REJECTED` as source-bit21 consumption.

### Manager `+0xE0`

Serialized flags `0x00000200/0x00000400` independently raise manager `+0xE0` bit21 at `0x140302CF9/0x140302D59`; `0x140303F2F` consumes that manager-domain bit. This is also `REJECTED` as serialized source bit21.

## Pointer-escape control

The path-insensitive candidate pass produced apparent stores/returns, but direct canonical-disassembly classification showed they were scalar state, draw/mesh pointers, allocation-array returns or packet values rather than exported `0x380` runtime-object pointers. No provenance-confirmed runtime-object getter/store escape was recovered from the audited surface.

## Canonical closure

```text
serialized bit21 presence                    CORPUS_CONFIRMED
serialized -> baseline/effective              EXE_CONFIRMED
state-machine carriage/restoration             EXE_CONFIRMED
local GS-state bit21 interpretation             EXE_CONFIRMED: none
common material bit21 interpretation            EXE_CONFIRMED: none
whole-image typed bit21 decoder                 EXE_CONFIRMED: none
manager +0xE0 equal-mask domain                 REJECTED
runtime object +0x304 equal-mask domain         REJECTED
canonical typed runtime effect                  EXE_CONFIRMED: dormant/uninterpreted
serialized high-level semantic                  PRESERVED_UNDECODED
writer policy                                   preserve exact source bit
```

## C++ / regression contract

`include/dmc_rengine/analysis/mod/object_flags.hpp` models exact source -> baseline/effective carriage, restoration, the external bit17 mutation and common-material interpreted mask. `tests/mod_skin_tests.cpp` verifies synthetic non-zero bit21 survives the modeled carry/mutation path while remaining outside every proven interpreted mask.

No parser/writer normalization is authorized.

## Rejected hypotheses

- `0x00200000 = cloth` from corpus clustering — `REJECTED`;
- equal bit21 mask across runtime domains means same semantic — `REJECTED`;
- whole-word propagation means every bit is interpreted — `REJECTED`;
- canonical runtime non-consumption makes the serialized bit padding/reserved — `REJECTED`;
- writer may clear or synthesize the bit — `REJECTED`.
