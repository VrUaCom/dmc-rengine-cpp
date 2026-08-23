# GDSpaces L1 Pass 90 — dual PTX entry materializer — 2026-08-23

Scope: **Layer 1 — Resource Materialization only**  
Status: **canonical EXE-confirmed read-side materializer; writer/provenance gates remain open**

## Executive correction

Pass 89 correctly narrowed the storage/runtime boundary but proposed the wrong mechanism. The DDS-bearing `0x70` entry is **not converted into a TM2 header** before the recovered PTX parser consumes it.

Canonical `dmc3.exe` proves that `0x1403365B0` is a per-entry dispatcher with two runtime-supported branches:

```text
PTX-style 0x800-sector envelope
  -> entry @ +0x800 / blockCount << 11
  -> 0x1403365B0
       if entry[0..3] == "TM2\0"
         -> TIM2/TM2 branch
       else
         -> serialized gfxTexture branch
            -> in-place placement/fixups
            -> embedded DDS-from-memory load
```

Therefore Pass 89 hypotheses H89.1–H89.4 are superseded as a **storage→TM2 adapter model**.

## Canonical target recovered

The original canonical executable is directly available again:

- file: `dmc3.exe`
- size: `6,356,432` bytes
- SHA-256: `e454272ed0fb0247fcbcf300e5d55d7a3e96d50b89b9ffaff81bb978dcbdd082`

Two independently modified derivatives remain useful controls:

- `dmc3_phase17_reng_probe.exe` — SHA-256 `9a3513db0f7cfeabed38f62836a5a6d55e42741b0965bfa5947d3c7b33532735`
- `dmc3_phase18_red_orb_x2_hook.exe` — SHA-256 `88febb349b4deff0b907de76f98359307f7484f7cb82b0018aa236a60591c5b0`

All Pass-90 instruction windows below are byte-identical across the canonical executable and both derivatives.

| Region | VA range | Length | SHA-256 |
|---|---:|---:|---|
| CPtxManager load A | `0x140314E00..0x140314F9F` | `0x1A0` | `9377e600890d02011fcb4504e562294179342574e4f2e5a67a8321ecd3e638cc` |
| CPtxManager load B | `0x140314FA0..0x14031514B` | `0x1AC` | `bd4141d9bf48ac8d2de80861c91be638bcaf10c0669229b98f2fade004747973` |
| entry dispatcher | `0x1403365B0..0x1403366D1` | `0x122` | `2eba03afdd023663ac43b6e766601bfda63290f152864188068ff0d539287ee2` |
| runtime build | `0x1403366E0..0x1403368EB` | `0x20C` | `159481901162dab0b0b999da96224355bfccad757eaa1885c3e83efa158c6b54` |
| PTX parser B | `0x140336A70..0x140336BAF` | `0x140` | `485e92b22e2d6ffa486efe9e125b7f6d52f3ca677636ff419b1caf6552359dba` |
| PTX parser A | `0x140336BB0..0x140336CE0` | `0x131` | `cfce51a63e7538c4e0ce6f8b3e6b61f0121407497daf4a9f23d1c6e32bd3198f` |
| serialized gfx placement/fixup | `0x140046510..0x1400465D6` | `0xC7` | `d8352921bd0f732e5fefb717797d3636e77b593ceff1a9f08a340bbf8541bfb6` |
| gfx lazy DDS load | `0x140046AF0..0x140046C37` | `0x148` | `24282a575eb688c5c1e40a515c74d607b66612be41b037446420e0fd9b25aaa3` |
| DDS wrapper | `0x1400499C0..0x140049A0F` | `0x50` | `3b7735348a705de7b377d66c669575ff3416114edbf5820dd1b8f8d8e559f85c` |
| DDS-from-memory | `0x140049A10..0x140049B9F` | `0x190` | `0a8439151e7fb85bfe74c78fd576fd5b36b1857e2bb3e3196abdc8082876b9db` |

This closes the canonical-byte parity limitation carried by Pass 87 for these exact regions.

## 1. Entry dispatcher — `0x1403365B0`

The decisive branch is literal:

```asm
1403365BA  cmp dword ptr [rcx], 0x00324D54 ; "TM2\0"
1403365C6  jne 0x140336679
```

### TM2 branch

The existing Pass-87/88 model remains valid. In particular:

```asm
1403365D3  mov ebp, dword ptr [rcx+0x08]
1403365DC  add rbp, rcx
```

