# Rengine LSG — Pass 5 Physical Device Acceptance Report

Branch: `rengine-living-surface-genome`

Tested runtime HEAD:
`8a82a911ddd5c80e9562033bf1715c059748311d`

Evidence:
- physical Android screen recording supplied by the user,
- duration approximately 137 seconds,
- portrait 1080x2340 capture,
- APK preflight SHA-256: `774b3627e69e337f9b9953e38ae8b746a46ba41ba7db889a59f6feb49a7284ce`,
- Android CI run 35432254843: PASS,
- Windows CI run 35432254749: PASS.

## Status

**CORRECTIVE_NEEDED**

This recording is valid physical-device evidence, but it does not satisfy the full final DEVICE PASS contract.

## Confirmed on physical Android

### Runtime stability — PASS

The application remains running throughout the recording. No crash, obvious memory corruption, broken swapchain, or catastrophic eye-geometry failure is visible.

### Both character profiles — PASS

Both Character 1 and Character 0 are exercised in the recording and retain fitted eyes in the head during close-up/orbit use.

### Camera / extreme close-up / orbit — PASS

Close-up inspection and strong side orbit are exercised successfully.

### Time-of-day runtime — PASS

The Time row visibly cycles through all four encoded UI states:
- Morning,
- Noon,
- Evening,
- Night.

The scene response changes materially; this is not a UI-colour-only state change.

### Pupil adaptation — PASS

At close range the pupil visibly changes size as effective scene illumination changes. Dark presets produce a larger visible pupil than bright daylight states, and the response is temporally smoothed rather than an obvious one-frame hard cut.

### Optical filter runtime — PASS

The Filter row visibly reaches all three encoded states:
- Clear,
- Tinted,
- Polarized Approx.

The rendered scene changes with the selected optical filter. The recording therefore confirms that the GPU filter path is live on the physical Android device.

### Cornea faceting / blue shell — PROVISIONAL PASS

Strong side views do not show the previous obvious polygonal blue shell. The eye remains seated during orbit and there is no obvious transparency-order corruption in Normal mode.

This remains provisional because the recording does not exercise Cornea Only mode.

### Iris / pupil proportion — PROVISIONAL PASS

The corrected iris and bright-state pupil are visibly smaller than the earlier device-test baseline and no longer produce the previous extreme doll-eye appearance.

Final proportion acceptance remains visual/tuning-sensitive rather than a biological measurement claim.

## Corrective findings

### Wet rim — FAIL (not visibly present)

User physical-device observation supersedes the earlier frame interpretation: the intended wet-rim cue is not visibly present on the real display.

The bright lower-eye line previously interpreted from compressed video frames is **not accepted as wet-rim evidence**.

The current issue is therefore the opposite of overbrightness:
- the wet-rim approximation is too weak or spatially misplaced to read as a subtle wet contact cue,
- it must become visible without turning into a bright white outline.

Recommended next bounded correction:
- preserve the current opaque inner-eye pipeline,
- increase wet response only in the intended exposed lower/peripheral contact zone,
- keep the active band thin,
- prefer directional/specular response over constant brightness,
- cap the contribution so it remains subtle,
- do not reintroduce fake alpha or full tear-meniscus geometry.

### Sclera — CORRECTIVE CANDIDATE

The sclera is no longer technically pure white in the shader, but in the physical recording it still reads visually very bright and relatively flat in several daylight close-ups.

A small additional warm/dim correction may be justified, but it should be tuned independently from the wet-rim correction because the intended wet-rim cue is currently not visible on the physical display.

## Missing evidence in this recording

The Eyes diagnostic row remains in its Normal-state colour throughout the sampled recording.

Therefore this recording does **not** provide physical-device evidence for:
- Components,
- Iris Only,
- Cornea Only.

These three modes were previously code/CI validated, but final Pass 5 requires physical visual evidence.

## Final gate

Do not mark the Eye/Lighting Corrective block DEVICE PASS yet.

