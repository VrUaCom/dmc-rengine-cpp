# DMC3 HD MOD — header and node-shell whole-image closure (2026-09-11)

**Branch:** `reverse/mod-completion-20260907`  
**Canonical executable:** `dmc3.exe`  
**SHA-256:** `e454272ed0fb0247fcbcf300e5d55d7a3e96d50b89b9ffaff81bb978dcbdd082`  
**Size:** `6,356,432` bytes  
**Architecture:** PE32+ x86-64

## Purpose

This pass closes the last two direct-EXE gates in the MOD unknown-byte phase:

```text
header +0x08..+0x0F
header +0x18..+0x1F
header +0x28..+0x3F
node-domain +0x10..+0x1F
```

The result is behavioral closure for the canonical DMC3 HD executable. It is not permission to rename zero-filled bytes as padding/reserved, and it is not permission to normalize them in a writer.

## Canonical artifact verification

The executable used for this pass was independently recovered from the persistent project corpus and re-hashed before disassembly:

```text
size     6356432
sha256   e454272ed0fb0247fcbcf300e5d55d7a3e96d50b89b9ffaff81bb978dcbdd082
imageBase 0x140000000
```

The `.pdata` runtime-function table contains 12,235 entries. A separate leaf-code pass was performed outside those runtime-function ranges so the negative result is not bounded only to unwind-described functions.

## 1. Positive-control model provenance

The canonical model manager is not identified by raw displacement equality. The type-aware surface is anchored by the recovered model-family chain:

```text
manager +0x108 = serialized model source
manager +0xE0  = model-family flags
manager +0xE8  = object count
manager +0xEA  = node count
manager +0x100 = runtime object array
MOD runtime object stride = 0x380
```

and by family classifier `0x1402FD650`:

```text
MOD  -> 0x10000000
EFM  -> 0x20000000
SCM  -> 0x30000000
```

The whole executable contains 14 direct calls to that classifier. Offset matches in unrelated owners are not promoted merely because those owners also contain a field at `+0x108`.

## 2. Header shell — whole-image closure

The shared manager initializer `0x1402F9570` is the positive-control source reader. It consumes serialized header fields:

```text
+0x10 -> manager +0xE8
+0x11 -> manager +0xEA
+0x13 -> manager +0xFA
+0x14 -> manager +0xE4
```

and it keeps the source pointer at manager `+0x108`. Therefore initializer-local absence was not enough: every later source-pointer path had to be considered.

### Whole-image source-pointer census

Across the 12,235 `.pdata` functions, a conservative scan finds:

```text
116 functions containing a non-stack +0x108 -> register source-like load
234 such load sites
```

Propagating those source-like values through registers and local stack slots yields raw header-shell candidates in unrelated layouts. The raw candidate surface is intentionally over-inclusive:

```text
53 functions
138 target-range read/write operations
```

The raw accesses are dominated by `+0x30..+0x3F` transform/state structures and include separate `+0x18` resource-owner layouts. Type-aware classification rejects them from the MOD model-manager domain.

Important controls include:

- `0x14030B550/0x14030B5C0`: owner reached from `0x14008BE50`; this is the already-rejected separate layout using its own `+0x78/+0x80/+0x88` state and byte `+0x18` count, not the model manager;
- `0x140291D60` and `0x14029581C`: their owner `+0x108` sources use `+0x18/+0x20` as another pointer/table domain. These paths cannot be canonical MOD headers because the bounded MOD corpus has the complete header `+0x18..+0x1F` region zero, while these functions dereference it as live pointer state;
- `0x14030E.../0x14031...` raw matches belong to the separate CMotion/runtime-transform owner family. Existing CMotion evidence independently places live state around owner `+0x108/+0x110`; equal displacement is not model-source provenance.

After provenance classification, canonical model-manager source paths have:

```text
header +0x08..+0x0F reads/writes = 0
header +0x18..+0x1F reads/writes = 0
header +0x28..+0x3F reads/writes = 0
```

### Leaf-code control

