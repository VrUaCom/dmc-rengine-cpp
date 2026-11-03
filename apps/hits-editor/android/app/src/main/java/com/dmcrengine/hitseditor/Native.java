package com.dmcrengine.hitseditor;

final class Native {
    static { System.loadLibrary("hitseditor"); }
    static native long create();
    static native void destroy(long handle);
    static native boolean open(long handle, byte[] bytes, boolean scm);
    static native float[] frame(long handle, float width, float height);
    static native boolean pick(long handle,float x,float y,float width,float height,boolean scm);
    static native void camera(long handle,float yaw,float pitch,float zoom);
    static native boolean action(long handle,int action,int preset);
    static native boolean geometry(long handle,float[] coordinates,boolean boundary,int preset);
    static native byte[] save(long handle);
    static native String status(long handle);
    static native boolean dirty(long handle);
}
