#include "dmc_rengine/runtime/android_platform.hpp"

#if defined(__ANDROID__)

#include <chrono>
#include <utility>

namespace dmc::rengine::runtime {

PlatformKind AndroidPlatform::kind() const noexcept {
    return PlatformKind::android;
}

std::string_view AndroidPlatform::id() const noexcept {
    return id_;
}

PlatformCapabilities AndroidPlatform::capabilities() const noexcept {
    // Android can destroy the drawing surface while the process survives, and
    // can suspend the process outright. Both are normal, not error paths.
    return PlatformCapabilities{true, true, true, true};
}

SurfaceGeometry AndroidPlatform::surface() const noexcept {
    const std::lock_guard<std::mutex> guard{mutex_};
    return surface_;
}

PlatformEventQueue AndroidPlatform::drain_events() {
    const std::lock_guard<std::mutex> guard{mutex_};
    PlatformEventQueue drained;
    drained.swap(pending_);
    return drained;
}

IPlatform::Clock::time_point AndroidPlatform::now() const {
    return Clock::now();
}

Status AndroidPlatform::request_exit() {
    post_quit();
    return ok();
}

void AndroidPlatform::post(PlatformEvent event) {
    const std::lock_guard<std::mutex> guard{mutex_};
    pending_.push_back(std::move(event));
}

void AndroidPlatform::post_surface_created(SurfaceGeometry geometry) {
    {
        const std::lock_guard<std::mutex> guard{mutex_};
        surface_ = geometry;
    }
    post(PlatformEvent{PlatformEventKind::surface_created, geometry});
}

void AndroidPlatform::post_surface_resized(SurfaceGeometry geometry) {
    {
        const std::lock_guard<std::mutex> guard{mutex_};
        surface_ = geometry;
    }
    post(PlatformEvent{PlatformEventKind::surface_resized, geometry});
}

void AndroidPlatform::post_surface_destroyed() {
    {
        const std::lock_guard<std::mutex> guard{mutex_};
        surface_ = SurfaceGeometry{};
    }
    post(PlatformEvent{PlatformEventKind::surface_destroyed, SurfaceGeometry{}});
}

void AndroidPlatform::post_focus(bool focused) {
    post(PlatformEvent{focused ? PlatformEventKind::focus_gained : PlatformEventKind::focus_lost,
                       surface()});
}

void AndroidPlatform::post_pause() {
    post(PlatformEvent{PlatformEventKind::suspend, surface()});
}

void AndroidPlatform::post_resume() {
    post(PlatformEvent{PlatformEventKind::resume, surface()});
}

void AndroidPlatform::post_low_memory() {
    post(PlatformEvent{PlatformEventKind::low_memory, surface()});
}

void AndroidPlatform::post_quit() {
    {
        const std::lock_guard<std::mutex> guard{mutex_};
        exit_requested_ = true;
    }
    post(PlatformEvent{PlatformEventKind::quit_requested, surface()});
}

bool AndroidPlatform::exit_requested() const noexcept {
    const std::lock_guard<std::mutex> guard{mutex_};
    return exit_requested_;
}

} // namespace dmc::rengine::runtime

#endif // __ANDROID__
