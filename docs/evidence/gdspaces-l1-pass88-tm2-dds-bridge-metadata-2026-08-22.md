# GDSpaces L1 Pass 88 — TM2 → embedded DDS bridge and texture metadata — 2026-08-22

## Scope

Layer 1 only. Pass 88 continues the read-side structural recovery from Pass 87 and closes the next runtime boundary inside one `TM2\0` entry.

The key correction is architectural:

```text
PTX envelope
  -> TM2 outer entry
     -> relative embedded DDS buffer
        -> original DDS memory parser
           -> D3D11 texture
              -> gfxTexture runtime object
```

The original DMC3 path therefore does **not** treat PTX/TIM2 and DDS as two mutually exclusive representations at this boundary. A TM2 entry can own metadata plus a relative DDS buffer and feed that buffer into the same DDS parser used by the engine's memory-loading path.

This does not close retail-source provenance for the historical `st001.pac` corpus and does not authorize a writer.

## Evidence authority

### Canonical Phase 16 package recovered from Library

Preserved package:

- `dmc3_exe_texture_resource_phase16_complete.zip`
- SHA-256: `f8343a0b28c201d5c54150d3abeba4c7d04652059fbc606775fed85e1d5e9a83`
- manifest target SHA-256: `e454272ed0fb0247fcbcf300e5d55d7a3e96d50b89b9ffaff81bb978dcbdd082`
- manifest file count: `41`
- historical C++ syntax validation: passed

The preserved canonical function map already contains the same functions now traced end-to-end:

- `0x1403365B0` — PTX/TM2 entry parser;
- `0x140046C40` — gfxTexture factory/cache;
- `0x140047000` — texture object initializer;
- `0x140049A10` — DDS from memory;
- `0x140049BA0` — D3D11 texture creation core.

The Phase16 disassembly from the canonical target directly preserves the `TM2\0` check, `entry+0x08` pointer construction, call to `0x140046E20`, the 24-byte metadata copy in `0x140047000`, and the DDS validator checks in `0x140049A10`.

### Independent derivative control

Pass 87 additionally compared two independently modified executables:

- `dmc3_phase17_reng_probe.exe` SHA-256 `9a3513db0f7cfeabed38f62836a5a6d55e42741b0965bfa5947d3c7b33532735`;
- `dmc3_phase18_red_orb_x2_hook.exe` SHA-256 `88febb349b4deff0b907de76f98359307f7484f7cb82b0018aa236a60591c5b0`.

Their full `.text` sections differ at an unrelated patch site, while the PTX/TM2 windows used by Pass 87 are byte-identical. This remains corroborating evidence for portions of the full gfxTexture method that the historical Phase16 runtime-range extraction truncated.

## Historical Phase16 correction

Phase16 correctly separated the supplied transformed DDS-bearing PAC corpus from unproven retail PTX writer authority. That safety boundary remains valid.

However, Phase16's high-level architecture graph described:

```text
runtime PTX/TIM2 parser path
```

and

```text
parallel independent DDS memory/file path
```

as if the two paths only converged later at GPU creation.

Pass 88 supersedes that specific topology. The TM2 parser itself feeds a relative payload through gfxTexture into the DDS memory parser.

A second historical label is corrected: Phase16 called `0x140046AF0` `gfxTexture_ready_query` because its recovered runtime range ended at `0x140046B25`. The complete body continues beyond that boundary and performs lazy resource materialization/upload, including the DDS-memory call and D3D11 resource/view creation. The canonical semantic label for Pass 88 is therefore **gfxTexture lazy materialize/upload**.

## TM2 source entry → gfxTexture chain

### `0x1403365B0` — parse current TM2 entry

Direct canonical instructions include:

```text
cmp dword ptr [rcx], 0x00324D54   ; "TM2\0"
mov ebp, dword ptr [rcx+0x08]
add rbp, rcx                       ; source entry + relative payload offset
...
call 0x140046C40                  ; cache/factory keyed by that pointer
...
lea rdx, [rbx+0x38]               ; source entry +0x38
mov r8, rbp                        ; relative payload pointer
call 0x140046E20
```

The `+0x08` field is therefore a source-entry-relative pointer field.

### `0x140046E20` — adapt TM2 source fields to gfxTexture initializer

Recovered instructions:

```text
mov eax, dword ptr [rdx+0x04]     ; TM2 entry +0x3C
mov r9, r8                         ; relative payload pointer
lea r8, [rdx+0x18]                ; TM2 entry +0x50
mov dword ptr [rsp+0x20], eax      ; fifth argument
xor edx, edx
call 0x140047000
```

Consequences:

- `TM2 +0x3C` is a 32-bit buffer-size value;
- `TM2 +0x50..+0x67` is a 24-byte runtime texture metadata block;
- the `TM2 +0x08` relative pointer is passed alongside that size.

### `0x140047000` — initialize gfxTexture

The function copies exactly 24 source bytes:

```text
TM2 +0x50..+0x67
    -> gfxTexture +0x08..+0x1F
```

Within that copied block:

```text
TM2 +0x58 -> gfxTexture +0x10 -> u16 width
TM2 +0x5A -> gfxTexture +0x12 -> u16 height
```

It immediately computes reciprocal width/height into `gfxTexture +0x48/+0x4C` and computes `width * height * 4` into `gfxTexture +0x38` when a CPU payload descriptor is installed.

When the fifth argument (`TM2 +0x3C`) is nonzero and the relative payload pointer is non-null, `0x140047000` allocates a 0x10-byte CPU payload descriptor and stores:

```text
payload +0x04 = TM2 +0x3C value
payload +0x08 = TM2 + *(u32*)(TM2+0x08)
```