Code outside `.pdata` contains only two source-like non-stack `+0x108` loads that reach this general offset neighborhood:

```text
0x140165B23
0x140165B46
```

Both are nested-owner paths through `owner +0x448`, and both write transform-like floats at source `+0x34/+0x38`. They do not carry the model-manager fingerprint or family-classifier provenance and are rejected as model-header consumers.

### Header promotion

For all three secondary header regions:

```text
bounded corpus             RESERVED_OBSERVED_ZERO
canonical runtime effect   EXE_CONFIRMED dormant/no effect
serialized semantic        PRESERVED_UNDECODED
writer policy              preserve exact source bytes
```

The word `reserved` remains a corpus observation only. No padding claim is made.

## 3. Node-domain shell — canonical dead read at +0x10

The common binder `0x1402F1DB0` obtains the node-domain block from serialized header `+0x20` and resolves exactly four relative dwords:

```text
node +0x00 -> manager +0x08
node +0x04 -> manager +0x10
node +0x08 -> manager +0x18
node +0x0C -> manager +0x20
```

It does not read `node +0x10..+0x1F`.

A fresh whole-image source/node propagation finds three raw node-shell candidate functions:

```text
0x140291D60  separate resource owner; REJECTED
0x14029581C  separate resource owner; REJECTED
0x1402FD9C0  canonical model layout planner; accepted
```

### Canonical `+0x10` read

`0x1402FD9C0`, called from the family-aware planner `0x1402FD8D0`, performs:

```text
0x1402FD9F1  manager +0x108 -> source
0x1402FDA00  source +0x20   -> node-domain
0x1402FDA0C  movzx eax, byte ptr [node +0x10]
0x1402FDA10  mov   byte ptr [rbp +0x18], al
```

The local byte at `[rbp+0x18]` has **zero later reads** before return. The same function has live positive controls: it repeatedly consumes serialized header `+0x11` for actual node-count-dependent size calculations.

Therefore `node +0x10` is not “never read”. The exact canonical statement is:

> `node-domain +0x10` is read once by the canonical layout planner, copied into a dead local, and has no observed behavioral effect.

The remaining bytes have no provenance-confirmed model-domain consumer:

```text
node +0x11..+0x1F reads/writes = 0
raw node-shell pointer escapes = 0
```

The binder stores only the four resolved arrays/pointers; it does not retain the raw shell pointer in manager state.

### Node-shell promotion

```text
node +0x10
  bounded corpus             RESERVED_OBSERVED_ZERO
  canonical read             EXE_CONFIRMED at 0x1402FDA0C
  later consumers            0
  canonical runtime effect   EXE_CONFIRMED dormant/no effect
  serialized semantic        PRESERVED_UNDECODED

node +0x11..+0x1F
  bounded corpus             RESERVED_OBSERVED_ZERO
  canonical consumers        0
  canonical runtime effect   EXE_CONFIRMED dormant/no effect
  serialized semantic        PRESERVED_UNDECODED
```

Writer policy for the complete `+0x10..+0x1F` shell remains exact-byte preservation.

## 4. Final unknown-byte phase result

The direct-EXE unknown-byte phase is now closed for the current canonical MOD contract. Every previously open byte/bit/region is in one of two states:

1. a recovered active semantic/technical role; or
2. a provenance-closed canonical runtime behavior with the serialized bytes still `PRESERVED_UNDECODED` and source-preserved.

No unresolved direct-EXE field-consumer gate remains.

This does **not** mean every unknown byte has a human semantic name. For dormant fields, inventing a name would be weaker than preserving the evidence boundary.

## 5. Next phase boundary

Only after this unknown-byte closure is recorded should MOD writer promotion begin:

1. preserve every undecoded source byte exactly;
2. prove byte-identical no-edit parse -> write round-trip;
3. only then authorize edited writer output;
4. finally require original `dmc3.exe` acceptance for edited assets.

Writer work is a separate gate and must not retroactively change the evidence status of dormant serialized fields.
