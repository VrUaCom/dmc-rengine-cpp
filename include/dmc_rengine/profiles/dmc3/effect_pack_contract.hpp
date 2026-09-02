#pragma once

#include <array>
#include <cstddef>
#include <cstdint>
#include <string_view>

namespace dmc::rengine::profiles::dmc3 {

// Corpus-backed contract recovered in naming reverse pass #254.
//
// `*_effect.pac` is a two-slot PNST. Slot 0 is a CRLF ASCII manifest and slot
// 1 is a PNST containing the records named by that manifest. This is direct
// enclosing-container naming evidence. It is NOT evidence that `.index` is a
// runtime manifest, and no original executable read site for this text has
// been proven yet.
struct EffectPackContract final {
    static constexpr std::string_view canonical_target_sha256 =
        "e454272ed0fb0247fcbcf300e5d55d7a3e96d50b89b9ffaff81bb978dcbdd082";

    static constexpr std::size_t outer_slot_count = 2U;
    static constexpr std::size_t manifest_slot_index = 0U;
    static constexpr std::size_t records_slot_index = 1U;

    static constexpr char comment_prefix = '#';
    static constexpr std::string_view terminator_line = "# End";
    static constexpr char field_separator = ' ';

    struct KindExtent final {
        char kind{};
        std::size_t extent{};
        std::size_t observed{};
    };

    // Observed across independent retained corpus examples st001_effect.pac
    // and st114_effect.pac. T is variable-size texture data.
    static constexpr std::array<KindExtent, 5> kinds{
        KindExtent{.kind = 'V', .extent = 368U, .observed = 4U},
        KindExtent{.kind = 'E', .extent = 544U, .observed = 9U},
        KindExtent{.kind = 'P', .extent = 704U, .observed = 2U},
        KindExtent{.kind = 'T', .extent = 0U, .observed = 3U},
        KindExtent{.kind = 'A', .extent = 336U, .observed = 2U},
    };

    static constexpr std::size_t texture_dimensions_offset = 0x10U;
    static constexpr std::uint32_t texture_observed_square_small = 128U;
    static constexpr std::uint32_t texture_observed_square_large = 256U;

    static constexpr bool manifest_read_site_found = false;

    [[nodiscard]] static constexpr std::size_t extent_for(char kind) noexcept {
        for (const auto& entry : kinds) {
            if (entry.kind == kind) {
                return entry.extent;
            }
        }
        return 0U;
    }

    [[nodiscard]] static constexpr bool is_known_kind(char kind) noexcept {
        for (const auto& entry : kinds) {
            if (entry.kind == kind) {
                return true;
            }
        }
        return false;
    }
};

} // namespace dmc::rengine::profiles::dmc3
