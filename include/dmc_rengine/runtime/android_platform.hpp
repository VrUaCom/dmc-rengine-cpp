#pragma once

#if defined(__ANDROID__)

#include "dmc_rengine/runtime/platform.hpp"

#include <mutex>
#include <string>

namespace dmc::rengine::runtime {

/// Android host driven by the Java shell.
///
/// The Android lifecycle runs on the UI thread while the loop runs on its own
/// thread, so every posting entry point is mutex-guarded. The platform owns no
/// JNI state: the shell translates callbacks into the events below and nothing
/// above this class knows Android exists.
class AndroidPlatform final : public IPlatform {
public:
    AndroidPlatform() = default;

    [[nodiscard]] PlatformKind kind() const noexcept override;
    [[nodiscard]] std::string_view id() const noexcept override;
    [[nodiscard]] PlatformCapabilities capabilities() const noexcept override;
    [[nodiscard]] SurfaceGeometry surface() const noexcept override;
    [[nodiscard]] PlatformEventQueue drain_events() override;
    [[nodiscard]] Clock::time_point now() const override;
    [[nodiscard]] Status request_exit() override;

    void post_surface_created(SurfaceGeometry geometry);
    void post_surface_resized(SurfaceGeometry geometry);
    void post_surface_destroyed();
    void post_focus(bool focused);
    void post_pause();
    void post_resume();
    void post_low_memory();
    void post_quit();

    [[nodiscard]] bool exit_requested() const noexcept;

private:
    void post(PlatformEvent event);

    mutable std::mutex mutex_{};
    std::string id_{"android"};
    SurfaceGeometry surface_{};
    PlatformEventQueue pending_{};
    bool exit_requested_{false};
};

} // namespace dmc::rengine::runtime

#endif // __ANDROID__
