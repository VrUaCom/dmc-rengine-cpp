# Rengine LSG — Close-up Shadow Slice B Decision Gate

Branch: `rengine-living-surface-genome`

Preparation baseline HEAD:
`2d89c1ac7b4857843d8aa3693ca8ece576ced60a`

This commit is prep-only. No runtime rendering changes.

## 1. Current status

Slice A is CI-complete:

- adaptive focused self-shadow projection,
- baseline / portrait / cinematic close-shadow levels,
- target-Y centering,
- light-space texel snapping,
- Shadow Visibility diagnostic,
- Shadow Compare diagnostic,
- Android arm64 PASS,
- Windows x64 PASS,
- Vulkan 1.2 shader PASS,
- zero baked character textures PASS.

The next implementation must be selected from physical-device evidence.

## 2. Evidence required from device

At the same rear head -> neck close-up that previously exposed the comb artifact, capture:

1. Normal
2. Shadow Visibility
3. Shadow Compare
4. Normals
5. Regions

Also inspect a very small slow left/right orbit and a small zoom change.

## 3. Decision rule A — FILTER / BIAS path

Choose Slice B-A only if the defect is present in Shadow Visibility and is accompanied by a
near-threshold pattern in Shadow Compare.

Typical evidence:

- comb or chatter tracks depth comparison,
- orange warning band follows the problematic edge,
- Normals are smooth across the same area,
- Regions do not explain the pattern.

### Slice B-A implementation scope

1. cinematic-only weighted 5x5 deterministic PCF;
2. keep baseline/portrait on current 3x3 unless evidence supports otherwise;
3. bias v3:
   - receiver normal offset,
   - slope-aware compare bias,
   - raster depth bias only if needed;
4. body and eye use compatible focused-shadow semantics;
5. no temporal/random jitter;
6. preserve contact under head/jaw;
7. no peter-panning.

### Slice B-A stop gate

- Android PASS;
- Windows PASS;
- Vulkan shaders PASS;
- eye shadow path PASS;
- no material/Living Skin retuning;
- device retest required before any larger shadow-system change.

## 4. Decision rule B — CASTER_GEOMETRY_LIMITATION path

Choose Slice B-B if:

- Shadow Compare is stable and not dominated by threshold chatter,
- Shadow Visibility still follows repeated triangle-shaped geometry,
- the pattern remains locked to caster triangles as the camera moves,
- Normals/Regions do not explain it.

Do not increase PCF radius to hide it.

### Slice B-B preparation target

Document the defect as:

`CASTER_GEOMETRY_LIMITATION`

Then evaluate, in this order:

1. shadow-only local subdivision for head / jaw / neck silhouette;
2. higher-density shared base mesh only if subdivision is insufficient;
3. bounded contact-shadow complement only if geometry alone cannot meet cutscene quality.

### Slice B-B constraints

- do not alter the canonical visible mesh in the first experiment;
- do not introduce per-character baked assets;
- do not add a third full-resolution shadow map;
- keep shadow-only geometry deterministic and shared;
- preserve current body and eye shading ABI.

## 5. Decision rule C — NORMAL / MATERIAL path

If the artifact is absent in Shadow Visibility but visible in Normal render:

- inspect Normals first;
- inspect Regions second;
- do not touch shadow filtering until the actual source is identified.

Possible classifications:

- `NORMAL_FIELD_REGRESSION`
- `MATERIAL_RESPONSE_DISCONTINUITY`

These are separate bounded slices.

## 6. Fixed constraints for the next pass

Must remain:

- Vulkan 1.2;
- Android arm64-v8a;
- Windows x64;
- Body Push = 128 B;
- Eye Push = 128 B;
- FrameLightingGpu = 112 B;
- one frame in flight;
- 2048 focused self-shadow target;
- zero mandatory character textures;
- zero generated microdetail on disk.

## 7. Material freeze

Do not change during shadow diagnosis:

- pore density,
- pore scale,
- skin redness,
- roughness distribution,
- wet-film response,
- eye colour,
- eyelashes,
- hair.

## 8. Next-command behavior

On the next user command to start work:

- if fresh device screenshots/video are available, classify them first and execute only the matching path;
- if no new device evidence is available, do not guess the cause and do not implement 5x5 PCF blindly;
- remain on `rengine-living-surface-genome` only.
