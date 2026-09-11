# DMC3 HD MOD — secondary zero-region census

**Branch:** `reverse/mod-completion-20260907`  
**Canonical executable:** `dmc3.exe`  
**SHA-256:** `e454272ed0fb0247fcbcf300e5d55d7a3e96d50b89b9ffaff81bb978dcbdd082`  
**Evidence follow-up:** 2026-09-11

## Rule

A zero-filled region is not promoted to padding. `RESERVED_OBSERVED_ZERO` means only that the bounded corpus is zero in that region. It never authorizes writer zero-normalization.

Canonical runtime dormancy is also a behavioral result, not a serialized semantic name. A dormant field remains `PRESERVED_UNDECODED` unless stronger format evidence gives it a semantic identity.

## Header shell — still open

Canonical manager/header initializer `0x1402F9570` positively consumes the known header domain including `+0x10`, `+0x11`, `+0x13`, and `+0x14`. Within that initializer there is no raw read from:

```text
header +0x08..+0x0F
header +0x18..+0x1F
header +0x28..+0x3F
```

Status remains loader-scoped:

```text
corpus zero observation       RESERVED_OBSERVED_ZERO
initializer-local no-read     EXE_CONFIRMED
whole-program no-read         not yet claimed
writer policy                 preserve
```

A type-aware whole-image check rejected apparent `source +0x18` candidates around `0x14030B568`: the owner reached from `0x14008BE50` is a different resource layout using manager-like fields `+0x78/+0x80/+0x88`, not the MOD manager `+0xE4/+0xE8/+0xEA` domain.

## Object record provenance

The canonical MOD/EFM object initializer `0x1403029E0` derives each serialized object as:

```text
source_header + 0x40 + object_index * 0x40
```

at `0x140302A70..0x140302A81`.

At `0x140302AAA` it stores the **complete serialized object-record pointer** into:

```text
runtime object +0x18
```

This is not a read of serialized object `+0x18/+0x1C`.

The retained pointer means initializer-local no-read evidence is insufficient. The object secondary regions therefore required a full retained-pointer and alternative-direct-source census.

## Live MOD-specific object +0x18/+0x1C

These are not part of the secondary-zero closure. When source flag `0x00000200` or `0x00000400` is active, `0x1403029E0` reads serialized object `+0x18` / `+0x1C` and propagates them into runtime render state. They remain live `PRESERVED_UNDECODED` fields under their separate contract.

## Target object regions

The bounded corpus-zero regions are:

```text
object +0x04..+0x07
object +0x14..+0x17
object +0x20..+0x2F
```

Current corpus contains 166 objects and all bytes in those regions are zero. This is `RESERVED_OBSERVED_ZERO` corpus evidence only.

## Whole-image canonical runtime-object census

The runtime object is structurally identified by the joint model-manager fingerprint:

```text
manager +0xE8 object count
manager +0x100 runtime-object array
runtime_object = manager +0x100 + object_index * 0x380
```

A whole-executable `.pdata` census finds **32 exact runtime-object derivation roots**. The previously bounded model-core pass counted 31; the additional root is `0x14029F0B0`, which calls model-core accessor `0x140089DE0`, validates manager `+0xE8`, derives the same `0x380` runtime object from manager `+0x100`, and therefore belongs to the same canonical model-manager domain.

Conservative pointer propagation from all 32 roots reaches:

```text
61 typed interprocedural states
49 direct pointer-tagged calls
5 provenance-confirmed runtime object +0x18 loads
0 retained serialized-object pointer escapes
0 retained serialized-object indirect-call escapes
```

The five retained-pointer loads are:

| Function | Load | Result |
|---|---|---|
| `0x14029F0B0` | `0x14029F0F0` | reads serialized object `+0x08`, then enters mesh-record domain |
| `0x1402F7D60` | `0x1402F7DB4` | EFM sibling builder; reads serialized object `+0x08`, then mesh records |
| `0x1402F8000` | `0x1402F845E` | EFM render builder; saved local is never dereferenced before return |
| `0x1402FE6A0` | `0x1402FE6F4` | MOD runtime mesh builder; reads serialized object `+0x08`, then mesh records |
| `0x1402FE930` | `0x1402FED8E` | MOD render builder; saved local is never dereferenced before return |

