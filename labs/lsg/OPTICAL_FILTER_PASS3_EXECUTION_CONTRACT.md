# Rengine LSG — Optical Filters / Polarized Approx Pass 3 Execution Contract

Branch scope: `rengine-living-surface-genome` only.

Prepared from baseline:
`1da6412f26ec7cd7e1cf57a01c79ceac2f699559`

This document prepares corrective pass 3/5 only. It does not implement the pass.

## Pass goal

Activate the optical-filter fields already carried by `FrameLightingUBO` and add the 11th compact HUD row:

`Clear -> Tinted -> Polarized Approx -> Clear`

Required behavior:
- Clear is the reference path.
- Tinted reduces transmitted light and applies a restrained glass tint.
- Polarized Approx applies bulk transmission plus angle-dependent attenuation of selected polarized/glare terms.
- pupil adaptation continues to use the same runtime filter state through `effective_eye_luminance`.
- Android and Windows expose the same cycle order and diagnostics.
- no descriptor-layout or push-constant redesign.

## Important scope / terminology

This is NOT:
- a Stokes-vector renderer,
- a Mueller-matrix material pipeline,
- a spectral glass solver,
- literal eyeglass geometry,
- a claim of physically complete polarization.

It is an R&D optical-transport approximation.

In v0 the filter is treated as a global optical pane in the lighting/view path because no explicit glass geometry exists yet. Therefore:
- bulk transmission/tint may affect the visible sky/body/eye response,
- angle-dependent polarization attenuation is restricted to terms where it has a meaningful prototype interpretation: atmospheric sky scattering and cornea/specular glare,
- diffuse skin is NOT given an invented polarization model.

The UI label must remain `Polarized Approx`.

## Current audit findings

Pass 2 already created the complete transport channel needed by pass 3.

`FrameLightingGpu` already contains:
- `filter_tint_transmission.rgb`
- `filter_tint_transmission.w`
- `eye_filter_misc.x = effective eye luminance`
- `eye_filter_misc.y = polarization strength`
- `modes.y = optical filter preset`

No UBO size/layout change is required.

Confirmed current state:
- FrameLightingGpu remains 112 B,
- Body Push remains 128 B,
- Eye Push remains 128 B,
- descriptor set 0 / binding 0 is already shared by body and eye fragment shaders,
- filter fields are currently uploaded but unused by both shaders.

Therefore pass 3 must NOT create another descriptor or enlarge any push block.

## Existing CPU presets

Current values from `lighting_runtime.cpp`:

### Clear
- transmission = 1.00
- polarization strength = 0.00
- tint = (1.00, 1.00, 1.00)

### Tinted
- transmission = 0.58
- polarization strength = 0.00
- tint = (0.90, 0.94, 0.98)

### Polarized Approx
- transmission = 0.78
- polarization strength = 0.68
- tint = (0.97, 0.985, 1.00)

Keep these as the initial test values for pass 3.
Do not artistic-tune them until device evidence exists.

## CPU reference polarization helpers

Add a small shared CPU reference module so the approximation is not shader-only magic.

Suggested files:
- `include/rengine/lsg/polarization_approx.hpp`
- `src/polarization_approx.cpp`

### Rayleigh degree of linear polarization

Use scattering cosine `mu = cos(theta)`.

```cpp
float rayleigh_dolp_from_mu(float mu) noexcept {
    mu = clamp(mu, -1.0f, 1.0f);
    const float mu2 = mu * mu;
    return (1.0f - mu2) / (1.0f + mu2);
}
```

Equivalent to:

```
DoLP = sin^2(theta) / (1 + cos^2(theta))
```

Expected:
- mu = 0 -> DoLP = 1
- mu = +1/-1 -> DoLP = 0
- symmetric for +/- mu
- finite and bounded [0,1]

### Approximate polarizer attenuation

Do not model full Malus/Stokes transport.

Use a bounded glare/scattering reduction:

```cpp
float polarized_attenuation(float dolp,
                            float axis_alignment_sq,
                            float strength) noexcept {
    dolp = clamp(dolp, 0, 1);
    axis_alignment_sq = clamp(axis_alignment_sq, 0, 1);
    strength = clamp(strength, 0, 1);

    return clamp(
        1.0f - 0.55f * strength * dolp * (1.0f - axis_alignment_sq),
        0.45f,
        1.0f);
}
```

This is deliberately an approximation:
- aligned polarized contribution passes more strongly,
- cross-oriented polarized glare/scattering is reduced,
- unpolarized/low-DoLP contribution is not aggressively destroyed.

The fixed `0.55` coefficient is a prototype strength cap, not a physical material constant.

## Polarizer axis convention

Pass 3 does not expose filter rotation.

