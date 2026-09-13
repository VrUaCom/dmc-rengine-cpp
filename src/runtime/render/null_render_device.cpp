#include "dmc_rengine/runtime/null_render_device.hpp"

#include <cstdint>
#include <utility>

namespace dmc::rengine::runtime {

NullRenderDevice::NullRenderDevice(DeviceCapabilities capabilities)
    : capabilities_(capabilities) {}

RenderBackendKind NullRenderDevice::kind() const noexcept {
    return RenderBackendKind::null;
}

DeviceCapabilities NullRenderDevice::capabilities() const noexcept {
    return capabilities_;
}

bool NullRenderDevice::initialized() const noexcept {
    return initialized_;
}

Status NullRenderDevice::initialize(const SurfaceGeometry& geometry) {
    if (initialized_) {
        return fail(make_error(RuntimeErrorCode::already_initialized, "null-render-device",
                               "device is already initialized"));
    }
    if (!geometry.renderable()) {
        return fail(make_error(RuntimeErrorCode::invalid_argument, "null-render-device",
                               "surface geometry is not renderable"));
    }

    surface_ = geometry;
    initialized_ = true;
    ++initialize_count_;
    return ok();
}

Status NullRenderDevice::resize(const SurfaceGeometry& geometry) {
    if (!initialized_) {
        return fail(make_error(RuntimeErrorCode::not_initialized, "null-render-device",
                               "resize before initialize"));
    }
    if (!geometry.renderable()) {
        return fail(make_error(RuntimeErrorCode::invalid_argument, "null-render-device",
                               "surface geometry is not renderable"));
    }

    surface_ = geometry;
    return ok();
}

Status NullRenderDevice::notify_surface_lost() {
    surface_ = SurfaceGeometry{};
    initialized_ = false;
    return ok();
}

Expected<FrameStats> NullRenderDevice::render(const FrameContext& context, const DrawList& draws) {
    if (!initialized_) {
        return fail(make_error(RuntimeErrorCode::not_initialized, "null-render-device",
                               "render before initialize"));
    }
    if (!context.geometry.renderable()) {
        return fail(make_error(RuntimeErrorCode::surface_lost, "null-render-device",
                               "frame context carries no renderable surface"));
    }

    FrameStats stats{};
    stats.frame_index = context.frame_index;

    for (const auto& draw : draws) {
        if (!draw.valid()) {
            ++stats.rejected_draw_calls;
            continue;
        }
        if (stats.submitted_draw_calls >= capabilities_.max_draw_calls_per_frame) {
            ++stats.rejected_draw_calls;
            continue;
        }
        ++stats.submitted_draw_calls;
    }

    frames_.push_back(stats);
    return stats;
}

void NullRenderDevice::shutdown() noexcept {
    initialized_ = false;
    surface_ = SurfaceGeometry{};
}

const std::vector<FrameStats>& NullRenderDevice::recorded_frames() const noexcept {
    return frames_;
}

SurfaceGeometry NullRenderDevice::current_surface() const noexcept {
    return surface_;
}

std::size_t NullRenderDevice::initialize_count() const noexcept {
    return initialize_count_;
}

} // namespace dmc::rengine::runtime
