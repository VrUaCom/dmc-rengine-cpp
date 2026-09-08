#pragma once

#include <array>
#include <cstddef>
#include <cstdint>

namespace dmc::rengine::profiles::dmc3 {

// Corpus-bound grouping evidence for em000_041.pnst. This contract is kept
// separate from the older st001/st114 one-line/one-payload EffectPackContract
// until the canonical parser is generalized and covered by real-corpus tests.
struct EffectPackEm000Contract final {
    static constexpr std::size_t manifest_entry_count = 173U;
    static constexpr std::size_t inner_declared_slot_count = 186U;
    static constexpr std::size_t inner_populated_slot_count = 183U;

    enum class GroupShape : std::uint8_t {
        single_member,
        mod_with_optional_companion,
        unknown,
    };

    struct KindObservation final {
        char kind{};
        std::size_t manifest_count{};
        GroupShape shape{GroupShape::unknown};
    };

    static constexpr std::array<KindObservation, 7> kinds{
        KindObservation{'G', 12U, GroupShape::single_member},
        KindObservation{'V', 50U, GroupShape::single_member},
        KindObservation{'E', 45U, GroupShape::single_member},
        KindObservation{'P', 34U, GroupShape::single_member},
        KindObservation{'T', 8U, GroupShape::single_member},
        KindObservation{'A', 11U, GroupShape::single_member},
        KindObservation{'M', 13U, GroupShape::mod_with_optional_companion},
    };

    static constexpr std::array<std::size_t, 3> empty_mod_companion_slots{
        161U, 163U, 167U,
    };

    static constexpr std::size_t mod_group_first_slot = 160U;
    static constexpr std::size_t mod_group_slot_stride = 2U;
    static constexpr std::size_t mod_optional_companion_size = 16U;
    static constexpr std::uint32_t mod_optional_companion_first_u32 = 0x31U;

    static constexpr std::size_t g_extent = 96U;
    static constexpr std::size_t v_extent = 368U;
    static constexpr std::size_t e_extent = 544U;
    static constexpr std::size_t a_extent = 336U;

    // P is explicitly variable-size in em000. These are observations, not an
    // exhaustive universal subtype table.
    static constexpr std::array<std::size_t, 4> observed_p_extents{
        336U, 528U, 704U, 896U,
    };

    static constexpr std::size_t wrapped_texture_descriptor_size = 0x70U;

    [[nodiscard]] static constexpr GroupShape shape_for(char kind) noexcept {
        for (const auto& entry : kinds) {
            if (entry.kind == kind) {
                return entry.shape;
            }
        }
        return GroupShape::unknown;
    }
};

static_assert(EffectPackEm000Contract::manifest_entry_count ==
    12U + 50U + 45U + 34U + 8U + 11U + 13U);
static_assert(EffectPackEm000Contract::inner_populated_slot_count ==
    EffectPackEm000Contract::inner_declared_slot_count - 3U);

} // namespace dmc::rengine::profiles::dmc3
