# Rengine LSG — Eye/Lighting Corrective Pass Contract

Branch scope: `rengine-living-surface-genome` only.

Baseline HEAD at preparation time:
`c7caa6764007f018bc5cab1246dee69e5542b406`

This pass is a corrective continuation of Eyes Foundation. It must not expand into hair, facial rigging, full spectral rendering, gameplay, or full Living Skin.

## Goals

Close the visual/device issues found in the physical Eyes Foundation test and add a real lighting-driven eye response.

Required outcomes:
- fix faceted / incorrect cornea rendering,
- tune iris and pupil proportions,
- refine sclera and wet-eye response,
- make pupil radius react automatically to effective scene luminance,
- add four time-of-day lighting presets,
- add three approximate optical-filter presets,
- expose those controls through the existing compact R&D UI,
- keep all behavior shared between Windows and Android,
- preserve zero mandatory eye textures.

## Current technical debt confirmed by audit

### Lighting is not yet a runtime system

Current body/sky shader contains a fixed:
`sun_world_direction() = normalize(vec3(-0.30, 0.82, 0.47))`

Current pupil runtime is driven by a fixed renderer constant:
`kDaylightSceneLuminance = 1.8f`

Therefore time-of-day controls MUST be implemented as a C++ lighting runtime and fed consistently into body, eye and environment shaders. Do not fake Morning/Noon/Evening/Night as colour-only UI states.

### Eye render order is insufficient

Current eye mesh is rendered once with a single blended pipeline:
- depth test ON,
- depth write OFF,
- inner and outer components submitted together.

This is not a reliable transparency/depth contract for nested eyeball + cornea geometry and can produce the shell/order artifacts seen on device.

### Cornea faceting

Eye RMS0 normals are regenerated on render vertices. OBJ UV seams duplicate render vertices; smoothing only render vertices can leave visible seam/facet artifacts.

Corrective plan:
- smooth eye normals by original OBJ source-position provenance before assigning them to UV-split render vertices,
- validate seam-equivalent positions receive equivalent normals within tolerance.

## New runtime contracts

### LightingPreset

```cpp
enum class LightingPreset : uint8_t {
    morning = 0,
    noon = 1,
    evening = 2,
    night = 3,
};
```

### OpticalFilterPreset

```cpp
enum class OpticalFilterPreset : uint8_t {
    clear = 0,
    tinted = 1,
    polarized_approx = 2,
};
```

The polarized mode MUST be documented as an approximation. It is not a Stokes/Mueller polarization pipeline.

### LightingRuntimeState

Suggested minimum:

```cpp
struct LightingRuntimeState {
    LightingPreset preset;
    OpticalFilterPreset filter;

    float sun_elevation_rad;
    float sun_azimuth_rad;
    float direct_sun_intensity;
    float sky_intensity;
    float exposure;
    float scene_luminance;
    float effective_eye_luminance;

    float filter_transmission;
    float polarization_strength;
    float filter_tint[3];
};
```

One shared state drives:
- procedural sky,
- direct body lighting,
- eye inner lighting,
- cornea response,
- pupil adaptation.

No duplicate hard-coded sun vectors are allowed after this pass.

## Time-of-day presets

These are visual/runtime test presets, not geographic astronomical simulation.

### Morning
Intent:
- lower sun,
- warmer direct light,
- medium luminance,
- pupil moderately open.

Initial target parameters:
- sun elevation: about 20–30 degrees,
- direct intensity: medium,
- sky intensity: medium,
- effective luminance below Noon.

### Noon
Intent:
- brightest preset,
- sun elevation about 55 degrees,
- neutral daylight baseline,
- smallest pupil of the four presets.

This is the main reference/acceptance lighting.

### Evening
Intent:
- low sun,
- warm direct contribution,
- reduced sky/direct intensity,
- pupil wider than Morning/Noon.

### Night
Intent:
- no strong solar disk contribution,
- low environment intensity,
- pupil clearly larger,
- cornea/specular remains visible but not self-illuminated.

Night must remain viewable through exposure/tone mapping without cheating pupil luminance upward to Noon levels.

## Optical filters

### Clear
- transmission = 1.0 baseline,
- neutral tint,
- no polarization attenuation.

### Tinted
- reduced transmission,
- subtle neutral/warm/cool glass tint,
- effective eye luminance reduced by transmission,
- pupil therefore opens somewhat relative to Clear under the same lighting preset.

### Polarized Approx
- moderate transmission loss,
- approximate reduction of sky/cornea glare,
- use Rayleigh-like sky polarization field where useful.

