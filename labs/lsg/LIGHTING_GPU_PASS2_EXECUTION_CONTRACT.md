# Rengine LSG — Shared GPU Lighting Pass 2 Execution Contract

Branch scope: `rengine-living-surface-genome` only.

Prepared from baseline:
`ec890b09e5501cf81ff057e4adc3168a19b2baf4`

This document prepares corrective pass 2/5 only. It does not implement the pass.

## Pass goal

Move the existing CPU `LightingRuntimeState` into one shared Vulkan frame-lighting contract used by:
- procedural sky/environment,
- body/skin lighting,
- inner-eye lighting,
- cornea lighting,
- pupil runtime input.

Add the `Time` R&D UI row:
`Morning -> Noon -> Evening -> Night`.

The pass is complete only when changing Time changes actual scene lighting and the pupil uses the same effective luminance source.

Optical filter visual response remains pass 3. The UBO layout should reserve the required fields now so pass 3 does not redesign descriptors.

## Current audit findings

### No descriptor infrastructure exists

The renderer currently uses:
- 128-byte body push constants,
- 128-byte eye push constants,
- no `VkDescriptorSetLayout`,
- no descriptor pool,
- no descriptor set,
- no uniform buffer binding.

Therefore pass 2 should introduce a clean descriptor path instead of extending either push block.

### One frame in flight

The renderer owns one `VkFence in_flight` and waits for it before recording/submitting the next frame.

Therefore v0 can safely use one persistently mapped HOST_VISIBLE + HOST_COHERENT uniform buffer:
- update only after `vkWaitForFences`,
- no per-swapchain-image UBO required yet,
- no dynamic offsets,
- no explicit flush on HOST_COHERENT memory.

This remains valid until the renderer moves to multiple frames in flight.

### Fixed GPU lighting still exists

`human.frag` currently hardcodes:
- sun direction,
- sky zenith/horizon colours,
- sun disk intensity,
- skin sun radiance.

`eye.frag` independently hardcodes:
- inner-eye light direction,
- cornea sun direction,
- cornea sky colours.

Pass 2 must remove those duplicate physical-light sources.

## CPU runtime extension needed

Pass 1 created geometric/intensity state but not enough colour state for visibly different Morning/Noon/Evening/Night.

Extend `LightingRuntimeState` with:

```cpp
std::array<float, 3> sun_tint;
std::array<float, 3> sky_zenith_tint;
std::array<float, 3> sky_horizon_tint;
```

Suggested intent:
- Morning: warm sun, slightly warm/soft horizon.
- Noon: near-neutral daylight reference.
- Evening: strongly warm sun, warmer horizon, reduced blue sky.
- Night: very low sun/direct light, dark blue sky palette.

These remain render-control values, not spectral measurements.

Add validation/tests for all colour channels finite and bounded.

## FrameLightingGpu — exact v0 layout

Use one std140-compatible UBO at set 0, binding 0.

C++:

```cpp
struct alignas(16) FrameLightingGpu {
    float sun_direction_intensity[4];   // xyz world direction, direct intensity
    float sun_tint_sky_intensity[4];    // rgb direct-light tint, sky intensity
    float sky_zenith_exposure[4];       // rgb zenith colour, exposure
    float sky_horizon_scene_lum[4];     // rgb horizon colour, scene luminance
    float filter_tint_transmission[4];  // rgb filter tint, transmission
    float eye_filter_misc[4];           // effective eye luminance, polarization strength, 0, 0
    uint32_t modes[4];                   // lighting preset, filter preset, reserved, reserved
};
```

Expected:
- size = 112 bytes,
- alignment = 16,
- all vec4/uvec4 offsets exactly 16-byte aligned.

Add compile-time assertions:
- `alignof(FrameLightingGpu) == 16`
- `sizeof(FrameLightingGpu) == 112`
- `offsetof(...)` = 0,16,32,48,64,80,96.

The body and eye push constants remain exactly 128 bytes.

## GLSL block

Both fragment shaders use the same logical layout:

