# Rengine Living Surface Genome Prototype v0

Experimental C++23/Vulkan lab. It is deliberately isolated from the canonical DMC reverse-engineering core.

## Current executable slice

The prototype now establishes the deterministic LSG contract and one shared Windows/Android Vulkan rendering lane:

- `LSG0` little-endian binary genome with version, generator revision, size and CRC32.
- generator revision `2` is the first revision for the stable cell-based pore field. Pre-v0 revision `1` files are rejected explicitly instead of silently producing a different surface; a future migration layer can convert archived revisions when real persisted profile assets exist.
- strict `lsg_compile <profile.lsg.json> <profile.lsg>` authoring path with range checking, unknown-field rejection, uint64 seed parsing and post-encode binary self-verification.
- two checked-in development JSON profiles compile to compact canonical binary genomes; generated microdetail remains zero bytes on disk.
- shared PCG-style integer hash contract in C++ and GLSL. The 64-bit genome seed is folded deterministically into the 32-bit Vulkan seed key so the high half is not silently discarded.
- body-region aware procedural surface reference sampler and matching shader logic.
- stable object-space cell/Worley-like pore field with deterministic position, radius, depth and orientation bias; no pore texture is stored on disk.
- pixel-footprint detail scheduler (`MACRO`, `MESO`, `MICRO`, `MICRO_HIGH`) driven from world/object-space derivatives rather than camera distance alone.
- band-limited meso variation, roughness response and low-frequency vascular variation.
- genome-driven skin controls and a dielectric GGX + wrapped/preintegrated-style subsurface approximation.
- interactive perspective camera with full-body, portrait and extreme-close-up presets.
- physiology presets exist in the C++ reference runtime: Normal, Exercise, Cold, Hot. Full GPU/UI physiology wiring remains open.
- Android `NativeActivity`, arm64-v8a only, thin shell; rendering/character logic remains native C++.
- Vulkan 1.2 device gate and one rendering core for Windows and Android.

The repository intentionally does not claim final hyperrealism, performance acceptance, shimmer acceptance, or complete human/eye rendering until those are measured on final licensed base assets and physical devices.

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

## Android build

Current stable build baseline (September 2026): AGP 9.4.0, Gradle 9.6, compile/target SDK 36, NDK r30 `30.0.16248370`, CMake 3.22.1. API 37 is not claimed as the runtime baseline while Android 17 remains a preview platform.

```sh
gradle -p labs/lsg/viewer/android :app:assembleDebug
```

Output: `labs/lsg/viewer/android/app/build/outputs/apk/debug/app-debug.apk`.

## Evidence policy

Do not mark hyperrealism, FPS, GPU-memory budget, shimmer stability or final device acceptance PASS until measured on a real build/device. Automated C++/shader/APK build evidence is necessary but does not replace physical-device visual and performance evidence.
