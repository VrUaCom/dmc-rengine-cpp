package com.vruacom.dmcrengine.runtime;

import android.app.Activity;
import android.os.Bundle;
import android.util.Log;
import android.view.SurfaceHolder;
import android.view.SurfaceView;
import android.view.ViewGroup;
import android.widget.FrameLayout;
import android.widget.TextView;

/**
 * Host activity for the runtime foundation.
 *
 * <p>No GPU backend has landed yet, so nothing is presented: the surface drives
 * the real lifecycle path while the overlay reports what this build actually
 * is. That keeps the Android shell honest about its own state instead of
 * showing a blank screen that could mean anything.
 */
public final class RuntimeActivity extends Activity implements SurfaceHolder.Callback {

    private static final String TAG = "DMCRengineRuntime";

    private long handle;
    private Thread loopThread;
    private volatile boolean looping;
    private TextView overlay;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);

        final FrameLayout root = new FrameLayout(this);
        final SurfaceView surface = new SurfaceView(this);
        surface.getHolder().addCallback(this);
        root.addView(surface, new FrameLayout.LayoutParams(
                ViewGroup.LayoutParams.MATCH_PARENT, ViewGroup.LayoutParams.MATCH_PARENT));

        overlay = new TextView(this);
        overlay.setPadding(32, 32, 32, 32);
        overlay.setTextSize(12.0f);
        root.addView(overlay, new FrameLayout.LayoutParams(
                ViewGroup.LayoutParams.MATCH_PARENT, ViewGroup.LayoutParams.WRAP_CONTENT));

        setContentView(root);

        try {
            handle = RengineRuntime.create();
            overlay.setText(RengineRuntime.buildInfo());
        } catch (UnsatisfiedLinkError error) {
            Log.e(TAG, "native runtime unavailable", error);
            overlay.setText("native runtime unavailable: " + error.getMessage());
        }
    }

    @Override
    protected void onResume() {
        super.onResume();
        RengineRuntime.resume(handle);
        startLoop();
    }

    @Override
    protected void onPause() {
        RengineRuntime.pause(handle);
        stopLoop();
        super.onPause();
    }

    @Override
    protected void onDestroy() {
        stopLoop();
        RengineRuntime.destroy(handle);
        handle = 0L;
        super.onDestroy();
    }

    @Override
    public void onWindowFocusChanged(boolean hasFocus) {
        super.onWindowFocusChanged(hasFocus);
        RengineRuntime.focusChanged(handle, hasFocus);
    }

    @Override
    public void onLowMemory() {
        super.onLowMemory();
        RengineRuntime.lowMemory(handle);
    }

    @Override
    public void surfaceCreated(SurfaceHolder holder) {
        // Geometry arrives with surfaceChanged; Android always follows one with
        // the other, and a zero-sized surface would be rejected downstream.
    }

    @Override
    public void surfaceChanged(SurfaceHolder holder, int format, int width, int height) {
        final float scale = getResources().getDisplayMetrics().density;
        RengineRuntime.surfaceCreated(handle, width, height, scale);
    }

    @Override
    public void surfaceDestroyed(SurfaceHolder holder) {
        RengineRuntime.surfaceDestroyed(handle);
    }

    private void startLoop() {
        if (looping || handle == 0L) {
            return;
        }
        if (!RengineRuntime.start(handle)) {
            Log.w(TAG, "runtime already started or refused to start");
        }

        looping = true;
        loopThread = new Thread(this::runLoop, "dmc-rengine-runtime");
        loopThread.start();
    }

    private void stopLoop() {
        looping = false;
        final Thread thread = loopThread;
        loopThread = null;
        if (thread == null) {
            return;
        }
        try {
            thread.join(1000L);
        } catch (InterruptedException interrupted) {
            Thread.currentThread().interrupt();
        }
    }

    private void runLoop() {
        while (looping) {
            final int outcome = RengineRuntime.tick(handle);
            if (outcome == RengineRuntime.OUTCOME_ERROR
                    || outcome == RengineRuntime.OUTCOME_EXIT_REQUESTED) {
                looping = false;
                break;
            }

            // No presentation backend yet, so the loop is paced here rather
            // than by a swapchain.
            try {
                Thread.sleep(16L);
            } catch (InterruptedException interrupted) {
                Thread.currentThread().interrupt();
                break;
            }
        }
    }
}
