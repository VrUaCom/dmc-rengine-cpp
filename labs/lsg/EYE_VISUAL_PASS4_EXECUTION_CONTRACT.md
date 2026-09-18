# Rengine LSG — Eye Visual Corrective Pass 4 Execution Contract

Branch scope: `rengine-living-surface-genome` only.

Prepared from baseline:
`f0ac48774087d5513faddb79ca474d5e60c84d68`

Pass 3 status at preparation time:
- Android CI: PASS
- Windows CI: PASS
- Clear / Tinted / Polarized Approx implemented
- Time presets implemented
- FrameLightingUBO remains 112 B
- Body Push remains 128 B
- Eye Push remains 128 B

This document prepares corrective pass 4/5 only. It does not implement the pass.

## Pass goal

Correct the remaining eye-visual defects seen in device testing:

- remove cornea faceting / UV-seam normal discontinuities,
- split opaque inner-eye rendering from transparent cornea rendering,
- correct iris and pupil proportions,
- reduce overly regular procedural iris appearance,
- refine sclera colour/vascular variation,
- add a subtle wet-eye edge approximation,
- preserve all existing time/filter/pupil behavior,
- keep zero mandatory eye textures.

This pass must not expand into eyelid rigging, full tear-fluid simulation, facial animation, hair, full Living Skin, spectral rendering, or full physical polarization.

---

## Audit findings

### 1. Eye normals are currently wrong for UV seams

`lsg_prepare_mesh` currently calls:

```cpp
regenerate_normals(mesh);
```

which accumulates normals by render-vertex index.

OBJ import duplicates vertices at UV seams, so identical source positions can receive different normals.

The tool already records:
`source_position_by_vertex`
for `--component-regions`.

Therefore the correct fix is available without changing RMS0.

### Required solution

For eye OBJ preparation only:

```cpp
regenerate_normals_by_source_position(
    mesh,
    source_position_by_vertex,
    source_position_count);
```

Algorithm:

1. allocate one face-normal accumulator per original OBJ position,
2. iterate mesh triangles,
3. map each render vertex index to its source-position id,
4. accumulate each face normal into the three source-position accumulators,
5. normalize source accumulators,
6. assign exactly the same normal to every render vertex sharing that source position,
7. regenerate tangents AFTER assigning the smoothed normals.

Do not change body mesh normal generation in this pass.

### Seam validation

Add a validation helper/report:

```
SOURCE NORMAL SEAMS PASS
max_duplicate_normal_delta=<value>
```

Gate:
- all render vertices sharing one source position have normal delta <= 1e-5,
- all normals finite,
- all normal lengths near 1,
- component count remains exactly 4.

---

## 2. Current eye draw architecture is incorrect for nested transparent geometry

Current eye pipeline:
- one pipeline,
- blend ENABLED,
- depth test ENABLED,
- depth write DISABLED,
- inner eye and outer cornea submitted in one indexed draw.

This is acceptable for diagnostics but not for final nested eye rendering.

### Required render order

Use explicit passes:

1. BODY
   - current opaque body pipeline

2. INNER EYE
   - components 1 / 3 only
   - blend OFF
   - depth test ON
   - depth write ON

3. CORNEA / WET SHELL
   - components 0 / 2 only
   - blend ON
   - depth test ON
   - depth write OFF

4. UI

### Recommended Vulkan implementation

Create two eye graphics pipelines sharing:
- eye vertex shader,
- eye fragment shader,
- same descriptor set layout,
- same 128-byte EyePush layout.

State differences:

#### Inner eye pipeline
```
blendEnable = VK_FALSE
depthTestEnable = VK_TRUE
depthWriteEnable = VK_TRUE
depthCompareOp = VK_COMPARE_OP_LESS_OR_EQUAL
```

#### Cornea pipeline
```
blendEnable = VK_TRUE
depthTestEnable = VK_TRUE
depthWriteEnable = VK_FALSE
depthCompareOp = VK_COMPARE_OP_LESS_OR_EQUAL
```

Do not create a second descriptor set.

Do not enlarge push constants.

### Eye pass selector

Current `flags.z`:
- bits 0..7: vascularity byte
- bits 8..9: EyeDiagnosticMode

Reserve:
- bits 10..11: EyeRenderPass

```cpp
enum class EyeRenderPass : uint32_t {
    inner = 0,
    cornea = 1,
};
```

Shader:
```glsl
uint eye_mode = (pc.flags.z >> 8u) & 3u;
uint eye_pass = (pc.flags.z >> 10u) & 3u;
```

The fragment shader discards components not belonging to the active pass.

### Diagnostic-mode behavior

Normal:
- draw inner pass
- draw cornea pass

Components:
- draw inner pass with diagnostic colors
- draw cornea pass with diagnostic colors

Iris Only:
- draw inner pass only
- discard sclera outside iris visualization region as currently intended

Cornea Only:
- draw cornea pass only

This preserves existing UI behavior while fixing normal render ordering.

---

## 3. Iris proportion correction

Current:
```glsl
const float iris_radius = 0.345;
```

Device test shows the iris is oversized.

