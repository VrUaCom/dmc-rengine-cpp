#pragma once

#include <cstdint>
#include <string_view>
#include <vector>

namespace dmc::rengine::runtime {

/// Host lifecycle and surface transitions the runtime must react to.
///
/// The set is dictated by the strictest host in scope. Android can destroy and
/// recreate the drawing surface while the process keeps running, so
/// `surface_destroyed` / `surface_created` are first-class events rather than
/// desktop-only edge cases.
enum class PlatformEventKind {
    surface_created,
    surface_resized,
    surface_destroyed,
    focus_gained,
    focus_lost,
    low_memory,
    suspend,
    resume,
    quit_requested,
};

[[nodiscard]] constexpr std::string_view to_string(PlatformEventKind kind) noexcept {
    switch (kind) {
    case PlatformEventKind::surface_created: return "surface-created";
    case PlatformEventKind::surface_resized: return "surface-resized";
    case PlatformEventKind::surface_destroyed: return "surface-destroyed";
    case PlatformEventKind::focus_gained: return "focus-gained";
    case PlatformEventKind::focus_lost: return "focus-lost";
    case PlatformEventKind::low_memory: return "low-memory";
    case PlatformEventKind::suspend: return "suspend";
    case PlatformEventKind::resume: return "resume";
    case PlatformEventKind::quit_requested: return "quit-requested";
    }
    return "quit-requested";
}

struct SurfaceGeometry final {
    std::uint32_t width{};
    std::uint32_t height{};
    float scale{1.0F};

    [[nodiscard]] bool renderable() const noexcept {
        return width > 0U && height > 0U && scale > 0.0F;
    }

    friend bool operator==(const SurfaceGeometry&, const SurfaceGeometry&) = default;
};

struct PlatformEvent final {
    PlatformEventKind kind{PlatformEventKind::quit_requested};
    SurfaceGeometry geometry{};

    friend bool operator==(const PlatformEvent&, const PlatformEvent&) = default;
};

using PlatformEventQueue = std::vector<PlatformEvent>;

} // namespace dmc::rengine::runtime