```glsl
layout(set = 0, binding = 0, std140) uniform FrameLighting {
    vec4 sun_direction_intensity;
    vec4 sun_tint_sky_intensity;
    vec4 sky_zenith_exposure;
    vec4 sky_horizon_scene_lum;
    vec4 filter_tint_transmission;
    vec4 eye_filter_misc;
    uvec4 modes;
} lighting;
```

For this bounded pass, duplicating this small block in `human.frag` and `eye.frag` is acceptable and lower-risk than changing the current shader include pipeline. The buffer/state itself is shared and authoritative.

A shared GLSL include may be introduced later when shader infrastructure is refactored.

## Vulkan resources

Add to `VulkanRenderer::Impl`:

```cpp
VkDescriptorSetLayout frame_lighting_set_layout;
VkDescriptorPool frame_lighting_descriptor_pool;
VkDescriptorSet frame_lighting_descriptor_set;

VkBuffer frame_lighting_buffer;
VkDeviceMemory frame_lighting_memory;
void* frame_lighting_mapped;
```

### Resource creation

New helper:
`create_frame_lighting_resources(state)`

Responsibilities:
1. descriptor-set layout:
   - binding 0,
   - type `VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER`,
   - count 1,
   - fragment-stage visibility.

2. allocate one HOST_VISIBLE + HOST_COHERENT UBO.

3. persistently map it for renderer lifetime.

4. create descriptor pool:
   - one uniform-buffer descriptor,
   - maxSets = 1.

5. allocate/update one descriptor set.

6. account UBO bytes in `estimated_bytes`.

Initialization order:

```
create device
create swapchain/render targets
create mesh buffers
create frame lighting resources
create body pipeline
create eye pipeline
create sync
```

The descriptor-set layout must exist before either pipeline layout is created.

## Pipeline layouts

Body pipeline:
- descriptor set layout count = 1,
- set 0 = `frame_lighting_set_layout`,
- existing 128-byte body push constant unchanged.

Eye pipeline:
- same descriptor set layout,
- existing 128-byte eye push constant unchanged.

Bind the same descriptor set for:
- body draw,
- eye draw,
- UI/environment draw when body pipeline is rebound.

Do not depend on descriptor compatibility surviving a pipeline switch implicitly; bind explicitly for clarity.

## Per-frame update

New helper:
`make_frame_lighting_gpu(const LightingRuntimeState&)`.

New helper:
`update_frame_lighting_buffer(state)`.

Update sequence inside `draw_frame`:

1. wait for `in_flight` fence,
2. validate/read `state.lighting`,
3. memcpy one `FrameLightingGpu` to persistent mapped UBO,
4. update pupil runtime using exactly:
   `state.lighting.effective_eye_luminance`,
5. acquire/record/submit frame.

Because the buffer is HOST_COHERENT and there is one frame in flight, no flush or ring buffer is needed.

## Human/sky shader migration

### Sun direction

Delete fixed:
`sun_world_direction() = normalize(vec3(-0.30, 0.82, 0.47))`.

Use:
`normalize(lighting.sun_direction_intensity.xyz)`.

Convert to view direction through the existing camera rotation.

### Direct light

Replace fixed skin radiance:
`vec3(1.0, 0.95, 0.86) * 3.1`

with:
```
lighting.sun_tint_sky_intensity.rgb
* base_radiance_scale
* lighting.sun_direction_intensity.w
```

Keep the current BRDF structure; this pass changes lighting source, not skin material architecture.

### Sky

Replace fixed zenith/horizon colours with UBO colours.

Scale:
- procedural Rayleigh/Mie background by `sky_intensity`,
- sun disk by `direct_sun_intensity` and sun tint.

Night:
- direct sun contribution naturally approaches zero,
- sky remains visible through low sky palette/intensity + exposure.

### Exposure

Use one UBO exposure value before the existing ACES-like mapping.

Avoid applying exposure twice:
- body final linear colour * exposure -> ACES,
- environment final linear colour * exposure -> ACES.

## Eye shader migration

Remove independent constants:
- `vec3(0.35, 0.70, 0.55)`,
- hardcoded cornea sky palette.

