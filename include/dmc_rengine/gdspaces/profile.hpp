#pragma once

#include <string_view>

namespace dmc::rengine::gdspaces {

enum class GameProfile {
    dmc1_hd,
    dmc2_hd,
    dmc3_hd,
    dmc_launcher_hd,
    unknown,
};

[[nodiscard]] constexpr std::string_view to_string(GameProfile profile) noexcept {
    switch (profile) {
    case GameProfile::dmc1_hd: return "dmc1-hd";
    case GameProfile::dmc2_hd: return "dmc2-hd";
    case GameProfile::dmc3_hd: return "dmc3-hd";
    case GameProfile::dmc_launcher_hd: return "dmclauncher-hd";
    case GameProfile::unknown: return "unknown";
    }
    return "unknown";
}

} // namespace dmc::rengine::gdspaces
