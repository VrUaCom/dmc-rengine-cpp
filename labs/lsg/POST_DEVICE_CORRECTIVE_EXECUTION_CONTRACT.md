# Rengine LSG — Post-Device Lighting & Surface Corrective Execution Contract

Branch scope: `rengine-living-surface-genome` only.

Preparation baseline HEAD:
`5686d4b1d9b7aa2a1ae83fbb2f7d53f2de227455`

This document is **prep-only**. It does not itself change runtime behaviour.

## Why this block exists

Physical Android Pass 5 evidence confirmed that the shared runtime, time-of-day lighting,
pupil adaptation and filter path are alive on device, but also exposed several limitations
that should be corrected before moving to a new large character subsystem.

The next work must close both:
1. previously unfinished eye/device acceptance,
2. newly observed lighting and Living Skin problems.

Do not move to hair, clothing, gameplay, facial animation or a new engine subsystem until
this corrective block is reviewed on device.

---

## Confirmed findings from the physical device test

### 1. Eyelid highlight was misidentified

The bright horizontal edge near the eye is a specular reflection from the eyelid geometry,
not tear-film evidence.

Current device state:
- eyelid-specific material variation is not yet mature,
- eyelashes are absent,
- a visibly readable tear-film/wet-eye gloss cue is absent.

Do not treat the eyelid reflection as wet-rim evidence.

### 2. Direct body specular has a correctness bug

Current GGX direct-light code computes a BRDF-like specular term but adds:

```
specular * sun_radiance
```

without the required final `N·L` light-projection factor.

When the sun moves behind the body:
- `N·L` approaches zero,
- the denominator still uses a small clamped value,
- the direct specular contribution can remain visible or become perceptually excessive.

This matches the physical-device observation that the body can look broadly glossy when
the sun is behind it.

Required correction:
- direct specular energy must be multiplied by the actual front-facing direct-light factor,
- back-facing direct GGX contribution must go to zero,
- any intentional subsurface/back-scatter approximation must remain a separate bounded term.

This correction is required even after a shadow map is added.

### 3. The renderer has camera depth, not light visibility

Current Vulkan renderer has:
- one swapchain colour attachment,
- one camera depth attachment,
- no light-space depth image,
- no shadow sampler,
- no directional shadow pass.

Therefore current direct lighting cannot know whether:
- the head blocks sunlight reaching the torso,
- an arm blocks sunlight reaching the body,
- the character blocks sunlight reaching the ground.

The missing head->torso / arm->body shadows are a renderer visibility problem, not a Genome problem.

### 4. Cornea reflection is being attenuated twice

Current cornea pipeline uses standard alpha blending:

```
srcColor = SRC_ALPHA
dstColor = ONE_MINUS_SRC_ALPHA
```

while the shader intentionally outputs a low alpha:
approximately `0.025 .. 0.38`.

The shader RGB is already a bounded reflection/glare contribution. Standard SRC_ALPHA blending
multiplies that reflection by low alpha again, making the physical-device wet/gloss cue very weak.

Corrective direction:
- use premultiplied-style cornea compositing,
- cornea RGB represents bounded reflection radiance,
- use `srcColor = ONE`, `dstColor = ONE_MINUS_SRC_ALPHA`,
- alpha controls transmitted background/coverage rather than suppressing the reflection twice,
- retain depth test ON / depth write OFF.

Do not increase shell opacity merely to make the glint visible.

### 5. Body-region infrastructure already exists

Canonical experimental BodyRegion ids:

```
head=0
neck=1
chest=2
back=3
abdomen=4
upper_arm=5
forearm=6
hand=7
thigh=8
lower_leg=9
foot=10
unknown=255
```

CPU `SurfaceSample` already uses BodyRegion for pore-density variation.
GPU `human.frag` already mirrors the first pore-density multipliers.

Therefore Living Skin refinement must extend this existing semantic path rather than create a
second region system.

### 6. BodyRegion::head is not an eyelid semantic

The broad head region is insufficient for a pink/warm eyelid margin.

Do not fake eyelids by tinting the entire head.

