#pragma once

#include <cstdint>
#include <span>
#include <string_view>
#include <vector>

// Where dmc3.exe's code starts each effect: every call of the spawn API and
// of the P/E/G/V kind factories, one row per (call, kind, id). Effects are
// started by code, not by the motion script; only the stage layout, the
// stage configuration's STE and the cutscene clips start them from data.
// Evidence: docs/research/dmc3-effect-spawn-sites-2026-10-24.md; every row's
// call is re-checked by tools/reverse/dmc3_gates.cpp.
namespace dmc::rengine::profiles::dmc3::effect_spawn_sites {

inline constexpr std::uint64_t spawn_va = 0x1402E7A90ULL;         // (kind, id, matrix, prep)
inline constexpr std::uint64_t spawn_flags_va = 0x1402E7AB0ULL;   // (kind, id, matrix, prep, flags)
inline constexpr std::uint64_t spawn_raised_va = 0x1402E7CA0ULL;  // (kind, id, matrix or none, flags): y + 2
inline constexpr std::uint64_t factory_p_va = 0x140312720ULL;
inline constexpr std::uint64_t factory_e_va = 0x1402E3B10ULL;
inline constexpr std::uint64_t factory_g_va = 0x1402EBA10ULL;
inline constexpr std::uint64_t factory_v_va = 0x140324460ULL;

enum class SpawnCall : std::uint8_t { spawn = 0, spawn_flags = 1, spawn_raised = 2, factory = 3 };

struct SpawnSite final {
    std::uint64_t address{};   ///< the call
    std::uint64_t function{};  ///< the function making it
    /// The class whose vtable methods surround that function (RTTI); the
    /// nearest one when the function is not a method itself.
    const char* owner{};
    char kind{};               ///< 'P', 'E', 'G', 'V'; '?' when computed
    std::uint16_t id{};        ///< 0xFFFF when computed
    SpawnCall call{};
    std::int16_t mode{};       ///< prep mode (spawn) or flags; -1 when computed
};

[[nodiscard]] std::span<const SpawnSite> spawn_sites() noexcept;
/// The classes whose code starts (kind, id), each once, in address order.
[[nodiscard]] std::vector<std::string_view> owners(char kind, std::uint16_t id);
/// The address a row's call goes to.
[[nodiscard]] std::uint64_t call_target(const SpawnSite& site) noexcept;

} // namespace dmc::rengine::profiles::dmc3::effect_spawn_sites