Allowed first-order sky DoLP:

```
DoLP = sin^2(theta) / (1 + cos^2(theta))
```

where theta is the scattering angle.

The filter may attenuate polarized sky/specular/glare response according to DoLP and a fixed prototype polarizer axis.

Do NOT label this as full physical polarization.

## Pupil response

Existing runtime state remains separate from Genome.

Correct dependency:

```
LightingPreset
  -> scene luminance
OpticalFilter
  -> transmission / polarization loss
  -> effective eye luminance
  -> pupil target
  -> exponential temporal smoothing
  -> visible pupil radius
```

Remove the fixed `kDaylightSceneLuminance = 1.8f`.

Acceptance direction:
- Noon/Clear = smallest pupil,
- Night/Clear = largest pupil,
- same time + Tinted must not produce a smaller pupil than Clear,
- response is gradual rather than instantaneous.

Tune current pupil bounds downward from the oversized device-test result.

Initial corrective range for visual tuning:
- bright target radius around 0.05–0.065 local-eye units,
- dark target radius around 0.105–0.125,
- hard safety clamp around 0.045–0.135.

Final constants are set by device test, not assumed biological measurements.

## Iris corrective tuning

Current device test shows oversized iris and overly regular procedural pattern.

Corrective changes:
- reduce iris radius from current 0.345 baseline,
- initial visual target around 0.285–0.305,
- reduce overly regular ring/sector appearance,
- add multi-frequency angular fibre variation,
- weaken perfectly circular collarette,
- make limbal ring irregular but stable,
- preserve band-limited filtering,
- preserve deterministic object/local-eye coordinates,
- left/right eyes derive distinct deterministic side keys.

No texture map.

## Sclera corrective tuning

Goals:
- less pure/flat white,
- subtle warm baseline,
- low-frequency colour variation,
- sparse vascular hints,
- vascularity remains genome-controlled,
- avoid conspicuous red procedural noise.

Do not attempt medical vascular simulation.

## Wet-eye / tear-line approximation

Full eyelid tear-meniscus geometry is out of scope for this corrective pass.

Implement a clearly documented approximation:
- thin wet specular rim near exposed cornea/eye boundary,
- stronger response along lower/edge region,
- deterministic/stable,
- no baked texture.

Do not call this a physically complete tear meniscus.

## Cornea corrective architecture

Replace the current single mixed eye draw with explicit ordered passes:

1. BODY
   - existing opaque body depth

2. INNER EYE
   - components 1/3
   - opaque
   - depth test ON
   - depth write ON

3. CORNEA/WET SHELL
   - components 0/2
   - alpha blend ON
   - depth test ON
   - depth write OFF

4. UI

Use separate Vulkan pipelines or equivalent fixed-function states. Do not rely on triangle/component ordering within one blended draw.

### Cornea normals

For eye RMS0 generation:
- preserve source OBJ position index provenance,
- average normals over topology using original position ids,
- copy that smooth normal to all UV-split render vertices sharing the same source position.

Add a host test that UV seam duplicates at the same source position receive matching normals.

### Cornea shading

Keep v0 as an approximation:
- IOR-like dielectric Fresnel,
- sun highlight,
- sky/environment reflection,
- optional approximate polarization/glare attenuation,
- no claim of full physical refraction.

Reduce the blue/plastic shell appearance observed in device test.

## Shader/runtime parameter transport

Current Body Push = 128 B.
Current Eye Push = 128 B.

Do NOT exceed Vulkan 1.2 guaranteed minimum push constant size.

Lighting parameters should move to a small uniform/storage buffer or another shared GPU parameter buffer instead of expanding existing push constants.

Preferred:
`FrameLightingUBO` shared by body + eye shaders.

Suggested fields:
- sun direction + intensity,
- sky intensity + exposure,
- RGB direct-light tint,
- RGB filter tint + transmission,
- effective luminance,
- polarization approximation parameters,
- active preset/filter ids for diagnostics.

This is also the correct architectural step toward the original procedural atmosphere/polarization requirement.

## R&D UI

Current rows 0..8 remain.

Add:
- row 9: Time
- row 10: Filter

To keep the panel compact:
- reduce row vertical spacing enough to fit 11 entries,
- do not return to large buttons,
- preserve long-press/hover help.

### Time row tap
Cycle:
`Morning -> Noon -> Evening -> Night -> Morning`

### Filter row tap
Cycle:
`Clear -> Tinted -> Polarized -> Clear`

