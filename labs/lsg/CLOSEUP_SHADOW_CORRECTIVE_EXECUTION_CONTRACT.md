# Rengine LSG — Close-up Shadow Corrective Execution Contract

Branch: `rengine-living-surface-genome`

Preparation baseline HEAD:
`f0473d3c3dd0ddafd3af17956e2cc276ddcbc6bc`

This commit is prep-only. It must not change runtime rendering.

## 0. Device finding that opens this block

Physical Android review after the Shadow / Normal Corrective pass shows a clear improvement
at ordinary viewing distance, but an unresolved close-range artifact remains.

At strong zoom / cutscene-like framing, the head-to-neck self-shadow boundary can show a
repeated triangular / comb-like edge.

This is important because a defect that is minor in gameplay distance becomes large and
obvious in portrait and cinematic framing.

Do **not** assume the artifact is only shadow-map aliasing.

Current plausible causes are:

1. shadow depth compare / PCF discretization,
2. receive bias / normal-offset interaction,
3. unstable light-space projection,
4. actual polygonal caster silhouette becoming visible at close range,
5. a combination of the above.

Body source-position normal seams and hard procedural BodyRegion phase jumps were already
corrected in the preceding pass, so they are no longer the primary explanation for this
specific close-up comb artifact.

---

## 1. Current shadow baseline

Current renderer has two 2048 x 2048 directional depth targets.

### Coarse cast-shadow map

Coverage:
- half extent = 5.50 m,
- total width = 11.0 m,
- approximately 5.37 mm / texel.

Purpose:
- character -> diagnostic ground,
- broad scene-scale directional visibility.

### Focused self-shadow map

Coverage:
- half extent = 1.20 m,
- total width = 2.40 m,
- approximately 1.17 mm / texel.

Purpose:
- body self-shadowing,
- eye / eyelid direct-light visibility.

Current filtering:
- explicit 3 x 3 PCF.

Current focused receive correction:
- body normal offset approximately 1.0 mm,
- eye normal offset approximately 0.7 mm,
- slope-aware normalized-depth bias.

No new texture assets are involved.

---

## 2. Why a cutscene-specific correction is justified

For a representative 1.75 m subject, current camera presets are approximately:

- Full Body: 4.67 m,
- Portrait: 2.00 m,
- Extreme Close-up: 0.93 m.

Manual zoom can approach:
- 0.32 m minimum camera distance.

The same 1.17 mm shadow texel that is visually small at gameplay distance may occupy
many visible screen pixels at cinematic distance.

Therefore the close-up solution should increase **light-space density**, not merely increase
the global shadow-map resolution.

Do not switch the whole renderer to a 4096 shadow map. That would multiply mobile depth
memory and bandwidth without solving the architectural issue cleanly.

---

## 3. First rule of the implementation pass: classify before tuning

The first bounded implementation slice must add enough diagnostic evidence to answer:

> Is the triangular comb produced by shadow comparison, or by the caster geometry itself?

### Add one extra corrective diagnostic mode

Extend `SurfaceDiagnosticMode` with:

`shadow_compare`

The existing diagnostic packed field may expand from 2 to 3 bits:

- bits 20..22: SurfaceDiagnosticMode,
- bit 23: reserved for close-up shadow mode.

Push-constant size must remain unchanged.

### Shadow Compare view

The view should expose the focused self-shadow comparison signal around the threshold.

Recommended representation:
- receiver clearly lit: neutral/light,
- receiver clearly shadowed: dark,
- near-threshold / bias-sensitive samples: visually distinct warning band.

Exact debug colours are not an artistic contract.

Purpose:
- if the triangular comb appears in `Shadow Visibility` and `Shadow Compare`, it is in
  the light-visibility path;
- if it is absent there but present in Normal, investigate material response;
- if the same shape is visible in the caster-depth edge after projection tightening, suspect
  actual polygonal silhouette.

Keep existing:
- Shadow Visibility,
- Normals,
- Regions,
- Raw Perspective,
- Raw Ortho.

---

## 4. Adaptive focused self-shadow projection

