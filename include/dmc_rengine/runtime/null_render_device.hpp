#pragma once

#include "dmc_rengine/runtime/render_device.hpp"

#include <cstddef>
#include <vector>

namespace dmc::rengine::runtime {

/// Reference backend that validates and records frames without presenting.
///
/// It exists so the whole runtime loop — lifecycle, surface loss, draw
/// submission and rejection — is testable on a build machine with no GPU, and
/// so every real backend has an observable behavioral baseline to match.
class NullRenderDevice final : public IRenderDevice {
public:
    NullRenderDevice() = default;
    explicit NullRenderDevice(DeviceCapabilities capabilities);

    [[nodiscard]] RenderBackendKind kind() const noexcept override;
    [[nodiscard]] DeviceCapabilities capabilities() const noexcept override;
    [[nodiscard]] bool initialized() const noexcept override;

    [[nodiscard]] Status initialize(const SurfaceGeometry& geometry) override;
    [[nodiscard]] Status resize(const SurfaceGeometry& geometry) override;
    [[nodiscard]] Status notify_surface_lost() override;
    [[nodiscard]] Expected<FrameStats> render(const FrameContext& context,
                                              const DrawList& draws) override;
    void shutdown() noexcept override;

    [[nodiscard]] const std::vector<FrameStats>& recorded_frames() const noexcept;
    [[nodiscard]] SurfaceGeometry current_surface() const noexcept;
    [[nodiscard]] std::size_t initialize_count() const noexcept;

private:
    DeviceCapabilities capabilities_{4096U, 4096U, false, true};
    SurfaceGeometry surface_{};
    std::vector<FrameStats> frames_{};
    std::size_t initialize_count_{};
    bool initialized_{false};
};

} // namespace dmc::rengine::runtime
