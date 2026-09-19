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

### Wet rim — FAIL

The lower/peripheral wet-eye contribution is not subtle enough.

In close-up and side/front views it becomes a bright white line along the lower eye boundary. This directly violates the Pass 4D requirement:

- no bright white outline,
- wet response should be only a subtle specular cue.

This is the primary visual blocker found by this recording.

Recommended next bounded correction:
- narrow the active edge band,
- substantially reduce constant/sky wet contribution,
- reduce direct-sun wet coefficient,
- reduce grazing contribution,
- clamp the wet contribution before adding it to inner-eye radiance,
- preserve the current opaque inner-eye pipeline and do not reintroduce fake alpha.

### Sclera — CORRECTIVE CANDIDATE

The sclera is no longer technically pure white in the shader, but in the physical recording it still reads visually very bright and relatively flat in several daylight close-ups.

A small additional warm/dim correction may be justified, but it should be tuned only after the wet-rim overbrightness is reduced because the current white rim contaminates perception of the sclera boundary.

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
1. apply a bounded wet-rim corrective slice,
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