Reuse the existing focused 2048 x 2048 depth target.

Do not allocate a third large shadow map in the first correction.

### Full-body baseline

Keep current:
- half extent about 1.20 m,
- depth half extent about 2.00 m.

### Portrait range

Target initial values:
- half extent about 0.85 m,
- total width about 1.70 m,
- about 0.83 mm / texel.

### Extreme close-up / cinematic range

Target initial values:
- half extent about 0.55 m,
- total width about 1.10 m,
- about 0.54 mm / texel,
- depth half extent about 1.0-1.2 m.

These are prototype tuning values, not physical constants.

### Activation

Do not rely only on CameraPreset because the user can manually zoom.

The renderer should derive a close-shadow level from:
- CameraPreset,
- actual camera distance.

Initial policy:

- Full Body / normal distance -> baseline focused coverage.
- Portrait or distance <= approximately 2.25 m -> portrait coverage.
- Extreme Close-up or distance <= approximately 1.35 m -> cinematic coverage.

Pack the resolved close-up state in currently unused push-flag bits rather than enlarging
the push constants.

---

## 5. Focus the shadow map around what the camera is inspecting

Current focused shadow projection is centred near world/object origin.

For portrait and extreme close-up, centre the focused light volume around the camera target,
especially in Y.

Use existing `camera.target_y_m`.

During the focused shadow pass:
- `pc.camera.xyz` remains the sun direction,
- `pc.camera.w` may carry the focused shadow target Y.

During the visible pass:
- `pc.camera.w` already contains the same camera target Y.

The shadow caster and receiver coordinate functions must use the same focus-centre formula.

Coarse ground shadow remains centred on the broad character/world volume.

---

## 6. Light-space stabilization

The close-up map must not crawl during small camera changes.

For focused/portrait/cinematic shadow projection:

1. build the light-space right/up/forward basis from sun direction;
2. project the chosen focus centre into that basis;
3. compute world-space texel size:
   `2 * half_extent / shadow_map_size`;
4. snap right/up focus coordinates to the texel grid;
5. use the exact same snapped-centre calculation in caster and receiver paths.

Do not use screen-space randomness.

Acceptance:
- very small orbit/zoom changes do not cause obvious shadow-edge crawling.

---

## 7. Close-up filtering

Current 3 x 3 PCF remains the gameplay baseline.

For cinematic close range, start with a deterministic weighted 5 x 5 PCF/tent filter.

Requirements:
- only enable the wider kernel when the close-shadow mode requires it;
- keep coarse ground shadow at the cheaper baseline;
- eye and upper-body self-shadow must share compatible filtering semantics;
- no temporal/random screen-space jitter.

Do not blur so much that head/neck contact becomes detached or visibly floating.

If 5 x 5 creates an unacceptable mobile regression, test a bounded deterministic
16-tap alternative before changing map resolution.

---

## 8. Bias v3

Do not solve the comb artifact by simply increasing global depth bias.

Tune together:

- caster raster depth bias,
- receiver normal offset,
- slope-aware compare bias.

Close-up initial goal:
- smaller, more precise receive bias than broad ground shadow,
- preserve contact under head/jaw,
- no visible peter-panning,
- no broad self-shadow acne.

The `Shadow Compare` diagnostic is the evidence source for this tuning.

---

## 9. Polygonal caster-silhouette stop gate

After:
- tighter focused projection,
- centred/stabilized light volume,
- close-up filtering,
- bias v3,

retest the same rear-head / neck angle.

If the triangular comb remains locked to individual mesh triangles even when the depth signal
is stable and well filtered:

**STOP.**

Do not keep increasing PCF radius.

Classify the remaining defect as a caster-geometry silhouette limitation.

Possible later solutions:
- shadow-only local subdivision/tessellation for head/neck,
- a higher-resolution shared base mesh,
- a bounded contact-shadow complement.

Those are a separate decision and must not be smuggled into this corrective slice.

---

## 10. Eye / eyelid protection

The preceding pass added focused self-shadow receiving to the eye.

The close-up correction must preserve it.