That descriptor is attached at `gfxTexture +0x20`.

## Independent width/height proof

`0x140046F90` is another gfxTexture initialization path. It receives two integer dimensions and writes:

```text
argument 1 -> gfxTexture +0x10
argument 2 -> gfxTexture +0x12
```

then calculates the same reciprocal values used by `0x140047000`.

Caller `0x14002DEEF` first invokes the texture object's COM method at vtable `+0x50` to fill a D3D11 texture descriptor, then passes the first two 32-bit fields of that descriptor to `0x140046F90`.

For `ID3D11Texture2D::GetDesc`, those first two fields are Width and Height. Therefore Pass 88 promotes:

```text
TM2 +0x58 = u16 width
TM2 +0x5A = u16 height
```

from candidate dimensions to **EXE-confirmed width/height**.

No semantic names are assigned yet to the other bytes in `TM2 +0x50..+0x67`.

## gfxTexture lazy materialize/upload → DDS parser

The full `0x140046AF0` body reads the CPU payload descriptor installed above:

```text
mov rdx, [gfxTexture+0x20]
mov r8d, [rdx+0x04]               ; byte size
mov rdx, [rdx+0x08]               ; source pointer
call 0x1400499C0
```

`0x1400499C0` is a thin adapter into `0x140049A10` (`DDS from memory`). The canonical DDS parser enforces:

```text
buffer size >= 0x80
u32 [buffer+0x00] == 0x20534444   ; "DDS "
u32 [buffer+0x04] == 0x7C         ; DDS_HEADER size
u32 [buffer+0x4C] == 0x20         ; DDS_PIXELFORMAT size
```

Therefore the two TM2 source fields can now be named by consumer behavior:

```text
TM2 +0x08 = u32 ddsRelativeOffset
TM2 +0x3C = u32 ddsByteSize
```

for the nonzero CPU-payload path.

This is stronger than simple signature similarity: the pointer and size are transported through two runtime structures and consumed as the actual arguments of the engine's DDS memory validator.

## Source-entry structural model after Pass 88

Only evidence-backed fields are named:

```text
TM2 entry
+0x00  u32 magic = "TM2\0"
+0x04  field copied elsewhere; semantics still open
+0x08  u32 ddsRelativeOffset
...
+0x3C  u32 ddsByteSize
...
+0x50  opaque runtimeTextureMetadata[0x18]
  +0x58 u16 width
  +0x5A u16 height
...
+ddsRelativeOffset
       embedded DDS buffer when ddsByteSize != 0
```

The `+0x50` block is copied byte-for-byte into gfxTexture. Pass 88 does not invent labels for `+0x50/+0x54/+0x5C/+0x5E/+0x60`.

## Product parser change

`Dmc3PtxEnvelopeParser` remains read-only and now exposes a nested `Dmc3Tm2DdsBridge` for each entry:

- DDS relative offset;
- DDS byte size;
- absolute DDS offset in the supplied PTX resource;
- width;
- height;
- opaque 24-byte runtime texture metadata block;
- whether a nonzero embedded DDS payload is present.

For `ddsByteSize != 0`, the parser fail-closes on the same bounded structural conditions established by the EXE:

- DDS range must remain inside the containing TM2 allocation;
- minimum buffer size `0x80`;
- `DDS ` magic;
- DDS header size `0x7C`;
- DDS pixel-format size `0x20`.

For `ddsByteSize == 0`, no DDS signature is required. The original gfxTexture path does not install a CPU DDS payload descriptor in that case, so rejecting such an entry as malformed would exceed the recovered runtime contract.

No writer, converter, DDS replacement, swizzle operation, or size-changing texture writeback API is added.

## Impact on Pass 86 provenance boundary

Pass 86 remains correct that the preserved Phase15/v6 DDS-bearing corpus does not by itself prove retail acquisition provenance or original-game writeback equivalence.

Pass 88 corrects only the representation topology:

```text
WRONG/OBSOLETE:
original PTX/TIM2 path  ||  separate DDS path

CURRENT:
original PTX envelope
 -> TM2 metadata entry
 -> embedded/relative DDS payload
 -> shared DDS memory parser
 -> D3D11 texture
```

The historical transformed corpus can still differ from the retail resource image by missing/replaced TM2 prefixes, altered headers, reordered payloads, extraction transforms, or repacking. Proven DDS nesting does not prove that a particular stage-drop PAC is pristine retail.

## What Pass 88 proves

- canonical original-EXE function identity for the PTX/TM2 and DDS nodes through the recovered Phase16 package;
- TM2 `+0x08` is a relative DDS buffer offset on the nonzero CPU-payload path;
- TM2 `+0x3C` is the DDS byte size passed to the DDS memory parser;
- TM2 `+0x50..+0x67` is copied verbatim into gfxTexture runtime metadata;
- TM2 `+0x58/+0x5A` are width/height;
- the TM2 source path converges into the same DDS parser used by engine memory loading;
- the old `gfxTexture_ready_query` label for `0x140046AF0` was incomplete and is superseded by lazy materialize/upload semantics.

## Still open

- semantic names for the remaining bytes in `TM2 +0x50..+0x67`;
- exact meaning of TM2 source `+0x04` and other unnamed fields;
- DX10/non-DX10 DDS variant coverage inside retail PTX resources;
- direct raw retail PTX sample receipt from `dmc3-0.nbz`;
- proof that Phase15 raw `st001.pac` is or is not byte-identical to direct retail extraction;
- exact transformation that produced the historical DDS-bearing stage-drop representation;
- safe original-resource writer/rebuilder;
- original-game consumption test of a rebuilt texture.

Layer 1 remains **NOT COMPLETE**.
