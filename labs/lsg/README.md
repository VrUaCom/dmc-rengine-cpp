# Rengine Living Surface Genome Prototype v0

Experimental C++23/Vulkan lab. It is deliberately isolated from the canonical DMC reverse-engineering core.

## Current executable slice

The prototype now establishes the deterministic LSG contract and one shared Windows/Android Vulkan rendering lane:

- `LSG0` little-endian binary genome with version, generator revision, size and CRC32.
- generator revision `3` is the current Human DNA authoring revision. Revision `2` remains an explicit migration input with neutral Face DNA; revision `1` is rejected instead of silently producing a different identity/surface.
- strict `lsg_compile <profile.lsg.json> <profile.lsg>` authoring path with range checking, unknown-field rejection, uint64 seed parsing and post-encode binary self-verification.
- three checked-in development JSON profiles compile to compact canonical binary genomes; generated microdetail remains zero bytes on disk. Male Base and Female Base own the two shared carriers; Character 2 (**Ada**) references the Female carrier and adds only her own Human/Face/Skin DNA. No duplicate Ada body/eye carrier is packaged.
- shared PCG-style integer hash contract in C++ and GLSL. The 64-bit genome seed is folded deterministically into the 32-bit Vulkan seed key so the high half is not silently discarded.
- shared `SkinPhenotype` decoder consumes Skin/MicroDetail/Physiology DNA for every profile; semantic `BodyRegion` labels no longer phase-shift procedural noise or create material seams.
- stable object-space cell/Worley-like pore field with deterministic position, radius, depth and orientation bias; no pore texture is stored on disk.
- pixel-footprint detail scheduler (`MACRO`, `MESO`, `MICRO`, `MICRO_HIGH`) driven from world/object-space derivatives rather than camera distance alone.
- band-limited meso variation, roughness response and low-frequency vascular variation.
- shared skin material library: pigment transport (melanin/haemoglobin/carotene), surface coat/oil/hydration, pores/follicles/freckles/wrinkles, physiology response, dielectric GGX and bounded real-time subsurface transport. The active GPU skin state is one 80-byte UBO shared by the renderer path; there are no per-character skin shader binaries.
- interactive perspective camera with full-body, portrait and extreme-close-up presets.
- continuous `AnatomicalField` deformation replaces hard per-region geometric scaling. `BodyRegion` remains semantic input for surface rules but no longer creates discontinuous geometry transforms at region boundaries.
- Android surface pre-rotation keeps swapchain/native extent separate from the logical camera extent; camera aspect is computed from the logical orientation instead of the pre-rotated swapchain dimensions.
- diagnostic renderer modes: `GENOME_PERSPECTIVE`, `RAW_PERSPECTIVE`, and `RAW_ORTHOGRAPHIC`.
- runtime diagnostics expose window, swapchain and logical extents, surface rotation, aspect, FOV, near/far planes, camera distance and estimated GPU bytes. Android and Windows viewers emit periodic FPS/CPU-frame telemetry to Logcat/stdout.
- `lsg_mesh_audit` validates continuous deformation on the same pinned Base Human RMS0 used by CI. The current pinned asset reports max edge stretch 1.20954x for Character 0 and 1.05174x for Character 1, with zero edges above 1.25x/1.50x.
- physiology presets (Normal, Exercise, Cold, Hot) feed the same SkinPhenotype path on CPU and GPU; genome perfusion/sweat/temperature biases remain profile data rather than shader variants.
- Android `NativeActivity`, arm64-v8a only, thin shell; rendering/character logic remains native C++.
- Vulkan 1.2 device gate and one rendering core for Windows and Android.

The repository intentionally does not claim final hyperrealism, GPU timing, shimmer acceptance, or complete human/eye rendering until those are measured on final licensed base assets and physical devices.

## Device geometry/projection diagnostic protocol

The fourth HUD button cycles:

1. `GENOME_PERSPECTIVE` — continuous genome deformation + perspective camera.
2. `RAW_PERSPECTIVE` — undeformed RMS0 + the same perspective camera.
3. `RAW_ORTHOGRAPHIC` — undeformed RMS0 + diagnostic orthographic projection.

Use this order when validating a physical Android device. If RAW orthographic is correct but RAW perspective is not, investigate camera/projection/orientation. If both RAW modes are correct but Genome perspective is not, investigate the continuous anatomy path. If RAW orthographic is already incorrect, investigate RMS0/index/import/render topology before material or genome work.

Android diagnostics are emitted under Logcat tag `RengineLSG`; Windows diagnostics are printed to stdout. A physical-device visual PASS still requires a real run and cannot be inferred from CI alone.

## Build host tests

```sh
cmake -S labs/lsg -B build-lsg -G Ninja -DRENGINE_LSG_BUILD_VIEWER=OFF
cmake --build build-lsg
ctest --test-dir build-lsg --output-on-failure
```

Compile a development profile explicitly with:

```sh
build-lsg/lsg_compile labs/lsg/assets/profiles/character_0.lsg.json build-lsg/character_0.lsg
```

Audit an RMS0 deformation contract with:

```sh
build-lsg/lsg_mesh_audit human_base.rmesh
```

## Android build

Current stable build baseline (September 2026): AGP 9.4.0, Gradle 9.6, compile/target SDK 36, NDK r30 `30.0.16248370`, CMake 3.22.1. API 37 is not claimed as the runtime baseline while Android 17 remains a preview platform.

```sh
gradle -p labs/lsg/viewer/android :app:assembleDebug
```

Output: `labs/lsg/viewer/android/app/build/outputs/apk/debug/app-debug.apk`.

## Evidence policy

Do not mark hyperrealism, GPU-memory budget, shimmer stability or final device acceptance PASS until measured on a real build/device. Automated C++/shader/APK build evidence is necessary but does not replace physical-device visual and performance evidence. FPS/CPU telemetry is diagnostic only until measured on the target device; current v0 does not yet expose true Vulkan GPU timestamp timing.