A separate stable **PeriocularField** is required for finer procedural variation around the eyes.

Initial field may be an analytic head-local/rest-space mask tied to existing eye placement.
It must remain deterministic and must not use screen-space coordinates.

---

## Architectural split

Keep these responsibilities separate.

### Renderer Lighting / Visibility
Owns:
- direct sun,
- sky illumination,
- directional shadow map,
- self-shadowing,
- character->ground cast shadow,
- contact-shadow readability,
- shadow bias/filtering.

### Living Skin
Owns:
- regional roughness,
- regional pore density/scale/depth,
- meso variation,
- redness/perfusion tendencies,
- regional specular scale.

### Periocular Surface
Owns:
- eyelid/lid-margin warm tint,
- local roughness,
- local microstructure,
- later eyelash integration.

### Eye Surface
Owns:
- iris/pupil/sclera,
- cornea,
- tear-film / wet clearcoat response,
- eye diagnostic modes.

---

## Shadow architecture target

Baseline: Vulkan 1.2, Android arm64 + Windows x64.

### Diagnostic ground

Add a temporary ground receiver around the character:
- approximately 10 m x 10 m,
- generated procedurally; no mesh asset required,
- neutral low-frequency material,
- not final scene content,
- used to verify sun direction, cast shadow and contact readability.

Ground should not become part of character storage accounting.

### Directional shadow map

First implementation target:
- one directional-sun depth map,
- body is the primary caster,
- ground is a receiver,
- body is a receiver for self-shadowing,
- no cascaded shadow maps in this first slice,
- no ray tracing.

Recommended prototype baseline:
- 2048 x 2048 on current High path if device cost is acceptable,
- depth-only sampled image,
- fixed character-centred orthographic light volume,
- central useful coverage around the character,
- 3x3 PCF or equivalent bounded filter,
- slope/normal-aware receive bias,
- no screen-space fake shadow.

If 2048 creates measurable mobile pressure, 1024 is an allowed fallback.
Record the actual chosen map size in diagnostics.

### Shared deformation rule

Shadow caster geometry must use the same genome/anatomy deformation as the visible body.

Do not maintain two independent deformation algorithms.

Preferred preparation:
- extract shared body-deformation GLSL helpers into an include used by both
  `human.vert` and the shadow vertex path,
- or otherwise prove byte/logic parity with a deterministic shader contract.

### Descriptor plan

Current binding 0:
- FrameLighting UBO, 112 B.

Keep FrameLightingGpu at exactly 112 B.

Planned shadow binding:
- binding 1: sampled directional shadow depth.

If additional light-space constants are needed, prefer deriving the fixed prototype light basis
from the normalized sun direction and known character-centred bounds before introducing another UBO.

Do not enlarge body or eye push constants above 128 B.

---

## Direct-light correction contract

Before judging shadows visually:

1. correct GGX direct specular:
   `direct_specular = BRDF_specular * sun_radiance * max(NdotL, 0)`

2. apply shadow visibility only to **direct** terms:
   - direct diffuse,
   - direct specular,
   - any direct-only component.

3. do not multiply sky/ambient illumination by the direct shadow map.

4. keep the current subsurface approximation explicitly separate.
   If it creates implausible backlighting after the GGX correction, bound/tune it as a separate slice.

Acceptance:
- with sun behind the body, front-facing skin must not gain broad direct GGX gloss,
- backlit silhouette may still receive sky/ambient and a restrained SSS approximation.

---

## Living Skin regional refinement target

Use existing BodyRegion semantics.

Add bounded CPU/GPU-parity regional controls for at least:
- pore density,
- pore scale,
- pore depth,
- roughness offset,
- specular scale,
- redness/perfusion bias.

Initial qualitative intent:
- head: finer/higher-detail surface, not globally glossy,
- hands: distinct pore/roughness response,
- torso: smoother transition and less uniform specular,
- lower legs/feet: lower pore-density baseline than face/head.

Do not claim anatomical truth for v0 constants; they are render-control approximations to be tuned on device.

### PeriocularField

Add only after broad BodyRegion parity is clean.