so TM2 `+0x08` is an entry-relative interior offset.

### Non-TM2 branch

The branch does **not** fail and does **not** build a TM2 header:

```asm
140336679  call 0x140046510
14033667E  mov rcx, rax
140336681  mov rbx, rax
140336684  mov rdx, [rax]
140336687  call qword ptr [rdx+0x10]
```

The returned object is then used directly as the gfx texture object and its `+0x10/+0x12` dimensions are copied into the parsed-entry result.

## 2. Serialized gfxTexture placement — `0x140046510`

This function proves the non-TM2 representation is a **serialized object image materialized in place**.

When source `qword +0x00 == 0`:

```asm
1400465B1  cmp qword ptr [rdi], 0
1400465B7  mov [rdi], rax              ; cached gfxTexture vtable
1400465BA  lea rax, [rdi+0x20]
1400465BE  add [rdi+0x20], rax         ; pointer = address(field) + serialized delta
1400465C2  mov rcx, [rdi+0x20]
1400465C6  lea rax, [rcx+0x08]
1400465CA  add [rcx+0x08], rax         ; nested pointer = address(field) + delta
1400465CE  mov rax, rdi                ; same object returned
```

The cached vtable is `0x1404C5388`.

Consequences:

- no new TM2 buffer is created;
- the loaded serialized entry itself becomes a live `gfxTexture` object;
- on-disk/source pointer cells are relocatable deltas, not absolute pointers;
- runtime materialization mutates the loaded in-memory copy by installing a vtable and resolving those deltas.

## 3. Descriptor semantics and DDS load — `0x140046AF0`

The virtual method invoked by the non-TM2 dispatcher lazily loads the embedded DDS.

The decisive reads are:

```asm
140046B5D  mov rdx, [rbx+0x20]  ; fixed descriptor pointer
140046B7C  mov r8d, [rdx+0x04]  ; byte size
140046B80  mov rdx, [rdx+0x08]  ; fixed data pointer
140046B93  call 0x1400499C0
```

Therefore the descriptor map is now EXE-confirmed:

```text
gfxTexture +0x20 -> descriptor

descriptor +0x04 : u32 DDS byte size
descriptor +0x08 : DDS data pointer after relocation
```

`0x1400499C0` forwards to `0x140049A10`, whose canonical validations include:

- minimum `0x80` bytes;
- magic `DDS ` (`0x20534444`);
- `DDS_HEADER.size == 0x7C`;
- `DDS_PIXELFORMAT.size == 0x20`;
- optional DX10 header handling.

So this branch is not merely a structurally similar object: it is directly tied to the canonical DDS-from-memory loader.

## 4. Phase16 storage bytes match the EXE relocation ABI 50/50

The preserved Phase16 texture corpus contains five PAC samples with **91 DDS signatures and 0 TM2 signatures**. The aligned `0x800`-bundle cohort contains 50 entries:

- `st001.pac`: 17
- `id100.pac`: 21
- `pl000.pac`: 4
- `em000.pac`: 8

Every one of those 50 entries satisfies all of the following:

```text
qword entry+0x00 == 0
qword entry+0x20 == 0x40
qword entry+0x68 == 0x08
entry+0x70 == "DDS "
blockCount == ceil((0x70 + u32[entry+0x64]) / 0x800)
```

The two serialized pointer fixups resolve exactly:

```text
entry+0x20 field address + 0x40 -> entry+0x60 descriptor
entry+0x68 field address + 0x08 -> entry+0x70 DDS
```

For the first `st001` entry:

```text
blockCount       = 11
entry width/height = 128 / 128
DDS size @ +0x64 = 0x55F0
DDS pointer      = entry+0x70
```

This is an exact corpus↔instruction ABI match, not a layout analogy.

## 5. Outer PTX envelope remains common

Both parser variants (`0x140336A70` and `0x140336BB0`) still use the Pass-87 envelope contract:

```text
+0x00 u32 textureCount
+0x04 u32 blockCount[]
+0x800 first entry
entry advance = blockCount << 11
```

They call `0x1403365B0` independently for every entry. Therefore representation selection is **per entry**, and the envelope itself is not TM2-only.

## 6. CPtxManager ownership clarification

`0x140314E00` passes the resource pointer directly into `0x140336BB0` and supplies a temporary `0x208` parsed bundle on the stack. On success that parsed bundle is copied into the selected CPtxManager cache slot.

This separates two different ownership effects:

1. the serialized non-TM2 entry is placement-fixed **in its loaded resource memory**;
2. the parsed runtime bundle is a separate temporary/cache object.

There is no storage→TM2 rewrite between those stages.

## 7. Pass 89 hypothesis disposition

| Pass 89 hypothesis | Pass 90 disposition |
|---|---|
| H89.1 `storage +0x64 -> TM2 +0x3C` | **Rejected as adapter mapping.** `storage +0x64` is instead `descriptor+0x04`, directly consumed as DDS byte size by the serialized-gfx path. |
| H89.2 `TM2 +0x08 = 0x70` for this storage family | **Rejected / not applicable.** `+0x70` is reached through the nested serialized pointer fixup at `entry+0x68`, not through a TM2 header. |
| H89.3 `storage +0x08..+0x1F -> TM2 +0x50..+0x67` | **Rejected.** No TM2 copy/remap exists on the non-TM2 branch. |
| H89.4 fixed `0x70` header canonicalization into TM2 | **Rejected.** The actual operation is in-place `gfxTexture` placement + relative-pointer relocation. |

The valuable Pass-89 corpus correlations remain evidence about the serialized DDS-bearing representation; only the proposed conversion mechanism is superseded.

## 8. Code correction

The previous `Dmc3PtxEnvelopeParser` failed any entry without `TM2\0`. That contradicted canonical runtime behavior.

Pass 90 changes the read-only parser to model two explicit representations:

```cpp
enum class Dmc3PtxEntryRepresentation : std::uint8_t {
  tim2,
  serialized_gfx_texture_dds,
};
```

The serialized branch is fail-closed:

- requires the source vtable placeholder qword at `+0x00` to be zero;
- resolves `+0x20` by the exact EXE rule `address(field) + raw delta`;
- bounds-checks the descriptor;
- reads DDS byte size from descriptor `+0x04`;
- resolves descriptor `+0x08` by the same relocation rule;
- bounds-checks the DDS range;
- applies the same DDS magic/header/pixel-format checks already used by the TM2 bridge;
- unknown non-TM2 shapes remain rejected.

No writer API is added.

Local validation performed on the exact changed parser/tests:

```text
g++ -std=c++20 -Wall -Wextra -Wpedantic ...
PASS90_LOCAL_TESTS_OK

g++ -std=c++20 -fsanitize=address,undefined ...
PASS90_SANITIZERS_OK
```

Tests cover existing TM2 behavior, mixed per-entry representations, serialized descriptor relocation, nested DDS relocation, and malformed-pointer fail-closed cases.

## 9. Provenance correction, not provenance closure

Pass 90 materially changes the interpretation of Pass 86:

- the aligned DDS-bearing representation now has a **direct canonical game-runtime consumer and materializer**;
- it can no longer be described as merely an editor/extraction-facing shape with no proven game-side path.

But Pass 86's source-chain warning remains valid:

- current preserved evidence still does not prove that the sampled PAC bytes were acquired byte-for-byte directly from retail `dmc3-0.nbz`;
- the exact producer of the preserved Phase15/Phase16 samples is still not receipted;
- opaque multi-DDS variants remain outside this 50-entry ABI cohort.

So the correct status is:

> **canonical-runtime-supported serialized gfxTexture/DDS representation; direct-retail provenance still open**

## Hard freeze

Pass 90 does **not** authorize:

- calling the DDS-bearing sample corpus direct-retail without a source-chain receipt;
- a generalized serialized-gfx writer;
- a TM2 writer or DDS→TM2 converter;
- normalizing unknown serialized object fields;
- treating the opaque multi-DDS population as this same format without separate evidence;
- claiming original-game modified writeback acceptance before a rebuilt physical resource is consumed by the game.

## Next Layer-1 boundary

The read-side materializer for the aligned DDS-bearing entry family is now substantially closed. The next evidence work should move to:

1. retail source-chain reacquisition/provenance for a texture-bearing PAC from `dmc3-0.nbz`;
2. inverse-serialization ownership: which relocated fields must be restored to relative deltas and which bytes remain verbatim;
3. exact mutation/writer authority for the serialized object header, not only DDS payload replacement;
4. separate reverse of opaque multi-DDS entries;
5. original-game acceptance test of a minimally modified, rebuilt physical resource chain.

Layer 1 remains **NOT COMPLETE**, but the previous storage→runtime texture mystery is no longer a generic unknown adapter problem.