### Initial corrective target

Set baseline:
```
iris_radius = 0.295
```

Allowed device-tuning band:
```
0.285 .. 0.305
```

Do not expose a user control in this pass.

### Acceptance

At Extreme Close-up:
- visible sclera clearly surrounds iris,
- iris no longer dominates the entire exposed eye,
- both characters remain anatomically plausible,
- no UV boundary clipping.

---

## 4. Pupil proportion correction

Current runtime:
- dark radius = 0.145
- bright radius = 0.082
- clamp = 0.070 .. 0.155

Device test shows pupil is too large, especially under daylight.

### Initial corrective runtime values

Use:

```cpp
dark_radius   = 0.118f;
bright_radius = 0.055f;
```

Runtime safety clamp:

```
0.045 .. 0.135
```

Initial state may remain around:
```
0.090 .. 0.100
```

Keep:
- luminance dependency,
- optical-filter dependency,
- exponential smoothing.

Do not change the current Time/Filter architecture.

### Required ordering tests

For same character:
- Noon + Clear produces smaller pupil than Night + Clear,
- Noon + Clear produces smaller pupil than Noon + Tinted,
- Noon + Clear produces smaller pupil than Noon + Polarized Approx.

All remain bounded after retuning.

---

## 5. Procedural iris refinement

Current pattern is deterministic but visibly regular.

Current structure uses:
- 192 angular sectors,
- 14 radial bands,
- two hash-derived fibre fields,
- circular collarette,
- circular limbal darkening.

### Corrective target

Keep the same deterministic seed system but reduce visible procedural regularity.

Recommended v0 refinement:

#### Angular warp

Add a low-frequency deterministic phase warp:

```
warped_angle =
    angle01
  + 0.010 * low_frequency_hash_or_interpolated_field
  + 0.004 * radial_dependent_warp;
```

No screen-space noise.

#### Multi-frequency fibre field

Combine at least three bands:
- coarse angular variation,
- medium fibres,
- fine fibres.

Example conceptual mix:
```
fibre =
    0.45 * medium
  + 0.30 * fine
  + 0.25 * coarse;
```

Do not simply increase octaves indiscriminately.

#### Collarette

Replace perfect circular collarette with seeded radial perturbation.

Concept:
```
collarette_radius =
    0.47
  + seeded_angle_variation * 0.035;
```

#### Limbal ring

Replace fully uniform ring with:
- slightly irregular thickness,
- slight seeded attenuation,
- still stable across camera movement.

### Filtering

Retain band-limited behavior.
Do not add unfiltered high-frequency shimmer.

---

## 6. Sclera refinement

Current baseline:
- warm = (0.935, 0.905, 0.875)
- neutral = (0.965, 0.955, 0.935)

This is already better than pure white but device output still appears too clean.

### Corrective model

Add two restrained components:

1. low-frequency colour variation
2. sparse vascular hints

### Low-frequency variation

Use deterministic eye-local coordinates.

Amplitude target:
- luminance variation only about 1–2%,
- chroma variation very small.

No obvious blotches.

### Vascular hints

Current cell threshold can appear synthetic.

Replace or soften with:
- lower opacity,
- fewer visible vessels,
- elongated / directional impression if possible,
- stronger near peripheral sclera,
- weakest near iris.

Do not attempt biological vessel topology.

Do not use a texture.

---

## 7. Wet-eye / tear-rim approximation

Full tear-meniscus geometry remains out of scope.

Add only a subtle shader approximation.

### Goal

Remove the “dry inserted eyeball” look at the visible edge.

Suggested method:

For inner eye local UV:
- compute distance toward exposed outer eye boundary,
- emphasize lower half slightly,
- add a thin high-specular wet rim.

Conceptual weight:

```
edge = smoothstep(edge_start, edge_end, radius_or_boundary_metric)
lower = smoothstep(-0.05, -0.80, local_uv.y)
wet = edge * mix(0.35, 1.0, lower)
```

Use:
- low alpha,
- high specular response,
- shared lighting,
- current filter response.

Do not create a bright white outline.

Do not call it a physically complete tear meniscus.

---

## 8. Cornea visual correction

Current cornea can look blue/plastic because:
- sky term can dominate,
- alpha can become too high,
- normal discontinuities amplify facets,
- inner and cornea are currently unordered in one blended pass.

After fixing normals/render order, retune conservatively.

### Initial targets

Keep IOR-like value:
```
IOR ~= 1.376
```

but reduce visible shell colour saturation.

Recommended:
- reduce direct sky colour contribution,
- keep Fresnel mainly as reflectance intensity,
- keep shell nearly colourless,
- preserve sun highlight,
- retain Polarized Approx glare attenuation.

Avoid blue shell appearance.

Suggested alpha range after retune:
```
0.025 .. 0.38
```

instead of current upper range near 0.72.

Do not add full refraction in this pass.

---

## 9. Mesh preparation gates

For both profile 0 and profile 1 eye assets:

Required:
- source eye vertices = 1064
- connected components = 4
- output render vertices remain deterministic
- no NaN/Inf
- source-smoothed normals PASS
- tangents finite
- RMS0 validate PASS
- repeated build hash-identical