### Tooltips
Time:
`TIME  MORNING NOON EVENING NIGHT`

Filter:
`FILTER  CLEAR TINT POLARIZED`

Eyes row remains:
`Normal / Components / Iris Only / Cornea Only`.

## Diagnostics

Extend RendererDiagnostics with at least:
- active LightingPreset,
- active OpticalFilterPreset,
- scene_luminance,
- effective_eye_luminance,
- pupil_target_radius,
- pupil_current_radius.

Android log and Windows diagnostics must print these.

If space permits, a compact numeric overlay can be added later; logging is sufficient for this corrective gate.

## Automated tests

### Lighting presets
- enum values valid,
- all preset outputs finite,
- Noon effective luminance > Morning/Evening > Night under Clear,
- sun direction normalized,
- Night direct sun intensity near zero.

### Filters
- Clear transmission = baseline,
- Tinted transmission < Clear,
- Polarized Approx transmission <= Clear,
- all filter RGB/tint values finite/bounded,
- effective eye luminance never negative.

### Pupil
- darker effective luminance -> larger target radius,
- Tinted under same time does not produce smaller target pupil than Clear,
- exponential smoothing monotonic toward target,
- no NaN/Inf,
- bounded radius.

### Iris
- deterministic same seed,
- distinct character seeds differ,
- left/right side keys differ,
- tuned radius bounds valid.

### Eye normals
- exactly 4 connected components still preserved,
- UV seam duplicates sharing source position receive equivalent smooth normals,
- no zero/NaN normals.

### GPU
- FrameLightingUBO layout checked,
- EyePush remains exactly 128 B,
- Body Push remains exactly 128 B,
- Vulkan 1.2 shaders compile on Windows/Android.

## Physical device acceptance

Capture/use Extreme Close-up.

Required sequence:

1. Character 0, Noon + Clear, Eyes Normal.
2. Character 1, Noon + Clear, Eyes Normal.
3. Side orbit showing cornea: no polygonal/faceted blue shell.
4. Eyes Components.
5. Iris Only.
6. Cornea Only.
7. Hold camera fixed and cycle:
   Morning -> Noon -> Evening -> Night.
   Pupil must visibly and smoothly adapt.
8. Hold Noon and cycle:
   Clear -> Tinted -> Polarized.
   Tinted must reduce effective eye luminance and affect pupil after smoothing.
   Polarized Approx should visibly reduce selected glare/sky response without pretending full physical polarization.

Final PASS conditions:
- eye remains correctly seated,
- iris/pupil proportions no longer look oversized,
- cornea appears smooth,
- inner eye remains correctly occluded by body/eyelids,
- no transparency ordering corruption,
- sclera no longer flat pure white,
- wet-edge approximation visible only subtly,
- pupil response visible and smooth,
- time presets change actual lighting, not only UI colour,
- optical filters affect transmitted/effective light,
- no mandatory eye texture,
- Android no crash/corruption,
- Windows CI PASS,
- all previous macro/body/skin/physiology gates remain PASS.

## Bounded implementation plan

### Pass 1 — Lighting runtime foundation (~20 min)
- LightingPreset / OpticalFilterPreset
- LightingRuntimeState
- preset/filter parameter tables
- tests
- renderer API
- remove fixed pupil luminance

STOP:
CPU tests PASS; pupil driven by runtime effective luminance.

### Pass 2 — Shared GPU lighting + time presets (~20 min)
- FrameLightingUBO
- body/sky/eye shader integration
- shared sun direction/intensity
- Morning/Noon/Evening/Night
- Time UI row + tooltip

STOP:
Windows/Android shaders compile; sky/body/eye all use one lighting state.

### Pass 3 — Optical filters + polarization approximation (~20 min)
- Clear/Tinted/Polarized Approx
- effective luminance path
- sky DoLP approximation
- cornea/sky glare attenuation
- Filter UI row + tooltip
- diagnostics

STOP:
filter tests PASS and shader compile PASS.

### Pass 4 — Eye visual corrective (~20 min)
- smooth source-position eye normals
- split inner/cornea pipeline ordering
- iris radius/pattern tuning
- pupil radius retuning
- sclera refinement
- wet-rim approximation

STOP:
CI PASS and no known render-order/faceting defect in code path.

### Pass 5 — APK/device acceptance
- Android APK
- Windows artifact
- device video/screenshots
- review all 8 acceptance views/actions above

Do not move to Living Skin until this corrective eye/lighting gate is reviewed.
