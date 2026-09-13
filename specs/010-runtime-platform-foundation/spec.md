# Specification 010 — Runtime Platform Foundation

## Status

Initial implementation. Host targets validated; Android device acceptance and
every GPU backend remain open.

## Problem

DMC Rengine can resolve, materialize, inspect and assemble DMC3 resources, but
it has nowhere to *run* them. Every consumer so far is a tool: a CLI, an
inspector, an editor. A playable target — Android first, desktop and other
platforms after — needs a host layer that owns a window/surface, a frame loop
and a rendering device.

Building that inside an existing component would break the canonical
architecture. A runtime that resolves its own files becomes a second resolver
(Article I). A runtime that assembles its own scene becomes a second scene
truth (Architecture rule 5). This specification defines the runtime as its own
responsibility boundary with both doors closed.

## Goals

- a host abstraction covering surface lifetime, lifecycle and time;
- a fixed-step frame loop independent of host frame rate;
- a rendering backend abstraction with an implemented reference backend;
- a single resource door onto GDSpaces;
- a runtime mirror of Stage Ops `StageBundle` state;
- an Android shell and build;
- C++23 for the runtime target, with graceful degradation to C++20;
- deterministic tests that need no display and no GPU.

## Non-goals

- **any claim of DMC3 behavioral equivalence.** Nothing here executes original
  game logic;
- a GPU backend. Vulkan, GLES, D3D11 and Metal are *declared*, not implemented;
- parsing DMC3 formats. The runtime consumes bytes and typed bundles only;
- scene assembly, resource resolution or container expansion;
- audio, input mapping, physics or gameplay systems;
- moving the core library off its C++20 baseline;
- shipping game content. The runtime reads user-supplied resources through
  GDSpaces like every other consumer.

## Architecture

A new responsibility boundary, **Runtime — the host layer**:

```text
GDSpaces payloads          Stage Ops StageBundle
        |                            |
        v                            v
   ResourceBridge  <-------->    StageHost
        |                            |
        |                            v
        |                        DrawList
        v                            |
   RuntimeApplication  <-------------+
     |            |
     v            v
  IPlatform   IRenderDevice
```

Components, all in `dmc::rengine::runtime`:

| Unit | Responsibility |
| --- | --- |
| `status.hpp` | `RuntimeError`, `Expected<T>`, `Status`. C++23 `std::expected` when the toolchain has it, an API-identical fallback otherwise. |
| `IPlatform` | Surface lifetime, lifecycle events, time, exit request. |
| `HeadlessPlatform` | Deterministic platform for tests and CI. |
| `AndroidPlatform` | Android host, fed by the Java shell over JNI. |
| `FrameClock` | Fixed-step accumulator with a bounded catch-up budget. |
| `IRenderDevice` | Device/swapchain lifetime and frame submission. |
| `NullRenderDevice` | Implemented reference backend; validates and records frames. |
| `RenderBackendRegistry` | Declared vs implemented backends; creation fails closed. |
| `ResourceBridge` | The only path to resource bytes; residency cache over `SourceRegistry`. |
| `StageHost` | Runtime mirror of a `StageBundle`; projects a deterministic `DrawList`. |
| `RuntimeApplication` | Event ordering, surface handling, frame loop. |

### Constitutional compliance

- **Article I.** `ResourceBridge` holds a `const SourceRegistry&` and is the
  only component in the layer that reads bytes. It adds residency and typed
  failures; it never resolves a path, expands a container or mints an identity.
- **Article II.** The runtime makes no reverse-engineering claim. `StageHost`
  will not synthesize geometry: a stage member draws only when an owner
  supplies a `GeometryBinding`, so an unparsed model contributes nothing rather
  than a guess.
- **Article III.** The layer is read-only. It has no writer and no working copy.
- **Article VI.** The core library stays on C++20. The runtime is a separate
  target on its own standard, so no existing translation unit moves.
