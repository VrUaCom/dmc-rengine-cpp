# DMC3 MRP direct-consumer sweep — 2026-08-31

**Status:** CANONICAL NEGATIVE-EVIDENCE ADDENDUM  
**Target:** canonical unpacked DMC3 HD analysis executable  
**SHA-256:** `e454272ed0fb0247fcbcf300e5d55d7a3e96d50b89b9ffaff81bb978dcbdd082`  
**Purpose:** determine whether the runtime evidence supports treating `MRP` as a fourth ordinary mesh family beside MOD/EFM/SCM.

## 1. Result

`MRP` remains a real byte-recognized primary family, but the direct consumer sweep finds **no evidence that it uses the ordinary MOD/EFM/SCM mesh-object path**.

Current strongest boundary:

```text
MRP byte family identity                    = EXE_CONFIRMED
MRP registry type 3                         = EXE_CONFIRMED
MRP four-byte mask 0x40000000              = EXE_CONFIRMED
MRP ordinary MOD/EFM/SCM model factory     = NEGATIVE EVIDENCE
MRP MOD/EFM/SCM family-specific allocation = NEGATIVE EVIDENCE
MRP own geometry                            = NOT PROVEN
MRP non-geometric semantics                = NOT PROVEN
```

Therefore a clean implementation must **not** create an MRP mesh adapter by analogy alone.

## 2. Family classifier baseline

The independent four-byte classifier at `0x1402FD650` returns:

```text
MOD  -> 0x10000000
EFM  -> 0x20000000
SCM  -> 0x30000000
MRP  -> 0x40000000
MCV  -> 0x50000000
SHW  -> 0x60000000
```

Fourteen direct calls to this function are present in the canonical executable.

Recognition by this classifier proves family identity. It does not by itself prove that every recognized family participates in the same graphics object factory or auxiliary mesh-memory layout.

## 3. Graphics/model factory excludes MRP

Function around `0x140248140` calls `0x1402FD650` at `0x14024815C` and selects runtime object construction by the returned family mask.

Recovered behavior:

```text
MOD / EFM
  -> allocation/object path with size 0x780
  -> constructor/factory path through 0x140089270

SCM
  -> allocation/object path with size 0x580
  -> constructor/factory path through 0x140089320

MRP / MCV / SHW / unknown
  -> no object allocation in this factory
  -> returns null/default result
```

The MOD/EFM test is compiled as an arithmetic/bitmask idiom rather than two literal equality compares:

```text
mask - 0x10000000
then test with 0xEFFFFFFF
```

This accepts `0x10000000` and `0x20000000`, then SCM is checked separately against `0x30000000`.

No MRP case exists in this factory.

A caller around `0x140270723` uses this factory before performing further model-object virtual calls. It separately checks whether the original family was SCM, but the preceding factory already bounds this path to MOD/EFM/SCM.

Consequence:

> MRP is not instantiated through this recovered ordinary MOD/EFM/SCM graphics/model object factory.

This is negative architectural evidence, not a claim that MRP can never own geometry through some other subsystem.

## 4. Auxiliary runtime allocation excludes MRP-specific mesh blocks

Function `0x1402FD8D0` computes/installs auxiliary runtime memory for a loaded resource object.

Recovered sequence:

```text
family = classifier(payload)
size += common/base allocation helper 0x1402FD9C0

if family == MOD or EFM:
    size += 0x1402FDB40(...)
else if family == SCM:
    size += 0x1402FDD10(...)
else:
    no family-specific block from this dispatcher

object +0x248 = allocation pointer/base
object +0x250 = resulting size
```

Thus:

```text
MOD -> common base + related mesh-specific auxiliary allocation
EFM -> common base + related mesh-specific auxiliary allocation
SCM -> common base + distinct stage-mesh auxiliary allocation
MRP -> common base only in this dispatcher
MCV -> common base only in this dispatcher
SHW -> common base only in this dispatcher
```

The SHW result is consistent with the independent finding that SHW is geometry-related but uses a structurally distinct shadow topology path rather than the MOD/EFM/SCM mesh shell.

For MRP, the important conclusion is narrower:

> this allocator does not expose an MRP-specific mesh-memory path equivalent to MOD/EFM/SCM.

## 5. Direct classifier-call sweep

All fourteen direct call sites of `0x1402FD650` were enumerated.

Observed call-site categories include:

- repeated model helpers around `0x1402F2A1F`, `0x1402F2B0D`, `0x1402F2D61`, `0x1402F2FE2`, `0x1402F313B`, `0x1402F39BE`, `0x1402F3A9C`, `0x1402F5310` — explicit MOD/EFM selection;
- `0x14024815C` — MOD/EFM object factory plus separate SCM branch;
- `0x140270735` — SCM distinction in a context whose preceding factory is already bounded to MOD/EFM/SCM;
- `0x1402F9628` — stores the family mask into runtime object field `+0xE0` rather than immediately selecting a family-specific handler;
- `0x1402FD8FD` — auxiliary allocation: MOD/EFM and SCM only receive family-specific blocks;
- `0x1402FDB68` — nested inside the MOD/EFM allocation/helper path;
- `0x140307374` — explicit MOD/EFM selection.

No inspected direct call site contains an immediate MRP-specific equality branch (`family == 0x40000000`).

This does not prove that the stored family mask can never be consumed indirectly later, so the result is recorded as **negative evidence**, not as proof of absence.

## 6. Stored family mask at runtime object +0xE0

The call around `0x1402F9628` stores the classifier result into runtime object field `+0xE0`. Other code preserves/copies this field and masks its high nibble with `0xF0000000`.

Within the recovered primary model/render region, explicit high-nibble family selections were found for:

```text
0x10000000 MOD
0x20000000 EFM
0x30000000 SCM
```

No corresponding `0x40000000` MRP selection was found in that scoped consumer sweep.

The executable contains unrelated uses of the numeric constant `0x40000000` in other flag domains. Those must **not** be promoted as MRP evidence merely because the numeric value matches the family mask.

## 7. String/shader negative evidence

A direct byte/string census of the canonical executable finds no contiguous:

```text
"MRP"
"MRP "
".mrp"
"mrp"
```

This is compatible with the classifier because it recognizes M/R/P through individual byte compares rather than a referenced string literal.

The executable does retain DMC3-specific shader source/path material for MOD, EFM, STG/SCM-side rendering and SHW, but no `DMC3_MRP.hlsl` path/source was identified in this pass.

Again, absence of an embedded shader label is not proof that MRP is non-geometric.

## 8. Interpretation boundary

The evidence currently supports this architecture:

```text
MOD / EFM / SCM
  -> ordinary model/mesh object family
  -> explicit family-specific graphics/runtime paths

SHW
  -> geometry-related shadow topology
  -> separate ABI/path and external spatial vector pool

MRP
  -> real primary runtime family
  -> not ordinary MOD/EFM/SCM object factory
  -> no recovered family-specific auxiliary mesh allocation
  -> exact consumer/ownership still unresolved
```

It is tempting to expand `MRP` into a material/render-parameter acronym based on this negative evidence. That would be speculation and is **not** promoted.

## 9. Remaining promotion gates

MRP can be promoted beyond the current companion/open-schema boundary only after at least one stronger evidence path is recovered:

```text
1. real retail MRP payload with exact source/provenance;
2. original-runtime consumer that dereferences MRP-specific fields;
3. an MRP-specific render/upload/buffer path;
4. an original debug/type/source identifier that bounds semantic purpose;
5. runtime observation tying a selected MRP payload to a visible render behavior.
```

Until then:

> **MRP is a confirmed runtime family, but not a confirmed 3D mesh file.**
