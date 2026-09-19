# Close-up Shadow Slice B-A — evidence and implementation

Branch: `rengine-living-surface-genome` only.
Baseline: `d90ccb92bd790a0eada7314da7463ae83ea5f1b5`.
Status: implementation candidate; physical retest required. No DEVICE PASS.

## Device evidence used

User supplied `14577.mp4`, identified as the Shadow Slice A test.
SHA-256: `01bed213b83837abb498ed1ff7164c67693000b6eeaafa9947df3e8d969ae70e`.
Duration 134.275 seconds, 1080 x 2340. The recording does not expose an APK
commit identifier; its Slice A identity is user-provided, not binary-verified.

Observed extracted frames (timestamps in seconds):

| Time | Mode / observation |
| --- | --- |
| 8 | Normal, rear head/neck: comb-like boundary persists at close range. |
| 58–60 | Shadow Visibility: repeated steps/teeth on forehead, nose, jaw and neck; the defect is in visibility, independently of skin colour. |
| 62–66 | Shadow Compare: orange near-threshold samples around face and shadow boundaries, including serrated forehead/neck edges. |
| 70–72 | Normals: smooth interpolated fields; no corresponding repeated fine comb in the normal field. |
| approximately 76 | Regions: broad semantic areas, not the fine repeated shadow edge. |

The camera/time state changes across the recording. Most of the diagnostic
sequence uses Night; it is not a registered, identical-camera Noon comparison
of the original rear-neck defect. The evidence supports a bounded B-A
filter/bias experiment, **not** proof that caster geometry has no remaining
limitation. Region/normal views are corroboration, not pixel-matched proof.

## Code-level findings

1. The existing 3x3 PCF compares all neighboring depth texels against one
   receiver depth. On a sloped receiver this can produce false self-occlusion.
2. Fixed nearest-texel PCF weights change discretely as the kernel crosses a
   texel boundary.
3. Normal rendering passes the procedural micro-normal into the shadow offset
   and bias, while Shadow Visibility and Compare use the smooth mesh normal.
   Thus diagnostic visibility and actual shaded visibility can disagree.

Filtering depth comparisons rather than averaging depth is retained; see the
[NVIDIA GPU Gems discussion of PCF and slope/bias limitations](https://developer.nvidia.com/gpugems/gpugems3/part-ii-light-and-shadows/chapter-8-summed-area-variance-shadow-maps).

## Implemented scope

- Cinematic only: shared body/eye deterministic 5x5 tent PCF. Subtexel weights
  vary continuously, with zero-weight taps at recentering boundaries.
- Per-tap receiver-plane depth correction from the shadow-coordinate Jacobian.
  Singular/nonfinite derivatives fall back to zero; gradient is bounded.
- Bias v3 uses metre-based offsets/depth bias, converted to focused depth range.
  Body normal offset <= one shadow texel (approximately 0.537 mm cinematic);
  eye offset <= 0.7 texels (approximately 0.376 mm).
- Body depth bias 0.20–0.60 mm; eye 0.15–0.45 mm, depending on NdotL.
  Per-tap correction capped at 2 mm for body / 1 mm for eyes to limit leaks.
  These are prototype controls, not physical constants.
- Shadow Compare uses the same corrected central tap as cinematic visibility.
- Normal rendering uses the smooth mesh normal for shadow receiving; pore
  micro-normals continue to affect the BRDF only.
- Baseline/portrait retain the existing 3x3 filter and offsets. Coarse ground
  path, raster bias, projection and geometry are unchanged.

No new shadow target, mesh, character texture, cache, descriptor or runtime
GPU allocation. Body/Eye push constants stay 128 B; lighting UBO stays 112 B.
Shader code grows; this is not a claim of zero binary-size overhead.

## Automated evidence

Local GNU C++23 Debug core tests: PASS (2/2).
Local Vulkan 1.2 shader compilation: PASS for human.vert, human.frag,
eye.vert and eye.frag (shaderc 2023.8).
CPU reference tests remain active in Release builds:

- 121 unoccluded analytic-plane subtexel positions: all remain fully lit with
  the correction; all 121 exhibit false shadowing without per-tap correction.
  These fixtures are within the configured correction cap.
- 600-step fixed-edge subtexel sweep: monotonic response, largest visibility
  increment 0.004161; no discrete kernel-recentering jump.
- Real foreground blocker, bounded grazing correction, singular/nonfinite
  Jacobian and mirrored/scaled derivatives: PASS.

The correction cap intentionally limits extreme grazing cases. These tests
validate the CPU reference mathematics, not rendered GPU/device equivalence.
Android/Windows shader compilation and packaging are separate CI gates.
No FPS/GPU-time improvement is claimed: cinematic taps increase from 9 to 25.

First CI revision: `8e76fce42487ab5c5e719268b25846ec6021bbf0`.
[Android run 35472802130](https://github.com/VrUaCom/dmc-rengine-cpp/actions/runs/35472802130)
passed all core/geometry/shader/APK gates. Windows compiled the viewer and
shaders, but CTest could not run the new shadow test because its executable
was omitted from the workflow's explicit build-target list. The follow-up
adds that target; it does not change the runtime. Both workflows must pass
on the follow-up commit before delivery. Local `-O2 -DNDEBUG` shadow tests
also passed, so these regression checks are confirmed active in Release.

## Physical retest / stop gate

Use Character 0, **Noon / Clear**, same rear head -> neck close-up. Capture
Normal, Shadow Visibility and Shadow Compare without changing camera/light,
then slow orbit and a small zoom. Repeat the face boundary seen at 60–66 s.
Check Character 1, eye/upper-eyelid contact and Portrait/Full Body regression.

If stable triangle-locked edges remain, stop filter enlargement and classify
the remaining issue for B-B (`CASTER_GEOMETRY_LIMITATION`). Do not blur away a
geometry problem. Wet-film/skin/hair work remains outside this slice.
