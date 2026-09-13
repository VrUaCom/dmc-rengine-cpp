# DMC3 HD SCM compatibility command → D3D11 renderer closure — 2026-09-13

**Branch:** `reverse/mod-completion-20260907`  
**Canonical executable:** `dmc3.exe`  
**SHA-256:** `e454272ed0fb0247fcbcf300e5d55d7a3e96d50b89b9ffaff81bb978dcbdd082`  
**Primary machine authority:** `data/reverse/dmc3-scm-hd-command-compatibility-pipeline-20260913.json`

## Result

This pass closes the previously missing producer between serialized SCM object state and the HD compatibility shader selector, and ties the real SCM material packet to the same compatibility interpreter that reaches the D3D11 backend.

The strongest safe statement is now:

> **EXE_CONFIRMED:** serialized SCM object/material state is provenance-traced through the HD DMA/VIF/GIF compatibility layer to real `ID3D11DeviceContext::VSSetShader` and `PSSetShader` calls.

This is stronger than the previous “legacy renderer state” conclusion. It does **not** mean every pixel-shader numeric variant or every D3D11 SRV/sampler slot is source-bound yet.

## 1. Source flag `0x00080000` → compatibility selector

The missing `runtimeObject+0x05` producer is closed.

```text
serialized SCM object+0x10 flag 0x00080000
    ↓
0x1403033E3..0x1403033FC
    ↓
runtimeObject+0x00 bit 0x20
    ↓
0x14030DB50
    ├─ bit set   -> runtimeObject+0x05 = 2
    └─ bit clear -> runtimeObject+0x05 = 3
    ↓
0x14030DBA0
    ↓
0x5C000002 / 0x5C000003
    ↓
0x1400331E6 compatibility object selector
    ↓
0x140044310
```

The bit is not merely an executable-only branch. The expanded retail stage-PAC census observes source flag `0x00080000` on **46 of 345** SCM objects.

Status:

- source bit occurrence: `CORPUS_CONFIRMED`;
- source-bit → runtime bit → selector producer: `EXE_CONFIRMED`.

## 2. Exact compatibility selector → VS base table

Jump table authority: `0x1400446C4`.

| Selector | VS base key |
|---:|---:|
| 2 | 13 |
| 3 | 13 |
| 4 | 5 |
| 5 | 8 |
| 6 | no shader-bind path |
| 7 | 10 |
| 8 | 11 |
| 9 | 9 |
| 10 | 7 |
| 11 | 12 |
| 12 | 6 |

The important SCM-specific consequence is that selectors **2 and 3 both resolve to VS base key 13**. Therefore source flag `0x00080000` changes compatibility selector state but does not switch the base vertex-shader family on the recovered SCM converter path.

The clean C++ authority is `scm_runtime_flags.hpp`:

```text
scm_compatibility_object_selector(source_flags)
scm_compatibility_vs_base_key(selector)
```

No artistic shader names are inferred from these numeric keys.

## 3. Real SCM material GIF packet

The SCM material producer at `0x1402F99B0 / 0x1402F9AC0` builds the following five-qword compatibility payload:

```text
GIFtag = 0x4000000000008001
REGS   = 0x000000000020EEEE
NLOOP  = 1
EOP    = 1
FLG    = PACKED
NREG   = 4
```

The four authoritative low register descriptors are all `A+D`, with addresses:

```text
0x06 -> TEX0_1
0x14 -> TEX1_1
0x08 -> CLAMP_1
0x34 -> MIPTBP1_1
```

This establishes the material-state bridge:

```text
SCM mesh texture_index +0x02
    -> runtime texture table, stride 0x40
    -> descriptor +0x20
    -> TEX0_1

SCM object flag 0x00004000
    -> TEX1 = 0x00 nearest
else
    -> TEX1 = 0x60 linear
    -> TEX1_1

SCM mesh +0x04..+0x0B
    -> GS REGION_REPEAT packing
    -> CLAMP_1

runtime texture descriptor +0x28
    -> MIPTBP1 register image
    -> MIPTBP1_1
```

The packet is submitted by `0x14030A5F8..0x14030A632` through a REF-like QWC=5 chain entry and `DIRECT(5)` into the recovered compatibility interpreter.

## 4. Negative proof: A+D `0x7D` is not SCM material authority

A previously interesting backend path handles A+D address `0x7D` and performs real shader/state setup. That fact alone is insufficient to make it SCM-specific.

The recovered real SCM material packet writes only:

```text
0x06 / 0x14 / 0x08 / 0x34
```

and the SCM geometry packet uses PACKED `ST / RGBAQ / XYZF2` descriptors.

Therefore the claim “handler `0x7D` is the SCM shader/material path” is terminally:

`REJECTED_AS_SCM_SPECIFIC_SHADER_EVIDENCE`.

