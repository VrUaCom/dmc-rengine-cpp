#include "dmc_rengine/profiles/dmc3/player_param_blocks.hpp"

#include <cmath>
#include <cstring>

namespace dmc::rengine::profiles::dmc3::player_params {
namespace {

[[nodiscard]] bool finite_floats(std::span<const std::byte> bytes) noexcept {
    if (bytes.empty() || bytes.size() % 4U != 0U) return false;
    for (std::size_t at = 0U; at < bytes.size(); at += 4U) {
        float value = 0.0F;
        std::memcpy(&value, bytes.data() + at, sizeof value);
        if (!std::isfinite(value)) return false;
    }
    return true;
}

[[nodiscard]] std::uint16_t u16(std::span<const std::byte> bytes, std::size_t at) noexcept {
    std::uint16_t value = 0U;
    std::memcpy(&value, bytes.data() + at, sizeof value);
    return value;
}

} // namespace

bool is_player_pac(std::string_view stem) noexcept {
    if (stem.size() != 5U || !stem.starts_with("pl")) return false;
    for (std::size_t index = 2U; index < 5U; ++index) {
        if (stem[index] < '0' || stem[index] > '9') return false;
    }
    return true;
}

std::size_t leading_pairs(std::span<const std::byte> bytes) noexcept {
    // A pair is two small counts (both under 0x1000); a float's upper half is
    // its exponent and never that small for the values these blocks hold.
    std::size_t pairs = 0U;
    while ((pairs + 1U) * 4U <= bytes.size()) {
        const auto first = u16(bytes, pairs * 4U);
        const auto second = u16(bytes, pairs * 4U + 2U);
        if (first >= 0x1000U || second >= 0x1000U || (first == 0U && second == 0U)) break;
        ++pairs;
    }
    return pairs;
}

std::optional<std::string_view> slot_format(
    std::string_view stem, std::uint32_t slot, std::span<const std::byte> bytes) noexcept {
    if (!is_player_pac(stem)) return std::nullopt;
    if ((slot == k_slot_params_a || slot == k_slot_params_b) && finite_floats(bytes)) return "player-params";
    if (slot == k_slot_pairs) {
        const auto pairs = leading_pairs(bytes);
        if (pairs > 0U && finite_floats(bytes.subspan(pairs * 4U))) return "player-pairs";
    }
    return std::nullopt;
}

std::vector<KnownRead> known_reads(std::size_t size) {
    if (size == 896U) {
        return {{.slot = 9U, .offset = 0x12CU, .address = 0x1401DFE96ULL, .note = "read by the player update"}};
    }
    if (size == 1792U) {
        return {
            {.slot = 11U, .offset = 0x2F4U, .address = 0x1401CA0FDULL, .note = "a limit compared with a counter"},
            {.slot = 11U, .offset = 0x2F8U, .address = 0x1401CA0FDULL, .note = "a limit compared with a counter"},
            {.slot = 11U, .offset = 0x2FCU, .address = 0x1401CA0FDULL, .note = "a limit compared with a counter"},
        };
    }
    return {};
}

} // namespace dmc::rengine::profiles::dmc3::player_params
