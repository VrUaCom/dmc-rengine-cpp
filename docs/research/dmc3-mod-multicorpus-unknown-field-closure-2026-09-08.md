# DMC3 HD MOD — multi-corpus unknown-field closure pass (2026-09-08)

**Branch:** `reverse/mod-completion-20260907`  
**Base synchronized to:** `main@e3ca8ac1f9ecc73a5e8a29d57adcb2fbdd392ff7` before this pass  
**Canonical EXE:** `dmc3.exe` SHA-256 `e454272ed0fb0247fcbcf300e5d55d7a3e96d50b89b9ffaff81bb978dcbdd082`

## Goal

The previous Wave-A audit was blocked on two kinds of evidence:

1. fresh canonical executable consumers;
2. MOD corpora outside `em000`.

This pass closes a substantial part of the **corpus blocker** by adding independently supplied `pl000` and `id100` model data and re-running the unresolved-field census. It deliberately does not convert repeated zeroes into global padding semantics.

---

## 1. Corpus provenance

### em000

Archive:

`em000-extract.zip`  
SHA-256 `306130125f09824811289366324f4208c3c1aba880c5a7efa3953a88d566d07b`

Existing recursive census:

```text
35 MOD
136 objects
147 meshes
14,804 vertices
226 transforms/nodes
```

### pl000 extraction set

Independent supplied MODs:

#### slot 1 / main model extraction

```text
size    216,544 bytes
SHA-256 e219e89285604cb6d800b0afdd3bec6684a6b00cd1862d464a669d2861ff3c89
header  objects=17 nodes=24 textures=3 default_joint=1
+0x14   217
meshes  18
verts   5,316
```

A duplicate upload with the same size has the exact same SHA-256 and is counted once.

#### slot 12 / cloth-associated model

```text
size    35,696 bytes
SHA-256 7a2be875b3702f59a607655f7a0a412801a6aea639dcb6e3b23d9b0a09c7e740
header  objects=6 nodes=33 textures=3 default_joint=0
+0x14   217
meshes  8
verts   828
```

Its adjacent slot-13 textual companion explicitly contains:

```text
;pl000_02.clt
```

CLT companion SHA-256:

`e722f24cddb34c45bbfcbdae722b8a1905aa62400a8267dcb281a591919b68aa`

The CLT contains Bone references up through node 32, matching the 33-node domain of the slot-12 MOD. This gives direct cross-resource provenance for the second pl000 MOD.

Repository/library audit evidence independently records `pl000.pac` as a 15-slot PAC with 12 populated slots and MOD resources. The supplied slot numbering is therefore retained as physical provenance rather than converted to non-empty ordinals.

### id100 HUD/model family

Bound model:

```text
id100_001_red_orb_counter.mod
size    2,304 bytes
SHA-256 9cbbaba99fdd008e257258dfe87c5dfed7fae2a13c4b1c2b08d0e318f0213b90
header  objects=7 nodes=2 textures=1 default_joint=0
+0x14   1,000,000
meshes  7
verts   28
```

The associated asset bundle documents this as the slot-1 Red Orb counter model from the `id\id100\id100.pac` HUD archive. The seven objects correspond to six digit meshes plus one Red Orb icon mesh in that evidence bundle.

### Duplicate excluded

A separately supplied 110,096-byte MOD had SHA-256:

`34f2c03795b007eac67abdb3a5808f675f1b1fd419ea6f39d4b77b8142c9e7ce`

It is byte-identical to `em000_001.mod` and is **not** counted a second time.

---

## 2. Expanded unique-corpus totals

```text
unique MODs          38
objects             166
meshes              180
vertices         20,976
transform records   285
```

Composition:

```text
em000  35
pl000   2
id100   1
```

This spans at least three materially different resource contexts:

- enemy/effect actor ecosystem (`em000`);
- player/cloth actor extraction (`pl000`);
- HUD/model resource (`id100`).

The goal is not to call this “all DMC3”. It is to stop treating a single enemy archive as the only corpus authority.

---