Use one fixed transmission axis:
- world vertical / up projected into the plane perpendicular to the current viewing/scattering ray.

For sky ray `r` and sun direction `s`:

```glsl
vec3 pol_dir = normalize(cross(r, s));
vec3 axis = world_up - r * dot(world_up, r);
axis = normalize(axis);
float alignment_sq = pow(dot(pol_dir, axis), 2.0);
```

Degenerate cases:
- if `cross(r,s)` is near zero,
- or projected axis length is near zero,

return polarization attenuation = 1.0.

This makes the approximation stable at/near the sun and zenith singularities.

## Shared GLSL helpers

Mirror the CPU reference in both fragment shaders or in a shared shader include if the build path is made reliable in this pass.

Minimum helpers:

```glsl
float rayleigh_dolp_from_mu(float mu);
float polarized_attenuation(float dolp, float alignment_sq, float strength);
vec3 bulk_filter_rgb();
```

`bulk_filter_rgb()`:

```glsl
return lighting.filter_tint_transmission.rgb *
       lighting.filter_tint_transmission.w;
```

Optical preset id:
`lighting.modes.y`.

Polarization strength:
`lighting.eye_filter_misc.y`.

## Human / environment shader behavior

### Clear

Must be numerically the reference path:
- bulk filter rgb = (1,1,1),
- polarization strength = 0,
- no angle-dependent attenuation.

### Tinted

Apply bulk transmission/tint to linear radiance BEFORE ACES:

- procedural sky output,
- ground/environment output,
- final body/skin linear colour.

Do not alter material roughness or skin genome values.

### Polarized Approx

Apply the same bulk transmission/tint.

Additionally, in `procedural_environment`:

1. compute existing:
   `mu = dot(ray_world, sun_world)`

2. compute:
   `DoLP = (1 - mu^2) / (1 + mu^2)`

3. derive polarization direction from the scattering plane.

4. compute fixed-axis alignment.

5. multiply the Rayleigh/Mie scattered sky contribution by the bounded polarization attenuation.

Do NOT apply angle-dependent polarization attenuation to the direct sun disk in v0.
The direct sun gets only bulk transmission/tint.

Ground gets only bulk transmission/tint.

### Body/skin

For pass 3:
- apply bulk transmission/tint to final linear skin radiance,
- do not add a polarization model to diffuse skin,
- leave existing skin BRDF unchanged.

This is intentionally conservative.

## Eye shader behavior

### Inner eye

Apply bulk transmission/tint to the linear inner-eye response.

Do not change iris procedural structure, sclera model, or pupil radius tuning in this pass.

### Cornea

Apply bulk transmission/tint to:
- environment reflection approximation,
- sun/specular response.

Then apply additional `Polarized Approx` attenuation specifically to glare/reflection.

Recommended v0 path:

1. compute reflected view direction in view space.
2. transform the reflected direction back to world space using the inverse camera rotation already available from `pc.camera`.
3. compute a sky/scattering proxy:
   `mu_reflect = dot(reflected_world, sun_world)`.
4. compute DoLP + fixed-axis alignment.
5. use the bounded attenuation helper to reduce the sky-reflection/glare term.

For the sharp direct sun highlight:
- either use the same bounded attenuation at lower weight,
- or leave it at bulk-only transmission if the first implementation becomes unstable.

Do NOT claim this is Fresnel polarization or Brewster-angle-accurate.

## Pupil behavior

No new pupil architecture is needed.

Current:
```
effective_eye_luminance =
    scene_luminance * filter_transmission
```

Keep that for pass 3.

Reason:
- bulk filter transmission is known on CPU,
- angle-dependent polarization loss would require integrating the full incident hemisphere,
- that hemispherical integration does not exist in v0.

Therefore:
- Clear Noon pupil target is smallest of the three filters,
- Polarized Approx Noon pupil target is larger than Clear,
- Tinted Noon pupil target is largest,
given the current transmission ordering:
`1.00 > 0.78 > 0.58`.

This is the correct bounded prototype behavior.

## Filter UI row

Current visible rows: 0..9.

Add:
- row 10 = Filter.

Tap cycle:
`Clear -> Tinted -> Polarized Approx -> Clear`.

Android and Windows must use identical order.

Long press / hover tooltip:
`FILTER  CYCLE OPTICS`

20 characters.

### UI geometry

Pass 2 already re-spaced the hit layout for 11 slots:
- center_y = 0.07 + row * 0.07
- row 10 center = 0.77
- panel accepts through y ≈ 0.82

Therefore pass 3 only activates row 10.

Shader geometry update:
- row count: 10 -> 11
- row vertices: 66 total for rows
- tooltip begins after:
  3 environment + 6 panel + 66 rows = vertex 75