- **Architecture rule 5.** `StageHost::bind` mirrors a bundle in bundle order
  and a rebind replaces the mirror wholesale. There is no merge path, because a
  merged mirror would be state Stage Ops never authored.

### Fail-closed rules

- an unimplemented backend is an error, never a silent substitution of `null`;
- an unknown backend and a declared-but-unimplemented one report different
  errors, so a typo is not read as "not ready yet";
- a payload carrying a GDSpaces error diagnostic is refused and does not become
  resident;
- a draw call without geometry or with a zero index count is rejected and
  counted, not dropped silently.

### C++ standard policy

`DMC_RENGINE_RUNTIME_CXX_STANDARD` accepts `20`, `23` (default) or `26`. The
build steps down to the highest standard the toolchain advertises and says so.
Library facilities are feature-detected: `__cpp_lib_expected` selects
`std::expected` over the fallback, `__cpp_lib_move_only_function` selects
`std::move_only_function` over `std::function`.

The standard is `PUBLIC` on the runtime target on purpose. The fallback types
are not layout-compatible with the standard ones, so a consumer compiled at a
lower standard than the library would be an ODR violation. Forcing consumers to
match removes that hazard.

## Android

`android/` is a Gradle project whose `externalNativeBuild` points at
`android/CMakeLists.txt`, which builds the core library and the runtime shared
object with the CLI and CTest suite switched off. minSdk 26, targetSdk 36,
64-bit ABIs only (`arm64-v8a`, `x86_64`) because no 32-bit validation story
exists.

The Java shell owns no runtime policy. It translates Android lifecycle and
surface callbacks into native events and drives `tick`. Surface destruction and
recreation are ordinary paths, not errors: Android can take the drawing surface
away while the process lives, and the device returns to a revivable state.

Suspension is dropped rather than replayed — a `resume` restarts the clock at
the current instant — so returning from a backgrounded process does not hand
the simulation a multi-minute backlog.

## Acceptance criteria

- [x] configure/build succeeds at C++20 and C++23 on the host toolchain;
- [x] a C++26 request degrades to C++23 with a reported reason;
- [x] the runtime library compiles warning-free under the repository's warning set;
- [x] every runtime test passes in both the `std::expected` and fallback builds;
- [x] `dmc-rengine-runtime-probe --frames 120` completes headless;
- [x] a declared backend fails closed instead of substituting `null`;
- [x] a resource error diagnostic prevents residency;
- [x] a rebind replaces rather than merges stage state;
- [ ] the Android APK assembles in CI;
- [ ] the Android shell is accepted on a physical device;
- [ ] a GPU backend exists.

## Risks

- **Backend absence read as backend failure.** Mitigated by distinguishing
  `unsupported` (declared, unimplemented) from `unavailable` and
  `invalid_argument` (unknown).
- **Runtime drifting into a second resolver.** Mitigated structurally: the
  layer holds no source, no path logic and no container code, and only
  `ResourceBridge` touches `SourceRegistry`.
- **Scope creep into gameplay.** The runtime has no game state. Anything
  resembling DMC3 behavior belongs to the Recovered Game Source Tree and
  requires its own evidence.
- **ODR hazard from the `Expected` fallback.** Mitigated by the `PUBLIC`
  standard requirement described above.
- **Android lifecycle races.** `AndroidPlatform` guards every posting entry
  point; the loop thread only drains.

## Constitutional amendments required

Article VII enumerates responsibility boundaries and does not yet include a
runtime host. Promoting this layer beyond initial implementation needs an
amendment adding it, with wording that keeps resource and scene authority where
they are. This specification does not amend the Constitution: Article X's
amendment process requires a dedicated pull request.

## Evidence

Completion is evidenced by CI results across both standards and both host
operating systems, the CTest runtime suite, the Android CI job, and — for the
device criterion — a physical-device acceptance record. No criterion above is
satisfied by the existence of source alone.