Acceptance:
- upper eyelid can produce a readable thin shadow on inner eye,
- sclera/iris do not remain unnaturally bright when the surrounding face is directly shadowed,
- no eye shadow acne,
- cornea direct highlight respects direct-light visibility,
- Cornea Only diagnostic remains functional.

Do not add eyelashes or full tear-meniscus geometry in this block.

---

## 11. Material and Living Skin freeze

Do not use Living Skin changes to hide this artifact.

During this block do not retune:
- pore density,
- pore scale,
- skin redness,
- regional roughness,
- periocular colour,
- wet-film intensity,
- hair/eyelashes.

The target is shadow quality only.

This keeps visual cause and effect auditable.

---

## 12. GPU / ABI constraints

Must remain:

- Vulkan 1.2 baseline,
- Android arm64-v8a,
- Windows x64,
- Body Push = 128 B,
- Eye Push = 128 B,
- FrameLightingGpu = 112 B,
- one frame in flight,
- zero mandatory high-resolution character textures,
- zero generated procedural microdetail stored on disk.

Prefer reusing the existing 2048 focused self-shadow target.

If GPU memory changes, include it in `estimated_gpu_bytes`.

---

## 13. Automated gates

Required:

### Shader
- human.vert Vulkan 1.2 compile PASS,
- human.frag Vulkan 1.2 compile PASS,
- eye.vert Vulkan 1.2 compile PASS,
- eye.frag Vulkan 1.2 compile PASS.

### Existing geometry
- body source-position duplicate normal delta remains 0,
- eye source-position normal seam gate remains PASS,
- RMS0 audits remain PASS.

### Shadow contract
Add tests/helpers where practical for:
- close-shadow level selection by preset/distance,
- focused half-extent selection,
- finite texel size,
- monotonic density:
  cinematic texel size < portrait texel size < baseline texel size,
- stable snapped light-space centre for tiny camera-distance changes that do not move the target.

### Platform
- Android arm64 APK PASS,
- zero-baked-texture gate PASS,
- Windows x64 runnable viewer PASS.

---

## 14. Physical-device acceptance

Use the same views that exposed the problem.

### Required static captures

1. rear head / neck — Normal;
2. same frame — Shadow Visibility;
3. same frame — Shadow Compare;
4. same frame — Normals;
5. same frame — Regions;
6. side jaw / neck;
7. eye close-up showing upper-eyelid shadow.

### Required motion check

At Extreme Close-up:
- make a very small slow left/right orbit;
- make a small zoom change.

Check:
- no comb-like shadow edge,
- no crawling/shimmer,
- no obvious peter-panning,
- no acne field,
- eye shadow stays attached.

### Distance regression

Also verify:
- Portrait,
- Full Body.

Close-up correction must not break ordinary-distance shadows or the 10 m diagnostic ground.

---

## 15. Bounded implementation order

### Slice A — Diagnostic + adaptive projection
- add Shadow Compare diagnostic,
- add close-shadow level selection,
- pass focus target Y,
- adaptive half extents,
- light-space texel snapping.

STOP:
- shaders compile,
- Android/Windows build,
- diagnostics distinguish the defect source.

### Slice B — Close-up filtering + bias
Only after Slice A evidence:
- weighted 5 x 5 cinematic PCF,
- bias v3,
- eye-compatible focused filtering.

STOP:
- CI PASS,
- no obvious code-level regressions.

### Slice C — Device acceptance
Build APK and retest the exact problem angles.

If comb persists as triangle-locked caster silhouette:
- stop this block,
- document `CASTER_GEOMETRY_LIMITATION`,
- design a separate geometry-level solution.

---

## 16. Success definition

Success is **not** merely making the shadow softer.

Success means:

- ordinary-distance shadow quality stays intact,
- extreme close-up no longer exposes a comb/jagged head->neck shadow boundary,
- the edge remains stable during small camera motion,
- eyelid->eye shadow remains readable,
- diagnostics prove the remaining signal is no longer a bias/filter artifact,
- no extra per-character baked assets are introduced.

The next implementation command starts with Slice A only.
