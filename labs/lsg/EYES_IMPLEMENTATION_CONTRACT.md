# Rengine LSG — Eyes Foundation Implementation Contract

Branch scope: `rengine-living-surface-genome` only.

## Goal

Implement the first physically meaningful LSG eye path without baked high-resolution eye textures and without changing the canonical LSG0 genome layout.

The v0 eye gate is:
- shared fitted eye geometry,
- separate cornea/wet-shell response,
- procedural iris,
- procedural pupil,
- sclera tint/vascular variation from EyeGenome,
- luminance-driven pupil response with temporal smoothing,
- Windows + Android parity,
- no mandatory character eye texture.

## Pinned source assets

MakeHuman commit:
`a8bc2d54ff0ac92e78ff71431b1023eda42bf482`

CC0 asset source headers are embedded in the MakeHuman OBJ/MHClO files.

High-poly eye:
- `makehuman/data/eyes/high-poly/high-poly.obj`
  - Git blob: `01562a9caf4dca9ebb1fd5c24db083c17e724330`
  - 1064 vertices
  - 1020 quad faces
  - 4 disconnected components, structurally two per eye
- `makehuman/data/eyes/high-poly/high-poly.mhclo`
  - Git blob: `22bc5f77f398c59088804f7f4c9bb0e39d38661d`
  - exactly 1064 proxy mapping rows
  - all mappings are triple-reference mappings
  - maximum referenced hm08 vertex index: 14738

Low-poly fallback:
- `makehuman/data/eyes/low-poly/low-poly.obj`
  - Git blob: `ec35d14bd580ed9ec80ac57d48a4f01377b5f899`
  - 96 vertices
  - 86 quad faces
- `makehuman/data/eyes/low-poly/low-poly.mhclo`
  - Git blob: `fdf7e4089fa66d7dcd6fc277d028884fbce1f410`

The existing MakeHuman brown eye texture/material MUST NOT become an LSG runtime dependency.

## MHClO fitting contract

Reproduce MakeHuman proxy fitting in C++23:

```
proxyVertex =
    base[v0] * w0 +
    base[v1] * w1 +
    base[v2] * w2 +
    ScaleMatrix * offset
```

For the scale matrix:
- x_scale maps to matrix[0][0]
- y_scale maps to matrix[1][1]
- z_scale maps to matrix[2][2]

Each axis scale is:

```
abs(base[vn1][axis] - base[vn2][axis]) / denominator
```

This tool operates on the fully morphed profile OBJ before `g body` extraction, so the eye geometry follows the male/female macro body and female bust/profile pipeline without manual authoring.

Proposed tool:
`lsg_fit_mhclo <profile.obj> <eye.obj> <eye.mhclo> <fitted-eye.obj>`

Hard validation:
- OBJ vertex count == MHClO mapping row count
- all referenced base indices in range
- weights finite
- offsets finite
- all output positions finite
- topology and UVs preserved
- deterministic byte-identical result for identical inputs

## Geometry representation

Reuse RMS0 for v0; do not create a second binary mesh format yet.

Add an eye-specific preparation tool or mode that:
- triangulates the fitted eye OBJ,
- regenerates normals/tangents,
- preserves UVs,
- assigns experimental eye region tags/component tags outside the body-region semantic path.

Do not weaken body-region validation globally without an explicit eye-mesh policy.

High-poly eye is the default v0 target for both Desktop and Mobile High because its topology cost is small relative to the body mesh. Low-poly remains a Mobile Safe fallback.

## Component semantics

High-poly has four disconnected components: two per eye.

Observed structure:
- one component per eye protrudes slightly farther forward,
- one component per eye sits slightly deeper.

Working hypothesis:
- outer component = cornea / wet shell,
- inner component = sclera + iris/pupil substrate.

This is STRUCTURAL_CANDIDATE only until diagnostic rendering confirms it.

The implementation MUST include a diagnostic component-colour mode before permanently binding semantic labels.

## Genome contract

Do not change LSG0 version for Eyes Foundation.

Existing `EyeGenome` already provides:
- iris primary RGB,
- iris secondary RGB,
- pupil_bias,
- sclera_tint,
- vascularity,
- eye_seed.

The current genome remains compact and generator-revision controlled.

## Derived eye parameters

Add an eye-specific derivation path, separate from skin/body parameters.

Suggested:
```
struct DerivedEyeParameters {
    float iris_primary[3];
    float iris_secondary[3];
    float pupil_bias;
    float sclera_tint;
    float vascularity;
    uint32_t eye_seed_low;
};
```

Fold `eye_seed` using the same deterministic seed policy used elsewhere.

## Runtime state

Eye runtime state is NOT stored in the genome.

Minimum:
```
struct EyeRuntimeState {
    float scene_luminance;
    float target_pupil_radius;
    float pupil_radius;
};
```

