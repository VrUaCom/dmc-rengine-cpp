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

// Independent corpus expansion performed on 2026-09-08. The two pl000
// resources and the id100 HUD model are separate from the em000 archive and
// expose distinct SHA-256 identities:
//
// pl000 slot 1 MOD (main actor/model extraction):
//   e219e89285604cb6d800b0afdd3bec6684a6b00cd1862d464a669d2861ff3c89
// pl000 slot 12 MOD, followed by source line ";pl000_02.clt" in slot 13:
//   7a2be875b3702f59a607655f7a0a412801a6aea639dcb6e3b23d9b0a09c7e740
// id100 Red Orb counter MOD:
//   9cbbaba99fdd008e257258dfe87c5dfed7fae2a13c4b1c2b08d0e318f0213b90
//
// A separately supplied 110096-byte MOD was byte-identical to em000_001.mod
// and is intentionally not double-counted.
struct MultiCorpusModSummary final {
    static constexpr std::size_t em000_mod_count = 35U;
    static constexpr std::size_t pl000_mod_count = 2U;
    static constexpr std::size_t id100_mod_count = 1U;
    static constexpr std::size_t unique_mod_count = 38U;

    static constexpr std::size_t version_082_count = 1U;
    static constexpr std::size_t version_084_count = 4U;
    static constexpr std::size_t version_100_count = 6U;
    static constexpr std::size_t version_101_count = 27U;

    static constexpr std::size_t object_count = 166U;
    static constexpr std::size_t mesh_count = 180U;
    static constexpr std::size_t vertex_count = 20976U;
    static constexpr std::size_t transform_record_count = 285U;

    // All values below remained zero across every counted MOD in the expanded
    // corpus. This upgrades the evidence from one enemy corpus to multiple
    // independently supplied resource families, but deliberately does NOT turn
    // any field into a global writer-zero/padding rule.
    static constexpr bool header_08_0f_all_zero = true;
    static constexpr bool header_18_1f_all_zero = true;
    static constexpr bool header_28_3f_all_zero = true;
    static constexpr bool object_04_07_all_zero = true;
    static constexpr bool object_14_17_all_zero = true;
    static constexpr bool object_20_2f_all_zero = true;
    static constexpr bool mesh_0c_all_zero = true;
    static constexpr bool mesh_38_all_zero = true;
    static constexpr bool mesh_48_serialized_all_zero = true;
    static constexpr bool mesh_4c_all_zero = true;
    static constexpr bool node_domain_10_1f_all_zero = true;
    static constexpr bool transform_1c_all_zero = true;
    static constexpr bool blendindices_x_all_zero = true;

    // The shared physical node-domain layout is exact in all 38 current MODs.
    // parent/order/motion-group/transform relative offsets follow
    // NodeDomainCoreAbi::expected_* and the parent/order arrays satisfy the
    // proven topological evaluation contract in every file.
    static constexpr std::size_t node_domain_exact_layout_count = 38U;
    static constexpr std::size_t topological_hierarchy_count = 38U;

    // Serialized translation_magnitude agrees with length(translation.xyz) for
    // all 285 transform records to an absolute tolerance of 1e-5. The largest
    // observed absolute difference in the scan was below 5e-6.
    static constexpr std::size_t translation_magnitude_match_count = 285U;

    // motion_group values remain restricted to the already EXE-confirmed byte
    // domain observed in em000. These counts are corpus evidence only: do not
    // name groups 0/1/2 as body/weapon/etc. without executable proof.
    static constexpr std::size_t motion_group_0_count = 178U;
    static constexpr std::size_t motion_group_1_count = 104U;
    static constexpr std::size_t motion_group_2_count = 3U;

    // Serialized generated-topology workspace physical invariants. In every
    // current mesh, mesh+0x40 resolves to a 16-byte-aligned trailing workspace
    // whose exact physical span is align16(6*(vertex_count-2)). Workspaces tile
    // the file trailer without gaps. All begin with 0x1212. Every non-final
    // workspace is byte-filled with 0x12; exactly one final workspace per MOD
    // ends with a final u16 zero while all preceding bytes remain 0x12.
    static constexpr std::size_t workspace_capacity_match_count = 180U;
    static constexpr std::size_t workspace_aligned_count = 180U;
    static constexpr std::size_t workspace_start_1212_count = 180U;
    static constexpr std::size_t workspace_all_12_count = 142U;
    static constexpr std::size_t workspace_final_12_then_0000_count = 38U;
    static constexpr std::size_t file_tail_1212_0000_count = 38U;
    static constexpr std::uint16_t workspace_observed_fill_word = 0x1212U;
    static constexpr std::uint16_t file_observed_terminal_word = 0x0000U;

    // Current source-flag/value census across 166 objects. These are frequency
    // observations, not new semantic names. alpha_control is 0x80 for every
    // object in this bounded corpus.
    static constexpr std::size_t alpha_control_80_count = 166U;
    static constexpr std::size_t source_flag_00100000_count = 45U;
    static constexpr std::size_t source_flag_00200000_count = 7U;
    static constexpr std::size_t populated_parameter18_1c_object_count = 2U;

    // Header +0x14 is not a globally fixed-width decimal semantic partition.
    // em000 has clustered six-digit values, while the pl000 MODs both carry
    // raw 217 and id100 carries raw 1000000. The raw manager-carried u32
    // remains canonical until manager+0xE4 consumers are closed.
    static constexpr std::uint32_t pl000_runtime_metadata_u32 = 217U;
    static constexpr std::uint32_t id100_runtime_metadata_u32 = 1000000U;
};

// Lossless arithmetic projection of serialized MOD header +0x14.
// The recursive em000 corpus shows strongly structured values such as
// 100407, 202900, 601715 and 700601. Every observed value can be represented
// as high*100000 + middle*100 + low.
//
// IMPORTANT multi-corpus correction (2026-09-08): this is an arithmetic/em000
// clustering utility only, NOT a universal MOD semantic field split. pl000
// carries raw value 217 in both independently observed MODs and id100 carries
// 1000000. The high-level meanings of these arithmetic components remain
// unpromoted until a MOD-specific executable consumer of manager+0xE4 is
// closed.
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

// Multi-corpus physical generated-topology workspace capacity. This exact
// formula holds for all 180 meshes in the current em000 + pl000 + id100 corpus.
// It is structural evidence useful for future layout planning; it is not, by
// itself, writer authority.
[[nodiscard]] constexpr std::size_t
mod_generated_workspace_capacity(std::size_t vertex_count) noexcept {
    if (vertex_count <= 2U) return 0U;
    const std::size_t raw = 6U * (vertex_count - 2U);
    return (raw + 0x0FU) & ~static_cast<std::size_t>(0x0FU);
}

// Compatibility name retained for existing code/tests. The formula was first
// observed on 147 em000 meshes and is now independently extended to the current
// multi-corpus MOD set above.
[[nodiscard]] constexpr std::size_t
em000_generated_workspace_capacity(std::size_t vertex_count) noexcept {
    return mod_generated_workspace_capacity(vertex_count);
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
