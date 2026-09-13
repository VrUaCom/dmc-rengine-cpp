# DMC Rengine Runtime — Android

Gradle project for the runtime host layer on Android. It is the first playable
*target*, not a playable game: the layer runs a frame loop and a lifecycle, and
executes no DMC3 game logic.

## What this build proves

- the core library and the runtime layer compile for `arm64-v8a` and `x86_64`;
- the Android lifecycle drives the native loop: surface creation, resize,
  destruction and recreation, pause/resume, focus and low-memory;
- a destroyed surface returns the device to a revivable state instead of
  failing the run;
- resuming from a backgrounded process does not replay the suspended time.

## What it does not do

There is no GPU backend. The device is the reference `null` backend, which
validates and records frames without presenting, so the activity shows a build
report rather than a rendered scene. A blank screen would be indistinguishable
from a crash; the report says what the build actually is.

## Layout

```text
android/
  CMakeLists.txt                 NDK entry point: core + runtime, no CLI, no CTest
  settings.gradle.kts
  build.gradle.kts
  app/build.gradle.kts           minSdk 26, targetSdk 36, 64-bit ABIs only
  app/src/main/AndroidManifest.xml
  app/src/main/java/com/vruacom/dmcrengine/runtime/
      RengineRuntime.java        JNI facade
      RuntimeActivity.java       lifecycle -> native events, loop thread
```

The Java shell owns no runtime policy. It translates callbacks into native
events and drives `tick`; pacing, surface state and rendering all live in C++.

## Build

```bash
cd android
gradle assembleDebug
```

Requires JDK 17, the Android SDK (API 36) and an NDK with CMake 3.25 or newer.
The native standard is selected through
`-DDMC_RENGINE_RUNTIME_CXX_STANDARD` (default `23`, see
[Specification 010](../specs/010-runtime-platform-foundation/spec.md)).

## Status

CI assembles the debug APK. **Physical-device acceptance has not been recorded**
— per the project's completion policy, an assembling APK is not an accepted
build.
