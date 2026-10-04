# DMC3 HD DDS texture limits — canonical EXE path

Date: 2026-10-04  
Branch: `reverse/dmc3-resource-render-limits-20261004`  
Canonical executable SHA-256: `e454272ed0fb0247fcbcf300e5d55d7a3e96d50b89b9ffaff81bb978dcbdd082`

## Result

The canonical DMC3 HD texture materialization path reaches the embedded DDS/D3D11 loader directly. For 2D DDS resources that loader contains an explicit pre-creation hard guard:

```text
width  <= 0x4000
height <= 0x4000
```

Therefore the recovered HD loader ceiling for a 2D DDS is **16,384 x 16,384**.

This is an EXE-confirmed loader-domain maximum, not proof that a texture at that size is practical, fits the game's live memory budget, or has been accepted in an original-game stress test.

## DMC3 source provenance

The DMC3 texture materialization path around `0x140330A00`:

1. obtains the DDS byte span from the texture source record;
2. reads the D3D11 device from `0x140C0B418`;
3. calls `0x1400499C0` at `0x140330A2D`;
4. on success, obtains the created resource;
5. creates a shader-resource view through the device vtable `+0x38`;
6. publishes the resulting resource/SRV into DMC3 runtime texture state.

This makes the dimension checks below source-proven for the DMC3 HD texture path rather than generic unused library code.

Status: EXE_CONFIRMED_DMC3_TEXTURE_PATH_TO_DDS_LOADER.

## DDS validation front-end

`0x1400499C0 -> 0x140049A10 -> 0x140049BA0` validates:

- minimum standard DDS size `0x80`;
- magic `DDS `;
- DDS header size `0x7C`;
- pixel-format structure size `0x20`;
- optional DX10 extension size when FourCC is `DX10`.

The DMC3 descriptor-backed retail resources currently evidenced by the project use standard DDS DXT1/DXT5 framing rather than requiring a DX10 header.

## Resource-dimension ceilings

Inside `0x140049BA0`, the resource dimension is classified and bounded before the lower texture-creation helper is called.

### Texture2D

At `0x140049D97..0x140049DB7`:

```text
array size <= 0x800  (2,048)
width      <= 0x4000 (16,384)
height     <= 0x4000 (16,384)
```

Status: EXE_CONFIRMED.

### Texture1D

At `0x140049DBC..0x140049DCF`:

```text
array size <= 0x800  (2,048)
width      <= 0x4000 (16,384)
```

Additional feature/capability checks follow.

Status: EXE_CONFIRMED_GENERIC_DDS_LOADER.

### Texture3D

At `0x140049D71..0x140049D92`:

```text
array size / resource-shape gate <= 1
width  <= 0x800 (2,048)
height <= 0x800 (2,048)
depth  <= 0x800 (2,048)
```

Status: EXE_CONFIRMED_GENERIC_DDS_LOADER.

## Mip-count gate

Before dimension-specific validation the loader rejects a recovered mip-count value above `0x0F` (15).

A full mip chain for a 16,384 dimension contains exactly 15 levels down to 1, so the 2D dimension ceiling and mip-count ceiling are internally consistent.

Status: EXE_CONFIRMED.

## Maximum encoded BC payload at the 2D loader ceiling

These are arithmetic consequences of the confirmed 16,384 x 16,384 ceiling and a complete mip chain, not claims that DMC3 can keep such a texture resident in gameplay.

For 16,384 x 16,384 with all 15 mips:

- DXT1 / BC1 payload: 178,956,984 bytes, plus 128-byte standard DDS header;
- DXT5 / BC3 payload: 357,913,968 bytes, plus 128-byte standard DDS header.

Classification: DERIVED_SIZE_FROM_EXE_CONFIRMED_DIMENSION_LIMIT.

These values describe the encoded file payload only. Runtime GPU allocation, staging overhead, residency, duplicate resources, SRVs and total scene memory are separate budgets.

## Retail vs product safety

The project's historical DMC3 DDS authoring safety object currently defaults to:

```text
min_dimension = 64
max_dimension = 1024
```

That value must remain classified as PRODUCT_SAFETY_LIMIT.

The project's retail corpus includes a real descriptor-backed DXT5 DDS at **1024 x 2048**, proving that a universal 1024-per-axis statement is false for DMC3 HD assets.

Current evidence hierarchy:

| value | meaning | status |
|---|---|---|
| 1024 | current Rengine default authoring safety max per axis | PRODUCT_SAFETY_LIMIT |
| 1024 x 2048 | real retail DDS observed | CORPUS_CONFIRMED |
| 16384 x 16384 | explicit canonical HD Texture2D loader guard | EXE_CONFIRMED |
| practical live-game maximum | not closed yet | OPEN |

The authoring safety should not be widened automatically to 16,384 merely because the loader can represent/validate it.

## PS2 boundary

The HD DDS loader is a D3D11 path. Its 16,384 x 16,384 2D ceiling must not be projected backward onto the original PS2 GS texture domain.

Rengine documentation must keep PS2_GS and DMC3_HD_RUNTIME limits as separate evidence domains.

## Remaining texture-budget gates

1. Census the complete retail texture population for maximum dimensions, mip counts, compressed payload size and textures per bundle.
2. Determine live D3D11 resource residency and release cadence for stage transitions.
3. Measure total simultaneous texture bytes, not just one-resource dimensions.
4. Determine the effective feature-level/resource-size rejection behavior on supported hardware.
5. Stress-test authored DDS dimensions above the current 1024 safety envelope.
6. Determine the practical failure boundary before the hard 16,384 loader guard.
7. Reconcile PTX payload/pool record capacity with texture count and mip-record fanout.

Until these gates close, **16,384 x 16,384 is the HD loader ceiling, not the recommended modding limit**.