- total draw vertices with tooltip quad:
  81

Expected changes in `human.vert`:
- `row < 11u`
- tooltip condition `vertex < 81u && tooltip < 11u`
- tooltip quad index base `vertex - 75u`

Renderer UI draw:
- `vkCmdDraw(..., 81u, ...)`

Tooltip acceptance:
- renderer allows rows 0..10.

Android/Windows hit loop:
- row < 11.

### Filter icon

Use a compact glass/filter symbol:
- narrow rectangular pane,
- one/two diagonal polarization lines,
- optional center bar.

Do not increase button size.

### Filter feedback colour

Suggested UI-only feedback:
- Clear: neutral pale/steel blue
- Tinted: dark desaturated blue
- Polarized Approx: cyan/violet

This is UI feedback only.
Actual optical response comes from UBO fields.

## HUD flag packing

No new allocation needed.

Pass 2 already reserves:
- bits 13..14 = LightingPreset
- bits 15..16 = OpticalFilterPreset

Decode in `human.frag`:

```glsl
uint optical_filter = (pc.flags.w >> 15u) & 3u;
```

Use only for HUD selected state / colour.
Do not use push flags as the physical filter source; physical values come from `FrameLightingUBO`.

## Diagnostics

Extend `RendererDiagnostics` with:
- `float filter_transmission`
- `float polarization_strength`

Existing diagnostics already expose:
- active filter preset,
- scene luminance,
- effective eye luminance,
- pupil target/current.

Android log and Windows diagnostics should report:

```
filter=CLEAR|TINTED|POLARIZED_APPROX
transmission=...
polar_strength=...
eye_lum=...
pupil_target=...
pupil=...
```

This allows the device recording to prove that the filter changes the runtime state rather than only recolouring the HUD.

## Automated tests

### CPU filter matrix

Test all 4 time presets × all 3 filters = 12 states.

For every state:
- valid_lighting_state PASS,
- transmission in (0,1],
- tint channels [0,1],
- polarization strength [0,1],
- effective eye luminance >= 0,
- no NaN/Inf.

For every time preset:
- Clear effective luminance > Polarized Approx > Tinted.

### Pupil ordering

For the same time + character:
- target_pupil(Clear) <= target_pupil(Polarized)
- target_pupil(Polarized) <= target_pupil(Tinted)

Use epsilon-safe comparisons.

### DoLP

CPU reference tests:
- DoLP(0) == 1
- DoLP(+1) == 0
- DoLP(-1) == 0
- DoLP(mu) == DoLP(-mu)
- representative `mu=0.5` finite and within (0,1)

### Polarization attenuation

- strength 0 -> 1.0
- DoLP 0 -> 1.0
- full alignment -> 1.0
- cross alignment + strong DoLP -> < 1.0
- result always [0.45,1.0]

### GPU layout

Must remain:
- FrameLightingGpu = 112 B
- Body Push = 128 B
- Eye Push = 128 B

No descriptor-layout changes.

### Shader gates

- Vulkan 1.2 human fragment compile PASS
- Vulkan 1.2 eye fragment compile PASS
- both shaders read `filter_tint_transmission`
- polarization helper used only when strength > 0 / appropriate path

### Platform

- host tests PASS
- Android arm64 APK build PASS
- Windows x64 Vulkan build PASS
- storage/zero-baked-eye-texture gates remain PASS

## Device acceptance for pass 3

Use one fixed camera and one fixed Time preset first: Noon.

Capture:
1. Noon + Clear
2. Noon + Tinted
3. Noon + Polarized Approx

Expected:

### Clear
Reference image.

### Tinted
- visibly reduced brightness,
- restrained glass tint,
- pupil gradually opens relative to Clear,
- not simply an exposure UI change.

### Polarized Approx
- some selected sky/cornea glare reduced relative to Clear,
- different from Tinted,
- bulk brightness reduced less than Tinted with current preset values,
- pupil opens relative to Clear but less than Tinted.

Then perform one secondary test:
- Evening + Clear
- Evening + Polarized Approx

Verify the polarization approximation remains stable at low sun angles and does not produce black bands, NaN, flicker, or orientation singularities.

## Stop condition for pass 3

Pass 3 is complete when:
- Filter row cycles all three modes,
- bulk transmission/tint affects the actual rendered scene,
- pupil responds to each filter through effective eye luminance,
- Polarized Approx produces angle-dependent sky/cornea glare reduction,
- no full-polarization claim is made,
- UBO/push sizes remain unchanged,
- Android and Windows CI are green.

Do not start:
- cornea geometry/faceting correction,
- iris/pupil visual retuning,
- sclera/wet-rim refinement,
- Living Skin expansion.

Those remain pass 4 / later work.