Across all five paths, the only provenance-confirmed serialized-object field read through the retained pointer is:

```text
serialized object +0x08 mesh-table pointer   3 reads
```

There are zero reads/writes of:

```text
+0x04..+0x07
+0x14..+0x17
+0x20..+0x2F
```

No retained serialized-object pointer is stored or passed through an indirect call from the typed surface.

## Additional whole-image source-header derivation control

The executable can also reconstruct serialized objects directly from the source header rather than following runtime `+0x18`. A separate whole-image census searched for the joint pattern:

```text
manager/source +0x108 -> source header
object_index * 0x40
source_header + 0x40 + index*0x40
```

Exactly five functions match:

```text
0x1402F9F20
0x1402FDB40
0x1402FDD10
0x1403029E0
0x140302F10
```

Classification:

- `0x1403029E0` is the canonical MOD/EFM initializer and consumes the already-known live object fields, not the three target zero regions;
- `0x1402FDB40` and `0x1402FDD10` are layout/planning paths. Their serialized-object reads are limited to the known object count/mesh-table domain (`+0x00`, `+0x08`) before entering 0x50-byte mesh records;
- `0x140302F10` uses a `0x3C0` runtime-object stride and is independently identified as the SCM object initializer; it is `REJECTED` as a MOD consumer;
- `0x1402F9F20` also uses the separate `0x3C0` runtime-owner domain. It is not accepted as MOD/EFM `0x380` object provenance and does not establish use of the target secondary bytes.

Thus no alternative direct-source path revives the target regions.

## SCM false-positive rejection

`0x140302F10..0x14030345A` is deliberately kept as a cross-format negative control. It also retains a 0x40-byte serialized record pointer at runtime `+0x18`, but its runtime object stride is `0x3C0`, not MOD/EFM `0x380`, and it contains the separately documented SCM `EA -> C5` / `C4 -> 80` compatibility behavior.

Therefore equal serialized stride and equal retained-pointer offset do not transfer semantics across families.

## Canonical closure of object secondary regions

The retained-pointer surface and the independent direct-source-header surface jointly close canonical runtime consumption of the three target regions:

```text
object +0x04..+0x07
  bounded corpus                       RESERVED_OBSERVED_ZERO
  canonical typed runtime effect       EXE_CONFIRMED: dormant / no effect
  serialized semantic                  PRESERVED_UNDECODED
  writer policy                        preserve exact source bytes

object +0x14..+0x17
  bounded corpus                       RESERVED_OBSERVED_ZERO
  canonical typed runtime effect       EXE_CONFIRMED: dormant / no effect
  serialized semantic                  PRESERVED_UNDECODED
  writer policy                        preserve exact source bytes

object +0x20..+0x2F
  bounded corpus                       RESERVED_OBSERVED_ZERO
  canonical typed runtime effect       EXE_CONFIRMED: dormant / no effect
  serialized semantic                  PRESERVED_UNDECODED
  writer policy                        preserve exact source bytes
```

“Dormant” does not mean padding/reserved. A future executable/version or non-zero corpus sample may reopen the semantic question, but the canonical DMC3 HD runtime-consumption gate is closed.

## Node-domain shell — still open

`node-domain +0x10..+0x1F` remains zero in the bounded corpus. Transform initializer `0x1402FA080` consumes domain-resolved arrays/pointers rather than providing enough raw serialized provenance to declare the shell globally unused.

Status remains:

```text
CORPUS_CONFIRMED
RESERVED_OBSERVED_ZERO
writer policy = preserve
whole-program closure = pending
```

## Rejected claims

- zero shell implies padding — `REJECTED`;
- no initializer read implies no later read — `REJECTED`;
- `0x140302AAA` reads serialized `+0x18/+0x1C` — `REJECTED`; it stores the serialized record pointer;
- equal `+0x18` displacement in another owner proves MOD use — `REJECTED`;
- the SCM `0x3C0` retained-pointer path is a MOD consumer because it also uses 0x40 source records — `REJECTED`;
- canonical runtime dormancy authorizes writer zero-normalization — `REJECTED`.

## Next gate

The object-secondary retained-pointer gate is closed. Remaining secondary work is now the **header shell** and **node-domain `+0x10..+0x1F`** whole-program census. Until those are independently closed, their source bytes remain preservation obligations.