## 5. SCM command stream → D3D11 shader binding

The provenance-clean shader path is:

```text
SCM constructor / converter setup
  ↓
0x140309DF0 emits 0x57000000
  ↓
0x140033174: active converter = 0x57
  ↓
0x14030DBA0 emits 0x5C000002 or 0x5C000003
  ↓
0x1400331E6 stores compatibility selector
  ↓
0x14030A705 emits 0x5B00001C + 0x1C-dword mesh descriptor
  ↓
0x140032D7A
  ↓
0x140044310
  ↓
0x140045FE0
  ├─ 0x14004C340 -> ID3D11DeviceContext::VSSetShader
  └─ 0x14004C140 -> ID3D11DeviceContext::PSSetShader
```

This is the point at which the SCM reverse crosses from legacy compatibility state into the actual HD D3D11 backend.

Status: `EXE_CONFIRMED_TO_D3D11_SHADER_BIND`.

## 6. Pixel-shader boundary

The vertex side is now substantially tighter than the pixel side.

For VS:

- compatibility selector producer is closed;
- selector table `2..12` is closed;
- SCM source initialization produces `2/3`;
- both map to base key `13`;
- a downstream conditional `+0x40` key variant exists and remains numeric/technical rather than artistically named.

For PS:

- the call to `PSSetShader` is proven;
- the PS key is runtime-derived from mesh/auxiliary state;
- conditional `+0x20/+0x40` variants exist;
- the complete formula for every combination is not yet terminally classified.

Therefore the correct evidence boundary is:

> PS **binding** is closed; complete PS **selection formula** remains open.

## 7. Texture SRV / sampler boundary

The D3D11 backend contains confirmed texture/resource binding wrappers around:

```text
0x14003F7A0 / 0x14003F580
```

with device-context vtable offsets `0x40 / 0x50` in the recovered family.

What is **not** yet proven to the same standard is the exact source-level bridge:

```text
SCM TEX0_1 / TEX1_1 / CLAMP_1 / MIPTBP1_1
    -> exact compatibility state object
    -> exact SRV slot + exact sampler slot
    -> 0x14003F7A0 / 0x14003F580
```

The deep reader intentionally labels this as `SRV_SAMPLER_SOURCE_BINDING_OPEN` instead of silently upgrading generic backend knowledge into SCM-specific provenance.

## 8. Product-facing deep reader

`scm_runtime_provenance.cpp` now exposes the renderer chain on the serialized fields that own it:

- object flags: source compatibility selector and VS base-key projection;
- object flags: TEX1 nearest/linear material projection;
- mesh texture index: TEX0 + MIPTBP1 material-packet ownership and external companion chain;
- mesh clamp words: CLAMP_1 material-packet ownership;
- shader path: `0x140044310 -> 0x140045FE0 -> VSSetShader / PSSetShader`;
- explicit SRV/sampler source-binding boundary.

This makes the evidence visible to `inspect-scm` / Native Reader rather than leaving it only in reverse notes.

## 9. Regression contract

Regression now locks:

- source flag clear -> selector `3`;
- source flag `0x00080000` -> selector `2`;
- the complete selector `2..12` VS-base table;
- selector `6` as no-bind;
- exact SCM GIF tag and REGS qwords;
- exact A+D register order `TEX0_1, TEX1_1, CLAMP_1, MIPTBP1_1`;
- negative absence of address `0x7D` from the SCM material packet;
- deep-reader annotations reaching `VSSetShader` and `PSSetShader`;
- explicit open tag for the SCM→SRV/sampler source bridge.

## 10. Remaining renderer frontier

After this pass, the old `runtimeObject+0x05` producer frontier is **closed**.

The renderer-specific unresolved work is reduced to:

1. derive the complete numeric PS table-key formula for every SCM mesh/aux/runtime-state combination in `0x140044310 / 0x140045FE0`;
2. bind the recovered SCM material state to exact D3D11 SRV and sampler slots behind `0x14003F7A0 / 0x14003F580` with source-level provenance;
3. keep source flag `0x00200000` preservation-only until a provenance-clean consumer exists.

Items 1 and 2 are backend-selection closure. They do not invalidate the now-proven serialized SCM layout, material GIF ABI, compatibility selector producer or D3D11 shader-bind chain.

## Assessment

Architecturally this is a major closure point. SCM is no longer merely a file format whose geometry and legacy GS states are understood. We can now follow a live retail source flag and live material state through Capcom's compatibility command representation into the HD Collection's real D3D11 shader binding layer.

The key discipline is to stop one step short of pretending generic backend wrappers are already source-bound to SCM. Keeping that last SRV/sampler bridge explicit gives the project a stronger specification: everything marked closed is reproducible evidence, while the two remaining renderer formulas are narrow, testable targets rather than a vague “shader system unknown” bucket.
