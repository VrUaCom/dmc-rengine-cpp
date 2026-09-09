# DMC3 HD MOD — direct canonical EXE unknown-byte closure (2026-09-08)

**Branch:** `reverse/mod-completion-20260907`  
**Canonical executable:** `dmc3.exe`  
**SHA-256:** `e454272ed0fb0247fcbcf300e5d55d7a3e96d50b89b9ffaff81bb978dcbdd082`

## Scope

This pass deliberately ignores third-party Blender tooling as authority and returns to the original DMC3 HD executable. The target is the remaining MOD fields that were still only corpus-constrained:

- transform `+0x1C`;
- mesh `+0x0C`, `+0x38`, `+0x4C`;
- `BLENDINDICES.x`;
- object source flag `0x00100000`.

The executable was recovered from the persistent project library and hash-verified before disassembly.

---

## 1. Transform `+0x1C`: ignored by the canonical MOD/EFM transform initializer

### Canonical initializer

`0x1402FA080` consumes serialized 0x20-byte transform records.

For every node it loads:

```text
serialized +0x00..+0x0F -> scratch xmm block A
serialized +0x10..+0x1F -> scratch xmm block B
```

It then calls:

```text
0x140330450(rotation helper, scratch block B)
0x140031200(translation helper, scratch block A)
```

The critical direct-machine-code result is inside `0x140330450`:

```text
14033045A  movss xmm2, [r8 + 0x00]
...
14033046A  movss xmm2, [rbx + 0x04]
...
14033047A  movss xmm2, [rbx + 0x08]
```

There is **no read of `[r8/rbx + 0x0C]`** in this helper.

Because scratch block B corresponds to serialized `+0x10..+0x1F`, that missing fourth read is exactly serialized transform `+0x1C`.

`0x1402FA080` also does not copy this fourth float into the runtime node outside the helper. It bulk-loads the 16 bytes only into temporary stack storage before calling the helper.

### Promotion

```text
serialized location          = STRUCTURAL_CONFIRMED
multi-corpus value           = 0.0 in 285/285 current MOD transforms
initializer behavior         = EXE_CONFIRMED_IGNORED
runtime-node transfer        = NOT OBSERVED in 0x1402FA080
high-level semantic name     = none
writer rule                  = preserve raw source bytes
```

This is now stronger than `RESERVED_OBSERVED_ZERO`.

Safe description:

> MOD transform `+0x1C` is an on-disk fourth float in the rotation 16-byte block that the canonical MOD/EFM transform initializer does not consume when constructing the local runtime matrix.

Do **not** rename it padding yet: a different loader/revision/tool path could still use it.

---

## 2. Mesh `+0x0C/+0x38/+0x4C`: direct MOD load path does not consume them

### MOD post-load — `0x1402FE3B0`

The original executable walks each `0x50` mesh and explicitly relocates:

```text
+0x10 positions
+0x18 normals
+0x20 UV
+0x28 blend indices
+0x30 packed control/topology
+0x40 generated topology workspace (record-relative)
```

and produces:

```text
+0x48 generated topology count
```

The function contains no corresponding relocation/read for serialized:

```text
+0x0C
+0x38
+0x4C
```

### MOD runtime mesh construction — `0x1402FE6A0`

The second-stage mesh builder transfers the active stream pointers and generated workspace/count into the runtime render object. It again transfers:

```text
raw +0x10
raw +0x18
raw +0x20
raw +0x28
raw +0x30
raw +0x40
raw +0x48
```

and does not transfer `+0x0C`, `+0x38`, or `+0x4C`.

### Important cross-format control: EFM `+0x38`

This negative MOD result must **not** become a shared-family padding claim.

The canonical EFM post-loader `0x1402F7A90` explicitly reads and relocates EFM mesh `+0x38`:

```text
1402F7BBD  mov rax, [rax + 0x38]
...
1402F7BCC  mov [rcx + 0x38], rax
```

and the EFM runtime builder later consumes that same stream. Existing EXE/HLSL evidence binds it to EFM `COLOR0`.

Therefore:

