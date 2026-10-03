# HITS Editor 0.1 Preview

Standalone product built in the DMC Rengine repository. Windows and Android
shells use `DMCRengine::HitsEditorCore`; parsers, authoring, SCM world transforms,
camera projection, selection and collision presets remain engine-owned C++.

## Preview workflow

1. Open an extracted `.hits` resource (one source per editing session).
2. Optionally open its `.scm` scene as an overlay.
3. Click/tap a HITS triangle; Connected selects adjoining triangles with the same
   flags. Blue/Orange/Green/Red change the selected surfaces' observed raw preset.
4. Switch picking to SCM, select an object, then SCM > HITS creates its collision.
5. Undo/Redo restores geometry and flags. Export writes a canonical `.hits`.

Windows: right-drag orbits, mouse wheel zooms, Fit resets framing.
Android: one-finger drag orbits, pinch zooms, tap selects. Move accepts three
world-space deltas; Boundary accepts min XYZ and max XYZ and creates eight wall
triangles. Boundary and Move controls are currently Android-only.

Export does not change the session's original baseline; Modified remains visible
and closing/loading another HITS asks before discarding edits. Reopen the export
to establish a new baseline. Invalid resource loads preserve the existing session.

## Build

Desktop: `cmake -S apps/hits-editor -B build/hits-editor`, then
`cmake --build build/hits-editor --config Release`.
Windows executable: `build/hits-editor/Release/hits-editor.exe` with MSVC.

Android: Java 17, Gradle 9.5.0, SDK 36, NDK 30.0.16248370 and SDK CMake 3.22.1.
Run `gradle :app:assembleDebug` in `apps/hits-editor/android`.
Separate application ID: `com.dmcrengine.hitseditor`; it does not replace Native
Reader. CI creates an ephemeral debug signer; separate CI builds may require
uninstalling the old preview before installing a newly signed APK.

## Current limits

Orthographic CPU preview with painter ordering (no z-buffer); overlapping or
intersecting triangles can have visual ordering artifacts. Picking interpolates
depth at the cursor. SCM is untextured. Large scenes have no BVH/culling yet.
No PAC/NBZ open/save UI, full gizmos, raw-flags input, draw-quad UI or embedded
Rengine UI in this preview. Original-game acceptance requires device/game tests.
Android file I/O runs off the UI thread; parsing/rebuild still runs on the UI
thread and may pause the interface on large resources. Process-death recovery of
unsaved edits is not implemented.

## Engine ownership and Native Reader

This product follows Native Reader's existing portable-core/thin-shell design,
SAF document I/O on Android and CPU preview approach. It does not copy or fork
Native Reader's resource parsers or move its entire application into Rengine.
`cmake/reader_core.cmake` is already the canonical read-side dependency shared
with Native Reader; `cmake/hits_editor_core.cmake` adds authoring and viewport.

The GDSpaces `ResourcePayload` adapter remains separate: discovery, identity,
provenance and PAC/NBZ reintegration belong to GDSpaces, not this viewport.
Future extraction of Native Reader's renderer/animation/resource-inspection
modules must preserve its evidence contracts and dependencies with explicit
tests before declaring full feature parity in the engine.
