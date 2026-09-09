# DMC3 HD MOD — secondary zero-region census

**Branch:** `reverse/mod-completion-20260907`  
**Canonical executable:** `dmc3.exe`  
**SHA-256:** `e454272ed0fb0247fcbcf300e5d55d7a3e96d50b89b9ffaff81bb978dcbdd082`

## Rule

A zero-filled region is not promoted to padding. `RESERVED_OBSERVED_ZERO` means only that the bounded corpus is zero in that region. It never authorizes writer zero-normalization.

## Header shell

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
whole-program no-read         not claimed
writer policy                 preserve
```

A type-aware whole-image check rejected apparent `source +0x18` candidates around `0x14030B568`: the owner reached from `0x14008BE50` is a different resource layout using manager-like fields `+0x78/+0x80/+0x88`, not the MOD manager `+0xE4/+0xE8/+0xEA` domain.

## Object record — provenance correction

The canonical MOD/EFM object initializer `0x1403029E0` derives each serialized object exactly as:

```text
source_header + 0x40 + object_index * 0x40
```

at `0x140302A70..0x140302A81`.

The important correction is `0x140302AAA`:

```text
runtime object +0x18 = pointer to complete serialized object record
```

It is **not** a read of serialized object `+0x18/+0x1C`.

This means later runtime code can still reach the entire original serialized object even when a field was not copied by the initializer. Therefore initializer-local no-read evidence is insufficient to call any middle-shell byte globally unused.

## Live MOD-specific object +0x18/+0x1C

The same initializer later directly uses the provenance-confirmed serialized object pointer.

When source flag `0x00000200` is active:

```text
0x140302D22  serialized object +0x18 f32 -> runtime +0x16C
0x140302D37  serialized object +0x1C u32 -> runtime +0x170
```

When source flag `0x00000400` is active the same copies repeat at:

```text
0x140302D82
0x140302D97
```

These fields were already present in the older runtime-projection research, and this pass independently re-confirmed their pointer provenance from direct disassembly. They are now promoted to a MOD-specific serialized ABI contract in:

```text
include/dmc_rengine/formats/mod/object_serialized.hpp
```

Downstream, runtime `+0x160..+0x16C` is copied as a four-float group into per-mesh/pass state, while runtime `+0x170` is converted to float at `0x1403040D9` and carried beside it. Thus these bytes are unquestionably live render-path inputs, although their artistic/material names remain `PRESERVED_UNDECODED`.

## Remaining object zero regions

The bounded corpus-zero regions under investigation are still:

```text
object +0x04..+0x07
object +0x14..+0x17
object +0x20..+0x2F
```

The initializer does not directly decode them. However the complete serialized pointer is retained at runtime `+0x18`, so the next-level evidence question is downstream pointer use, not initializer use.

One confirmed retained-pointer consumer is `0x1402F9BCB`: it loads runtime object `+0x18` and then reads serialized object `+0x08` to recover the child mesh table. This is a positive control proving the retained pointer is genuinely used later rather than being dead bookkeeping.

Render-command builders also load runtime object `+0x18` (`0x1402F845E` and the homologous second path), although the bounded first builder does not dereference its saved local source pointer afterward. These are escape edges and are being classified rather than treated as proof of unknown-field use.

Therefore the three remaining zero regions keep:

```text
CORPUS_CONFIRMED
RESERVED_OBSERVED_ZERO
initializer-local no-read = EXE_CONFIRMED
whole-program non-use = not yet promoted
writer policy = preserve
```

## Node-domain shell

`node-domain +0x10..+0x1F` remains zero in the bounded corpus. Transform initializer `0x1402FA080` consumes domain-resolved arrays/pointers rather than providing enough raw serialized provenance to declare the shell globally unused.

Status remains:

```text
CORPUS_CONFIRMED
RESERVED_OBSERVED_ZERO
writer policy = preserve
```

## Rejected claims

- zero shell implies padding — `REJECTED`;
- no initializer read implies no later read — `REJECTED`;
- `0x140302AAA` reads serialized `+0x18/+0x1C` — `REJECTED`; it stores the serialized record pointer;
- equal `+0x18` displacement in another owner proves MOD use — `REJECTED`;
- writer may normalize these regions to zero — `REJECTED`.

## Next gate

Continue from provenance-confirmed `runtime object +0x18` loads and classify every downstream dereference against the 0x40-byte serialized object layout. The objective is to determine whether `+0x04..07`, `+0x14..17`, or `+0x20..2F` are ever read by canonical MOD code. In parallel, keep header and node-domain zero shells preservation-only until equivalent whole-program provenance closure exists.
