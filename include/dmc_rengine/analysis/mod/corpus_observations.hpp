#pragma once

#include "dmc_rengine/formats/mod/version.hpp"

#include <cstddef>
#include <cstdint>

namespace dmc::rengine::analysis::mod {

// Hash-bound retail corpus authority:
// em000-extract.zip
// SHA-256 306130125f09824811289366324f4208c3c1aba880c5a7efa3953a88d566d07b
//
// These constants describe observations from that corpus only. They are not a
// claim that every DMC3-HD MOD family globally shares the same invariants.
struct Em000CorpusSummary final {
    static constexpr std::size_t mod_count = 35U;
    static constexpr std::size_t top_level_mod_count = 22U;
    static constexpr std::size_t nested_mod_count = 13U;
    static constexpr std::size_t object_count = 136U;
    static constexpr std::size_t mesh_count = 147U;
    static constexpr std::size_t vertex_count = 14804U;
    static constexpr std::size_t node_position_count = 226U;
    static constexpr std::size_t uncovered_zero_padding_bytes = 5106U;

    static constexpr std::size_t one_influence_vertices = 11995U;
    static constexpr std::size_t two_influence_vertices = 2353U;
    static constexpr std::size_t three_influence_vertices = 456U;
    static constexpr std::size_t topology_break_vertices = 4693U;

    static constexpr std::size_t motion_group_0_count = 132U;
    static constexpr std::size_t motion_group_1_count = 91U;
    static constexpr std::size_t motion_group_2_count = 3U;
};

// Lossless arithmetic projection of serialized MOD header +0x14.
// The recursive em000 corpus shows strongly structured values such as
// 100407, 202900, 601715 and 700601. Every observed value can be represented
// as high*100000 + middle*100 + low. The high-level meanings of the three
// components remain semantic candidates until a MOD-specific executable
// consumer of manager+0xE4 is closed.
struct RuntimeMetadataDecimalProjection final {
    std::uint32_t high_component{};
    std::uint32_t middle_component{};
    std::uint32_t low_component{};
};

[[nodiscard]] constexpr RuntimeMetadataDecimalProjection
project_runtime_metadata_decimal(std::uint32_t raw) noexcept {
    return RuntimeMetadataDecimalProjection{
        .high_component = raw / 100000U,
        .middle_component = (raw / 100U) % 1000U,
        .low_component = raw % 100U,
    };
}

[[nodiscard]] constexpr std::uint32_t
recompose_runtime_metadata_decimal(
    RuntimeMetadataDecimalProjection parts) noexcept {
    return parts.high_component * 100000U +
           parts.middle_component * 100U +
           parts.low_component;
}

// Physical generated-topology workspace capacity observed for every one of the
// 147 retail em000 meshes. The formula is evidence for future layout planning,
// not standalone writer authority.
[[nodiscard]] constexpr std::size_t
em000_generated_workspace_capacity(std::size_t vertex_count) noexcept {
    if (vertex_count <= 2U) return 0U;
    const std::size_t raw = 6U * (vertex_count - 2U);
    return (raw + 0x0FU) & ~static_cast<std::size_t>(0x0FU);
}

// Corpus-wide zero observations. Keep these as named evidence predicates so
// tooling can report the exact status without turning them into global
// "padding" or writer-zero rules.
struct Em000ObservedZeroFields final {
    static constexpr bool header_08_0f = true;
    static constexpr bool header_18_1f = true;
    static constexpr bool header_28_3f = true;
    static constexpr bool object_04_07 = true;
    static constexpr bool object_14_17 = true;
    static constexpr bool object_20_2f = true;
    static constexpr bool mesh_0c = true;
    static constexpr bool mesh_38 = true;
    static constexpr bool mesh_48_serialized = true;
    static constexpr bool mesh_4c = true;
    static constexpr bool node_domain_10_1f = true;
    static constexpr bool transform_1c = true;
    static constexpr bool blendindices_x = true;
};

} // namespace dmc::rengine::analysis::mod
