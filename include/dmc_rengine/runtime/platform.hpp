#pragma once

#include "dmc_rengine/runtime/platform_event.hpp"
#include "dmc_rengine/runtime/status.hpp"

#include <chrono>
#include <string_view>

namespace dmc::rengine::runtime {

enum class PlatformKind {
    headless,
    android,
    linux_desktop,
    windows_desktop,
    macos_desktop,
};

[[nodiscard]] constexpr std::string_view to_string(PlatformKind kind) noexcept {
    switch (kind) {
    case PlatformKind::headless: return "headless";
    case PlatformKind::android: return "android";
    case PlatformKind::linux_desktop: return "linux-desktop";
    case PlatformKind::windows_desktop: return "windows-desktop";
    case PlatformKind::macos_desktop: return "macos-desktop";
    }
    return "headless";
}

struct PlatformCapabilities final {
    bool presents_surface{false};
    bool suspends_process{false};
    bool recreates_surface{false};
    bool has_filesystem_paths{true};

    friend bool operator==(const PlatformCapabilities&, const PlatformCapabilities&) = default;
};

/// Host abstraction. A platform owns the surface lifetime and the event queue;
/// it never owns resources, scene state or rendering policy.
class IPlatform {
public:
    using Clock = std::chrono::steady_clock;

    virtual ~IPlatform() = default;

    [[nodiscard]] virtual PlatformKind kind() const noexcept = 0;
    [[nodiscard]] virtual std::string_view id() const noexcept = 0;
    [[nodiscard]] virtual PlatformCapabilities capabilities() const noexcept = 0;

    /// Current surface geometry. Undefined-but-safe (zero sized) while no
    /// surface exists; callers must check `SurfaceGeometry::renderable()`.
    [[nodiscard]] virtual SurfaceGeometry surface() const noexcept = 0;

    /// Drains pending host events. The returned queue is ordered oldest first
    /// and is emptied from the platform by this call.
    [[nodiscard]] virtual PlatformEventQueue drain_events() = 0;

    [[nodiscard]] virtual Clock::time_point now() const = 0;

    /// Asks the host to end the run loop. Hosts that cannot self-terminate
    /// report `RuntimeErrorCode::unsupported`.
    [[nodiscard]] virtual Status request_exit() = 0;
};

} // namespace dmc::rengine::runtime