Pupil response:
- map scene luminance to a bounded target radius,
- apply exponential temporal smoothing,
- no frame-rate-dependent linear stepping,
- no medical accuracy claim.

## Renderer architecture

Do not enlarge the existing 128-byte body push constant beyond the Vulkan 1.2 guaranteed minimum.

Implement a separate eye draw path/pipeline with its own EyePush <= 128 bytes.

Recommended passes:
1. body pass,
2. inner-eye pass,
3. cornea/wet-shell pass,
4. UI pass.

The eye pass uses the same camera/projection contract and the same Vulkan renderer on Windows and Android.

## Eye shading v0

### Inner eye
Sclera:
- off-white base, never pure white,
- subtle genome-driven tint,
- low-amplitude deterministic vascular variation,
- no baked sclera texture.

Iris:
- procedural radial fibres,
- angular seeded variation,
- radial colour gradient between primary/secondary EyeGenome colours,
- band-limited/filtered at distance,
- stable object/local-eye coordinates,
- no screen-space random.

Pupil:
- procedural aperture/dark region,
- radius from smoothed EyeRuntimeState,
- pupil_bias offsets the response.

### Cornea / wet shell
- transparent/specular dielectric layer,
- IOR-like response around real corneal range as an approximation,
- strong environment/sun highlight,
- no baked gloss texture,
- initial v0 may use simplified refraction; do not label it full physical refraction.

## Eye local coordinates

Do not derive iris pattern in screen space.

For each eye:
- derive a stable eye center and forward axis from fitted geometry/component bounds,
- compute local radial coordinates,
- use `eye_seed` + side id for deterministic variation.

Left/right eyes share genome controls but MUST NOT produce pixel-identical fibre layouts; derive side-specific seed keys deterministically.

## UI / diagnostics

Extend the compact R&D panel with an Eyes entry after the current Physiology row.

Short tap:
- cycle eye diagnostic state:
  - EYES NORMAL
  - COMPONENTS
  - IRIS ONLY
  - CORNEA ONLY

Long press/hover:
- tooltip describing the current eye function.

Camera:
- existing Extreme Close-up is reused for acceptance.
- no new complex camera system in this slice.

## Tests

Host:
- MHClO parser rejects malformed mapping
- mapping row count matches eye OBJ vertex count
- all referenced base vertices valid
- deterministic fitted OBJ hash
- left/right eye components preserved
- no NaN/Inf
- eye seed deterministic
- different eye seeds produce different CPU reference iris samples
- pupil smoothing converges and stays bounded
- EyePush <= 128 bytes

Shader:
- eye.vert compiles Vulkan 1.2
- eye.frag compiles Vulkan 1.2

Android/Windows:
- fitted profile 0 eyes generated
- fitted profile 1 eyes generated
- eye RMS0 assets packaged
- no PNG/JPG/DDS/KTX eye texture dependency
- viewer links and runs

## Evidence / acceptance gate

Eyes Foundation PASS requires:
- both characters show fitted eyes in correct sockets,
- no obvious floating/intersecting eye geometry,
- separate visible cornea highlight,
- procedural iris visibly differs between Character 0 and Character 1,
- pupil radius changes smoothly with scene luminance/debug luminance control,
- sclera is not flat pure white,
- eye close-up is clearly more than one textured sphere,
- Android device run has no corruption/crash,
- Windows build PASS,
- existing body/macro/physiology tests remain PASS.

Pixel-identical cross-GPU rendering is not required.

## 20-minute implementation slices

### Slice A — MHClO fitter
Implement `lsg_fit_mhclo`, deterministic tests, pinned high-poly source verification.

Stop gate:
fitted eyes generated for profile 0 and profile 1 with deterministic hashes.

### Slice B — Eye RMS0 + component diagnostics
Prepare eye RMS0, preserve/classify four components, add component-colour diagnostic.

Stop gate:
both eyes render in the correct sockets on Windows/Android without semantic assumptions about cornea yet.

### Slice C — EyeGenome runtime + procedural inner eye
Add DerivedEyeParameters, eye seed reference generator, sclera/iris/pupil shader.

Stop gate:
iris is procedural, stable, deterministic, no baked eye texture.

### Slice D — Cornea + pupil response + UI
Add outer-shell shader, pupil temporal smoothing, Eyes R&D row/tooltip/diagnostics.

Stop gate:
close-up passes visual acceptance on Android and Windows CI is green.

### Slice E — physical device acceptance
Build APK and capture:
- Character 0 eye close-up
- Character 1 eye close-up
- component diagnostic
- iris-only diagnostic
- cornea-only diagnostic
- pupil response sequence

No progression to the next large character subsystem until this gate is reviewed.