Use shared UBO:
- world sun direction -> camera/view transform using existing eye camera data,
- direct intensity/tint for inner eye and cornea,
- sky zenith/horizon tint for cornea reflection approximation,
- exposure on final eye linear response where appropriate.

This pass does NOT fix cornea render ordering/faceting; that remains corrective pass 4.

## Time UI row

Current rows: 0..8.

Pass 2 adds:
- row 9 = Time.

Tap:
`Morning -> Noon -> Evening -> Night -> Morning`.

Long press/hover:
`TIME  CYCLE DAYLIGHT`.

Android and Windows must use the same cycle order.

### Reserve compact geometry for pass 3

Re-space panel now for 11 total eventual rows so adding Filter in pass 3 does not move everything again.

Recommended screen-normalized hit centres:
```
center_y = 0.07 + row * 0.07
```

For rows 0..10:
- first centre = 0.07,
- last centre = 0.77.

Hit half-height about 0.027–0.030.

Keep x range approximately:
`0.020 .. 0.185`.

Panel accepts through about y = 0.82.

Visual NDC equivalent:
```
center_y_ndc = 0.86 - row * 0.14
```

Button half-height around 0.050 NDC.

Pass 2 renders 10 rows; pass 3 activates the already-reserved 11th slot.

## HUD state packing

No push-constant growth.

Current body `flags.w` allocation:
- bit 0: UI environment pass,
- bits 1..2: diagnostic mode,
- bits 3..4: camera preset,
- bits 5..8: tooltip row,
- bits 9..10: physiology,
- bits 11..12: eye mode.

Reserve:
- bits 13..14: LightingPreset,
- bits 15..16: OpticalFilterPreset (pass 3).

Pass 2 packs Time preset into bits 13..14.

The Time row colour should show the active preset:
- Morning: warm amber,
- Noon: pale daylight,
- Evening: orange,
- Night: dark blue.

This colour is UI feedback only; actual lighting comes from the UBO.

## Diagnostics

Extend `RendererDiagnostics` in pass 2 with:
- `LightingPreset lighting_preset`,
- `OpticalFilterPreset optical_filter`,
- `float scene_luminance`,
- `float effective_eye_luminance`,
- `float pupil_target_radius`,
- `float pupil_current_radius`.

The pupil fields should report the currently selected character's eye state. If the existing diagnostics API has no character parameter, cache the last-rendered profile index in renderer state and use that profile.

Android log and Windows stdout diagnostics should include:
- time preset,
- scene luminance,
- effective eye luminance,
- pupil target/current.

## Tests / gates

### CPU
- all four time presets have finite colour state,
- sun direction normalized,
- Noon direct and scene luminance > Night,
- Morning/Evening sun tint warmer than Noon by chosen render-control metric,
- Night direct intensity remains near zero.

### Layout
- `FrameLightingGpu == 112 B`,
- exact std140 offsets asserted,
- Body Push == 128 B,
- Eye Push == 128 B.

### Vulkan
- descriptor set layout creation PASS,
- UBO allocation/map PASS,
- body pipeline uses set 0,
- eye pipeline uses set 0,
- explicit descriptor bind before body/eye/UI draws.

### Shader
- Vulkan 1.2 human fragment compile PASS,
- Vulkan 1.2 eye fragment compile PASS,
- no remaining fixed body sun direction,
- no remaining fixed eye sun direction.

### Runtime
- changing LightingPreset changes UBO bytes,
- pupil continues to use `effective_eye_luminance`,
- default remains Noon + Clear.

### Platform
- host contract tests PASS,
- Android arm64 APK build PASS,
- Windows x64 Vulkan build PASS,
- existing body/eye/physiology gates remain PASS.

## Stop condition for pass 2

Pass 2 is complete when:
- one shared frame UBO drives sky, body and eyes,
- Time row cycles four actual lighting states,
- pupil consumes the same lighting state's effective luminance,
- no body/eye push constant was enlarged,
- Android and Windows CI are green.

Do not implement Tinted/Polarized visual attenuation in this pass.
Do not start cornea faceting/order correction in this pass.
Do not add new character systems.
