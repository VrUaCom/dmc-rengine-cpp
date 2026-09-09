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

## 2026-09-09 direct retained-pointer consumer census

This follow-up was performed directly against the hash-verified 6,356,432-byte canonical executable, not from raw offset matches in notes.

### MOD runtime mesh builder — positive control

`.pdata` bounds the function exactly as:

```text
0x1402FE6A0..0x1402FE921
```

The path has canonical MOD runtime-object provenance (`0x380` stride). It performs:

```text
0x1402FE6F4  runtime object +0x18 -> retained serialized object pointer
0x1402FE700  serialized object +0x08 -> mesh-table pointer
```

After `+0x08`, provenance changes into the 0x50-byte serialized mesh-record domain.

This is the strongest positive control for the retained pointer so far: the pointer is definitely live, but this canonical MOD path does **not** read object `+0x04..07`, `+0x14..17`, or `+0x20..2F` before entering mesh records.

### MOD render-command builder — bounded dead-after-load

`.pdata` bounds the canonical MOD render builder as:

```text
0x1402FE930..0x1402FF563
```

Within the exact function:

```text
0x1402FED8E  load runtime object +0x18
0x1402FED92  save pointer to local rbp+0x30
```

A full bounded-function disassembly contains no later read of `rbp+0x30` before return. Therefore this builder is now classified as:

```text
retained pointer load        EXE_CONFIRMED
saved local                  EXE_CONFIRMED
later saved-local deref      EXE_CONFIRMED: none
secondary-object consumption not established
```

This is stronger than the earlier label “unclassified escape edge”, but it remains function-scoped negative evidence rather than whole-program proof.

### EFM homologous control

The homologous EFM render builder is bounded by `.pdata` as:

```text
0x1402F8000..0x1402F8C4A
```

It shows the same pattern:

```text
0x1402F845E  load retained runtime +0x18 pointer
0x1402F8462  save to local rbp+0x30
later rbp+0x30 reads = 0
```

This is useful as an independent family control. It is **not** authority to assign EFM semantics to MOD fields.

## SCM false-positive rejection

A broad retained-pointer search found a highly deceptive cluster in:

```text
0x140302F10..0x14030345A
```

It also:

- stores a serialized 0x40-record pointer at runtime `+0x18`;
- later dereferences that pointer;
- walks serialized records with `0x40` stride.

However this is **not** the canonical MOD/EFM runtime-object path. Its runtime object stride is `0x3C0`, not `0x380`.

Independent SCM evidence identifies this exact function as the SCM object initializer. Direct code also contains the already documented SCM narrow compatibility behavior for runtime object `+0x07`, including the `EA -> C5` and `C4 -> 80` special rewrites.

Therefore:

```text
same serialized stride 0x40      insufficient
same retained pointer offset +18 insufficient
runtime owner/stride 0x3C0       SCM-specific
MOD/EFM runtime stride 0x380      different provenance domain
classification as MOD consumer   REJECTED
```

This is an important reverse-engineering guardrail: offset equality plus a familiar source-record stride can still produce a false positive when the runtime owner differs.

## Current status of object secondary regions

The new direct-EXE pass narrows the escape surface but still does not prove global non-use.

```text
object +0x04..+0x07
  corpus zero                         RESERVED_OBSERVED_ZERO
  initializer direct read             none
  audited MOD runtime-mesh path       none
  audited MOD render-builder path     retained pointer dead after load
  whole-program non-use               not yet promoted
  writer policy                       preserve

object +0x14..+0x17
  same bounded status

object +0x20..+0x2F
  same bounded status
```

No field is renamed padding/reserved from this result.

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
- the SCM `0x3C0` retained-pointer path is a MOD consumer because it also uses 0x40 source records — `REJECTED`;
- writer may normalize these regions to zero — `REJECTED`.

## Next gate

Continue a provenance-directed census from canonical MOD runtime-object derivations (`manager +0x100 + object_index*0x380`) and classify each actual `runtime +0x18` load through exact `.pdata` function bounds. Raw `+0x18` displacement searches are insufficient. The promotion gate for `object +0x04..07`, `+0x14..17`, or `+0x20..2F` remains either a positive semantic consumer or a sufficiently complete provenance-aware whole-program non-use proof. Until then, source preservation is mandatory.
