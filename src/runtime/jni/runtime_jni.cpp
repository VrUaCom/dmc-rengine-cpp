#if defined(__ANDROID__)

#include "dmc_rengine/runtime/android_platform.hpp"
#include "dmc_rengine/runtime/application.hpp"
#include "dmc_rengine/runtime/null_render_device.hpp"
#include "dmc_rengine/runtime/render_backend_registry.hpp"
#include "dmc_rengine/runtime/runtime_info.hpp"

#include <jni.h>

#include <cstdint>
#include <memory>
#include <new>
#include <string>

namespace {

using dmc::rengine::runtime::AndroidPlatform;
using dmc::rengine::runtime::FrameOutcome;
using dmc::rengine::runtime::NullRenderDevice;
using dmc::rengine::runtime::RenderBackendRegistry;
using dmc::rengine::runtime::RuntimeApplication;
using dmc::rengine::runtime::SurfaceGeometry;

/// Everything one Android runtime instance owns.
///
/// The device is the `null` backend on purpose: no GPU backend has landed yet,
/// and the runtime refuses to pretend otherwise. The shell is still exercised
/// end to end — lifecycle, surface loss, frame pacing — which is what this
/// stage of the port needs to prove.
struct RuntimeHost final {
    AndroidPlatform platform{};
    NullRenderDevice device{};
    RuntimeApplication application{platform, device};
};

[[nodiscard]] RuntimeHost* as_host(jlong handle) noexcept {
    return reinterpret_cast<RuntimeHost*>(static_cast<std::intptr_t>(handle));
}

[[nodiscard]] jstring to_jstring(JNIEnv* env, const std::string& text) {
    return env->NewStringUTF(text.c_str());
}

[[nodiscard]] SurfaceGeometry geometry_of(jint width, jint height, jfloat scale) {
    SurfaceGeometry geometry{};
    geometry.width = width > 0 ? static_cast<std::uint32_t>(width) : 0U;
    geometry.height = height > 0 ? static_cast<std::uint32_t>(height) : 0U;
    geometry.scale = scale > 0.0F ? scale : 1.0F;
    return geometry;
}

/// Mirrors `RengineRuntime.Outcome` on the Java side.
[[nodiscard]] jint outcome_code(FrameOutcome outcome) noexcept {
    switch (outcome) {
    case FrameOutcome::rendered: return 0;
    case FrameOutcome::no_surface: return 1;
    case FrameOutcome::suspended: return 2;
    case FrameOutcome::exit_requested: return 3;
    }
    return 3;
}

constexpr jint kOutcomeError = -1;

} // namespace

extern "C" {

JNIEXPORT jstring JNICALL
Java_com_vruacom_dmcrengine_runtime_RengineRuntime_nativeBuildInfo(JNIEnv* env, jclass) {
    const auto registry = RenderBackendRegistry::with_defaults();
    return to_jstring(env, describe_build(dmc::rengine::runtime::build_info(), registry));
}

JNIEXPORT jlong JNICALL
Java_com_vruacom_dmcrengine_runtime_RengineRuntime_nativeCreate(JNIEnv*, jclass) {
    auto* host = new (std::nothrow) RuntimeHost{};
    return static_cast<jlong>(reinterpret_cast<std::intptr_t>(host));
}

JNIEXPORT void JNICALL
Java_com_vruacom_dmcrengine_runtime_RengineRuntime_nativeDestroy(JNIEnv*, jclass, jlong handle) {
    auto* host = as_host(handle);
    if (host == nullptr) {
        return;
    }
    host->application.stop();
    delete host;
}

JNIEXPORT jboolean JNICALL
Java_com_vruacom_dmcrengine_runtime_RengineRuntime_nativeStart(JNIEnv*, jclass, jlong handle) {
    auto* host = as_host(handle);
    if (host == nullptr) {
        return JNI_FALSE;
    }
    return host->application.start().has_value() ? JNI_TRUE : JNI_FALSE;
}

JNIEXPORT jint JNICALL
Java_com_vruacom_dmcrengine_runtime_RengineRuntime_nativeTick(JNIEnv*, jclass, jlong handle) {
    auto* host = as_host(handle);
    if (host == nullptr) {
        return kOutcomeError;
    }

    auto outcome = host->application.tick();
    if (!outcome.has_value()) {
        return kOutcomeError;
    }
    return outcome_code(*outcome);
}

JNIEXPORT void JNICALL
Java_com_vruacom_dmcrengine_runtime_RengineRuntime_nativeSurfaceCreated(
    JNIEnv*, jclass, jlong handle, jint width, jint height, jfloat scale) {
    if (auto* host = as_host(handle); host != nullptr) {
        host->platform.post_surface_created(geometry_of(width, height, scale));
    }
}

JNIEXPORT void JNICALL
Java_com_vruacom_dmcrengine_runtime_RengineRuntime_nativeSurfaceChanged(
    JNIEnv*, jclass, jlong handle, jint width, jint height, jfloat scale) {
    if (auto* host = as_host(handle); host != nullptr) {
        host->platform.post_surface_resized(geometry_of(width, height, scale));
    }
}

JNIEXPORT void JNICALL
Java_com_vruacom_dmcrengine_runtime_RengineRuntime_nativeSurfaceDestroyed(JNIEnv*, jclass,
                                                                          jlong handle) {
    if (auto* host = as_host(handle); host != nullptr) {
        host->platform.post_surface_destroyed();
    }
}

JNIEXPORT void JNICALL
Java_com_vruacom_dmcrengine_runtime_RengineRuntime_nativePause(JNIEnv*, jclass, jlong handle) {
    if (auto* host = as_host(handle); host != nullptr) {
        host->platform.post_pause();
    }
}

JNIEXPORT void JNICALL
Java_com_vruacom_dmcrengine_runtime_RengineRuntime_nativeResume(JNIEnv*, jclass, jlong handle) {
    if (auto* host = as_host(handle); host != nullptr) {
        host->platform.post_resume();
    }
}

JNIEXPORT void JNICALL
Java_com_vruacom_dmcrengine_runtime_RengineRuntime_nativeLowMemory(JNIEnv*, jclass, jlong handle) {
    if (auto* host = as_host(handle); host != nullptr) {
        host->platform.post_low_memory();
    }
}

JNIEXPORT void JNICALL
Java_com_vruacom_dmcrengine_runtime_RengineRuntime_nativeFocusChanged(JNIEnv*, jclass, jlong handle,
                                                                      jboolean focused) {
    if (auto* host = as_host(handle); host != nullptr) {
        host->platform.post_focus(focused == JNI_TRUE);
    }
}

JNIEXPORT jlong JNICALL
Java_com_vruacom_dmcrengine_runtime_RengineRuntime_nativeFrameIndex(JNIEnv*, jclass, jlong handle) {
    auto* host = as_host(handle);
    if (host == nullptr) {
        return 0;
    }
    return static_cast<jlong>(host->application.frame_index());
}

} // extern "C"

#endif // __ANDROID__
