# Rengine Living Surface Genome Prototype v0

Experimental C++23/Vulkan lab. It is deliberately isolated from the canonical DMC reverse-engineering core.

## Current executable slice

The first slice establishes the deterministic contract and an Android arm64 Vulkan runtime lane:

- `LSG0` little-endian binary genome with version, generator revision, size and CRC32.
- two built-in character profiles; current encoded profile size is below 512 bytes and the hard limit is 4096 bytes.
- shared PCG-style integer hash in C++ and GLSL.
- body-region aware procedural surface reference sampler (meso, pore gate, roughness, redness/specular physiology response).
- pixel-footprint detail scheduler (`MACRO`, `MESO`, `MICRO`, `MICRO_HIGH`).
- physiology presets: Normal, Exercise, Cold, Hot.
- Android `NativeActivity`, arm64-v8a only, thin shell; rendering/character logic remains native C++.
- Vulkan 1.2 device gate and swapchain/render-pass bootstrap. The current APK renders a deterministic diagnostic clear surface; human geometry is intentionally not faked before a licensed base mesh is selected.

## Build host tests

```sh
cmake -S labs/lsg -B build-lsg -G Ninja -DRENGINE_LSG_BUILD_VIEWER=OFF
cmake --build build-lsg
ctest --test-dir build-lsg --output-on-failure
```

## Android build

Pinned baseline (September 2026): AGP 9.4.0, Gradle 9.6, Android API 37, NDK r30 `30.0.16248370`, CMake 3.22.1.

```sh
gradle -p labs/lsg/viewer/android :app:assembleDebug
```

Output: `labs/lsg/viewer/android/app/build/outputs/apk/debug/app-debug.apk`.

## Evidence policy

Do not mark hyperrealism, FPS, GPU-memory budget, shimmer stability or device acceptance PASS until measured on a real build/device. `NativeActivity` APK launch + Vulkan initialization is Phase 0 evidence only; surface quality gates remain open.