```text
same 0x50 model-family slot +0x38
    MOD -> inactive/unconsumed in canonical MOD load path
    EFM -> active per-vertex stream
```

This strongly argues that `+0x38` is a **format-family variant slot**, not universal padding.

### Promotion

For MOD only:

```text
+0x0C = EXE_CONFIRMED_UNCONSUMED_IN_CANONICAL_LOAD_PATH
+0x38 = EXE_CONFIRMED_UNCONSUMED_IN_CANONICAL_LOAD_PATH
+0x4C = EXE_CONFIRMED_UNCONSUMED_IN_CANONICAL_LOAD_PATH
```

Current multi-corpus values remain zero in `180/180` meshes, but writer policy remains byte-preserve.

---

## 3. `BLENDINDICES.x`: original DMC3 shader source does not reference lane X

The canonical executable embeds original HLSL source paths for all three known MOD vertex-shader families:

```text
DMC3_MOD.hlsl
DMC3_MOD_SP.hlsl
DMC3_MOD_STX.hlsl
```

Each source path occurs twice in the executable image.

A full executable ASCII-source census finds:

```text
"matIndex.x" occurrences =   0
"matIndex.y" occurrences = 112
"matIndex.z" occurrences =  16
"matIndex.w" occurrences =  16
```

Representative embedded source uses:

```text
uint matIndxY = vi.matIndex.y;
...
uint matIndxZ = vi.matIndex.z;
uint matIndxW = vi.matIndex.w;
```

and skin matrices are assembled from Y/Z/W.

The executable CPU path independently contains the already recovered active-lane access:

```text
0x1402F3D0A  movzx eax, byte ptr [rcx + 4*vertex + 1]
0x1402F3D0F  sar   eax, 2
```

which is lane Y (`+1`), not X.

### Promotion

```text
lane X multi-corpus value      = zero in 20,976 / 20,976 current vertices
embedded MOD HLSL references   = zero direct `matIndex.x` references
active shader lanes            = y/z/w
known CPU skin reference       = lane y
```

Safe semantic statement:

> `BLENDINDICES.x` is not part of the active three-influence skin-index contract in the original embedded MOD shader sources.

Still **do not** normalize it away in a writer. This pass closes the shader role, not every hypothetical CPU/debug/revision consumer in the executable.

---

## 4. Object flag `0x00100000`: exact packet mutation recovered

Canonical helper `0x140302640` directly tests source flag `0x00100000`.

Two concrete low-level consequences are visible in machine code.

### Packet word at `+0x08`

When the relevant control path reaches this branch:

```text
flag clear -> packet +0x08 = 0x000000000005000D
flag set   -> packet +0x08 = 0x000000000005010D
```

So the flag sets packet bit/value delta `0x100` in this descriptor word.

### Packet word at `+0x00`

A second branch tests `0x00100000`; when the flag is clear in the relevant path it ORs:

```text
0x0000000100000000
```

into the packet word, while the set path omits that OR.

Therefore the bit is conclusively a live legacy render/material packet selector. The artistic/user-facing meaning is still not named because the downstream GPU/GS state consumer must be bound before calling it e.g. alpha mode, depth mode, blending mode, etc.

### Promotion

```text
bit live                     = EXE_CONFIRMED
packet +0x08 delta           = EXE_CONFIRMED
packet +0x00 bit32 behavior  = EXE_CONFIRMED
artistic semantic            = OPEN
```

---

## 5. What is genuinely still open after this pass

The remaining difficult MOD closure is now narrower:

1. header `+0x14` -> manager `+0xE4` downstream consumer/semantic;
2. whether mesh `+0x0C/+0x4C` have any noncanonical/revision consumer outside the confirmed MOD load path;
3. whether `BLENDINDICES.x` has any CPU-only use outside the skin path (shader role is now negatively closed);
4. exact artistic meaning of source flag `0x00100000` packet mutations;
5. source flag `0x00200000` downstream behavior;
6. writer authority / byte-exact no-edit replay / edited original-game acceptance.

## Evidence rule

Third-party importers may be used only as corroboration. Every promotion in this note comes from the hash-verified original executable or the already hash-bound retail MOD corpus.
