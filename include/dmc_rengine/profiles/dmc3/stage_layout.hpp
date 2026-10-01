#pragma once

#include <array>
#include <cstdint>
#include <string>
#include <string_view>
#include <vector>

#include "dmc_rengine/profiles/dmc3/fx/fx_types.hpp"

// Stage layout text: the top-level "# GAME" slot of a stage archive (record
// parser near 0x140247720). Per "# SET n <kind>" block: `model` (byte +0),
// `pos` (+0x10), `rot` in degrees (+0x20, stored as radians), `scale`
// (+0x30), `uv part, texture, U, V` scrolls, `eff K ID` + `epos x, y, z`,
// and for BREAK blocks `bmodel`, `beff`, `remain`;
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
    // "# SET n BREAK" (CStageSetBreak, parser 0x14024A540, update 0x14024AE40):
    // `bmodel` (shown once broken; none = the object is removed), `beff`
    // (spawned once on breaking; an `epos` after it is its own position,
    // object +0x6F0), `remain` (0 = the bmodel fades out after its break
    // motion, alpha 64 - 2 per tick; 1 `on`, 2 `on2` = it stays).
    int broken_model{-1};
    char broken_effect_kind{};
    int broken_effect_id{-1};
    Vec3 broken_effect_pos{};
    std::uint8_t remain{};
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

// What a layout object shows in either state: its model (-1: none) and the
// effect with its object-local position (kind 0: none). Broken applies to
// BREAK blocks only; `once` marks the one-shot beff.
struct ObjectState final {
    int model{-1};
    char effect_kind{};
    int effect_id{-1};
    Vec3 effect_pos{};
    bool once{};
};
[[nodiscard]] ObjectState object_state(const GameSet& set, bool broken) noexcept;

}  // namespace dmc::rengine::profiles::dmc3::stage_layout
