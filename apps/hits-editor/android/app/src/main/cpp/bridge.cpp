#include "dmc_rengine/hits/viewport.hpp"

#include <jni.h>
#include <limits>
#include <memory>

using Controller = dmc::rengine::hits::viewport::Controller;
using Preset = dmc::rengine::hits::editor::CollisionPreset;
namespace {
Controller* controller(jlong handle) {
    return reinterpret_cast<Controller*>(handle);
}
void failure(JNIEnv* env, const char* message) {
    if (!env->ExceptionCheck())
        env->ThrowNew(env->FindClass("java/lang/IllegalStateException"), message);
}
} // namespace
#define JNI(name) Java_com_dmcrengine_hitseditor_Native_##name
extern "C" JNIEXPORT jlong JNICALL JNI(create)(JNIEnv* env, jclass) {
    try {
        return reinterpret_cast<jlong>(new Controller);
    } catch (...) {
        failure(env, "Cannot create editor");
        return 0;
    }
}
extern "C" JNIEXPORT void JNICALL JNI(destroy)(JNIEnv*, jclass, jlong h) {
    delete controller(h);
}
extern "C" JNIEXPORT jboolean JNICALL JNI(open)(JNIEnv* env, jclass, jlong h, jbyteArray input,
                                                jboolean scm) {
    try {
        if (!h || !input)
            return false;
        auto size = env->GetArrayLength(input);
        if (size <= 0 || size > 256 * 1024 * 1024)
            return false;
        std::vector<std::byte> bytes(static_cast<std::size_t>(size));
        env->GetByteArrayRegion(input, 0, size, reinterpret_cast<jbyte*>(bytes.data()));
        if (env->ExceptionCheck())
            return false;
        return scm ? controller(h)->open_scm(bytes) : controller(h)->open_hits(bytes);
    } catch (...) {
        failure(env, "Resource loading failed");
        return false;
    }
}
extern "C" JNIEXPORT jfloatArray JNICALL JNI(frame)(JNIEnv* env, jclass, jlong h, jfloat w,
                                                    jfloat height) {
    try {
        if (!h)
            return nullptr;
        auto triangles = controller(h)->frame(w, height);
        if (triangles.size() > static_cast<std::size_t>(std::numeric_limits<jsize>::max() / 8))
            return nullptr;
        std::vector<float> packets;
        packets.reserve(triangles.size() * 8);
        for (const auto& t : triangles) {
            for (auto p : t.points) {
                packets.push_back(p.x);
                packets.push_back(p.y);
            }
            packets.push_back(static_cast<float>(t.color));
            packets.push_back(t.selected ? 1.0F : 0.0F);
        }
        auto out = env->NewFloatArray(static_cast<jsize>(packets.size()));
        if (out)
            env->SetFloatArrayRegion(out, 0, static_cast<jsize>(packets.size()), packets.data());
        return out;
    } catch (...) {
        failure(env, "Viewport failed");
        return nullptr;
    }
}
extern "C" JNIEXPORT jboolean JNICALL JNI(pick)(JNIEnv* env, jclass, jlong h, jfloat x, jfloat y,
                                                jfloat w, jfloat height, jboolean scm) {
    try {
        return h && controller(h)->pick(x, y, w, height, scm);
    } catch (...) {
        failure(env, "Selection failed");
        return false;
    }
}
extern "C" JNIEXPORT void JNICALL JNI(camera)(JNIEnv* env, jclass, jlong h, jfloat yaw,
                                              jfloat pitch, jfloat zoom) {
    try {
        if (h) {
            controller(h)->orbit(yaw, pitch);
            controller(h)->zoom(zoom);
        }
    } catch (...) {
        failure(env, "Camera failed");
    }
}
extern "C" JNIEXPORT jboolean JNICALL JNI(action)(JNIEnv* env, jclass, jlong h, jint action,
                                                  jint preset) {
    try {
        if (!h || preset < 0 || preset > 3)
            return false;
        auto& c = *controller(h);
        auto p = static_cast<Preset>(preset);
        switch (action) {
        case 0:
            c.fit();
            return true;
        case 1:
            return c.select_connected();
        case 2:
            return c.paint(p);
        case 3:
            return c.undo();
        case 4:
            return c.redo();
        case 5:
            return c.import_selected_object(p);
        default:
            return false;
        }
    } catch (...) {
        failure(env, "Edit failed");
        return false;
    }
}
extern "C" JNIEXPORT jboolean JNICALL JNI(geometry)(JNIEnv* env, jclass, jlong h, jfloatArray input,
                                                    jboolean boundary, jint preset) {
    try {
        if (!h || !input || preset < 0 || preset > 3)
            return false;
        const int size = boundary ? 6 : 3;
        if (env->GetArrayLength(input) != size)
            return false;
        float v[6]{};
        env->GetFloatArrayRegion(input, 0, size, v);
        if (env->ExceptionCheck())
            return false;
        auto& c = *controller(h);
        return boundary
                   ? c.boundary({v[0], v[1], v[2]}, {v[3], v[4], v[5]}, static_cast<Preset>(preset))
                   : c.translate({v[0], v[1], v[2]});
    } catch (...) {
        failure(env, "Geometry edit failed");
        return false;
    }
}
extern "C" JNIEXPORT jbyteArray JNICALL JNI(save)(JNIEnv* env, jclass, jlong h) {
    try {
        if (!h)
            return nullptr;
        auto saved = controller(h)->save();
        if (!saved.ok())
            return nullptr;
        if (saved.bytes.size() > static_cast<std::size_t>(std::numeric_limits<jsize>::max()))
            return nullptr;
        auto out = env->NewByteArray(static_cast<jsize>(saved.bytes.size()));
        if (out)
            env->SetByteArrayRegion(out, 0, static_cast<jsize>(saved.bytes.size()),
                                    reinterpret_cast<const jbyte*>(saved.bytes.data()));
        return out;
    } catch (...) {
        failure(env, "Canonical rebuild failed");
        return nullptr;
    }
}
extern "C" JNIEXPORT jstring JNICALL JNI(status)(JNIEnv* env, jclass, jlong h) {
    try {
        return env->NewStringUTF(h ? controller(h)->status().c_str() : "Editor closed");
    } catch (...) {
        return env->NewStringUTF("Status unavailable");
    }
}
extern "C" JNIEXPORT jboolean JNICALL JNI(dirty)(JNIEnv*, jclass, jlong h) {
    return h && controller(h)->dirty();
}