## 3. Transform `+0x1C`

Previous em000 result:

```text
0.0f in 226 / 226
```

New independent resources:

```text
pl000 slot 1   0.0f in 24 / 24
pl000 slot 12  0.0f in 33 / 33
id100          0.0f in  2 /  2
```

Combined:

```text
transform +0x1C == 0.0f in 285 / 285
```

### Promotion boundary

This is now **MULTI_CORPUS_OBSERVED_ZERO**, not merely an em000 invariant.

It is still **not** a global `reserved = must_be_zero` rule, because the fresh executable consumer census has not been completed and other actor/revision families may exist.

Safe canonical writer rule remains:

```text
preserve source +0x1C exactly
```

---

## 4. Mesh `+0x0C`, `+0x38`, serialized `+0x48`, `+0x4C`

Previous EXE evidence already established:

- MOD post-load `0x1402FE3B0` does not relocate/rewrite `+0x0C/+0x38/+0x4C`;
- MOD runtime mesh builder `0x1402FE6A0` has no confirmed transfer of those fields;
- `+0x48` is runtime-generated topology count and is zero in serialized em000 data.

Expanded corpus:

```text
mesh +0x0C == 0       180 / 180
mesh +0x38 == 0       180 / 180
mesh +0x48 == 0       180 / 180 serialized meshes
mesh +0x4C == 0       180 / 180
```

### Important cross-format correction for `+0x38`

EFM uses the homologous 0x50-byte model-family mesh shell differently: the EFM handler relocates `+0x38`, the real `em000_023.efm` has a `vertex_count * 4` stream there, and the EFM shader has `COLOR0` input.

Therefore shared-offset analogy proves the opposite of “padding”:

> `+0x38` is a format-sensitive slot in the related model-family mesh shell. It is live in EFM and unconsumed/zero in every currently observed MOD.

For MOD the safest status is therefore:

```text
PRESERVED_UNCONSUMED_MOD_SLOT
+ MULTI_CORPUS_OBSERVED_ZERO
```

not `padding` and not `EFM color pointer`.

`+0x0C` and `+0x4C` remain preserved/undecoded with the stronger multi-corpus zero observation.

---

## 5. `BLENDINDICES.x`

Existing EXE evidence:

- MOD skin path uses `BLENDINDICES.y/z/w` as three influence matrix starts;
- active raw matrix-row index maps to node/bone index with `/4`;
- packed three-weight ABI is independent of lane x.

Expanded corpus:

```text
em000          0 / 14,804 nonzero
pl000 slot 1   0 /  5,316 nonzero
pl000 slot 12  0 /    828 nonzero
id100          0 /     28 nonzero
---------------------------------
combined        0 / 20,976 nonzero
```

Status becomes:

```text
MULTI_CORPUS_OBSERVED_ZERO
PRESERVED_UNDECODED semantic role
```

This makes lane x increasingly likely to be a reserved/unused lane in the observed HD MOD skin ABI, but the global claim still requires the pending CPU census and all runtime-selected MOD shader variants (`MOD`, `MOD_SP`, `MOD_STX`).

---

## 6. Header `+0x14`: important semantic correction

The em000-only pass observed visually structured values such as:

```text
100407
202900
601715
700601
```

and used a lossless arithmetic projection:

```text
high   = raw / 100000
middle = (raw / 100) % 1000
low    = raw % 100
```

### New values

Both independently observed pl000 MODs carry:

```text
raw +0x14 = 217
```

The id100 HUD MOD carries:

```text
raw +0x14 = 1000000
```

These values are particularly useful because they break the temptation to treat the em000 visual formatting as a universal fixed semantic partition.

Any unsigned integer can of course be decomposed and recomposed by the arithmetic above. Therefore the arithmetic itself is **not evidence of semantic field boundaries**.

### Stronger conclusion

The universal claim:

```text
MOD +0x14 == family_class*100000 + model_set*100 + sub_index
```

is **not promoted and must be treated as rejected as a global semantic interpretation** unless a MOD-specific executable consumer proves those components.

