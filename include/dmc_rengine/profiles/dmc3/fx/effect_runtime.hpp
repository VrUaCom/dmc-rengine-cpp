#pragma once

#include <cstdint>
#include <optional>

#include "dmc_rengine/profiles/dmc3/fx/fx_types.hpp"

// The generic P / E / G / V spawn and parent rules of dmc3.exe
// (docs/research/dmc3-effect-runtime-2026-10-01.md).
namespace dmc::rengine::profiles::dmc3::fx::runtime {

// Effect kinds as the spawn API and the V entries dispatch them.
enum class Kind : std::uint8_t { P = 0, E = 1, G = 2, V = 3 };
[[nodiscard]] constexpr char kind_letter(Kind kind) noexcept {
    constexpr char letters[] = {'P', 'E', 'G', 'V'};
    return letters[static_cast<std::uint8_t>(kind) & 3U];
}

// Spawn API: 0x1402E7A90(kind, id, matrix, prep) = 0x1402E7AB0(..., flags 0x20);
// 0x1402E7CA0(kind, id, matrix-or-null, flags) copies (identity for null) and
// adds 2.0 (.rdata 0x14035D570) to the translation y. Kind factories: P
// 0x140312720, E 0x1402E3B10, G 0x1402EBA10, V 0x140324460.
inline constexpr std::uint64_t kSpawnA90 = 0x1402E7A90U;
inline constexpr std::uint64_t kSpawnAB0 = 0x1402E7AB0U;
inline constexpr std::uint64_t kSpawnCA0 = 0x1402E7CA0U;
inline constexpr std::uint64_t kSpawnA80 = 0x1402E7A80U;
inline constexpr float kCa0LiftY = 2.0F;

// Prep modes of 0x1402E7AB0 and the parent modes of both live-parent
// resolvers (0x1402E7DE0 on effect +0xC0 / +0xD8, 0x1402E7FF0 on +0xC8 /
// +0xD4): 0 copy, 1 identity rotation + translation, 2 copy without
// translation, 3 rows 0..2 normalised (xyz, 0x140330390).
enum class MatrixMode : std::uint8_t { Copy = 0, PositionOnly = 1, RotationOnly = 2, Normalized = 3 };

[[nodiscard]] Matrix4 apply_mode(const Matrix4& source, MatrixMode mode) noexcept;
// 0x1402E7CA0: copy (or identity) with translation.y += 2.
[[nodiscard]] Matrix4 ca0_spawn_matrix(const Matrix4* source) noexcept;

// Clocks: the common delta 0x1403261B0 is 1.0 per 60 Hz tick at unit speed.
// A V entry with threshold a (signed i16 entry +0x04) appears at V age
// floor(a) + 1 (age 1 for a <= 0); an E with record +0x80 = n is drawn for
// ages 0..n unless +0x84 holds it for its parent.
[[nodiscard]] constexpr float v_entry_spawn_age(std::int16_t threshold) noexcept {
    return static_cast<float>(threshold < 0 ? 0 : threshold) + 1.0F;
}
[[nodiscard]] constexpr bool e_alive(float age, std::int32_t lifetime, bool held_by_parent) noexcept {
    return held_by_parent || !(age > static_cast<float>(lifetime));
}

}  // namespace dmc::rengine::profiles::dmc3::fx::runtime
