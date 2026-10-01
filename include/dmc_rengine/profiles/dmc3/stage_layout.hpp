#pragma once

#include <array>
#include <string>
#include <string_view>
#include <vector>

#include "dmc_rengine/profiles/dmc3/fx/fx_types.hpp"

// Stage layout text: the top-level "# GAME" slot of a stage archive (record
// parser near 0x140247720). Per "# SET n <kind>" block: `model` (byte +0),
// `pos` (+0x10), `rot` in degrees (+0x20, stored as radians), `scale`
// (+0x30), `uv part, texture, U, V` scrolls, `eff K ID` + `epos x, y, z`;
// the CONFIG block holds `cam_init`. Parsing stops at "$" or "# GAME_END".
namespace dmc::rengine::profiles::dmc3::stage_layout {

struct GameSet final {
    std::string kind;
    int model{-1};
    Vec3 pos{};
    Vec3 rot{};  // degrees
    Vec3 scale{1.0F, 1.0F, 1.0F};
    std::vector<std::array<float, 4>> uv;
    char effect_kind{};
    int effect_id{-1};
    Vec3 effect_pos{};
};

struct GameLayout final {
    std::vector<GameSet> sets;
    bool has_camera{};
    Vec3 camera{};
};

// Numbers of a line separated by ',', blanks; a ';' starts a comment.
[[nodiscard]] std::vector<float> numbers(std::string_view line);
[[nodiscard]] GameLayout parse_game(std::string_view text);

// Scale, rotate (X, then Y, then Z, degrees) and translate one point of a
// placed object; place_normal applies only the rotation.
[[nodiscard]] Vec3 place_point(const GameSet& set, const Vec3& point) noexcept;
[[nodiscard]] Vec3 place_normal(const GameSet& set, const Vec3& normal) noexcept;

}  // namespace dmc::rengine::profiles::dmc3::stage_layout