Required before final closure:
1. apply a bounded wet-rim visibility corrective slice,
2. rebuild Android/Windows and keep all existing CI gates green,
3. run a short physical-device retest,
4. include at least:
   - Normal,
   - Components,
   - Iris Only,
   - Cornea Only,
   - side cornea view,
   - one fixed-camera Time sequence,
   - one fixed-Time Filter sequence.

Only after those items pass should work advance to Living Skin / Detail-on-Demand refinement.


## Additional physical-device observations from user review

These observations supersede any interpretation based only on compressed video frames.

### Eyelid highlight clarification

The bright horizontal/light line visible near the eye is not wet-rim evidence.

The user identified it on the physical display as a specular reflection from the relatively flat/horizontal eyelid surface. The current character does not yet have:
- eyelid-specific texture/surface variation,
- eyelashes,
- a visibly readable tear-film/wet-eye gloss layer.

Therefore:
- do not use that eyelid highlight as evidence for tear-film response,
- wet-eye gloss remains visually absent in the current device test,
- eyelid material response must be treated separately from cornea/tear-film response.

### Missing local/self shadowing — major lighting finding

The user reports that direct-light/time-of-day changes are clearly visible, but local self-shadowing is not convincing.

Observed symptoms:
- little/no readable head shadow falling across the torso,
- little/no arm shadow falling across the body,
- when the sun moves behind the character, the body can become broadly glossy/bright instead of reading as backlit with strong front-side occlusion.

This indicates that the current shared sun/sky shading path is not equivalent to a real shadowing solution.

The distinction is architectural:

1. **Character material shading**
   - BRDF,
   - skin roughness/specular,
   - subsurface approximation,
   - region-dependent procedural surface variation.

2. **Lighting / visibility**
   - cast shadows,
   - self-shadowing,
   - occlusion between body parts,
   - ground/contact shadows,
   - shadow bias/filtering.

Self-shadowing belongs to the renderer/lighting system, not to the genome identity and not to a fake baked character texture.

### Diagnostic ground plane requested

Add a simple temporary diagnostic ground receiver around the character, approximately 10 m in useful visible extent, for physical-device validation.

Purpose:
- verify whether the character casts a coherent shadow,
- verify sun direction,
- reveal missing contact shadowing,
- distinguish material brightness problems from visibility/shadow problems.

This is a diagnostic environment feature, not final scene content.

### Living Skin / shader-surface refinement requested

The user explicitly wants further procedural shader refinement of the body rather than baked character textures.

Target variation should include, by anatomical/semantic region:
- lighter/darker skin response,
- smoother/rougher areas,
- pore density/scale variation,
- local redness/perfusion tendencies,
- less uniform body-wide specular response.

Examples:
- eyelid/lid margin should read warmer/pinker than surrounding skin,
- different body regions should not share identical roughness and pore character,
- current procedural surface should become anatomically differentiated while preserving the zero mandatory per-character high-resolution texture goal.

This belongs to the Living Skin / Detail-on-Demand system.

### Current subsystem split after device test

**Eye Surface**
- tear-film / wet-eye gloss,
- cornea response,
- iris/pupil/sclera,
- eyelid-eye contact cue.

**Eyelid / Periocular Skin**
- lid-margin colour,
- local roughness,
- local microstructure,
- later eyelashes.

**Living Skin**
- anatomical procedural roughness,
- pore and meso variation,
- regional colour/perfusion variation,
- region-aware specular response.

**Renderer Lighting**
- direct/sky illumination,
- cast shadows,
- self-shadowing,
- contact shadows,
- diagnostic ground receiver.

These systems should remain separate even though they are reviewed together visually.

## Revised next-step recommendation

Do not treat the next work item as only a wet-rim correction.

Use two bounded technical slices:

### Slice A — Shadow/visibility diagnostic foundation
- temporary diagnostic ground plane,
- first real directional-sun shadow path,
- body self-shadowing,
- ground/contact shadow visibility,
- validate backlit view.

### Slice B — Living Skin + periocular refinement
- region-aware procedural roughness/pore/redness,
- eyelid/lid-margin warm tint,
- reduce uniform body-wide gloss,
- make tear-film/wet-eye specular actually readable.

Hair/eyelashes may remain later work unless they become necessary for the current visual gate.