Add eye-preparation evidence to both workflows.

Suggested report fields:

```
EYE RMS0 PASS
components=4
source_positions=1064
render_vertices=<...>
normal_seam_max_delta=<...>
normal_length_min=<...>
normal_length_max=<...>
```

---

## 10. Renderer gates

After split:

Expected eye pipeline state:

### Inner
- one pipeline layout
- same FrameLighting descriptor
- 128-byte EyePush
- opaque
- depth write ON

### Cornea
- same pipeline layout contract
- same FrameLighting descriptor
- 128-byte EyePush
- blend ON
- depth write OFF

Renderer resource cleanup must destroy:
- inner eye pipeline
- cornea pipeline
- shared eye pipeline layout once

Do not duplicate eye pipeline layout unless required by Vulkan implementation.

### Draw count

Normal eye render:
- one indexed inner-eye draw
- one indexed cornea draw

No extra generated geometry.

---

## 11. Preserve pass 3 behavior

Pass 4 must not regress:

- Morning / Noon / Evening / Night
- Clear / Tinted / Polarized Approx
- shared FrameLightingUBO
- pupil filter response
- polarization approximation
- Eye diagnostics
- character switching
- camera presets
- body physiology
- zero mandatory character textures

Filter behavior remains:
- Clear reference
- Tinted bulk transmission
- Polarized Approx bulk + bounded sky/cornea glare attenuation

---

## 12. Automated tests

### Eye runtime
- new bright/dark target values bounded
- monotonic luminance response preserved
- all 12 Time × Filter states preserve pupil ordering

### Eye normal smoothing
- seam duplicate normals exactly/effectively match
- all normal lengths finite
- min normal length > 0.99
- max normal length < 1.01
- four components remain four

### Iris reference
If new helper functions are mirrored on CPU:
- same seed -> same samples
- different seed -> different samples
- left/right differ
- no NaN
- perturbation remains bounded

Do not require pixel-perfect GPU equivalence.

### Pipeline contract
Host/static checks or CI evidence:
- Body Push = 128 B
- Eye Push = 128 B
- FrameLightingGpu = 112 B
- inner eye pipeline depthWrite = TRUE
- cornea pipeline depthWrite = FALSE
- inner blend = FALSE
- cornea blend = TRUE

### Shaders
- eye.vert Vulkan 1.2 compile PASS
- eye.frag Vulkan 1.2 compile PASS
- human shaders unchanged except required shared contracts
- no eye texture sampler introduced

### Platforms
- Android arm64 PASS
- Windows x64 Vulkan PASS
- all existing host tests PASS

---

## 13. Bounded implementation plan

### Slice 4A — Source-position normal smoothing (~20 min)

Implement:
- source-position normal accumulation,
- eye-only preparation path,
- seam delta validation,
- CI evidence.

STOP:
- both eye profiles report 4 components,
- seam normal delta <= 1e-5,
- Android/Windows host preparation PASS.

### Slice 4B — Split eye rendering (~20 min)

Implement:
- inner eye pipeline,
- cornea pipeline,
- eye-pass selector,
- ordered draws,
- cleanup.

STOP:
- shaders compile,
- pipeline creation PASS,
- Android/Windows build PASS.

### Slice 4C — Iris/pupil/sclera tuning (~20 min)

Implement:
- iris radius 0.295 baseline,
- pupil runtime retune,
- multi-frequency iris irregularity,
- non-perfect collarette/limbal ring,
- sclera refinement.

STOP:
- host tests PASS,
- shader compile PASS,
- no new texture dependencies.

### Slice 4D — Wet rim + cornea tuning (~20 min)

Implement:
- subtle wet-edge approximation,
- reduce cornea blue/plastic response,
- reduce cornea alpha,
- preserve filter/polarization interaction.

STOP:
- CI fully green,
- no known render-order/faceting issue remains in code path.

### Slice 4E — APK/device visual test

Capture:
1. Character 0 Noon/Clear Normal
2. Character 1 Noon/Clear Normal
3. extreme close-up front
4. strong side orbit
5. Components
6. Iris Only
7. Cornea Only
8. Noon -> Night pupil adaptation
9. Clear -> Tinted -> Polarized Approx

Final visual DEVICE PASS requires:
- no obvious faceted cornea,
- no blue plastic shell,
- iris size plausible,
- daylight pupil visibly smaller,
- sclera not flat white,
- wet rim subtle,
- no transparency corruption,
- time/filter behavior preserved.

---

## 14. Stop condition for pass 4

Pass 4 is complete when:

- source-position eye normals are smooth,
- inner/cornea render order is explicit,
- iris/pupil proportions are corrected,
- procedural iris regularity is visibly reduced,
- sclera is less synthetic,
- wet-edge approximation exists,
- cornea shell is visually restrained,
- no baked eye texture is added,
- Android and Windows CI are green.

After that, move to pass 5 only:
physical device acceptance / final corrective review.

Do not start Living Skin until pass 5 is reviewed.
