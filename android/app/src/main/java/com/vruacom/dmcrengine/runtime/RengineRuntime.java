package com.vruacom.dmcrengine.runtime;

/**
 * Thin JNI facade over the native runtime host.
 *
 * <p>The Java shell owns no runtime policy. It translates Android lifecycle
 * and surface callbacks into native events and drives {@link #tick(long)};
 * everything else — pacing, surface state, rendering — lives in C++.
 */
public final class RengineRuntime {

    /** Mirrors {@code FrameOutcome} in {@code application.hpp}. */
    public static final int OUTCOME_ERROR = -1;
    public static final int OUTCOME_RENDERED = 0;
    public static final int OUTCOME_NO_SURFACE = 1;
    public static final int OUTCOME_SUSPENDED = 2;
    public static final int OUTCOME_EXIT_REQUESTED = 3;

    private static boolean loaded;

    private RengineRuntime() {
    }

    /** Loads the native library once. Throws if the ABI is missing. */
    public static synchronized void load() {
        if (!loaded) {
            System.loadLibrary("dmc_rengine_runtime");
            loaded = true;
        }
    }

    public static String buildInfo() {
        load();
        return nativeBuildInfo();
    }

    public static long create() {
        load();
        return nativeCreate();
    }

    public static void destroy(long handle) {
        if (handle != 0L) {
            nativeDestroy(handle);
        }
    }

    public static boolean start(long handle) {
        return handle != 0L && nativeStart(handle);
    }

    public static int tick(long handle) {
        return handle == 0L ? OUTCOME_ERROR : nativeTick(handle);
    }

    public static void surfaceCreated(long handle, int width, int height, float scale) {
        if (handle != 0L) {
            nativeSurfaceCreated(handle, width, height, scale);
        }
    }

    public static void surfaceChanged(long handle, int width, int height, float scale) {
        if (handle != 0L) {
            nativeSurfaceChanged(handle, width, height, scale);
        }
    }

    public static void surfaceDestroyed(long handle) {
        if (handle != 0L) {
            nativeSurfaceDestroyed(handle);
        }
    }

    public static void pause(long handle) {
        if (handle != 0L) {
            nativePause(handle);
        }
    }

    public static void resume(long handle) {
        if (handle != 0L) {
            nativeResume(handle);
        }
    }

    public static void lowMemory(long handle) {
        if (handle != 0L) {
            nativeLowMemory(handle);
        }
    }

    public static void focusChanged(long handle, boolean focused) {
        if (handle != 0L) {
            nativeFocusChanged(handle, focused);
        }
    }

    public static long frameIndex(long handle) {
        return handle == 0L ? 0L : nativeFrameIndex(handle);
    }

    private static native String nativeBuildInfo();

    private static native long nativeCreate();

    private static native void nativeDestroy(long handle);

    private static native boolean nativeStart(long handle);

    private static native int nativeTick(long handle);

    private static native void nativeSurfaceCreated(long handle, int width, int height, float scale);

    private static native void nativeSurfaceChanged(long handle, int width, int height, float scale);

    private static native void nativeSurfaceDestroyed(long handle);

    private static native void nativePause(long handle);

    private static native void nativeResume(long handle);

    private static native void nativeLowMemory(long handle);

    private static native void nativeFocusChanged(long handle, boolean focused);

    private static native long nativeFrameIndex(long handle);
}
