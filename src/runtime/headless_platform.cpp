#include "dmc_rengine/runtime/headless_platform.hpp"

#include <utility>

namespace dmc::rengine::runtime {

HeadlessPlatform::HeadlessPlatform() = default;

HeadlessPlatform::HeadlessPlatform(SurfaceGeometry initial_surface) : surface_(initial_surface) {}

PlatformKind HeadlessPlatform::kind() const noexcept {
    return PlatformKind::headless;
}

std::string_view HeadlessPlatform::id() const noexcept {
    return id_;
}

PlatformCapabilities HeadlessPlatform::capabilities() const noexcept {
    return PlatformCapabilities{false, false, true, true};
}

SurfaceGeometry HeadlessPlatform::surface() const noexcept {
    return surface_;
}

PlatformEventQueue HeadlessPlatform::drain_events() {
    PlatformEventQueue drained;
    drained.swap(pending_);
    return drained;
}

IPlatform::Clock::time_point HeadlessPlatform::now() const {
    return now_;
}

Status HeadlessPlatform::request_exit() {
    exit_requested_ = true;
    pending_.push_back(PlatformEvent{PlatformEventKind::quit_requested, surface_});
    return ok();
}

void HeadlessPlatform::post(PlatformEvent event) {
    pending_.push_back(std::move(event));
}

void HeadlessPlatform::post_surface(PlatformEventKind kind, SurfaceGeometry geometry) {
    switch (kind) {
    case PlatformEventKind::surface_created:
    case PlatformEventKind::surface_resized:
        surface_ = geometry;
        break;
    case PlatformEventKind::surface_destroyed:
        surface_ = SurfaceGeometry{};
        break;
    default:
        break;
    }

    pending_.push_back(PlatformEvent{kind, surface_});
}

void HeadlessPlatform::advance(Clock::duration delta) {
    if (delta.count() > 0) {
        now_ += delta;
    }
}

bool HeadlessPlatform::exit_requested() const noexcept {
    return exit_requested_;
}

} // namespace dmc::rengine::runtime
