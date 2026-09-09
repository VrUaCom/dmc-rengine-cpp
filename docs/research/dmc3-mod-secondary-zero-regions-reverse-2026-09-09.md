# DMC3 HD MOD — secondary zero-region census

**Branch:** `reverse/mod-completion-20260907`  
**Canonical executable:** `dmc3.exe`  
**SHA-256:** `e454272ed0fb0247fcbcf300e5d55d7a3e96d50b89b9ffaff81bb978dcbdd082`

## Rule

A zero-filled region is not promoted to padding. `RESERVED_OBSERVED_ZERO` below means only that the current bounded corpus is zero in the region. It does not authorize zero-normalization by a writer.

## Header shell

Canonical manager/header initializer `0x1402F9570` provides the bounded loader census. It positively reads:

```text
header +0x10
header +0x11
header +0x13
header +0x14
```

Within this initializer there is no raw read from:

```text
header +0x08..+0x0F
header +0x18..+0x1F
header +0x28..+0x3F
```

Those regions therefore have:

```text
corpus                  CORPUS_CONFIRMED
zero-shell label        RESERVED_OBSERVED_ZERO
loader-local no-read    EXE_CONFIRMED
writer policy           preserve
```

The `EXE_CONFIRMED` statement is intentionally scoped to `0x1402F9570`; it is not a whole-executable no-read proof.

## Object shell

Canonical object initializer rooted at `0x1403029E0` uses a 0x40-byte serialized object record. Positive source reads include:

```text
+0x00
+0x01
+0x10 source_flags
+0x18
+0x1C
+0x30..+0x3F bounding data
```

No source read is present in this initializer for:

```text
object +0x04..+0x07
object +0x14..+0x17
object +0x20..+0x2F
```

These regions remain `RESERVED_OBSERVED_ZERO` with source-byte preservation. Their absence from this initializer is bounded negative runtime evidence only.

## Node-domain shell

`node-domain +0x10..+0x1F` remains zero in the current corpus. Transform initializer `0x1402FA080` consumes already-resolved domain pointers/arrays supplied through manager fields and does not provide sufficient raw serialized provenance to declare the shell globally unused.

Accordingly this region remains:

```text
CORPUS_CONFIRMED
RESERVED_OBSERVED_ZERO
writer policy = preserve
```

No `EXE_CONFIRMED` global no-read claim is made for this region in this pass.

## Rejected promotions

- zero shell → padding — `REJECTED`;
- no read in one initializer → globally unused — `REJECTED`;
- safe writer zero-normalization — `REJECTED`.

## Next gate

For stronger closure, start from provenance-confirmed serialized base pointers and search every escape/copy of the header, object, and node-domain records before relocation. Only then can a region move beyond loader-local negative evidence.
