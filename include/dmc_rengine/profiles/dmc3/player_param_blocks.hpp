#pragma once

#include <cstddef>
#include <cstdint>
#include <optional>
#include <span>
#include <string_view>
#include <vector>

/**
 * The parameter blocks a player character PAC carries in slots 9, 10 and 11.
 *
 * CPlDante init (0x140212C5C) keeps slot 9 at player+0x3DE8, slot 10 at
 * +0x3DF0 and slot 11 at +0x3DF8. Slots 9 and 11 are blocks of floats read at
 * fixed offsets — slot 9 +0x12C (0x1401DFE96); slot 11 +0x2F4 / +0x2F8 /
 * +0x2FC, limits compared with counters at 0x1401CA0FD. Slot 10 opens with
 * u16 pairs and continues with floats; no reader of it has been found.
 * (docs/research/dmc3-collision-tables-2026-09-24.md, section 4.) The
 * pl000, pl001 and pl011 samples carry byte-identical blocks: 896, 480 and
 * 1792 bytes.
 *
 * A block has no tag: what makes it one is the slot of a player PAC it sits
 * in, and that its bytes read as what that slot holds.
 */
namespace dmc::rengine::profiles::dmc3::player_params {

inline constexpr std::uint32_t k_slot_params_a = 9U;
inline constexpr std::uint32_t k_slot_pairs = 10U;
inline constexpr std::uint32_t k_slot_params_b = 11U;

/// A player character PAC: `pl` and three digits (not `plwp_*`, a weapon).
[[nodiscard]] bool is_player_pac(std::string_view stem) noexcept;

/**
 * The format of `slot` of the player PAC `stem` when `bytes` read as that
 * slot's block: `player-params` (slots 9, 11: whole finite floats) or
 * `player-pairs` (slot 10: u16 pairs, then whole finite floats). Nothing for
 * any other slot, PAC or byte image.
 */
[[nodiscard]] std::optional<std::string_view> slot_format(
    std::string_view stem, std::uint32_t slot, std::span<const std::byte> bytes) noexcept;

/// A fixed-offset read of a block the executable makes.
struct KnownRead final {
    std::uint32_t slot{};
    std::uint32_t offset{};
    std::uint64_t address{};
    std::string_view note;
};

/// The reads recovered so far, for the slot a block of `size` bytes fills in the corpus.
[[nodiscard]] std::vector<KnownRead> known_reads(std::size_t size);

/// Leading u16 pairs of a slot-10 block: how many before the floats begin.
[[nodiscard]] std::size_t leading_pairs(std::span<const std::byte> bytes) noexcept;

} // namespace dmc::rengine::profiles::dmc3::player_params
