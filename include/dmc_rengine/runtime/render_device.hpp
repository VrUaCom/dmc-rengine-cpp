#pragma once

#include "dmc_rengine/runtime/platform_event.hpp"
#include "dmc_rengine/runtime/status.hpp"

#include <cstdint>
#include <string>
#include <string_view>
#include <vector>

namespace dmc::rengine::runtime {

enum class RenderBackendKind {
    null,
    vulkan,
    gles,
    d3d11,
    metal,
};

[[nodiscard]] constexpr std::string_view to_string(RenderBackendKind kind) noexcept {
    switch (kind) {
    case RenderBackendKind::null: return "null";
    case RenderBackendKind::vulkan: return "vulkan";
    case RenderBackendKind::gles: return "gles";
    case RenderBackendKind::d3d11: return "d3d11";
    case RenderBackendKind::metal: return "metal";
    }
    return "null";
}

struct DeviceCapabilities final {
    std::uint32_t max_texture_dimension{};
    std::uint32_t max_draw_calls_per_frame{};
    bool presents{false};
    bool block_compressed_textures{false};

    friend bool operator==(const DeviceCapabilities&, const DeviceCapabilities&) = default;
};

/// One submitted unit of work.
///
/// Keys are opaque runtime handles, not DMC3 identities: translating a stage
/// member into a geometry/material key is the stage host's job, and the render
/// device must stay unaware of any game format.
struct DrawCall final {
    std::uint64_t geometry_key{};
    std::uint64_t material_key{};
    std::uint32_t index_count{};
    std::string debug_name{};

    [[nodiscard]] bool valid() const noexcept {
        return geometry_key != 0U && index_count > 0U;
    }

    friend bool operator==(const DrawCall&, const DrawCall&) = default;
};

using DrawList = std::vector<DrawCall>;

struct FrameContext final {
    SurfaceGeometry geometry{};
    std::uint64_t frame_index{};
    float interpolation{};

    friend bool operator==(const FrameContext&, const FrameContext&) = default;
};

struct FrameStats final {
    std::uint64_t frame_index{};
    std::uint32_t submitted_draw_calls{};
    std::uint32_t rejected_draw_calls{};

    friend bool operator==(const FrameStats&, const FrameStats&) = default;
};

/// Rendering backend abstraction.
///
/// Implementations own device/swapchain lifetime only. They do not resolve
/// resources and do not hold scene truth; both belong upstream.
class IRenderDevice {
public:
    virtual ~IRenderDevice() = default;

    [[nodiscard]] virtual RenderBackendKind kind() const noexcept = 0;
    [[nodiscard]] virtual DeviceCapabilities capabilities() const noexcept = 0;
    [[nodiscard]] virtual bool initialized() const noexcept = 0;

    [[nodiscard]] virtual Status initialize(const SurfaceGeometry& geometry) = 0;
    [[nodiscard]] virtual Status resize(const SurfaceGeometry& geometry) = 0;

    /// Signals that the host destroyed the drawing surface. The device must
    /// return to an uninitialized state that a later `initialize` can revive.
    [[nodiscard]] virtual Status notify_surface_lost() = 0;

    [[nodiscard]] virtual Expected<FrameStats> render(const FrameContext& context,
                                                      const DrawList& draws) = 0;

    virtual void shutdown() noexcept = 0;
};

} // namespace dmc::rengine::runtime