What remains EXE-confirmed:

```text
serialized u32 +0x14
-> common model manager +0xE4
```

What the broader corpus adds:

1. em000 values form useful clusters;
2. pl000 main and cloth-associated MOD both share raw `217`;
3. therefore `+0x14` is not obviously a unique per-resource identity;
4. id100 uses a materially different magnitude (`1000000`);
5. the field remains an opaque runtime-carried u32 until downstream manager consumers are closed.

This is a real narrowing of the hypothesis space.

---

## 7. Object source-flag correlations

### `0x00100000`

The canonical executable already proves this is live render-path input consumed by `0x140302640`.

Broader corpus adds more real uses:

```text
pl000 slot 1: 8 / 17 objects carry 0x00100001
pl000 slot12: 1 /  6 objects carries 0x00100001
```

Together with em000, this shows the bit is not an enemy-only special case. The exact artistic/material meaning is still open.

### `0x00200000`

In em000 the bit appears in seven retail objects. All seven occur inside the `100407` raw-`+0x14` cluster, including several small cloth-associated model records.

The two current pl000 MODs and id100 sample do not use `0x00200000`.

This is a useful **corpus correlation**, not enough to name the bit. It strengthens the need to correlate the downstream consumer with model/material/deformation context rather than assigning a generic “render flag” label.

---

## 8. Other observed-zero shell regions

The newly added pl000/id100 resources were also checked for the previously observed zero shell regions:

```text
header +0x08..+0x0F
header +0x18..+0x1F
header +0x28..+0x3F
object +0x04..+0x07
object +0x14..+0x17
object +0x20..+0x2F
node-domain +0x10..+0x1F
```

All remained zero in these new samples.

This strengthens preservation evidence but does not authorize removing those bytes from the serialized ABI or forcing zero during a future writer.

---

## 9. Current closure table

| Field | New status after multi-corpus pass | What still blocks full semantic closure |
|---|---|---|
| header `+0x14` | raw runtime u32; em000 decimal semantic interpretation explicitly not global | manager `+0xE4` downstream EXE consumers |
| transform `+0x1C` | `MULTI_CORPUS_OBSERVED_ZERO` (285/285) | global EXE consumer census + wider revisions |
| mesh `+0x0C` | unconsumed in confirmed MOD load path; zero 180/180 | global raw-MOD xrefs + wider corpus |
| mesh `+0x38` | format-sensitive shared slot; MOD-unconsumed/zero 180/180; EFM uses COLOR0 here | prove no alternate MOD handler/variant |
| mesh `+0x4C` | unconsumed in confirmed MOD load path; zero 180/180 | global raw-MOD xrefs + wider corpus |
| BLENDINDICES.x | `MULTI_CORPUS_OBSERVED_ZERO` (20,976/20,976) | MOD/SP/STX shader + CPU lane-0 census |
| source `0x00100000` | live EXE render input; observed outside em000 | downstream packet semantic meaning |
| source `0x00200000` | real em000 bit with strong 100407-cluster correlation | direct EXE consumer/packet meaning |

---

## 10. What is now most valuable

The corpus side of Wave A is no longer em000-only. The next highest-value work is now direct executable evidence:

1. manager `+0xE4` global consumer census;
2. MOD/EFM transform initializer/local-node consumer census for `+0x1C`;
3. raw MOD mesh-record xrefs for `+0x0C/+0x38/+0x4C` outside the known post-load path;
4. `DMC3_MOD`, `DMC3_MOD_SP`, `DMC3_MOD_STX` lane-0 shader census;
5. CPU blend-stream read census;
6. downstream packet tracing for `0x00100000/0x00200000`.

Additional corpus acquisition should target boss/weapon/accessory and other player variants, but it is no longer legitimate to describe the unknown-zero evidence as enemy-only.

## Writer rule remains unchanged

No field above is writer-authorized merely because it stayed zero across 38 unique MODs.

Until the executable and variant gates close:

```text
read exact
preserve exact
report evidence status
never normalize unknown bytes
```
