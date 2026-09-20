# Close-up shadow: fixed-view device probe

Base: `a51c4d82815b20a580dbe678ea6069be06dae1fb`, branch
`rengine-living-surface-genome` only.

## Device evidence received

User recording `14701.mp4`, 35.668 seconds, 1080 x 2340.
SHA-256: `fbc6c016ffc8b841278317b9d85ac9252d3f57edd6e47a2180c436990574fb48`.
The recording does not expose a build identifier: association with the preceding
Slice B-A delivery is conversational, not independently verified from the video.

- Around 2–8 s, the male rear neck shadow is soft but retains broad irregular lobes.
- Around 12 s, the side ear silhouette shows actual polygonal geometry.
- Around 21–31 s, the female rear-neck view changes angle and lighting; foreground
  shoulder occlusion at the low angle is not evidence of a broken shadow map.
- No matched Shadow Visibility, Shadow Compare, Normals or Regions captures appear.

Result: **DEVICE ACCEPTANCE OPEN; CAUSE INCONCLUSIVE**. Visible ear faceting is not
sufficient to label the remaining neck defect `CASTER_GEOMETRY_LIMITATION`.
No further filter expansion or caster subdivision is justified by this video alone.

## Implemented capture helper

A twelfth UI row, **T / Test**, starts this sequence at the current camera:

1. Normal (3 s)
2. Shadow Visibility (3 s)
3. Shadow Compare (3 s)
4. Normals (3 s)
5. Regions (3 s)

The sequence pins Noon/Clear, genome perspective and normal eyes. Body, eye and
shadow-caster shaders share a frozen pose time. Character, camera, physiology,
geometry and detail setting are preserved. Captions identify each view. Lighting,
eye and diagnostic settings restore at completion or cancellation. Camera motion,
zoom, another UI action, character/detail change or a second Test tap cancels.
Long press still shows help and does not trigger the sequence. Orientation-driven
renderer recreation ends the probe; restart it after rotating the device.

This is a video-capture aid, not an automated screenshot exporter. Start the phone's
screen recorder, frame the defective neck edge, tap the bottom **T**, and leave the
camera untouched for approximately 15 seconds. Repeat for the other character if
needed. A stall is limited to 0.25 s of capture-clock progress per rendered frame,
so a suspended application cannot skip entire diagnostic views.

## Implementation and verification

- Platform-neutral `ShadowProbe` clock, used by the shared Vulkan renderer.
- Existing test target exercises all five stage boundaries, frozen pose,
  completion, cancellation/restart, backward/nonfinite clock and long stall.
- Existing shadow-filter regression remains: 121 sloped planes, 600 subtexel
  edge steps, blocker rejection and degenerate derivative fallback.
- Human/eye vertex and fragment shaders compile for Vulkan 1.2 locally.
- Body/Eye push constants remain 128 B; FrameLightingGpu remains 112 B.
- HUD uses free bit 25 and 87 vertices; no new GPU images or buffers.
- Surface generation, PCF, bias, geometry and genomes are unchanged.

Platform CI and downloadable artifacts must be tied to this implementation commit.
A build pass does not establish device shadow quality or probe interaction PASS.
