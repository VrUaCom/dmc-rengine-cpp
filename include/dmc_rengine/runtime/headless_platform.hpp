#pragma once

#include "dmc_rengine/runtime/platform.hpp"

#include <chrono>
#include <string>

namespace dmc::rengine::runtime {

/// Deterministic platform used by tests, CI and headless validation runs.
///
/// It never touches a window system: time only moves when `advance` is called
/// and events only exist when they are posted. That makes every runtime test
/// reproducible without a display, which is what the build lab needs.
class HeadlessPlatform final : public IPlatform {
public:
    HeadlessPlatform();
    explicit HeadlessPlatform(SurfaceGeometry initial_surface);

    [[nodiscard]] PlatformKind kind() const noexcept override;
    [[nodiscard]] std::string_view id() const noexcept override;
    [[nodiscard]] PlatformCapabilities capabilities() const noexcept override;
    [[nodiscard]] SurfaceGeometry surface() const noexcept override;
    [[nodiscard]] PlatformEventQueue drain_events() override;
    [[nodiscard]] Clock::time_point now() const override;
    [[nodiscard]] Status request_exit() override;

    /// Posts an event as if the host had produced it.
    void post(PlatformEvent event);

    /// Posts a surface transition and updates the reported geometry to match.
    void post_surface(PlatformEventKind kind, SurfaceGeometry geometry);

    void advance(Clock::duration delta);

    [[nodiscard]] bool exit_requested() const noexcept;

private:
    std::string id_{"headless"};
    SurfaceGeometry surface_{};
    PlatformEventQueue pending_{};
    Clock::time_point now_{};
    bool exit_requested_{false};
};

} // namespace dmc::rengine::runtime