Requirements:
- deterministic rest/head-local field,
- localized around the eye/eyelid zone,
- warmer/pinker lid margin,
- distinct local roughness,
- no texture asset,
- no whole-head tint hack.

---

## Eye wet-film corrective target

The physical device currently shows transparency but no convincing wet/gloss response.

Corrective rules:
- preserve separate opaque inner eye and transparent cornea passes,
- preserve source-position smooth normals,
- change cornea blending to physically more appropriate premultiplied-style reflection compositing,
- keep alpha low enough that the shell remains transparent,
- strengthen reflection/glint through reflection energy, not shell opacity,
- maintain Polarized Approx attenuation for reflected sky/glare,
- do not claim full tear-meniscus geometry.

The old inner-eye peripheral wet-rim term may remain only if it contributes visibly and does not
produce a painted white line. If redundant after cornea clearcoat correction, reduce or remove it.

---

## Missing physical acceptance evidence to close

A future short device recording must explicitly include:

1. Eyes Normal.
2. Eyes Components.
3. Iris Only.
4. Cornea Only.
5. side cornea orbit.
6. fixed-camera Morning -> Noon -> Evening -> Night.
7. fixed-Time Clear -> Tinted -> Polarized Approx.
8. backlit sun position showing body direct-light behaviour.
9. ground plane with visible character cast shadow.
10. close-up periocular/wet-eye response.

Final DEVICE PASS is not allowed until these are reviewed.

---

## Bounded execution order

### Slice A — Direct-light correctness + shadow foundation

Scope:
- fix missing NdotL on direct GGX specular,
- introduce shared shadow-space helper contract,
- create directional shadow depth resource/pipeline,
- create diagnostic ground,
- body casts to shadow map,
- body + ground receive first shadow visibility,
- basic bias + bounded PCF.

STOP:
- shaders compile Vulkan 1.2,
- body no longer gains direct GGX gloss from a back-facing sun,
- ground displays a coherent character shadow in runtime,
- head/arm self-shadow path exists,
- Android + Windows build PASS.

Do not add Living Skin tuning in Slice A.

### Slice B — Shadow quality + eye visibility

Scope:
- tune shadow bias/filtering,
- validate backlit silhouette,
- decide whether eye receives directional visibility in v0,
- switch cornea to premultiplied-style reflection compositing,
- make wet/gloss cue visible without increasing shell opacity.

STOP:
- no obvious shadow acne/peter-panning in reference views,
- cornea/wet reflection visible under at least one controlled angle,
- Cornea Only still works,
- CI PASS.

### Slice C — Regional Living Skin parity

Scope:
- central region-material parameter table/helper,
- CPU `SurfaceSample` and GPU shader parity,
- roughness/specular/redness + pore controls by BodyRegion,
- keep zero baked character textures.

STOP:
- deterministic host tests cover region parameter ranges,
- GPU shader compile PASS,
- no new character texture dependency,
- visible body-wide uniform-gloss impression reduced.

### Slice D — Periocular field

Scope:
- stable procedural eye-area mask,
- lid-margin warmth,
- localized roughness/micro variation,
- no eyelashes yet unless required by the test.

STOP:
- eyelid treatment is localized and does not tint the whole head,
- close-up is deterministic/stable during orbit,
- CI PASS.

### Slice E — Physical acceptance

Build APK and capture the ten required views/actions above.

Only after this gate should the project move to the next major Living Skin / anatomy direction.

---

## ABI / regression guards

Must remain:
- Body Push = 128 B,
- Eye Push = 128 B,
- FrameLightingGpu = 112 B,
- 1 frame in flight unless deliberately redesigned,
- Vulkan 1.2 baseline,
- zero mandatory character high-resolution textures,
- zero generated procedural detail stored on disk,
- Android arm64-v8a,
- Windows x64,
- existing eye component split,
- existing deterministic genome/seed contract.

Any new shadow GPU memory must be included in `estimated_gpu_bytes`.

---

## First implementation command

The next implementation pass begins only from this contract and starts with **Slice A**.

Do not combine Slice A with hair, eyelashes, full tear-meniscus geometry, neural rendering,
muscle simulation or other large systems.
